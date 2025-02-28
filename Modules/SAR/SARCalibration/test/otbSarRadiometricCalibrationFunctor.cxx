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

#include "otbSarRadiometricCalibrationFunctor.h"

int otbSarRadiometricCalibrationFunctor(int itkNotUsed(argc), char* itkNotUsed(argv)[])
{
  typedef double ScalarType;

  typedef otb::Functor::SarRadiometricCalibrationFunctor<ScalarType, ScalarType> FunctorType;

  FunctorType funct;

  funct.SetNoise(10.0);
  if (abs(funct.GetNoise() - 10.0) > 0.0)
  {
    return false;
  }
  funct.SetScale(10.0);
  if (abs(funct.GetScale() - 10.0) > 0.0)
  {
    return false;
  }
  funct.SetAntennaPatternNewGain(10.0);
  if (abs(funct.GetAntennaPatternNewGain() - 10.0) > 0.0)
  {
    return false;
  }
  funct.SetAntennaPatternOldGain(10.0);
  if (abs(funct.GetAntennaPatternOldGain() - 10.0) > 0.0)
  {
    return false;
  }
  funct.SetIncidenceAngle(10.0);
  if (abs(funct.GetIncidenceAngle() - 10.0) > 0.0)
  {
    return false;
  }
  funct.SetRangeSpreadLoss(10.0);
  if (abs(funct.GetRangeSpreadLoss() - 10.0) > 0.0)
  {
    return false;
  }
  std::cout << "First  sigma : " << funct.operator()(0.) << std::endl;
  std::cout << "Second sigma : " << funct.operator()(1.) << std::endl;


  return EXIT_SUCCESS;
}
