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
  Program:   ITK-SNAP
  Module:    $RCSfile: IRISSlicer.h,v $
  Language:  C++
  Date:      $Date: 2007/12/30 04:05:15 $
  Version:   $Revision: 1.3 $
  Copyright (c) 2007 Paul A. Yushkevich
  
  This file is part of ITK-SNAP 

  ITK-SNAP is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
 
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  -----

  Copyright (c) 2003 Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information. 

=========================================================================*/
#ifndef __IRISSlicer_h_
#define __IRISSlicer_h_

#include <ImageCoordinateTransform.h>

#include <itkImageToImageFilter.h>
#include <itkImageSliceConstIteratorWithIndex.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageLinearIteratorWithIndex.h>

/**
 * \class IRISSlicer
 * \brief A slice extraction filter for 3D images.  
 *
 * This filter takes a transform
 * from image space (x=pixel, y=line, z=slice) to display space.  A slice 
 * shows either x-y, y-z, or x-z in the display space.  This filter is necessary
 * because the origin in the slice can correspond to any corner of the image, not
 * just to the origin of the image.  This filter can traverse the image in different
 * directions, accomodating different positions of the display space origin in the
 * image space.
 */
template <class TPixel>
class ITK_EXPORT IRISSlicer 
: public itk::ImageToImageFilter<itk::Image<TPixel,3>, itk::Image<TPixel,2> >
{
public:
  /** Standard class typedefs. */
  typedef IRISSlicer  Self;
  typedef itk::Image<TPixel,3>  InputImageType;
  typedef itk::Image<TPixel,2>  OutputImageType;
  typedef itk::ImageToImageFilter<InputImageType,OutputImageType> Superclass;
  typedef itk::SmartPointer<Self>   Pointer;
  typedef itk::SmartPointer<const Self>  ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);
  
  /** Run-time type information (and related methods). */
  itkTypeMacro(IRISSlicer, ImageToImageFilter);

  /** Some typedefs. */
  typedef typename InputImageType::ConstPointer  InputImagePointer;
  typedef typename InputImageType::RegionType  InputImageRegionType; 
  typedef typename OutputImageType::Pointer  OutputImagePointer;
  typedef typename OutputImageType::RegionType  OutputImageRegionType;
  typedef itk::ImageSliceConstIteratorWithIndex<InputImageType>  InputIteratorType;
  typedef itk::ImageRegionIteratorWithIndex<OutputImageType>  SimpleOutputIteratorType;
  typedef itk::ImageLinearIteratorWithIndex<OutputImageType> OutputIteratorType;

  /** Set the current slice index */
  itkSetMacro(SliceIndex,unsigned int);
  itkGetMacro(SliceIndex,unsigned int);

  /** Set the image axis along which the subsequent slices lie */
  itkSetMacro(SliceDirectionImageAxis,unsigned int);
  itkGetMacro(SliceDirectionImageAxis,unsigned int);

  /** Set the image axis along which the subsequent lines in a slice lie */
  itkSetMacro(LineDirectionImageAxis,unsigned int);
  itkGetMacro(LineDirectionImageAxis,unsigned int);

  /** Set the image axis along which the subsequent pixels in a line lie */
  itkSetMacro(PixelDirectionImageAxis,unsigned int);
  itkGetMacro(PixelDirectionImageAxis,unsigned int);

  /** Set the direction of line traversal */
  itkSetMacro(LineTraverseForward,bool);
  itkGetMacro(LineTraverseForward,bool);

  /** Set the direction of pixel traversal */
  itkSetMacro(PixelTraverseForward,bool);
  itkGetMacro(PixelTraverseForward,bool);

protected:
  IRISSlicer();
  virtual ~IRISSlicer() {};
  void PrintSelf(std::ostream &s, itk::Indent indent) const;

  /** 
   * IRISSlicer can produce an image which is a different
   * resolution than its input image.  As such, IRISSlicer
   * needs to provide an implementation for
   * GenerateOutputInformation() in order to inform the pipeline
   * execution model.  The original documentation of this method is
   * below.
   *
   * \sa ProcessObject::GenerateOutputInformaton()  */
  virtual void GenerateOutputInformation();

  /**
   * This method maps an input region to an output region
   */
  virtual void CallCopyOutputRegionToInputRegion(InputImageRegionType &destRegion,
                              const OutputImageRegionType &srcRegion);

  /** 
   * IRISSlicer is not implemented as a multithreaded filter.
   * \sa ImageToImageFilter::GenerateData()  
   */
  virtual void GenerateData();

private:
  IRISSlicer(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

  // Current slice in each of the dimensions
  unsigned int m_SliceIndex;

  // Image axis corresponding to the slice direction
  unsigned int m_SliceDirectionImageAxis;

  // Image axis corresponding to the line direction
  unsigned int m_LineDirectionImageAxis;
  
  // Image axis corresponding to the pixel direction  
  unsigned int m_PixelDirectionImageAxis;

  // Whether the line direction is reversed
  bool m_LineTraverseForward;

  // Whether the pixel direction is reversed
  bool m_PixelTraverseForward;
  
  // The worker methods in this filter
  // void CopySliceLineForwardPixelForward(InputIteratorType, OutputImageType *);
  // void CopySliceLineForwardPixelBackward(InputIteratorType, OutputImageType *);
  // void CopySliceLineBackwardPixelForward(InputIteratorType, OutputImageType *);
  // void CopySliceLineBackwardPixelBackward(InputIteratorType, OutputImageType *);
};

#ifndef ITK_MANUAL_INSTANTIATION
#include "IRISSlicer.txx"
#endif

#endif //__IRISSlicer_h_
