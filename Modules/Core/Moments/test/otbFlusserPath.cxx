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
#include <iomanip>
#include <fstream>
#include "otbFlusserPathFunction.h"
#include "itkPolyLineParametricPath.h"
#include "itkMacro.h"

int otbFlusserPath(int itkNotUsed(argc), char* argv[])
{
  unsigned int                                   Number;
  const unsigned int                             Dimension      = 2;
  const char*                                    outputFilename = argv[1];
  typedef itk::PolyLineParametricPath<Dimension> PathType;
  typedef otb::FlusserPathFunction<PathType>     FunctionType;
  typedef FunctionType::RealType                 RealType;

  // Draw a square
  PathType::ContinuousIndexType cindex;
  PathType::Pointer             pathElt = PathType::New();

  pathElt->Initialize();

  cindex[0] = 30;
  cindex[1] = 30;
  pathElt->AddVertex(cindex);
  cindex[0] = 30;
  cindex[1] = 130;
  pathElt->AddVertex(cindex);
  cindex[0] = 130;
  cindex[1] = 130;
  pathElt->AddVertex(cindex);
  cindex[0] = 130;
  cindex[1] = 30;
  pathElt->AddVertex(cindex);

  FunctionType::Pointer function = FunctionType::New();
  // OTB-FA-00022-CS
  function->SetInputPath(pathElt);

  std::ofstream outputStream(outputFilename);

  outputStream << std::setprecision(10) << "Flusser Path moments: [12]" << std::endl;

  RealType Result;

  for (Number = 1; Number < 12; Number++)
  {
    // OTB-FA-00024-CS
    function->SetMomentNumber(Number);
    Result = function->Evaluate();
    outputStream << "Flusser(" << Number << ") = " << Result << std::endl;
  }

  outputStream.close();

  return EXIT_SUCCESS;
}
