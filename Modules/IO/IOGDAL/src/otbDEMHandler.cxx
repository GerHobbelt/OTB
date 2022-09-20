/*
 * Copyright (C) 2005-2022 Centre National d'Etudes Spatiales (CNES)
 *
 * This file is part of Orfeo Toolbox
 *
 *     https://www.orfeo-toolbox.org/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "otbDEMHandler.h"
#include "otbGDALDriverManagerWrapper.h"
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>
#include "gdal_utils.h"

//Warp utility
#include "gdalwarper.h"
#include "vrtdataset.h"

#include "ogr_spatialref.h"

//TODO C++ 17 : use std::optional instead
#include <boost/optional.hpp>

#include <mutex>
#include <thread>

namespace
{ // Anonymous namespace
std::mutex demMutex;
} // Anonymous namespace

namespace otb {

namespace DEMDetails {

// Adapted from boost filesystem documentation : https://www.boost.org/doc/libs/1_53_0/libs/filesystem/doc/reference.html
std::vector<std::string> GetFilesInDirectory(const std::string & directoryPath)
{
  std::vector<std::string> fileList;

  if ( !boost::filesystem::exists( directoryPath) )
  {
    return fileList;
  }
  else if (!boost::filesystem::is_directory(directoryPath))
  {
    fileList.push_back(directoryPath);
    return fileList;
  }

  // End iterator : default construction yields past-the-end
  for ( const auto & item : boost::make_iterator_range(boost::filesystem::directory_iterator(directoryPath), {}) )
  {
    try
    {
      if ( boost::filesystem::is_directory(item.status()) )
      {
        auto subDirList = GetFilesInDirectory(item.path().string());
        fileList.insert(fileList.end(), subDirList.begin(), subDirList.end());
      }
      else
      {
        fileList.push_back(item.path().string());
      }
    }
    catch (boost::filesystem::filesystem_error const& e)
    {
      otbLogMacro(Warning, << e.what())
    }
  }

  return fileList;
}

struct DatasetCache
{
  GDALDataset * get          () const { return m_Dataset.get();}
  GDALDataset * operator->   () const { return m_Dataset.get();}
  GDALDataset & operator*    () const { assert(m_Dataset); return *m_Dataset;}
  explicit      operator bool() const { return bool(m_Dataset); }

  DatasetCache() = default;
  ~DatasetCache() = default;

  void reset(GDALDataset* ds)
  {
    m_Dataset.reset(ds);
    if (m_Dataset)
    {
#if GDAL_VERSION_NUM >= 3000000
      auto srs = ds->GetSpatialRef();
#else
      auto projRef = ds->GetProjectionRef();

      std::unique_ptr<OGRSpatialReference> srsUniquePtr;
      OGRSpatialReference* srs = nullptr;
      // GetProjectionRef() returns an empty non null string if no projection is available
      if (strlen(projRef) != 0 )
      {
        srsUniquePtr = std::make_unique<OGRSpatialReference>(ds->GetProjectionRef());
        srs = srsUniquePtr.get();
      }
#endif

      auto wgs84Srs = OGRSpatialReference::GetWGS84SRS(); // 1%
      m_isWGS84 = false;

      // Convert input lon lat into the coordinates defined by the dataset if needed.
      if (srs && !srs->IsSame(wgs84Srs)) // 10% of the time
      {
        m_isWGS84 = true;
        m_poCT = std::unique_ptr<OGRCoordinateTransformation>
          (OGRCreateCoordinateTransformation(wgs84Srs, srs));
      }

      m_Dataset->GetGeoTransform(m_geoTransform);
    }
    else
    {
      m_poCT.reset(nullptr);
    }
  }

  bool convert_lon_lat(double & lon, double & lat) const
  {
    if (isWGS84())
    {
      if (m_poCT && !m_poCT->Transform( 1, &lon, &lat ) )
      {
        return false;
      }
    }
    return true;
  }

  std::pair<double, double> transform(double lon, double lat) const
  {
    auto const x = (lon - m_geoTransform[0]) / m_geoTransform[1] - 0.5;
    auto const y = (lat - m_geoTransform[3]) / m_geoTransform[5] - 0.5;
    return std::make_pair(x, y);
  }

  bool isWGS84() const noexcept { return m_isWGS84; }

  double * getGeoTransform() noexcept { return m_geoTransform;}
private:
  using DatasetUPtr = std::unique_ptr<GDALDataset, void(*)(GDALDataset*)>;
  using OCT_ptr     = std::unique_ptr<OGRCoordinateTransformation>;
  DatasetUPtr m_Dataset = DatasetUPtr(nullptr, [](GDALDataset* DS){if(DS){GDALClose(DS);}});
  OCT_ptr     m_poCT;
  bool        m_isWGS84 = false;
  double      m_geoTransform[6] = {};
};

boost::optional<double> GetDEMValue(double lon, double lat, DatasetCache const& dsc)
{
  if (!dsc.convert_lon_lat(lon, lat))
  {
    return boost::none;
  }

  auto const xy = dsc.transform(lon, lat);
  auto const x = xy.first;
  auto const y = xy.second;

  auto is_out_raster = [&](auto x, auto y, GDALDataset & ds) {
    return x < 0
      ||   y < 0
      || 1+x > ds.GetRasterXSize()   // no need to test x > size
      || 1+y > ds.GetRasterYSize();  // no need to test Y > size
  };

  if (is_out_raster(x, y, *dsc))
  {
    return boost::none;
  }

  auto const x_int = static_cast<int>(x);
  auto const y_int = static_cast<int>(y);

  auto const deltaX = x - x_int;
  auto const deltaY = y - y_int;

  // Bilinear interpolation.
  double elevData[4];

  auto const err = dsc->GetRasterBand(1)->RasterIO( GF_Read, x_int, y_int, 2, 2,
      elevData, 2, 2, GDT_Float64,
      0, 0, nullptr);

  if (err)
  {
    return boost::none;
  }

  // Test for no data. Don't return a value if one pixel
  // of the interpolation is no data.
  for (int i =0; i<4; i++)
  {
    if (elevData[i] == dsc->GetRasterBand(1)->GetNoDataValue())
    {
      return boost::none;
    }
  }

  auto const xBil1 = elevData[0] * (1-deltaX) + elevData[1] * deltaX;
  auto const xBil2 = elevData[2] * (1-deltaX) + elevData[3] * deltaX;

  auto const yBil = xBil1 * (1.0 - deltaY) + xBil2 * deltaY;
  return yBil;
}

#if 0
boost::optional<double> GetDEMValueSynchronized(double lon, double lat, GDALDataset& ds)
{
  const std::lock_guard<std::mutex> lock(demMutex);
  return GetDEMValue(lon, lat, ds);
}
#endif

}  // namespace DEMDetails

/** Internal actual instance of DEM handling wrapper for current thread.
 * \ingroup OTBIOGDAL
 *
 * \internal This class is not exposed to other code. It's meant to be
 * used through `DEMHandler` singleton.
 *
 * The current design has one `DEMHandlerTLS` per actual thread. In case
 * threads are terminated and restarted multiple time (this is the case
 * with default old ITK 4 architecture that will terminate and respawn
 * thread for new OTB stream), `DEMHandlerTLS` instance will be
 * destroyed and re-recreated instead of being reused.
 *
 * To reuse the instances of `DEMHandlerTLS` we could:
 * - have a kind of pool of `DEMHandlerTLS` instances, and a
 *   thread_local pointer of the instance attributed to the current
 *   thread. Attributing an instance the first time would require a lock
 *   to search a free instance in the pool. The TLS object could be a
 *   RAII capsule that returns the `DEMHandlerTLS` instance to the pool.
 */
