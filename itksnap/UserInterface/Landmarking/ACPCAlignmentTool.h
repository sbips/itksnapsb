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

#ifndef __ACPCAlignmentTool__h_
#define __ACPCAlignmentTool__h_

#include <itkImage.h>
#include <itkCenteredEuler3DTransform.h>
#include <itkResampleImageFilter.h>
#include <itkNearestNeighborInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>
#include "GenericImageData.h"
#include "GreyImageWrapper.h"
#include "SNAPCommon.h"



class ACPCAlignmentTool
{
private:
  typedef GreyImageWrapper::ImageType ImageType;

public:
  ACPCAlignmentTool();
  ~ACPCAlignmentTool();

  /* Tool must be intialized with existing image before use.  
     Contents of image imgGrey will be copied to m_Image and will
     serve as the source image for resampling operations.  */
  void Initialize( ImageType::Pointer imgGrey );

  /* Alignment changes require specification of rotation and translation 
     along the 3 major axes, the cursor location (req'd so that the resampling
     is performed only in the 3 planes visible to the user), and a pointer
     to the main grey image in program memory (note that this function will
     alter the contents of the main grey image, but any alterations can be
     undone by referencing the original m_Image) */
  void AlignmentChange( int degreesX, int degreesY, int degreesZ, 
                        int shiftX, int shiftY, int shiftZ,
                        Vector3ui cursor,
                        ImageType::Pointer imgGrey );

  /* Return the source image (note that this will be either the main grey 
     image originally loaded into memory or a resampled version of the original
     image if the user has explicitly requested resampling of the full volume
     through the Resample() function). */
  ImageType::Pointer GetOriginalImage() { return m_Image; }

  /* Resample the full volume (note that this will overwrite m_Image such that the
     resampled output will be the new input to any subsequent alignment changes). */
  void                Resample();

  /* Turn on/off linear interpolation. */
  void                SetInterpolate( bool value ) { m_IsInterpolate = value; };
  
  /* Return transform in analyze format (i.e. transposed, signs changed and inverted). */  
  vnl_matrix< double >GetAnalyzeTransform();

  /* Set transform to identity. */
  void                SetIdentityTransform();

  /* Set transform to inverse. */
  void                SetInverseTransform();

  /* Save transform in analyze format to text file. */
  void                SaveTransform( const char* fn );
  
  /* Load transform in analyze format from text file. */
  void                LoadTransform( const char* fn );
  

private:
  typedef itk::CenteredEuler3DTransform< double >                   TransformType;
  typedef itk::ResampleImageFilter< ImageType, ImageType >          ResampleType;
  typedef itk::NearestNeighborInterpolateImageFunction< ImageType > NInterpolatorType;
  typedef itk::LinearInterpolateImageFunction< ImageType >          LInterpolatorType;

  TransformType::Pointer     m_CummulativeTransform;
  
  ImageType::Pointer         m_Image;
  ResampleType::Pointer      m_Resampler;
  NInterpolatorType::Pointer m_NInterpolator;
  LInterpolatorType::Pointer m_LInterpolator;
  
  bool m_IsInterpolate;
};


#endif
