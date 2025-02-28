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

#ifndef otbPCAImageFilter_h
#define otbPCAImageFilter_h

#include "otbMacro.h"
#include "otbMatrixImageFilter.h"
#include "otbNormalizeVectorImageFilter.h"


namespace otb
{

namespace Transform
{
enum TransformDirection
{
  FORWARD = 0,
  INVERSE = 1
};
}

/** \class PCAImageFilter
 * \brief Performs a Principal Component Analysis
 *
 * The internal structure of this filter is a filter-to-filter like structure.
 * The estimation of the covariance matrix has persistent capabilities...
 *
 * \sa otbStreamingStatisticsVectorImageFilter
 * \sa MatrixMultiplyImageFilter
 *
 * \ingroup OTBDimensionalityReduction
 */
template <class TInputImage, class TOutputImage, Transform::TransformDirection TDirectionOfTransformation>
class ITK_EXPORT PCAImageFilter : public itk::ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Standard typedefs */
  typedef PCAImageFilter Self;
  typedef itk::ImageToImageFilter<TInputImage, TOutputImage> Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Type macro */
  itkNewMacro(Self);

  /** Creation through object factory macro */
  itkTypeMacro(PCAImageFilter, ImageToImageFilter);

  /** Dimension */
  itkStaticConstMacro(InputImageDimension, unsigned int, TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int, TOutputImage::ImageDimension);

  typedef Transform::TransformDirection TransformDirectionEnumType;
  itkStaticConstMacro(DirectionOfTransformation, TransformDirectionEnumType, TDirectionOfTransformation);

  /** Template parameters typedefs */
  typedef TInputImage  InputImageType;
  typedef TOutputImage OutputImageType;

  /** Filter types and related */
  typedef StreamingStatisticsVectorImageFilter<InputImageType> CovarianceEstimatorFilterType;
  typedef typename CovarianceEstimatorFilterType::Pointer      CovarianceEstimatorFilterPointerType;

  typedef typename CovarianceEstimatorFilterType::RealType         RealType;
  typedef typename CovarianceEstimatorFilterType::RealPixelType    VectorType;
  typedef typename CovarianceEstimatorFilterType::MatrixObjectType MatrixObjectType;
  typedef typename MatrixObjectType::ComponentType                 MatrixType;
  typedef typename MatrixType::InternalMatrixType                  InternalMatrixType;
  typedef typename InternalMatrixType::element_type                MatrixElementType;

  typedef MatrixImageFilter<TInputImage, TOutputImage> TransformFilterType;
  typedef typename TransformFilterType::Pointer TransformFilterPointerType;

  typedef NormalizeVectorImageFilter<TInputImage, TOutputImage> NormalizeFilterType;
  typedef typename NormalizeFilterType::Pointer NormalizeFilterPointerType;

  /**
   * Set/Get the number of required largest principal components.
   * The filter produces the required number of principal components plus one outputs.
   * Output index 0 represents the mean image and the remaining outputs the requested principal components.
   */
  itkSetMacro(NumberOfPrincipalComponentsRequired, unsigned int);
  itkGetMacro(NumberOfPrincipalComponentsRequired, unsigned int);

  itkSetMacro(Whitening, bool);
  itkGetMacro(Whitening, bool);

  itkGetMacro(CovarianceEstimator, CovarianceEstimatorFilterType*);
  itkGetMacro(Transformer, TransformFilterType*);

  itkGetMacro(GivenCovarianceMatrix, bool);
  MatrixType GetCovarianceMatrix() const
  {
    if (m_GivenCovarianceMatrix)
      return m_CovarianceMatrix;
    else
      return this->GetCovarianceEstimator()->GetCovariance();
  }

  void SetCovarianceMatrix(const MatrixType& cov)
  {
    m_CovarianceMatrix      = cov;
    m_GivenCovarianceMatrix = true;
    this->Modified();
  }

  itkGetMacro(TransformationMatrix, MatrixType);
  itkGetMacro(GivenTransformationMatrix, bool);
  void SetTransformationMatrix(const MatrixType& transf, bool isForward = true)
  {
    m_TransformationMatrix          = transf;
    m_GivenTransformationMatrix     = true;
    m_IsTransformationMatrixForward = isForward;
    this->Modified();
  }

  itkGetConstMacro(EigenValues, VectorType);

  itkGetMacro(UseNormalization, bool);
  void SetUseNormalization(bool norm)
  {
    m_UseNormalization            = norm;
    m_UseVarianceForNormalization = norm;
    this->Modified();
  }

  itkGetConstMacro(MeanValues, VectorType);
  void SetMeanValues(const VectorType& data)
  {
    m_UseNormalization = true;
    m_GivenMeanValues  = true;
    m_MeanValues       = data;
    this->Modified();
  }

  itkGetConstMacro(UseVarianceForNormalization, bool);
  itkSetMacro(UseVarianceForNormalization, bool);

  itkGetConstMacro(StdDevValues, VectorType);
  void SetStdDevValues(const VectorType& vec)
  {
    m_UseNormalization            = true;
    m_UseVarianceForNormalization = true;
    m_GivenStdDevValues           = true;
    m_StdDevValues                = vec;
    this->Modified();
  }

  void SetStatisticsUserIgnoredValue(RealType value)
  {
    /** User ignored value for the normalizer */
    m_Normalizer->GetCovarianceEstimator()->SetUserIgnoredValue(value);
    m_Normalizer->GetCovarianceEstimator()->SetIgnoreUserDefinedValue(true);
    /** User ignored value for the covariance estimator */
    m_CovarianceEstimator->SetUserIgnoredValue(value);
    m_CovarianceEstimator->SetIgnoreUserDefinedValue(true);
  }

protected:
  PCAImageFilter();
  ~PCAImageFilter() override
  {
  }

  /** GenerateOutputInformation
   * Propagate vector length info and modify if needed
   * NumberOfPrincipalComponentsRequired
   *
   * In REVERSE mode, the covariance matrix or the transformation matrix
   * (which may not be square) has to be given,
   * otherwise, GenerateOutputInformation throws an itk::ExceptionObject
   */
  void GenerateOutputInformation() override;

  /** GenerateData
   * Through a filter of filter structure
   */
  void GenerateData() override;

  void PrintSelf(std::ostream& os, itk::Indent indent) const override;

  /** Internal methods */
  virtual void ForwardGenerateOutputInformation();
  virtual void ReverseGenerateOutputInformation();
  virtual void ForwardGenerateData();
  virtual void ReverseGenerateData();

  void GenerateTransformationMatrix();

  /** Internal attributes */
  unsigned int m_NumberOfPrincipalComponentsRequired;
  bool         m_UseNormalization;
  bool         m_UseVarianceForNormalization;
  bool         m_GivenMeanValues;
  bool         m_GivenStdDevValues;
  bool         m_GivenCovarianceMatrix;
  bool         m_GivenTransformationMatrix;
  bool         m_IsTransformationMatrixForward;
  bool         m_Whitening;

  VectorType m_MeanValues;
  VectorType m_StdDevValues;
  MatrixType m_CovarianceMatrix;
  VectorType m_EigenValues;
  MatrixType m_TransformationMatrix;

  CovarianceEstimatorFilterPointerType m_CovarianceEstimator;
  TransformFilterPointerType           m_Transformer;
  NormalizeFilterPointerType           m_Normalizer;

private:
  PCAImageFilter(const Self&); // not implemented
  void operator=(const Self&); // not implemented
};

} // end of namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbPCAImageFilter.hxx"
#endif

#endif // otbPCAImageFilter_h