class DEMHandlerTLS
{
public:
  double GetHeightAboveEllipsoid(double lon, double lat, double defaultHeight) const;
  boost::optional<double> GetHeightAboveMSL(double lon, double lat) const;
  boost::optional<double> GetGeoidHeight(double lon, double lat) const;

  // TODO: simplify: no back-registration, but instead the DEMHAndler
  // passes all the relevant construction parameters
  DEMHandlerTLS() { Register(); }
  ~DEMHandlerTLS() { Unregister(); }

  /** Try to open the DEM directory.
   * \param path input path
   */
  void OpenDEMFile(std::string const& path);

  /** Open all raster in the directory.
   * \param DEMDirectory input directory
   */
  bool OpenDEMDirectory(const std::string& DEMDirectory);

  /** Try to open a geoid file
   * \param geoidFile input geoid path
   */
  bool OpenGeoidFile(std::string const& geoidFile);

  /** Return the number of DEM opened */
  unsigned int GetDEMCount() const;

  /** Clear the DEM list and geoid filename, close all elevation datasets
   * and reset the default height above ellipsoid */
  void ClearElevationParameters();

private:
  DEMHandlerTLS(DEMHandlerTLS const&) = delete;
  void operator=(DEMHandlerTLS const&) = delete;

  using DatasetUPtr = std::unique_ptr<GDALDataset, void(*)(GDALDataset*)>;

