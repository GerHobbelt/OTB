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


#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "otbExtractROI.h"

#include "otbMuellerToReciprocalCovarianceImageFilter.h"

int otbMuellerToReciprocalCovarianceImageFilter(int itkNotUsed(argc), char* argv[])
{
  const char* inputFilename  = argv[1];
  const char* outputFilename = argv[2];

  typedef double                  PixelType;
  typedef std::complex<PixelType> ComplexPixelType;

  typedef otb::VectorImage<PixelType>        RealImageType;
  typedef otb::VectorImage<ComplexPixelType> ComplexImageType;
  typedef otb::MuellerToReciprocalCovarianceImageFilter<RealImageType, ComplexImageType> FilterType;

  typedef otb::ImageFileReader<RealImageType>    ReaderType;
  typedef otb::ImageFileWriter<ComplexImageType> WriterType;

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();

  reader->SetFileName(inputFilename);

  FilterType::Pointer filter = FilterType::New();
  filter->SetInput<0>(reader->GetOutput());

  writer->SetFileName(outputFilename);
  writer->SetInput(filter->GetOutput());
  writer->Update();

  return EXIT_SUCCESS;
}
