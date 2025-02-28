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


#include <iostream>
#include <typeinfo>

#include "otbImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"

#include "otbSinclairToCoherencyMatrixImageFilter.h"
#include "otbSinclairToCovarianceMatrixImageFilter.h"
#include "otbSinclairToCircularCovarianceMatrixImageFilter.h"
#include "otbSinclairToMuellerMatrixImageFilter.h"

#include "otbMultiChannelExtractROI.h"


using namespace otb;

template <class TFilter>
int generic_SinclairImageFilter(int itkNotUsed(argc), char* argv[])
{
  const char* outputFilename = argv[4];

  using OutputImageType = typename TFilter::OutputImageType;
  using InputImageType  = typename TFilter::Superclass::template InputImageType<0>;
  using OutputPixelType = typename OutputImageType::InternalPixelType;

  typedef otb::ImageFileWriter<OutputImageType> WriterType;

  const char* inputFilename1 = argv[1];
  const char* inputFilename2 = argv[2];
  const char* inputFilename3 = argv[3];

  typedef otb::ImageFileReader<InputImageType> ReaderType;
  typedef otb::MultiChannelExtractROI<OutputPixelType, OutputPixelType> ExtractROIType;
  using FilterType                     = TFilter;
  typename FilterType::Pointer filter  = FilterType::New();
  typename ReaderType::Pointer reader1 = ReaderType::New();
  typename ReaderType::Pointer reader2 = ReaderType::New();
  typename ReaderType::Pointer reader3 = ReaderType::New();

  reader1->SetFileName(inputFilename1);
  reader2->SetFileName(inputFilename2);
  reader3->SetFileName(inputFilename3);
  filter->SetInput(polarimetry_tags::hh{}, reader1->GetOutput());
  filter->SetInput(polarimetry_tags::hv{}, reader2->GetOutput());
  filter->SetInput(polarimetry_tags::vh{}, reader2->GetOutput());
  filter->SetInput(polarimetry_tags::vv{}, reader3->GetOutput());

  filter->UpdateOutputInformation();

  typename ExtractROIType::Pointer extract = ExtractROIType::New();
  extract->SetStartX(10);
  extract->SetStartY(10);
  extract->SetSizeX(30);
  extract->SetSizeY(30);
  extract->SetInput(filter->GetOutput());

  typename WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(outputFilename);

  writer->SetInput(extract->GetOutput());
  writer->Update();

  return EXIT_SUCCESS;
}

int otbSinclairImageFilter(int argc, char* argv[])
{
  const unsigned int           Dimension = 2;
  typedef std::complex<double> InputPixelType;
  typedef std::complex<double> OutputPixelType;
  typedef double               OutputRealPixelType;

  typedef otb::Image<InputPixelType, Dimension>            InputImageType;
  typedef otb::VectorImage<OutputPixelType, Dimension>     OutputImageType;
  typedef otb::VectorImage<OutputRealPixelType, Dimension> OutputRealImageType;

  using CohSRFilterType = SinclairToCoherencyMatrixImageFilter<InputImageType, OutputImageType>;
  using CovSRFilterType = SinclairToCovarianceMatrixImageFilter<InputImageType, OutputImageType>;
  using CCSRFilterType  = SinclairToCircularCovarianceMatrixImageFilter<InputImageType, OutputImageType>;
  using MSRFilterType   = SinclairToMuellerMatrixImageFilter<InputImageType, OutputRealImageType>;


  std::string strArgv(argv[1]);
  argc--;
  argv++;
  if (strArgv == "SinclairToCovarianceMatrix")
    return (generic_SinclairImageFilter<CovSRFilterType>(argc, argv));
  else if (strArgv == "SinclairToCircularCovarianceMatrix")
    return (generic_SinclairImageFilter<CCSRFilterType>(argc, argv));
  else if (strArgv == "SinclairToCoherencyMatrix")
    return (generic_SinclairImageFilter<CohSRFilterType>(argc, argv));
  else if (strArgv == "SinclairToMuellerMatrix")
    return (generic_SinclairImageFilter<MSRFilterType>(argc, argv));
  else
    return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