  void Register();
  void Unregister();
  void CreateShiftedDataset();

  /** Clear the DEM list and close all DEM datasets */
  void ClearDEMs();

  /** List of RAII capsules on all opened DEM datasets for memory management */
  std::vector<otb::GDALDatasetWrapper::Pointer> m_DatasetList;

  /** Pointer to the DEM dataset */
  DEMDetails::DatasetCache m_DEMDS;

  /** Pointer to the geoid dataset */
  DEMDetails::DatasetCache m_GeoidDS;
};

thread_local std::unique_ptr<DEMHandlerTLS> DEMHandler::m_tls;

DEMHandlerTLS & DEMHandler::GetHandlerForCurrentThread() const
{
  // std::cout << "DEMHandler::GetHandlerForCurrentThread("<<std::this_thread::get_id() << "); tls: "<< m_tls.get() << "\n";
  if (!m_tls) {
    // TODO: find a way to pre-contruct every thing
    m_tls = std::make_unique<DEMHandlerTLS>(); // no need to lock as this is a TLS
    // std::cout << "DEMHandler::GetHandlerForCurrentThread("<<std::this_thread::get_id() << "); tls built "<< m_tls.get() << "\n";
  }
  return *m_tls;
}

// Meyer singleton design pattern
DEMHandler & DEMHandler::GetInstance()
{
  // std::cout << "DEMHandler::GetInstance()" << std::endl;
  static DEMHandler s_instance;
  return s_instance;
}

DEMHandler::DEMHandler()
: m_DefaultHeightAboveEllipsoid(0.0)
{
  // std::cout << "DEMHandler::constructor()" << std::endl;
  GDALAllRegister();
}

DEMHandler::~DEMHandler()
{
  // Close all elevation datasets
  ClearElevationParameters();

  VSIUnlink(DEMHandler::DEM_DATASET_PATH);
  VSIUnlink(DEMHandler::DEM_WARPED_DATASET_PATH);
  VSIUnlink(DEMHandler::DEM_SHIFTED_DATASET_PATH);
}

void DEMHandlerTLS::OpenDEMFile(std::string const& path)
{
  // std::cout << "DEMHandlerTLS::OpenDEMFile(" << path << ")\n";
  m_DatasetList.push_back(otb::GDALDriverManagerWrapper::GetInstance().Open(path));
  std::array<GDALDatasetH, 1> vrtDatasetList { m_DatasetList[0]->GetDataSet() };
  auto close_me = GDALBuildVRT(DEMHandler::DEM_DATASET_PATH, 1, vrtDatasetList.data(),
      nullptr, nullptr, nullptr);
  // Need to close the dataset, so it is flushed into memory.
  GDALClose(close_me);
  m_DEMDS.reset(static_cast<GDALDataset*>(GDALOpen(DEMHandler::DEM_DATASET_PATH, GA_ReadOnly)));

  if(m_GeoidDS)
  {
    CreateShiftedDataset();
  }
}

void DEMHandler::OpenDEMFile(std::string path)
{
  // std::cout << "DEMHandler::OpenDEMFile(" << path << ")\n";
  const std::lock_guard<std::mutex> lock(demMutex);
  for (auto tls : m_tlses) {
    tls->OpenDEMFile(path);
  }
  m_DEMDirectories.push_back(move(path));
  Notify();
}

void DEMHandler::OpenDEMDirectory(std::string DEMDirectory)
{
  // std::cout << "DEMHandler::OpenDEMDirectory(" << DEMDirectory << ")\n";
  auto isSameDirectory = [&DEMDirectory](std::string const& s)
  {
    return s == DEMDirectory;
  };
  if(std::any_of(std::begin(m_DEMDirectories), std::end(m_DEMDirectories), isSameDirectory))
  {
    otbLogMacro(Info, << "Directory '"<< DEMDirectory << "' already opened.");
    return;
  }

  // In case the directory is not valid, we still try to open it for
  // real, even if the DEMHandlerTLS will not serve to anything...
  auto & tls = GetHandlerForCurrentThread();
  (void) tls;

  bool is_new_and_valid = false;
  {
    const std::lock_guard<std::mutex> lock(demMutex);
    for (auto tls : m_tlses) {
      is_new_and_valid = tls->OpenDEMDirectory(DEMDirectory);
      // if any returns true, all should
    }
  }

  if (is_new_and_valid) {
    m_DEMDirectories.push_back(move(DEMDirectory)); // => parameter voluntary taken by value
  }

  Notify();
}

