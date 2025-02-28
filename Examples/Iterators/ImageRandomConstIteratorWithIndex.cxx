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


// \doxygen{itk}{ImageRandomConstIteratorWithIndex} was developed to randomly
// sample pixel values.  When incremented or decremented, it jumps to a random
// location in its image region.
//
// \index{itk::Image\-Random\-Const\-Iterator\-With\-Index!sample size}
// \index{itk::Image\-Random\-Const\-Iterator\-With\-Index!begin and end positions}
//
// The user must specify a sample size when creating this iterator. The sample
// size, rather than a specific image index, defines the end position for the
// iterator.  \code{IsAtEnd()} returns
// \code{true} when the current sample number equals the sample size.
// \code{IsAtBegin()} returns \code{true} when the current sample number equals
// zero.  An important difference from other image iterators is that
// ImageRandomConstIteratorWithIndex may visit the same pixel
//  more than once.
//
// \index{itk::Image\-Random\-Const\-Iterator\-With\-Index!example of using|(}
// \index{itk::Image\-Random\-Const\-Iterator\-With\-Index!and statistics}
// Let's use the random iterator to estimate some simple image statistics. The next
// example calculates an estimate of the arithmetic mean of pixel values.
//
// First, include the appropriate header and declare pixel and image types.

#include "otbImage.h"
#include "itkImageRandomConstIteratorWithIndex.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"

int main(int argc, char* argv[])
{
  // Verify the number of parameters on the command line.
  if (argc < 3)
  {
    std::cerr << "Missing parameters. " << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " inputImageFile numberOfSamples" << std::endl;
    return -1;
  }

  const unsigned int Dimension = 2;

  using PixelType         = unsigned short;
  using ImageType         = otb::Image<PixelType, Dimension>;
  using ConstIteratorType = itk::ImageRandomConstIteratorWithIndex<ImageType>;

  using ReaderType = otb::ImageFileReader<ImageType>;

  ImageType::ConstPointer inputImage;
  ReaderType::Pointer     reader = ReaderType::New();
  reader->SetFileName(argv[1]);
  try
  {
    reader->Update();
    inputImage = reader->GetOutput();
  }
  catch (itk::ExceptionObject& err)
  {
    std::cout << "ExceptionObject caught !" << std::endl;
    std::cout << err << std::endl;
    return -1;
  }

  // The input image has been read as \code{inputImage}.  We now create an
  // iterator with a number of samples set by command line argument. The call to
  // \code{ReinitializeSeed} seeds the random number generator.  The iterator is
  // initialized over the entire valid image region.
  //
  //  \index{itk::Image\-Random\-Const\-Iterator\-With\-Index!SetNumberOfSamples()}
  //  \index{itk::Image\-Random\-Const\-Iterator\-With\-Index!ReinitializeSeed()}

  ConstIteratorType inputIt(inputImage, inputImage->GetRequestedRegion());
  inputIt.SetNumberOfSamples(::atoi(argv[2]));
  inputIt.ReinitializeSeed();

  // Now take the specified number of samples and calculate their average value.

  float mean = 0.0f;
  for (inputIt.GoToBegin(); !inputIt.IsAtEnd(); ++inputIt)
  {
    mean += static_cast<float>(inputIt.Get());
  }
  mean = mean / ::atof(argv[2]);
  std::cout << "Mean estimate with " << argv[2] << " samples is " << mean << std::endl;

  // Table~\ref{fig:ImageRandomConstIteratorWithIndexExample} shows the results
  // of running this example on several of the data files from
  // \code{Examples/Data} with a range of sample sizes.
  //
  // \begin{table}
  // \begin{center}
  // \begin{tabular}[]{rc|c|c|c}
  // & \multicolumn{4}{c}{\emph{Sample Size}} \\  & \code{\textbf{10}} & \code{\textbf{100}}
  // & \code{\textbf{1000}}
  // & \code{\textbf{10000}} \\ \cline{2-5}
  // \code{RatLungSlice1.mha} & 50.5 & 52.4 & 53.0 & 52.4 \\ \code{RatLungSlice2.mha}
  // & 46.7 & 47.5 & 47.4 & 47.6  \\ \code{BrainT1Slice.png}
  // & 47.2 & 64.1 & 68.0 & 67.8  \\ \end{tabular}
  // \protect\label{fig:ImageRandomConstIteratorWithIndexExample}
  // \itkcaption[ImageRandomConstIteratorWithIndex usage]{Estimates of mean image pixel
  // value using the ImageRandomConstIteratorWithIndex at different sample
  // sizes.}
  // \end{center}
  // \end{table}
  //
  // \index{itk::Image\-Random\-Const\-Iterator\-With\-Index!example of using|)}

  return EXIT_SUCCESS;
}
