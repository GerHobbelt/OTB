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


#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "otbImage.h"
#include "otbMeanDifferenceImageFilter.h"
#include "otbCommandProgressUpdate.h"

int otbMeanDiffChangeDetectionTest(int argc, char* argv[])
{

  if (argc < 5)
  {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " inputImageFile1 inputImageFile2  radius outputImageFile " << std::endl;
    return -1;
  }

  // Define the dimension of the images
  const unsigned int Dimension = 2;

  // Declare the types of the images
  typedef double InternalPixelType;
  typedef double OutputPixelType;
  typedef otb::Image<InternalPixelType, Dimension> InputImageType1;
  typedef otb::Image<InternalPixelType, Dimension> InputImageType2;
  typedef otb::Image<InternalPixelType, Dimension> ChangeImageType;
  typedef otb::Image<OutputPixelType, Dimension>   OutputImageType;

  typedef otb::ImageFileReader<InputImageType1> ReaderType1;
  typedef otb::ImageFileReader<InputImageType2> ReaderType2;
  typedef otb::ImageFileWriter<OutputImageType> WriterType;
  typedef itk::RescaleIntensityImageFilter<ChangeImageType, OutputImageType> RescalerType;

  // Declare the type for the filter
  typedef otb::MeanDifferenceImageFilter<InputImageType1, InputImageType2, ChangeImageType> FilterType;

  ReaderType1::Pointer  reader1  = ReaderType1::New();
  ReaderType2::Pointer  reader2  = ReaderType2::New();
  WriterType::Pointer   writer   = WriterType::New();
  FilterType::Pointer   filter   = FilterType::New();
  RescalerType::Pointer rescaler = RescalerType::New();

  const char* inputFilename1 = argv[1];
  const char* inputFilename2 = argv[2];
  const char* outputFilename = argv[4];

  reader1->SetFileName(inputFilename1);
  reader2->SetFileName(inputFilename2);
  writer->SetFileName(outputFilename);
  rescaler->SetOutputMinimum(-1);
  rescaler->SetOutputMaximum(1);

  filter->SetInput1(reader1->GetOutput());
  filter->SetInput2(reader2->GetOutput());
  filter->SetRadius(atoi(argv[3]));

  rescaler->SetInput(filter->GetOutput());
  writer->SetInput(rescaler->GetOutput());

  typedef otb::CommandProgressUpdate<FilterType> CommandType;

  CommandType::Pointer observer = CommandType::New();
  filter->AddObserver(itk::ProgressEvent(), observer);

  writer->Update();

  return EXIT_SUCCESS;
}