bool DEMHandlerTLS::OpenDEMDirectory(const std::string& DEMDirectory)
{
  // std::cout << "DEMHandlerTLS::OpenDEMDirectory(" << DEMDirectory << ")\n";
  bool is_new_and_valid = false;
  // Free the previous in-memory dataset (if any)
  if (!m_DatasetList.empty())
    VSIUnlink(DEMHandler::DEM_DATASET_PATH);

  auto demFiles = DEMDetails::GetFilesInDirectory(DEMDirectory);
  for (const auto & file : demFiles)
  {
    auto ds = otb::GDALDriverManagerWrapper::GetInstance().Open(file);
    if (ds)
    {
      m_DatasetList.push_back(ds);
    }
  }

  int vrtSize = m_DatasetList.size();

  // Don't build a vrt if there is no dataset
  if (m_DatasetList.empty())
  {
    m_DEMDS.reset(nullptr);
    otbLogMacro(Warning, << "No DEM found in "<< DEMDirectory)
  }
  else
  {
    std::vector<GDALDatasetH> vrtDatasetList(vrtSize);
    for (int i = 0; i < vrtSize; i++)
    {
      vrtDatasetList[i] = m_DatasetList[i]->GetDataSet();
    }

    auto close_me = GDALBuildVRT(DEMHandler::DEM_DATASET_PATH, vrtSize, vrtDatasetList.data(),
        nullptr, nullptr, nullptr);
    // Need to close the dataset, so it is flushed into memory.
    GDALClose(close_me);
    m_DEMDS.reset(static_cast<GDALDataset*>(GDALOpen(DEMHandler::DEM_DATASET_PATH, GA_ReadOnly)));
    is_new_and_valid = true;
  }

  if(m_GeoidDS)
  {
    CreateShiftedDataset();
  }
  return is_new_and_valid;
}

bool DEMHandlerTLS::OpenGeoidFile(const std::string& geoidFile)
{
  // std::cout << "DEMHandlerTLS::OpenGeoidFile(" << geoidFile << ")\n";
  auto gdalds = static_cast<GDALDataset*>(GDALOpen(geoidFile.c_str(), GA_ReadOnly));

  if (!gdalds)
  {
    otbLogMacro(Warning, << "Cannot open geoid file "<< geoidFile);
    return false;
  }

#if GDAL_VERSION_NUM >= 3000000
  if (!(gdalds->GetSpatialRef()) || gdalds->GetSpatialRef()->IsEmpty())
#else
    if (strlen(gdalds->GetProjectionRef()) == 0 )
#endif
    {
      otbLogMacro(Warning, << "input geoid file "<< geoidFile << " will not be used because it has no input projection.")
        return false;
    }

  m_GeoidDS.reset(gdalds);

  if(m_DEMDS)
  {
    CreateShiftedDataset();
  }

  return true;
}

void DEMHandler::OpenGeoidFile(std::string geoidFile)
{
  // std::cout << "DEMHandler::OpenGeoidFile(" << geoidFile << ")\n";

  // In case the geoid is not valid, we still try to open it for real,
  // even if the DEMHandlerTLS will not serve to anything...
  auto & tls = GetHandlerForCurrentThread();
  (void) tls;

  bool success = false;
  {
    const std::lock_guard<std::mutex> lock(demMutex);
    for (auto tls : m_tlses) {
      success = tls->OpenGeoidFile(geoidFile);
      // if any is true, all should be!
    }
  }

  if (success) {
    m_GeoidFilename = move(geoidFile); // => parameter voluntary taken by value
  }
  Notify();
}

