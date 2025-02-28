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

#include "otbReflectanceToImageImageFilter.h"

//#include "otbReflectanceToRadianceImageFilter.h"
//#include "otbRadianceToImageImageFilter.h"

#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "itkVariableLengthVector.h"

int otbReflectanceToImageImageFilter(int itkNotUsed(argc), char* argv[])
{
  const char*  inputFileName   = argv[1];
  const char*  outputFileName  = argv[2];
  const double angle           = static_cast<double>(atof(argv[3]));
  double       flux            = 0.;
  int          day             = 1;
  int          month           = 1;
  double       solarDistance   = 1.0;
  char         fluxMode[]      = "0";
  char         solarDistMode[] = "1";

  if (strcmp(argv[16], fluxMode) == 0)
  {
    flux = static_cast<double>(atof(argv[17]));
  }
  else if (strcmp(argv[16], solarDistMode) == 0)
  {
    solarDistance = static_cast<double>(atof(argv[17]));
  }
  else
  {
    day   = atoi(argv[17]);
    month = atoi(argv[18]);
  }

  const unsigned int Dimension = 2;
  typedef double     PixelType;
  typedef otb::VectorImage<PixelType, Dimension> InputImageType;
  typedef otb::VectorImage<PixelType, Dimension> OutputImageType;
  typedef otb::ImageFileReader<InputImageType>  ReaderType;
  typedef otb::ImageFileWriter<OutputImageType> WriterType;
  typedef otb::ReflectanceToImageImageFilter<InputImageType, OutputImageType> ReflectanceToImageImageFilterType;
  typedef ReflectanceToImageImageFilterType::VectorType VectorType;

  ReaderType::Pointer reader = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();
  reader->SetFileName(inputFileName);
  writer->SetFileName(outputFileName);
  reader->UpdateOutputInformation();

  unsigned int nbOfComponent = reader->GetOutput()->GetNumberOfComponentsPerPixel();

  VectorType alpha(nbOfComponent);
  VectorType beta(nbOfComponent);
  VectorType solarIllumination(nbOfComponent);
  alpha.Fill(0);
  beta.Fill(0);
  solarIllumination.Fill(0);

  for (unsigned int i = 0; i < nbOfComponent; ++i)
  {
    alpha[i]             = static_cast<double>(atof(argv[i + 4]));
    beta[i]              = static_cast<double>(atof(argv[i + 8]));
    solarIllumination[i] = static_cast<double>(atof(argv[i + 12]));
  }

  // Instantiating object
  ReflectanceToImageImageFilterType::Pointer filter = ReflectanceToImageImageFilterType::New();

  filter->SetAlpha(alpha);
  filter->SetBeta(beta);
  filter->SetZenithalSolarAngle(angle);
  filter->SetSolarIllumination(solarIllumination);

  if (strcmp(argv[16], fluxMode) == 0)
  {
    filter->SetFluxNormalizationCoefficient(flux);
  }
  else if (strcmp(argv[16], solarDistMode) == 0)
  {
    filter->SetSolarDistance(solarDistance);
  }
  else
  {
    filter->SetDay(day);
    filter->SetMonth(month);
  }

  filter->SetInput(reader->GetOutput());
  writer->SetInput(filter->GetOutput());
  writer->Update();

  return EXIT_SUCCESS;


  /*const char * inputFileName  = argv[1];
  const char * outputFileName = argv[2];
  const double angle = static_cast<double>(atof(argv[3]));
  double       flux = 0.;
  int          day = 1;
  int          month = 1;

  if (argc == 17)
    {
    flux = static_cast<double>(atof(argv[16]));
    }
  else
    {
    day = atoi(argv[16]);
    month = atoi(argv[17]);
    }

  const unsigned int Dimension = 2;
  typedef double                                                              PixelType;
  typedef otb::VectorImage<PixelType, Dimension>                              InputImageType;
  typedef otb::VectorImage<PixelType, Dimension>                              OutputImageType;
  typedef otb::ImageFileReader<InputImageType>                                ReaderType;
  typedef otb::ImageFileWriter<OutputImageType>                               WriterType;

typedef otb::ReflectanceToRadianceImageFilter<InputImageType, InputImageType> ReflectanceToRadianceImageFilterType;
typedef otb::RadianceToImageImageFilter<InputImageType, OutputImageType> RadianceToImageImageFilterType;

  typedef ReflectanceToRadianceImageFilterType::VectorType                       VectorType;

  ReaderType::Pointer reader  = ReaderType::New();
  WriterType::Pointer writer = WriterType::New();
  reader->SetFileName(inputFileName);
  writer->SetFileName(outputFileName);
  reader->UpdateOutputInformation();

  unsigned int nbOfComponent = reader->GetOutput()->GetNumberOfComponentsPerPixel();

  VectorType alpha(nbOfComponent);
  VectorType beta(nbOfComponent);
  VectorType solarIllumination(nbOfComponent);
  alpha.Fill(0);
  beta.Fill(0);
  solarIllumination.Fill(0);

  for (unsigned int i = 0; i < nbOfComponent; ++i)
    {
    alpha[i] = static_cast<double>(atof(argv[i + 4]));
    beta[i] = static_cast<double>(atof(argv[i + 8]));
    solarIllumination[i] = static_cast<double>(atof(argv[i + 12]));
    }

  // Instantiating object

ReflectanceToRadianceImageFilterType::Pointer filter = ReflectanceToRadianceImageFilterType::New();
RadianceToImageImageFilterType::Pointer filter2 = RadianceToImageImageFilterType::New();

  filter2->SetAlpha(alpha);
  filter2->SetBeta(beta);
  filter->SetZenithalSolarAngle(angle);
  filter->SetSolarIllumination(solarIllumination);

  if (argc == 17)
    {
    filter->SetFluxNormalizationCoefficient(flux);
    }
  else
    {
    filter->SetDay(day);
    filter->SetMonth(month);
    }

  filter->SetInput(reader->GetOutput());
  filter2->SetInput(filter->GetOutput());
  writer->SetInput(filter2->GetOutput());
  writer->Update();

  return EXIT_SUCCESS; */
}
