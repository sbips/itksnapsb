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
=========================================================================*/

#ifndef __FloodFillBinaryThresholdImageFunction_h_
#define __FloodFillBinaryThresholdImageFunction_h_

#include "itkImageFunction.h"
#include <list>

template < class TInputImage, class TLimitImage, class TCoordRep = float >
class FloodFillBinaryThresholdImageFunction :
  public itk::ImageFunction<TInputImage,bool,TCoordRep> 
{
public:
/** Standard class typedefs */
  typedef FloodFillBinaryThresholdImageFunction Self;
  typedef itk::BinaryThresholdImageFunction< TInputImage, TCoordRep > Superclass;
  typedef itk::SmartPointer< Self >                                   Pointer;
  typedef itk::SmartPointer< const Self >                             ConstPointer;
  
  itkTypeMacro( FloodFillBinaryThresholdImageFunction, itk::BinaryThresholdImageFunction );
  itkNewMacro( Self );
  itkStaticConstMacro(ImageDimension, unsigned int,Superclass::ImageDimension);

  typedef typename Superclass::InputImageType      InputImageType;
  typedef TLimitImage                              LimitImageType;
  typedef typename TLimitImage::ConstPointer       LimitImageConstPointer;
  typedef typename TInputImage::PixelType          PixelType;
  typedef typename Superclass::PointType           PointType;
  typedef typename Superclass::IndexType           IndexType;
  typedef typename Superclass::ContinuousIndexType ContinuousIndexType;

  virtual bool Evaluate( const PointType& point ) const
  {
    IndexType index;
    this->ConvertPointToNearestIndex( point, index );
    return ( this->EvaluateAtIndex( index ) );
  }

  virtual bool EvaluateAtContinuousIndex( 
  const ContinuousIndexType & index ) const
  {
    IndexType nindex;

    this->ConvertContinuousIndexToNearestIndex (index, nindex);
    return this->EvaluateAtIndex(nindex);
  }

  virtual bool EvaluateAtIndex( const IndexType & index ) const
  {
    PixelType imgValue = this->GetInputImage()->GetPixel(index);
    typename LimitImageType::PixelType limitValue = this->GetLimitImage()->GetPixel(index);
    if ( m_FillAll )
    {
      if ( limitValue == 0 ) return true;
      return false;
    }
    else
    {
      if ( limitValue == 0 && DoFillVoxel( imgValue ) ) return true;
      return false;
    }
  }

  // should add check for same dimensions on m_LimitImage and m_Image //
  void SetLimitImage( const LimitImageType* ptr ) { m_LimitImage = ptr; }
  const LimitImageType* GetLimitImage() const { return m_LimitImage.GetPointer(); }

  void AddFillValue( PixelType value )    { m_FillValues.push_back( value ); }
  void ClearFillValues( PixelType value ) { m_FillValues.clear(); }
  void FillAll( bool value )              { m_FillAll = value; };

protected:
  FloodFillBinaryThresholdImageFunction() : m_FillAll( false ) {};
  ~FloodFillBinaryThresholdImageFunction() {};
  LimitImageConstPointer m_LimitImage;

private:
  std::list< PixelType > m_FillValues;

  bool                   m_FillAll;

  bool DoFillVoxel( const PixelType imgValue ) const
  {
    typename std::list< PixelType >::const_iterator it;
    for ( it = m_FillValues.begin(); it != m_FillValues.end(); ++it )
    {
     if ( *it == imgValue )
       return true;
    }
    return false;
  }
}
;

#endif