void DEMHandlerTLS::CreateShiftedDataset()
{
  // WIP : no data is not handled at the moment

  // Warp geoid dataset
  auto warpOptions = GDALCreateWarpOptions();;
  warpOptions->hSrcDS           = m_GeoidDS.get();
  //warpOptions->hDstDS         = m_Dataset;
  warpOptions->eResampleAlg     =  GRA_Bilinear;
  warpOptions->eWorkingDataType = GDT_Float64;
  warpOptions->nBandCount       = 1;
  warpOptions->panSrcBands      =
    (int *) CPLMalloc(sizeof(int) * warpOptions->nBandCount );
  warpOptions->panSrcBands[0]   = 1;
  warpOptions->panDstBands      =
    (int *) CPLMalloc(sizeof(int) * warpOptions->nBandCount );
  warpOptions->panDstBands[0]   = 1;

  // Establish reprojection transformer.
  warpOptions->pTransformerArg =
    GDALCreateGenImgProjTransformer( m_GeoidDS.get(),
        GDALGetProjectionRef(m_GeoidDS.get()),
        m_DEMDS.get(),
        GDALGetProjectionRef(m_DEMDS.get()),
        FALSE, 0.0, 1 );
  warpOptions->pfnTransformer = GDALGenImgProjTransform;

  auto ds = static_cast<GDALDataset *> (GDALCreateWarpedVRT(m_GeoidDS.get(),
        m_DEMDS->GetRasterXSize(),
        m_DEMDS->GetRasterYSize(),
        m_DEMDS.getGeoTransform(),
        warpOptions));

  ds->SetDescription(DEMHandler::DEM_WARPED_DATASET_PATH);
  GDALClose(ds);

  GDALDriver *poDriver = (GDALDriver *) GDALGetDriverByName( "VRT" );
  GDALDataset *poVRTDS;

  poVRTDS = poDriver->Create( DEMHandler::DEM_SHIFTED_DATASET_PATH, m_DEMDS->GetRasterXSize(), m_DEMDS->GetRasterYSize(), 0, GDT_Float64, NULL );

  poVRTDS->SetGeoTransform(m_DEMDS.getGeoTransform());

  poVRTDS->SetProjection(m_DEMDS->GetProjectionRef());

  char** derivedBandOptions = NULL;

  derivedBandOptions = CSLAddNameValue(derivedBandOptions, "subclass", "VRTDerivedRasterBand");
  derivedBandOptions = CSLAddNameValue(derivedBandOptions, "PixelFunctionType", "sum");
  poVRTDS->AddBand(GDT_Float64, derivedBandOptions);

  GDALRasterBand *poBand = poVRTDS->GetRasterBand( 1 );

  // TODO use std string (and boost format ?) or stream
  char demVrtXml[10000];

  snprintf( demVrtXml, sizeof(demVrtXml),
      "<SimpleSource>"
      "  <SourceFilename>%s</SourceFilename>"
      "  <SourceBand>1</SourceBand>"
      "</SimpleSource>",
      DEMHandler::DEM_DATASET_PATH);

  poBand->SetMetadataItem( "source_0", demVrtXml, "new_vrt_sources" );


  char geoidVrtXml[10000];

  snprintf( geoidVrtXml, sizeof(geoidVrtXml),
      "<SimpleSource>"
      "  <SourceFilename>%s</SourceFilename>"
      "  <SourceBand>1</SourceBand>"
      "</SimpleSource>",
      DEMHandler::DEM_WARPED_DATASET_PATH);


  poBand->SetMetadataItem( "source_1", geoidVrtXml, "new_vrt_sources" );

  GDALClose(poVRTDS);
}

boost::optional<double> DEMHandlerTLS::GetHeightAboveMSL(double lon, double lat) const
{
  if (m_DEMDS)
  {
    return DEMDetails::GetDEMValue(lon, lat, m_DEMDS);
  }
  return boost::none;
}

boost::optional<double> DEMHandlerTLS::GetGeoidHeight(double lon, double lat) const
{
  if (m_GeoidDS)
  {
    return DEMDetails::GetDEMValue(lon, lat, m_GeoidDS);
  }
  return boost::none;
}

double DEMHandlerTLS::GetHeightAboveEllipsoid(double lon, double lat, double defaultHeight) const
{
  boost::optional<double> DEMresult   = GetHeightAboveMSL(lon, lat);
  boost::optional<double> geoidResult = GetGeoidHeight(lon, lat);

  if (DEMresult || geoidResult)
  {
    double result = 0.;
    if (DEMresult)
    {
      result += *DEMresult;
    }

    if (geoidResult)
    {
      result += *geoidResult;
    }
    return result;
  }
  else
    return defaultHeight;
}

double DEMHandler::GetHeightAboveEllipsoid(double lon, double lat) const
{
  auto & tls = GetHandlerForCurrentThread();
  return tls.GetHeightAboveEllipsoid(lon, lat, m_DefaultHeightAboveEllipsoid);
}
double DEMHandler::GetHeightAboveEllipsoid(const PointType& geoPoint) const
{
  return GetHeightAboveEllipsoid(geoPoint[0], geoPoint[1]);
}

