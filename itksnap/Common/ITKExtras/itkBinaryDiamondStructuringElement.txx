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
  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkBinaryDiamondStructuringElement.txx,v $
  Language:  C++
  Date:      $Date: 2008/12/10 18:42:15 $
  Version:   $Revision: 1.2 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkBinaryDiamondStructuringElement_txx
#define __itkBinaryDiamondStructuringElement_txx
#include "itkBinaryDiamondStructuringElement.h"

#include "itkNumericTraits.h"

namespace itk
{

// Create the structuring element
template <class TPixel, unsigned int VDimension, class TAllocator>
void
BinaryDiamondStructuringElement<TPixel, VDimension, TAllocator>
::CreateStructuringElement()
{

  unsigned int minRadius = this->GetRadius( 0 );
  for ( unsigned d = 1; d < NeighborhoodDimension; d++ )
    {
    if ( minRadius > this->GetRadius( d ) )
      {
      minRadius = this->GetRadius( d );
      }    
    }

  for ( unsigned int n = 0; n < this->Size(); n++ )
    {
    OffsetType offset = this->GetOffset( n );
    unsigned int manhattanDistance = 0;
    for ( unsigned int d = 0; d < NeighborhoodDimension; d++ )
      {
      manhattanDistance += vnl_math_abs( offset[d] );
      } 
    if ( manhattanDistance <= minRadius )
      {
      this->operator[]( n ) = NumericTraits<TPixel>::One;
      } 
    else
      {
      this->operator[]( n ) = NumericTraits<TPixel>::Zero;
      }
    }  
}

} // namespace itk

#endif
