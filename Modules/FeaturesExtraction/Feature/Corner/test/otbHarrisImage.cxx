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


#include "itkMacro.h"
#include "otbImage.h"

#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "otbHarrisImageFilter.h"

int otbHarrisImage(int itkNotUsed(argc), char* argv[])
{
  const char* inputFilename  = argv[1];
  const char* outputFilename = argv[2];

  double SigmaD((double)::atof(argv[3]));
  double SigmaI((double)::atof(argv[4]));
  double Alpha((double)::atof(argv[5]));

  typedef unsigned char InputPixelType;
  const unsigned int    Dimension = 2;
  typedef unsigned char OutputPixelType;

  typedef otb::Image<InputPixelType, Dimension>  InputImageType;
  typedef otb::Image<OutputPixelType, Dimension> OutputImageType;
  typedef otb::ImageFileReader<InputImageType> ReaderType;
  typedef otb::HarrisImageFilter<InputImageType, OutputImageType> FunctionType;
  typedef otb::ImageFileWriter<OutputImageType> WriterType;

  ReaderType::Pointer   reader = ReaderType::New();
  WriterType::Pointer   writer = WriterType::New();
  FunctionType::Pointer harris = FunctionType::New();

  reader->SetFileName(inputFilename);
  writer->SetFileName(outputFilename);

  harris->SetInput(reader->GetOutput());
  harris->SetSigmaD(SigmaD);
  harris->SetSigmaI(SigmaI);
  harris->SetAlpha(Alpha);

  writer->SetInput(harris->GetOutput());
  writer->Update();

  return EXIT_SUCCESS;
}
