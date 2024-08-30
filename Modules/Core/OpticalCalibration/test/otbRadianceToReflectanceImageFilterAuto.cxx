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


#include "otbImageToRadianceImageFilter.h"
#include "otbRadianceToReflectanceImageFilter.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "otbMultiChannelExtractROI.h"

int otbRadianceToReflectanceImageFilterAuto(int itkNotUsed(argc), char* argv[])
{
  const char* inputFileName  = argv[1];
  const char* outputFileName = argv[2];

  const unsigned int Dimension = 2;
  typedef double     PixelType;
  typedef otb::VectorImage<PixelType, Dimension> InputImageType;
  typedef otb::VectorImage<PixelType, Dimension> OutputImageType;
  typedef otb::ImageFileReader<InputImageType>  ReaderType;
  typedef otb::ImageFileWriter<OutputImageType> WriterType;
  typedef otb::ImageToRadianceImageFilter<InputImageType, OutputImageType>        ImageToRadianceImageFilterType;
  typedef otb::RadianceToReflectanceImageFilter<OutputImageType, OutputImageType> RadianceToReflectanceImageFilterType;
  typedef otb::MultiChannelExtractROI<PixelType, PixelType>                       RoiFilterType;

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();
  reader->SetFileName(inputFileName);
  writer->SetFileName(outputFileName);
  reader->UpdateOutputInformation();

  // Instantiating object
  ImageToRadianceImageFilterType::Pointer filterToRadiance = ImageToRadianceImageFilterType::New();
  filterToRadiance->SetInput(reader->GetOutput());

  RadianceToReflectanceImageFilterType::Pointer filterToReflectance = RadianceToReflectanceImageFilterType::New();
  filterToReflectance->SetInput(filterToRadiance->GetOutput());
  filterToReflectance->SetUseClamp(false);

  RoiFilterType::Pointer roiFilter = RoiFilterType::New();
  roiFilter->SetStartX(1000);
  roiFilter->SetStartY(1000);
  roiFilter->SetSizeX(100);
  roiFilter->SetSizeY(100);
  roiFilter->SetInput(filterToReflectance->GetOutput());

  writer->SetInput(roiFilter->GetOutput());
  writer->Update();

  return EXIT_SUCCESS;
}
