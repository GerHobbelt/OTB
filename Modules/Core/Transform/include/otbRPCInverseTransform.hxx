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

#ifndef otbRPCInverseTransform_hxx
#define otbRPCInverseTransform_hxx

#include "otbRPCInverseTransform.h"

namespace otb
{
template <class TScalarType, unsigned int NInputDimensions, unsigned int NOutputDimensions>
RPCInverseTransform<TScalarType, NInputDimensions, NOutputDimensions>::RPCInverseTransform() : Superclass(TransformDirection::INVERSE)
{}

template <class TScalarType, unsigned int NInputDimensions, unsigned int NOutputDimensions>
typename RPCInverseTransform<TScalarType, NInputDimensions, NOutputDimensions>::OutputPointType
RPCInverseTransform<TScalarType, NInputDimensions, NOutputDimensions>::TransformPoint(const InputPointType& point) const
{
  GDALRPCTransformer::PointType zePoint;
  zePoint[0] = static_cast<double>(point[0]);
  zePoint[1] = static_cast<double>(point[1]);
  if (NInputDimensions > 2)
    zePoint[2] = static_cast<double>(point[2]);
  else
    zePoint[2] = 0.;

  zePoint = this->m_Transformer->InverseTransform(zePoint);

  OutputPointType pOut;
  pOut[0] = static_cast<TScalarType>(zePoint[0]);
  pOut[1] = static_cast<TScalarType>(zePoint[1]);

  if (NOutputDimensions > 2)
    pOut[2] = static_cast<TScalarType>(zePoint[2]);

  return pOut;
}

/**
 * PrintSelf method
 */
template <class TScalarType, unsigned int NInputDimensions, unsigned int NOutputDimensions>
void RPCInverseTransform<TScalarType, NInputDimensions, NOutputDimensions>::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Transformation direction: Inverse" << std::endl;
}

}

#endif