double DEMHandler::GetGeoidHeight(double lon, double lat) const
{
  auto & tls = GetHandlerForCurrentThread();
  return tls.GetGeoidHeight(lon, lat).value_or(0);
}
double DEMHandler::GetGeoidHeight(const PointType& geoPoint) const
{
  return GetGeoidHeight(geoPoint[0], geoPoint[1]);
}

double DEMHandler::GetHeightAboveMSL(double lon, double lat) const
{
  auto & tls = GetHandlerForCurrentThread();
  return tls.GetHeightAboveMSL(lon, lat).value_or(0);
}
double DEMHandler::GetHeightAboveMSL(const PointType& geoPoint) const
{
  return GetHeightAboveMSL(geoPoint[0], geoPoint[1]);
}

unsigned int DEMHandlerTLS::GetDEMCount() const
{
  return m_DatasetList.size();
}

unsigned int DEMHandler::GetDEMCount() const
{
  // std::cout << "DEMHandler::GetDEMCount()\n";
  auto & tls = GetHandlerForCurrentThread();
  return tls.GetDEMCount();
}

bool DEMHandler::IsValidDEMDirectory(const std::string& DEMDirectory) const
{
  for (const auto & filename : DEMDetails::GetFilesInDirectory(DEMDirectory))
  {
    // test if a driver can identify this dataset
    auto identifyDriverH = GDALIdentifyDriver(filename.c_str(), nullptr);
    if (identifyDriverH)
    {
      return true;
    }
  }

  return false;
}

std::string const& DEMHandler::GetDEMDirectory(unsigned int idx) const
{
  if (idx >= m_DEMDirectories.size())
  {
    std::ostringstream oss;
    oss << "Requested DEM directory " << idx
      << ", but only "<< m_DEMDirectories.size() << " have been set.";
    // TODO: std::out_of_range is a logic (i.e. a programming) error =>
    // use another exception type!
    throw std::out_of_range (oss.str());
  }

  return m_DEMDirectories[idx];
}

std::string const& DEMHandler::GetGeoidFile() const
{
  return m_GeoidFilename;
}

void DEMHandlerTLS::ClearDEMs()
{
  // This will call GDALClose on all datasets
  m_DatasetList.clear();
}

void DEMHandlerTLS::ClearElevationParameters()
{
  m_GeoidDS.reset(nullptr);

  ClearDEMs(); // ClearDEMs calls Notify()
}

void DEMHandler::ClearElevationParameters()
{
  m_DefaultHeightAboveEllipsoid = 0.;
  m_GeoidFilename.clear();

  const std::lock_guard<std::mutex> lock(demMutex);
  for (auto tls : m_tlses) {
    tls->ClearElevationParameters();
  }
  m_DEMDirectories.clear();

  Notify();
}


void DEMHandler::SetDefaultHeightAboveEllipsoid(double height)
{
  m_DefaultHeightAboveEllipsoid = height;
  Notify();
}

double DEMHandler::GetDefaultHeightAboveEllipsoid() const
{
  return m_DefaultHeightAboveEllipsoid;
}

void DEMHandler::Notify() const
{
  for (const auto & observer: m_ObserverList)
  {
    observer->Update();
  }
};

// TODO: optimize the registration of DEMHandlerTLS
// Indeed. In current version of OTB, new set of threads are spwaned for
// each stream. The result is that we will create and destroy
// DEMHandlerTLS more than needed. Instead, we should be able to reuse
// the previous ones constructed for the last stream
// -> pool to DEMHandlerTLS??
void DEMHandlerTLS::Register() {
  DEMHandler::GetInstance().RegisterTLS(this);
}

void DEMHandlerTLS::Unregister() {
  DEMHandler::GetInstance().UnregisterTLS(this);
}

void DEMHandler::RegisterTLS(DEMHandlerTLS *tls) {
  // std::cout << "DEMHandler::RegisterTLS(" << tls << ")\n";
  const std::lock_guard<std::mutex> lock(demMutex);
  m_tlses.insert(tls);
  if (!m_GeoidFilename.empty()) {
    tls->OpenGeoidFile(m_GeoidFilename);
  }
  for (auto const& dir : m_DEMDirectories) {
    tls->OpenDEMDirectory(dir);
  }
}

void DEMHandler::UnregisterTLS(DEMHandlerTLS *tls) {
  // std::cout << "DEMHandler::UnregisterTLS(" << tls << ")\n";
  const std::lock_guard<std::mutex> lock(demMutex);
  m_tlses.erase(tls);
}

} // namespace otb
