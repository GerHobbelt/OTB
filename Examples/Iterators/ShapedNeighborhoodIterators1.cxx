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
#include "otbImageFileWriter.h"
#include "itkNeighborhoodAlgorithm.h"
#include <math.h>

// This example uses \doxygen{itk}{ShapedNeighborhoodIterator} to implement a binary
// erosion algorithm.  If we think of an image $I$ as a set of pixel indices,
// then erosion of $I$ by a smaller set $E$, called the \emph{structuring
// element}, is the set of all indices at locations $x$ in $I$ such that when
// $E$ is positioned at $x$, every element in $E$ is also contained in $I$.
//
// This type of algorithm is easy to implement with shaped neighborhood
// iterators because we can use the iterator itself as the structuring element
// $E$ and move it sequentially through all positions $x$.  The result at $x$
// is obtained by checking values in a simple iteration loop through the
// neighborhood stencil.
//
// We need two iterators, a shaped iterator for the input image and a regular
// image iterator for writing results to the output image.

#include "itkConstShapedNeighborhoodIterator.h"
#include "itkImageRegionIterator.h"

int main(int argc, char* argv[])
{
  if (argc < 4)
  {
    std::cerr << "Missing parameters. " << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " inputImageFile outputImageFile element_radius" << std::endl;
    return -1;
  }

  // Since we are working with binary images in this example, an \code{unsigned
  // char} pixel type will do.  The image and iterator types are defined using
  // the pixel type.

  using PixelType = unsigned char;
  using ImageType = otb::Image<PixelType, 2>;

  using ShapedNeighborhoodIteratorType = itk::ConstShapedNeighborhoodIterator<ImageType>;

  using IteratorType = itk::ImageRegionIterator<ImageType>;

  using ReaderType           = otb::ImageFileReader<ImageType>;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName(argv[1]);

  try
  {
    reader->Update();
  }
  catch (itk::ExceptionObject& err)
  {
    std::cout << "ExceptionObject caught !" << std::endl;
    std::cout << err << std::endl;
    return -1;
  }

  ImageType::Pointer output = ImageType::New();
  output->SetRegions(reader->GetOutput()->GetRequestedRegion());
  output->Allocate();

  // Refer to the examples in Section~\ref{sec:itkNeighborhoodIterator} or the
  // source code of this example for a description of how to read the input image
  // and allocate a matching output image.
  //
  // The size of the structuring element is read from the command line and used
  // to define a radius for the shaped neighborhood iterator.  Using the method
  // developed in section~\ref{sec:itkNeighborhoodIterator} to minimize bounds
  // checking, the iterator itself is not initialized until entering the
  // main processing loop.

  unsigned int                               element_radius = ::atoi(argv[3]);
  ShapedNeighborhoodIteratorType::RadiusType radius;
  radius.Fill(element_radius);

  // The face calculator object introduced in
  // Section~\ref{sec:NeighborhoodExample3} is created and used as before.

  using FaceCalculatorType = itk::NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<ImageType>;

  FaceCalculatorType                         faceCalculator;
  FaceCalculatorType::FaceListType           faceList;
  FaceCalculatorType::FaceListType::iterator fit;

  faceList = faceCalculator(reader->GetOutput(), output->GetRequestedRegion(), radius);

  // Now we initialize some variables and constants.

  IteratorType out;

  const PixelType background_value = 0;
  const PixelType foreground_value = 255;
  const float     rad              = static_cast<float>(element_radius);

  // The outer loop of the algorithm is structured as in previous neighborhood
  // iterator examples.  Each region in the face list is processed in turn.  As each new
  // region is processed, the input and output iterators are initialized on that
  // region.
  //
  // The shaped iterator that ranges over the input is our structuring element
  // and its active stencil must be created accordingly.  For this example, the
  // structuring element is shaped like a circle of radius
  // \code{element\_radius}.  Each of the appropriate neighborhood offsets is
  // activated in the double \code{for} loop.

  for (fit = faceList.begin(); fit != faceList.end(); ++fit)
  {
    ShapedNeighborhoodIteratorType it(radius, reader->GetOutput(), *fit);
    out = IteratorType(output, *fit);

    // Creates a circular structuring element by activating all the pixels less
    // than radius distance from the center of the neighborhood.

    for (float y = -rad; y <= rad; y++)
    {
      for (float x = -rad; x <= rad; x++)
      {
        ShapedNeighborhoodIteratorType::OffsetType off;

        float dis = ::sqrt(x * x + y * y);
        if (dis <= rad)
        {
          off[0] = static_cast<int>(x);
          off[1] = static_cast<int>(y);
          it.ActivateOffset(off);
        }
      }
    }

    // The inner loop, which implements the erosion algorithm, is fairly simple.
    // The \code{for} loop steps the input and output iterators through their
    // respective images.  At each step, the active stencil of the shaped iterator
    // is traversed to determine whether all pixels underneath the stencil contain
    // the foreground value, i.e. are contained within the set $I$.  Note the use
    // of the stencil iterator, \code{ci}, in performing this check.

    // Implements erosion
    for (it.GoToBegin(), out.GoToBegin(); !it.IsAtEnd(); ++it, ++out)
    {
      ShapedNeighborhoodIteratorType::ConstIterator ci;

      bool flag = true;
      for (ci = it.Begin(); ci != it.End(); ci++)
      {
        if (ci.Get() == background_value)
        {
          flag = false;
          break;
        }
      }
      if (flag == true)
      {
        out.Set(foreground_value);
      }
      else
      {
        out.Set(background_value);
      }
    }
  }

  using WriterType = otb::ImageFileWriter<ImageType>;

  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName(argv[2]);
  writer->SetInput(output);
  try
  {
    writer->Update();
  }
  catch (itk::ExceptionObject& err)
  {
    std::cout << "ExceptionObject caught !" << std::endl;
    std::cout << err << std::endl;
    return -1;
  }

  return EXIT_SUCCESS;
}
