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
#include <fstream>


#include "otbMacro.h"

int otbCompareAsciiTests5(int argc, char* argv[])
{
  if (argc != 3)
  {
    std::cerr << "Usage: " << argv[0];
    std::cerr << " referenceFile testFile" << std::endl;
    return EXIT_FAILURE;
  }

  std::ofstream fileRef;
  fileRef.open(argv[1]);
  fileRef << "[ 1 1 1 ]\n";
  fileRef << "[ 2 2 2 ]\n";
  fileRef << "[ 3 3 3 ]\n";
  fileRef.close();

  std::ofstream fileTest;
  fileTest.open(argv[2]);
  fileTest << "[ 3.010 2.999 3.000 ]\n";
  fileTest << "[ 1.999 2.000 2.001 ]\n";
  fileTest << "[ 1.001 0.999 1.0005 ]\n";
  fileTest.close();

  return EXIT_SUCCESS;
}
