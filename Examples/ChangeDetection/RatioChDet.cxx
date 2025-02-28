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


/* Example usage:
./RatioChDet Input/GomaAvant.png Input/GomaApres.png Output/RatioChDet.tif 3
*/

#include "otbMeanRatioImageFilter.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "otbImage.h"
#include "itkShiftScaleImageFilter.h"
#include "otbCommandProgressUpdate.h"

int main(int argc, char* argv[])
{

  if (argc < 5)
  {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " inputImageFile1 inputImageFile2  outputImageFile radius" << std::endl;
    return -1;
  }

  // Define the dimension of the images
  const unsigned int Dimension = 2;

  // We start by declaring the types for the two input images, the
  // change image and the image to be stored in a file for visualization.
  using InternalPixelType = float;
  using OutputPixelType   = unsigned char;
  using InputImageType1   = otb::Image<InternalPixelType, Dimension>;
  using InputImageType2   = otb::Image<InternalPixelType, Dimension>;
  using ChangeImageType   = otb::Image<InternalPixelType, Dimension>;
  using OutputImageType   = otb::Image<OutputPixelType, Dimension>;

  // We can now declare the types for the readers. Since the images
  // can be vey large, we will force the pipeline to use
  // streaming. For this purpose, the file writer will be
  // streamed. This is achieved by using the
  // ImageFileWriter class.
  using ReaderType1 = otb::ImageFileReader<InputImageType1>;
  using ReaderType2 = otb::ImageFileReader<InputImageType2>;
  using WriterType  = otb::ImageFileWriter<OutputImageType>;

  // The change detector will give a normalized result between 0 and
  // 1. In order to store the result in PNG format we will
  // rescale the results of the change detection in order to use all
  // the output pixel type range of values.
  using RescalerType = itk::ShiftScaleImageFilter<ChangeImageType, OutputImageType>;

  // The MeanRatioImageFilter is templated over
  // the types of the two input images and the type of the generated change
  // image.
  using FilterType = otb::MeanRatioImageFilter<InputImageType1, InputImageType2, ChangeImageType>;

  // The different elements of the pipeline can now be instantiated.
  ReaderType1::Pointer  reader1  = ReaderType1::New();
  ReaderType2::Pointer  reader2  = ReaderType2::New();
  WriterType::Pointer   writer   = WriterType::New();
  FilterType::Pointer   filter   = FilterType::New();
  RescalerType::Pointer rescaler = RescalerType::New();

  const char* inputFilename1 = argv[1];
  const char* inputFilename2 = argv[2];
  const char* outputFilename = argv[3];

  // We set the parameters of the different elements of the pipeline.
  reader1->SetFileName(inputFilename1);
  reader2->SetFileName(inputFilename2);
  writer->SetFileName(outputFilename);
  float scale = itk::NumericTraits<OutputPixelType>::max();
  rescaler->SetScale(scale);

  // The only parameter for this change detector is the radius of
  // the window used for computing the mean of the intensities.
  filter->SetRadius(atoi(argv[4]));

  // We build the pipeline by plugging all the elements together.
  filter->SetInput1(reader1->GetOutput());
  filter->SetInput2(reader2->GetOutput());

  rescaler->SetInput(filter->GetOutput());
  writer->SetInput(rescaler->GetOutput());

  using CommandType = otb::CommandProgressUpdate<FilterType>;

  CommandType::Pointer observer = CommandType::New();
  filter->AddObserver(itk::ProgressEvent(), observer);

  writer->Update();
}
