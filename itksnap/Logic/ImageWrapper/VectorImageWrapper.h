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
  Module:    $RCSfile: VectorImageWrapper.h,v $
  Language:  C++
  Date:      $Date: 2009/01/23 20:09:38 $
  Version:   $Revision: 1.2 $
  Copyright (c) 2003 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#ifndef __VectorImageWrapper_h_
#define __VectorImageWrapper_h_

// Smart pointers have to be included from ITK, can't forward reference them
#include "ImageWrapper.h"

/**
 * \class VectorImageWrapper
 * \brief A wrapper around an itk::Image and related pipelines.
 * 
 * VectorImage Wrapper serves as a wrapper around an image pointer, and 
 * is used to unify the treatment of different kinds of vector images in
 * SNaP.  
 */
template<class TPixel> class VectorImageWrapper : public ImageWrapper<TPixel>
{
public:

  // Basic type definitions
  typedef itk::OrientedImage<TPixel,3> ImageType;
  typedef typename itk::SmartPointer<ImageType> ImagePointer;

  // Slice image type
  typedef itk::Image<TPixel,2> SliceType;
  typedef typename itk::SmartPointer<SliceType> SlicePointer;

  // Slicer type
  typedef IRISSlicer<TPixel> SlicerType;
  typedef typename itk::SmartPointer<SlicerType> SlicerPointer;

  // Iterator types
  typedef typename itk::ImageRegionIterator<ImageType> Iterator;
  typedef typename itk::ImageRegionConstIterator<ImageType> ConstIterator;

  /** 
   * Default constructor.  Creates an image wrapper with a blank internal 
   * image 
   */
  VectorImageWrapper() : ImageWrapper<TPixel>() {};

  /** 
   * Copy constructor.  Copies the contents of the passed-in image wrapper. 
   */
  VectorImageWrapper(const VectorImageWrapper<TPixel> &copy) : ImageWrapper<TPixel>(copy) {};
  
  /** Destructor */
  virtual ~VectorImageWrapper() {};

  /**
   * This method is used to perform a deep copy of a region of this image 
   * into another image, potentially resampling the region to use a different
   * voxel size
   */
  ImagePointer DeepCopyRegion(const SNAPSegmentationROISettings &roi,
                              itk::Command *progressCommand = NULL) const;

protected:

};

#endif // __VectorImageWrapper_h_
