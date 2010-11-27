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


=========================================================================*/
#ifndef __RenderWindow3D_h_
#define __RenderWindow3D_h_

#include "GreyImageWrapper.h"
#include "vtkSmartPointer.h"
#include "GlobalState.h"
#include "Sabre3DInteractionMode.h"
#include "Sabre3DTrackballMode.h"
#include <vnl/vnl_vector_fixed.h>
#include "itkImageToVTKImageFilter.h"

class vtkFlRenderWindowInteractor;
class vtkRenderWindow;
class vtkRenderer;
class vtkRenderWindowInteractor;
class vtkPolyDataMapper;
class vtkFixedPointVolumeRayCastMapper;
class vtkColorTransferFunction;
class vtkPiecewiseFunction;
class vtkVolumeProperty;
class vtkVolume;
class vtkInteractorStyleTrackballCamera;
class vtkVolumePicker;

class RenderWindow3D
{
public:
  struct TransferFunctionParameters 
  {
    float x1;
    float x2;
    float x3;
    float y1;
    float y2;
    float y3;
    float z1;
    float z2;
    float z3;
  };


private:
  typedef GreyImageWrapper::ImageType ImageType;

  typedef vnl_vector_fixed< double, 3 > Vector3d; 
  typedef itk::ImageToVTKImageFilter< ImageType > ConnectorType;

  enum InteractionMode { SABRE3D, TRACKBALL };
  enum ViewMode{ RIGHT, LEFT, TOP, BOTTOM, FRONT, BACK, UNDEFINED };

public:
  RenderWindow3D( vtkFlRenderWindowInteractor* vtkflWindow );
  ~RenderWindow3D();
  
  void SetImage( ImageType::Pointer imgGrey );
  
  void RenderRight();
  void RenderLeft();
  void RenderTop();
  void RenderBottom();
  void RenderFront();
  void RenderBack();
  
  void AdjustTransferFunction( double x1, double y1, double z1,
                               double x2, double y2, double z2,
                               double x3, double y3, double z3
                               );
  void AdjustTransferFunction();

  TransferFunctionParameters GuessAtTransformFunction( ImageType::Pointer imgGrey );
  TransferFunctionParameters GetDefaultTransformFunction() { return  m_DefaultTransferFunctionParameters; }
  
  void SetInteractionMode(InteractionMode mode );
  void ToggleInteractionMode();
  
  Sabre3DInteractionMode* GetInteractor() { return m_InteractorSabre3D; }

  void ResetClippingRange();
  
  void FixCamera();
  void FixCameraErrorMessage();
  void ResetCamera();
  void ResetWindow();


private:
  InteractionMode m_CurrentInteractionMode;
  vtkFlRenderWindowInteractor*                         m_VTKflWindow;
  vtkSmartPointer< vtkRenderWindow >                   m_RenWin;
  vtkSmartPointer< vtkRenderer >                       m_Ren;
  vtkSmartPointer< vtkVolumePicker >                   m_Picker;
  vtkSmartPointer< Sabre3DTrackballMode >              m_InteractorTrackball;
  vtkSmartPointer< Sabre3DInteractionMode >            m_InteractorSabre3D;
  ImageType::Pointer                                   m_ImgGrey;
  vtkSmartPointer< vtkFixedPointVolumeRayCastMapper >  m_VolumeMapper;
  vtkSmartPointer< vtkColorTransferFunction >          m_VolumeColor;
  vtkSmartPointer< vtkPiecewiseFunction >              m_VolumeScalarOpacity;
  vtkSmartPointer< vtkPiecewiseFunction >              m_VolumeGradientOpacity;
  vtkSmartPointer< vtkVolumeProperty >                 m_VolumeProperty;
  vtkSmartPointer< vtkVolume >                         m_Volume;
  ConnectorType::Pointer                               m_Connector;
  
  double    m_InitialDollyDistance;
  double    m_CurrentDollyDistance;
  ViewMode  m_CurrentViewMode;
  Vector3d  m_Spacing;
  Vector3d  m_Size;

  TransferFunctionParameters m_DefaultTransferFunctionParameters;

};

#endif