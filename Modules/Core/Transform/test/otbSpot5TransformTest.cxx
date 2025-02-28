/*
 * Copyright (C) 2005-2024 Centre National d'Etudes Spatiales (CNES)
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

#include "otbImage.h"
#include "otbImageFileReader.h"
#include "otbSpot5TransformBase.h"
#include "otbSpot5ForwardTransform.h"
#include "otbSpot5InverseTransform.h"
#include "otbGenericRSTransform.h"
#include "otbGeographicalDistance.h"
#include "itkPoint.h"
#include "itkEuclideanDistanceMetric.h"
#include <string>
#include <vector>
#include <iomanip>


#include "otbSpot5SensorModel.h"
#include "otbSpot5Metadata.h"

#include "otbPolygon.h"


using Point2DType = itk::Point<double, 2>;
using Point3DType = itk::Point<double, 3>;
using Points2DContainerType = std::vector<Point2DType>;
using Points3DContainerType = std::vector<Point3DType>;
using ForwardTransformType = otb::Spot5ForwardTransform<double, 2, 3>;
using InverseTransformType = otb::Spot5InverseTransform<double, 3, 2>;
// using SensorModelType = otb::Spot5SensorModel<double, 3, 2>;

using GenericRSTransformType = otb::GenericRSTransform<double, 3, 3>;
using DistanceType = itk::Statistics::EuclideanDistanceMetric<Point2DType>;
using GeographicalDistanceType = otb::GeographicalDistance<Point3DType>;

using PolygonType = otb::Polygon<>;
using ContinuousIndexType = PolygonType::ContinuousIndexType;

int otbSpot5TransformTest(int itkNotUsed(argc), char* argv[])
{

  // TODO enable large input after otb 10

  // bool success = true;
  // Point2DType imagePoint;
  // Point3DType geo3dPoint;
  // // Inputs
  // std::string Spot5File(argv[1]);

  // using ImageType = otb::Image<double, 2>;
  // using ImageFileReaderType = otb::ImageFileReader<ImageType>;
  // auto reader = ImageFileReaderType::New();
  // reader->SetFileName(Spot5File);
  // reader->UpdateOutputInformation();
  // auto imd = reader->GetOutput()->GetImageMetadata();

  // otb::Spot5SensorModel SensorModel(imd);

  // // Setting the transforms
  // auto ForwardTransform = ForwardTransformType::New();
  // ForwardTransform->SetMetadata(imd);

  // if (!ForwardTransform->IsValidSensorModel())
  // {
  //   std::cout << "Model not set!" << std::endl;
  //   return EXIT_FAILURE;
  // }

  // auto InverseTransform = InverseTransformType::New();
  // InverseTransform->SetMetadata(imd);
  // if (!InverseTransform->IsValidSensorModel())
  // {
  //   std::cout << "Model not set!" << std::endl;
  //   return EXIT_FAILURE;
  // }


  // imagePoint[0] = 5999;
  // imagePoint[1] = 5999;

  // std::cout << "Testing geopoint: " << imagePoint << "\n\n";
  // auto geoPoint = ForwardTransform->TransformPoint(imagePoint);
  // std::cout << "Testing ForwardSensorModel: " << imagePoint << " -> " << geoPoint << "\n";


  // auto newImagePoint = InverseTransform->TransformPoint(geoPoint);

  // std::cout << "Testing InverseSensorModel: " << geoPoint << " -> " << newImagePoint << "\n";

}
