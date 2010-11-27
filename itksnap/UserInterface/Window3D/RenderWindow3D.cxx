/*=========================================================================

   Copyright 2010, Dr. Sandra Black
   Linda center. Campbell Cognitive Neurology Unit
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
#include "vtkFlRenderWindowInteractor.h"
#include "RenderWindow3D.h"
#include "vtkFlRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "itkImageToVTKImageFilter.h"
#include "vtkFixedPointVolumeRayCastMapper.h"
#include "vtkPiecewiseFunction.h"
#include "vtkColorTransferFunction.h"
#include "vtkVolumeProperty.h"
#include "vtkVolume.h"
#include "vtkCamera.h"
#include "itkRescaleIntensityImageFilter.h"
#include "vtkInteractorStyleTrackballCamera.h"
#include "Sabre3DTrackballMode.h"
#include "Sabre3DInteractionMode.h"

#include "vtkConeSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "itkImageFileReader.h"
#include <iostream>
#include "vtkLineSource.h"

#include "itkImageRegionIterator.h"

#include <FL/fl_message.H>

RenderWindow3D::RenderWindow3D( vtkFlRenderWindowInteractor* vtkflWindow )
{
  m_VTKflWindow = vtkflWindow;
  
  m_Ren = vtkSmartPointer< vtkRenderer >::New();
  m_RenWin = vtkSmartPointer< vtkRenderWindow >::New();

  m_RenWin->AddRenderer( m_Ren ); 

  m_Picker = vtkSmartPointer< vtkVolumePicker >::New();
  m_Picker->PickFromListOn();
  
  m_InteractorTrackball = vtkSmartPointer< Sabre3DTrackballMode >::New();
  m_InteractorSabre3D   = vtkSmartPointer< Sabre3DInteractionMode >::New();
  m_VolumeMapper = vtkSmartPointer< vtkFixedPointVolumeRayCastMapper >::New();
  m_VolumeColor = vtkSmartPointer< vtkColorTransferFunction >::New();
  m_VolumeScalarOpacity = vtkSmartPointer< vtkPiecewiseFunction >::New();
  m_VolumeGradientOpacity = vtkSmartPointer< vtkPiecewiseFunction >::New();
  m_VolumeProperty = vtkSmartPointer< vtkVolumeProperty >::New();
  m_Volume = vtkSmartPointer< vtkVolume >::New();

 
  m_VTKflWindow->SetRenderWindow( m_RenWin );
 
  SetInteractionMode( TRACKBALL );

  m_VTKflWindow->Initialize();

  m_VTKflWindow->SetPicker( m_Picker );

  m_Picker->AddPickList( m_Volume );

  m_CurrentDollyDistance = -1;
  m_InitialDollyDistance = -1;

  m_DefaultTransferFunctionParameters.x1 = 0.0;
  m_DefaultTransferFunctionParameters.x2 = 0.0;
  m_DefaultTransferFunctionParameters.x3 = 0.0;
  m_DefaultTransferFunctionParameters.y1 = 40;
  m_DefaultTransferFunctionParameters.y2 = 0.5;
  m_DefaultTransferFunctionParameters.y3 = 0.25;
  m_DefaultTransferFunctionParameters.z1 = 110;
  m_DefaultTransferFunctionParameters.z2 = 1.0;
  m_DefaultTransferFunctionParameters.z3 = 0.1;

}


RenderWindow3D::~RenderWindow3D()
{
}

void RenderWindow3D::SetInteractionMode( InteractionMode mode )
{
  if ( mode == SABRE3D )
  {
    m_VTKflWindow->SetInteractorStyle( m_InteractorSabre3D);
    m_CurrentInteractionMode = SABRE3D;
  }
  else 
  {
    m_VTKflWindow->SetInteractorStyle( m_InteractorTrackball );
    m_CurrentInteractionMode = TRACKBALL;
  }

}

void RenderWindow3D::ToggleInteractionMode()
{
  if ( m_CurrentInteractionMode == SABRE3D )
  {
    SetInteractionMode( TRACKBALL );
  }
  else
  {
    SetInteractionMode( SABRE3D ) ;
  }
}

void RenderWindow3D::SetImage( ImageType::Pointer imgGrey )
{
 m_ImgGrey = imgGrey;
 
 // Find largest dimension.
 ImageType::SpacingType spacing = m_ImgGrey->GetSpacing();
 m_Spacing(0) = spacing[0]; m_Spacing(1) = spacing[1]; m_Spacing(2) = spacing[2];
 
 ImageType::SizeType size = m_ImgGrey->GetLargestPossibleRegion().GetSize();
 m_Size(0) = size[0]; m_Size(1) = size[1]; m_Size(2) = size[2];
 
 unsigned int numDimensions = m_ImgGrey->GetImageDimension();
 
 int index = 0;
 for ( int i = 1; i < numDimensions; ++i )
 {
   if ( spacing[i] > spacing[index] )
     index = i;
 }

 // Set a reasonable dolly distance.
 m_CurrentDollyDistance = spacing[index] * size[index] * 1.9;
 m_InitialDollyDistance = m_CurrentDollyDistance;

 // Connect ITK and VTK.
 typedef itk::ImageToVTKImageFilter< ImageType > ConnectorType;
 ConnectorType::Pointer connector = ConnectorType::New();
 m_Connector = connector;
 connector->SetInput( m_ImgGrey );
 connector->Update();

 // Set up volume mapping.
 vtkSmartPointer< vtkFixedPointVolumeRayCastMapper > volumeMapper = vtkSmartPointer< vtkFixedPointVolumeRayCastMapper >::New();;
 m_VolumeMapper = volumeMapper;
 m_VolumeMapper->SetInput( connector->GetOutput() );

 //AdjustTransferFunction( 0.0, 0.0, 0.0, 40, 0.5, 0.25, 110, 1.0, 0.1 );
 AdjustTransferFunction();

 m_VolumeProperty->SetColor( m_VolumeColor );
 m_VolumeProperty->SetScalarOpacity( m_VolumeScalarOpacity );
 m_VolumeProperty->SetInterpolationTypeToLinear();
 m_VolumeProperty->ShadeOn();
 m_VolumeProperty->SetAmbient( 0.4 );
 m_VolumeProperty->SetDiffuse( 0.6 );
 m_VolumeProperty->SetSpecular( 0.2 );

 vtkSmartPointer< vtkVolume > volume = vtkSmartPointer< vtkVolume >::New();
 m_Volume = volume;
 m_Volume->SetMapper( m_VolumeMapper );
 m_Volume->SetProperty( m_VolumeProperty );

 m_Ren->RemoveAllViewProps();
 m_Ren->AddViewProp( m_Volume ); 
 m_Ren->SetBackground( 0.0, 0.0, 0.0 );

 // Initialize SABRE interactor.
 m_InteractorSabre3D->Initialize( m_VTKflWindow, m_Ren, m_Picker, m_ImgGrey );
 m_Picker->AddPickList( m_Volume );

 // Update view mode.
 m_CurrentViewMode = UNDEFINED;
}

RenderWindow3D::TransferFunctionParameters RenderWindow3D::GuessAtTransformFunction( ImageType::Pointer imgGrey )
{
  
  typedef itk::ImageRegionIterator< ImageType > IteratorType;
  IteratorType it( imgGrey, imgGrey->GetLargestPossibleRegion() );

  it.GoToBegin();
  ImageType::PixelType max = it.Get();
  ++it;
  for ( ; !it.IsAtEnd(); ++it )
  {
    if ( it.Get() > max )
      max = it.Get();
  }

  std::vector< unsigned int > histogram;
  histogram.resize( max + 1 );

  for ( it.GoToBegin(); !it.IsAtEnd(); ++it )
  {
    if ( it.Get() == 0 )
      continue;
    else
      ++histogram[ it.Get() ];
  }
  

  unsigned int effectiveMax;
  for ( int i = max; i >= 0; i-- )
  {
    if ( histogram[i] >= 20 )
    {
      effectiveMax = i;
      break;
    }
  }

  double point1 = effectiveMax/3;
  double point2 = effectiveMax/3*2;

  AdjustTransferFunction( 0.0, 0.0, 0.0, point1, 0.5, 0.25, point2, 1.0, 0.1 );
  m_VTKflWindow->redraw();

  TransferFunctionParameters parameters;
  parameters.x1 = 0.0;
  parameters.x2 = 0.0;
  parameters.x3 = 0.0;
  parameters.y1 = point1;
  parameters.y2 = 0.25;
  parameters.y3 = 0.25;
  parameters.z1 = point2;
  parameters.z2 = 1.0;
  parameters.z3 = 0.1;

  return parameters;
}


void RenderWindow3D::AdjustTransferFunction()
{
 if ( m_VolumeColor == NULL || m_VolumeScalarOpacity == NULL ) 
    return;

  m_VolumeColor->RemoveAllPoints();
  float x2, y2, z2;
  x2 =  m_DefaultTransferFunctionParameters.x2;
  y2 =  m_DefaultTransferFunctionParameters.y2;
  z2 =  m_DefaultTransferFunctionParameters.z2;
  m_VolumeColor->AddRGBPoint(  m_DefaultTransferFunctionParameters.x1, x2, x2, x2 );
  m_VolumeColor->AddRGBPoint(  m_DefaultTransferFunctionParameters.y1, y2, y2, y2 );
  m_VolumeColor->AddRGBPoint(  m_DefaultTransferFunctionParameters.z1, z2, z2, z2 );
  
  m_VolumeScalarOpacity->RemoveAllPoints();
  m_VolumeScalarOpacity->AddPoint( m_DefaultTransferFunctionParameters.x1, m_DefaultTransferFunctionParameters.x3 );
  m_VolumeScalarOpacity->AddPoint( m_DefaultTransferFunctionParameters.y1, m_DefaultTransferFunctionParameters.y3 );
  m_VolumeScalarOpacity->AddPoint( m_DefaultTransferFunctionParameters.z1, m_DefaultTransferFunctionParameters.z3 );

}

void RenderWindow3D::AdjustTransferFunction( double x1, double y1, double z1,
                                             double x2, double y2, double z2,
                                             double x3, double y3, double z3
                                             )
{
  if ( m_VolumeColor == NULL || m_VolumeScalarOpacity == NULL ) 
    return;

  m_VolumeColor->RemoveAllPoints();
  m_VolumeColor->AddRGBPoint( x1, y1, y1, y1 );
  m_VolumeColor->AddRGBPoint( x2, y2, y2, y2 );
  m_VolumeColor->AddRGBPoint( x3, y3, y3, y3 );
  
  m_VolumeScalarOpacity->RemoveAllPoints();
  m_VolumeScalarOpacity->AddPoint( x1, z1 );
  m_VolumeScalarOpacity->AddPoint( x2, z2 );
  m_VolumeScalarOpacity->AddPoint( x3, z3 );

}

void RenderWindow3D::ResetClippingRange()
{
  m_Ren->ResetCameraClippingRange(0, (m_Size(0)-1)*m_Spacing(0), 0, (m_Size(1)-1)*m_Spacing(1), 0, (m_Size(2)-1)*m_Spacing(2));
}

void RenderWindow3D::RenderLeft()
{ 
  vtkCamera* camera = m_Ren->GetActiveCamera();
  double* center = m_Volume->GetCenter();
  camera->SetFocalPoint( center[0], center[1], center[2] );
  camera->SetPosition( center[0] + m_CurrentDollyDistance, center[1], center[2] );
  camera->SetViewUp( 0, 0, 1 );
  camera->UpdateViewport( m_Ren );

  ResetClippingRange();

  m_VTKflWindow->redraw();

  m_CurrentViewMode = LEFT; 
}

void RenderWindow3D::RenderRight()
{

  vtkCamera* camera = m_Ren->GetActiveCamera();
  double* center = m_Volume->GetCenter();
  camera->SetFocalPoint( center[0], center[1], center[2] );
  camera->SetPosition( center[0] - m_CurrentDollyDistance, center[1], center[2] );
  camera->SetViewUp( 0, 0, 1 );
  camera->UpdateViewport( m_Ren );

  ResetClippingRange();  
  m_VTKflWindow->redraw();
  m_CurrentViewMode = RIGHT;
}

void RenderWindow3D::RenderTop()
{
  vtkCamera* camera = m_Ren->GetActiveCamera();
  double* center = m_Volume->GetCenter();
  camera->SetFocalPoint( center[0], center[1], center[2] );
  camera->SetPosition( center[0], center[1], center[2] + m_CurrentDollyDistance );
  camera->SetViewUp( 0, 1, 1 );
  camera->UpdateViewport( m_Ren );

  ResetClippingRange();
  m_VTKflWindow->redraw();
  m_CurrentViewMode = TOP;
}

void RenderWindow3D::RenderBottom()
{
  vtkCamera* camera = m_Ren->GetActiveCamera();
  double* center = m_Volume->GetCenter();
  camera->SetFocalPoint( center[0], center[1], center[2] );
  camera->SetPosition( center[0], center[1], center[2] - m_CurrentDollyDistance );
  camera->SetViewUp( 0, 0, 0 );
  camera->UpdateViewport( m_Ren );

  ResetClippingRange();
  m_VTKflWindow->redraw();
  m_CurrentViewMode = BOTTOM;
}

void RenderWindow3D::RenderFront()
{
  vtkCamera* camera = m_Ren->GetActiveCamera();
  double* center = m_Volume->GetCenter();
  camera->SetFocalPoint( center[0], center[1], center[2] );
  camera->SetPosition( center[0], center[1] + m_CurrentDollyDistance, center[2] );
  camera->SetViewUp( 0, 0, 1 );
  camera->UpdateViewport( m_Ren );

  ResetClippingRange();
  m_VTKflWindow->redraw();
  m_CurrentViewMode = FRONT;
}

void RenderWindow3D::RenderBack()
{
  vtkCamera* camera = m_Ren->GetActiveCamera();
  double* center = m_Volume->GetCenter();
  camera->SetFocalPoint( center[0], center[1], center[2] );
  camera->SetPosition( center[0], center[1] - m_CurrentDollyDistance, center[2] );
  camera->SetViewUp( 0, 0, 1 );
  camera->UpdateViewport( m_Ren );

  ResetClippingRange();
  m_VTKflWindow->redraw();
  m_CurrentViewMode = BACK;
}

void 
RenderWindow3D::ResetCamera()
{
  switch ( m_CurrentViewMode )
  {
  case RIGHT:
    RenderRight();
    break;
  case LEFT:
    RenderLeft();
    break;
  case TOP:
    RenderTop();
    break;
  case BOTTOM:
    RenderBottom();
    break;
  case FRONT:
    RenderFront();
    break;
  case BACK:
    RenderBack();
    break;
  case UNDEFINED:
    break;
  }
  m_VTKflWindow->redraw();
}

void 
RenderWindow3D::ResetWindow()
{ 
  m_Ren->RemoveAllViewProps(); 
  m_VTKflWindow->redraw();
  
  m_Picker = NULL;
  m_Picker = vtkSmartPointer< vtkVolumePicker >::New();

  m_InteractorSabre3D->Purge();

  m_InteractorSabre3D->SetLandmarkMode( LSF );

  // (Note: these objects will be created during SetImage call)
  m_Volume = NULL;
  m_ImgGrey = NULL;
  m_Connector = NULL;
  m_VolumeMapper = NULL;
  
  //m_VTKflWindow->redraw();
}

void
RenderWindow3D::FixCamera()
{
  vtkCamera* camera = m_Ren->GetActiveCamera();
  double* center = camera->GetFocalPoint();
  double* position = camera->GetPosition();

  if ( m_CurrentViewMode == LEFT )
  {
    // Check that only dolly operations have been performed
    if ( position[1] != center[1] || position[2] != center[2] )
    {
      FixCameraErrorMessage();
      return;
    }
    // Get the current dolly distance
    m_CurrentDollyDistance = position[0] - center[0];
    
    // Render
    RenderLeft();
  }
  else if ( m_CurrentViewMode == RIGHT )
  {
    if ( position[1] != center[1] || position[2] != center[2] )
    {
      FixCameraErrorMessage();
      return;
    }
    m_CurrentDollyDistance = center[0] - position[0];
    RenderRight();
  }
  else if ( m_CurrentViewMode == TOP )
  {
    if ( position[0] != center[0] || position[1] != center[1] )
    {
      FixCameraErrorMessage();
      return;
    }
    m_CurrentDollyDistance = position[2] - center[2];
    RenderTop();
  }
  else if ( m_CurrentViewMode == BOTTOM )
  {
    if ( position[0] != center[0] || position[1] != center[1] )
    {
      FixCameraErrorMessage();
      return;
    }
    m_CurrentDollyDistance = center[2] - position[2];
    RenderBottom();
  }
  else if ( m_CurrentViewMode == FRONT )
  {
    if ( position[0] != center[0] || position[2] != center[2] )
    {
      FixCameraErrorMessage();
      return;
    }
    m_CurrentDollyDistance = position[1] - center[1]; 
    RenderFront();
  }
  else if ( m_CurrentViewMode == BACK )
  {
    if ( position[0] != center[0] || position[2] != center[2] )
    {
      FixCameraErrorMessage();
      return;
    }
    m_CurrentDollyDistance = center[1] - position[1];
    RenderBack();
  }
}

void RenderWindow3D::FixCameraErrorMessage()
{
  fl_alert("Error:  Do not fix camera after rotating the volume.\nPress one of the directional buttons first and then \nzoom using the right mouse button.");
}

