/*=========================================================================
 
   Copyright 2010, Dr. Sandra Black
   Linda C. Campbell Cognitive Neurology Unit
   Sunnybrook Health Sciences Center

   This file is part of ITK-SNAP -SB

   ITK-SNAP-SB is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

   ------
 ========================================================================*/
#ifndef __itkTopologyPreservingDigitalSurfaceEvolutionImageFilter_h
#define __itkTopologyPreservingDigitalSurfaceEvolutionImageFilter_h

#include "itkArray.h"
#include "itkInPlaceImageFilter.h"
#include "itkNeighborhoodIterator.h"

#include <deque>

/** \class TopologyPreservingDigitalSurfaceEvolutionImageFilter
 *  
 */

namespace itk
{
template <class TImage>
class TopologyPreservingDigitalSurfaceEvolutionImageFilter
: public InPlaceImageFilter<TImage>
{
public:

  /** Extract dimension from input image. */
  itkStaticConstMacro( ImageDimension,
         unsigned int, TImage::ImageDimension );

  /** Convenient typedefs for simplifying declarations. */
  typedef TImage                                ImageType;

  /** Standard class typedefs. */
  typedef TopologyPreservingDigitalSurfaceEvolutionImageFilter    Self;
  typedef InPlaceImageFilter<ImageType>                           Superclass;
  typedef SmartPointer<Self>                                      Pointer;
  typedef SmartPointer<const Self>                                ConstPointer;

  /** Run-time type information (and related methods) */
  itkTypeMacro( TopologyPreservingDigitalSurfaceEvolutionImageFilter, 
    InPlaceImageFilter );

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** Image typedef support. */
  typedef typename ImageType::PixelType         PixelType;
  typedef typename ImageType::IndexType         IndexType;
  typedef std::deque<IndexType>                 IndexContainerType;
  typedef NeighborhoodIterator<ImageType>       NeighborhoodIteratorType;

  typedef float                                 RealType;

  itkSetObjectMacro( TargetImage, ImageType );
  itkGetObjectMacro( TargetImage, ImageType );

  itkSetMacro( NumberOfIterations, unsigned int );
  itkGetConstMacro( NumberOfIterations, unsigned int );
  
  itkSetMacro( ForegroundValue, PixelType );
  itkGetConstMacro( ForegroundValue, PixelType );
  
  itkSetMacro( UseInversionMode, bool );
  itkGetConstMacro( UseInversionMode, bool );
  
protected:
  TopologyPreservingDigitalSurfaceEvolutionImageFilter();
  virtual ~TopologyPreservingDigitalSurfaceEvolutionImageFilter();
  void PrintSelf( std::ostream& os, Indent indent ) const;
  void GenerateData();

private:
  //purposely not implemented
  TopologyPreservingDigitalSurfaceEvolutionImageFilter( const Self& ); 
  void operator=( const Self& );

  typename ImageType::Pointer                m_TargetImage;
  typename ImageType::Pointer                m_LabelSurfaceImage;

  /**
   * user-selected parameters
   */  
  bool                                       m_UseInversionMode; 
  unsigned int                               m_NumberOfIterations;
  RealType                                   m_ThresholdValue;
  PixelType                                  m_ForegroundValue;
  PixelType                                  m_BackgroundValue;

  /**
   * variables for internal use
   */
  PixelType                                  m_SurfaceLabel;
  IndexContainerType                         m_CriticalConfigurationIndices;

  /**
   * Functions/data for the 2-D case
   */
  void InitializeIndices2D();
  bool IsChangeSafe2D( IndexType );
  bool IsCriticalC1Configuration2D( Array<short> );
  bool IsCriticalC2Configuration2D( Array<short> );
  bool IsCriticalC3Configuration2D( Array<short> );
  bool IsCriticalC4Configuration2D( Array<short> );
  bool IsSpecialCaseOfC4Configuration2D( PixelType, IndexType, 
                                         IndexType, IndexType );

  Array<unsigned int>                        m_RotationIndices[4];
  Array<unsigned int>                        m_ReflectionIndices[2];

  void CreateLabelSurfaceImage();

  void InitializeIndices3D();
  bool IsCriticalC1Configuration3D( Array<short> );
  unsigned int IsCriticalC2Configuration3D( Array<short> );
  bool IsChangeSafe3D( IndexType );

  bool IsCriticalTopologicalConfiguration( IndexType );

  Array<unsigned int>             m_C1Indices[12];
  Array<unsigned int>             m_C2Indices[8];
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTopologyPreservingDigitalSurfaceEvolutionImageFilter.txx"
#endif

#endif
