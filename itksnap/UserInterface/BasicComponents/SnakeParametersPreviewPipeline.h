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
  Module:    $RCSfile: SnakeParametersPreviewPipeline.h,v $
  Language:  C++
  Date:      $Date: 2009/01/23 20:09:38 $
  Version:   $Revision: 1.4 $
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
#ifndef __SnakeParametersPreviewPipeline_h_
#define __SnakeParametersPreviewPipeline_h_

#include "SNAPOpenGL.h"
#include "SNAPCommonUI.h"
#include "itkOrientedImage.h"
#include "itkRGBAPixel.h"
#include "itkCovariantVector.h"
#include "itkUnaryFunctorImageFilter.h"
#include "SnakeParameters.h"
#include "SpeedColorMap.h"

template<class TInputImage, class TOutputImage> class SignedDistanceFilter;
template<class TInputImage> class SNAPLevelSetFunction;
template<class TFilter> class LevelSetExtensionFilter;
class Fl_Gl_Window;

class vtkImageImport;
class vtkContourFilter;
class LevelSetPreviewPipeline2D;

namespace itk {
  template<class TInputImage, class TOutputImage> 
    class ParallelSparseFieldLevelSetImageFilter;
};

class LevelSetPreview2d;

// #define SNAKE_PREVIEW_ADVANCED 1

/** 
 * \class SnakeParametersPreviewPipeline
 * \brief A pipeline used to preview snake parameters.
 * 
 * Given a set of control points an image, and some snake parameters, this
 * class computes a b-spline curve based on those control points, creates a
 * level set embedding of the curve, and computes various level set evolution
 * forces acting on the curve.
 */
class SnakeParametersPreviewPipeline
{
public:
  SnakeParametersPreviewPipeline(GlobalState *state);
  virtual ~SnakeParametersPreviewPipeline();

  // Images used by this class (internally and externally)
  typedef itk::OrientedImage<unsigned char, 2> CharImageType;
  typedef itk::OrientedImage<float, 2> FloatImageType;

  // Define a color image for display
  typedef itk::RGBAPixel<unsigned char> DisplayPixelType;
  typedef itk::OrientedImage<DisplayPixelType,2> DisplayImageType;

  // Index type used to refer to pixels
  typedef FloatImageType::IndexType IndexType;

  // Force types
  enum ForceType {CURVATURE=0, ADVECTION, PROPAGATION, TOTAL };
  
  // A sample from the curve
  struct SampledPoint {
    // The geometry of the point
    double t;
    Vector2d x;
    Vector2d n;
    double kappa;
    
    // The forces acting on the point
    double PropagationForce;
    double CurvatureForce;
    double AdvectionForce;
    
    double operator[](unsigned int i) const
      { return x[i]; } 
  };

  // Various list types
  typedef std::vector<Vector2d> ControlPointList;
  typedef std::vector<Vector2d> LevelSetContourType;
  typedef std::vector<SampledPoint> SampledPointList;  

  /** Set the speed image */
  void SetSpeedImage(FloatImageType *image);

  /** Set the snake parameters */
  void SetSnakeParameters(const SnakeParameters &parameters);

  /** Set the control points of the interface curve */
  void SetControlPoints(const ControlPointList &points);

  /** Change just one control point, with an option of changing it
   * quickly, ie, not recomputing the level set, only the curve */
  void ChangeControlPoint(unsigned int index, const Vector2d &point,
    bool quickUpdate);

  /** Set the number of points sampled for display of the curve */
  void SetNumberOfSampledPoints(unsigned int number);
  
  /** Update the internals of the pipeline and compute the curve and the
   * force points.  This method requires a GL context because it relies on
   * GL tesselation code for generating an image from the curve */
  void Update(Fl_Gl_Window *context);

  /** Get the speed image */
  irisGetMacro(SpeedImage,FloatImageType *);
  
  /** Get a reference to the control points of the interface curve */
  irisGetMacro(ControlPoints,const ControlPointList &);

  /** Get a list of densely interpolated points on the curve (for drawing) */
  irisGetMacro(SampledPoints,const SampledPointList &);

  /** Is the demo loop running ? */
  irisIsMacro(DemoLoopRunning);
  irisSetMacro(DemoLoopRunning, bool);

  /** Get the demo loop contour */
  std::vector<Vector2d> &GetDemoLoopContour();

  /** Get the color image corresponding to the speed image */
  DisplayImageType *GetDisplayImage()  
    { return m_DisplayMapper->GetOutput(); }

  /** Set the idle callback function that FLTK should call in demo mode */
  void AnimationCallback();

private:

  /** The global state */
  GlobalState *m_GlobalState;
      
  /** The speed image */
  itk::SmartPointer<FloatImageType> m_SpeedImage;

  // Gradient image used by this component
  typedef itk::CovariantVector<float,2> VectorType;
  typedef itk::Image<VectorType,2> VectorImageType;
  typedef itk::SmartPointer<VectorImageType> VectorImagePointer;
  
  /** The grandient of the speed image */
  VectorImagePointer m_GradientImage;
  
  /** A set of snake parameters */
  SnakeParameters m_Parameters;
    
  /** A list of control points */
  ControlPointList m_ControlPoints;
  
  /** Number of points to sample from the curve */
  unsigned int m_NumberOfSampledPoints;

  /** A list of sampled points */
  SampledPointList m_SampledPoints;

  // Flags indicating which part of the pipeline should be refreshed
  bool m_ControlsModified;
  bool m_SpeedModified;
  bool m_ParametersModified;
  bool m_QuickUpdate;
    
  // Internal components of the Update method
  void UpdateLevelSetFunction();
  void UpdateContour();
  void UpdateLevelSet(Fl_Gl_Window *context);
  void UpdateForces();
  void Update();

  // A filter used to convert the speed image to a color image to display on the screen
  typedef itk::UnaryFunctorImageFilter<
    FloatImageType,DisplayImageType,SpeedColorMap> IntensityFilterType;
  typedef itk::SmartPointer<IntensityFilterType> IntensityFilterPointer;
  IntensityFilterPointer m_DisplayMapper;

  // Demo loop object
  LevelSetPreview2d *m_DemoLoop;

  // Whether the demo loop is currently running
  bool m_DemoLoopRunning;

  // The color map preset currently in use for speed image rendering
  ColorMapPreset m_ColorMapPreset;
};


#endif // __SnakeParametersPreviewPipeline_h_
