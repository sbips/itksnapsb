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

#include "Sabre3DInteractionMode.h"
#include "vtkInteractorStyleUser.h"
#include "GlobalState.h"
#include "LabelImageWrapper.h"
#include "GreyImageWrapper.h"
#include "vtkLineSource.h"
#include "vtkSphereSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkActor.h"
#include "vtkProperty.h"
#include "vtkActor.h"
#include "vtkMath.h"
#include "vtkCommand.h"
#include "vtkSmartPointer.h"
#include <list>
#include <itkImageRegionIteratorWithIndex.h>
#include <map>

vtkCxxRevisionMacro(Sabre3DInteractionMode, "$Revision$");

Sabre3DInteractionMode::Sabre3DInteractionMode()
{
  m_ImgGrey = NULL;
  m_CurrentLandmarkMode = LSF;
  m_NumberOfLandmarks = 0;
  m_Landmarks.resize(4);
  m_LandmarkMap[RC] = m_NumberOfLandmarks++;
  m_LandmarkMap[LC] = m_NumberOfLandmarks++;
  m_LandmarkMap[RSF] = m_NumberOfLandmarks++;
  m_LandmarkMap[LSF] = m_NumberOfLandmarks++;
  m_Landmarks[ m_LandmarkMap[LC] ] = Sabre3DLandmark( LC, LC + 10, true, Vector3d(  1.0, 0.078, 0.576 ) );
  m_Landmarks[ m_LandmarkMap[LSF] ] = Sabre3DLandmark( LSF, LSF + 10, false, Vector3d(  0.627, 0.169, 0.941  ) );
  m_Landmarks[ m_LandmarkMap[RSF] ] = Sabre3DLandmark( RSF, RSF + 10, false, Vector3d(  0.0, 1.0, 1.0 ) );
  m_Landmarks[ m_LandmarkMap[RC] ] = Sabre3DLandmark( RC, RC + 10, true, Vector3d(  0.0, 1.0, 0.0 ) );
}

Sabre3DInteractionMode::~Sabre3DInteractionMode()
{
}

void
Sabre3DInteractionMode::Initialize( vtkFlRenderWindowInteractor* renwin, 
                                    vtkRenderer* ren, 
                                    vtkVolumePicker* picker, 
                                    GreyImageWrapper::ImagePointer grey )
{
  m_Ren = ren;
  m_RenWin = renwin;
  m_Picker = picker;
  m_ImgGrey = grey;
  m_IsInitialized = true;
}

Sabre3DInteractionMode::Vector3d 
Sabre3DInteractionMode::GetPickPosition()
{
  m_Picker->PickClippingPlanesOff();
  m_Picker->PickCroppingPlanesOff();
  m_Picker->SetTolerance( 0 );
  
  // Convert X,Y window cooridinates to X,Y,Z image volume cooridinates.
  int x,y;
  double pos[3];

  m_RenWin->GetLastEventPosition(x,y);
  m_Picker->Pick( x, y, 0.0, m_Ren );
  m_Picker->GetPickPosition( pos );

  pos[0] = vtkMath::Round( pos[0] );
  pos[1] = vtkMath::Round( pos[1] );
  pos[2] = vtkMath::Round( pos[2] );

  return Vector3d( pos[0], pos[1], pos[2] );
}


void 
Sabre3DInteractionMode::OnLeftButtonDown()
{
 if ( !m_IsInitialized )
    return;
 
  // Get pick position.
  Vector3d pos = GetPickPosition();

  // If necessary, make sure landmark does not turn back on itself (applicable to central sulcus).
  int index = m_LandmarkMap[ m_CurrentLandmarkMode ];
  if ( m_Landmarks[ index ].m_LineRestriction == true )
    if ( m_Landmarks[ index ].m_LastNode != NULL )
      if ( pos[1] > m_Landmarks[ index ].m_LastNode->GetPosition()[1] )
        pos[1] = m_Landmarks[ index ].m_LastNode->GetPosition()[1];

  // Add point.
  AddPoint( pos );

  // Redraw.
  m_RenWin->redraw();
}

void
Sabre3DInteractionMode::AddPoint( Vector3d pos )
{
  if ( !m_IsInitialized )
    return;
  // Create visualization objects.

  // Node:
  vtkSmartPointer< vtkSphereSource > sphere = vtkSmartPointer< vtkSphereSource >::New();
  sphere->SetCenter( 0.0, 0.0, 0.0 );
  sphere->SetRadius( 1.5 );
 
  vtkSmartPointer< vtkPolyDataMapper > sphereMapper = vtkSmartPointer< vtkPolyDataMapper >::New();
  sphereMapper->SetInput( sphere->GetOutput() );

  vtkSmartPointer< vtkActor > sphereActor = vtkSmartPointer< vtkActor >::New();
  sphereActor->SetMapper( sphereMapper );

  sphereActor->GetProperty()->SetInterpolationToFlat();
  sphereActor->GetProperty()->SetAmbient( 1.0 );
  sphereActor->GetProperty()->SetSpecular( 0.0 );
  sphereActor->GetProperty()->SetDiffuse( 0.0 );
  sphereActor->SetPosition( pos[0], pos[1], pos[2] );
    
  m_Ren->AddActor( sphereActor );

  // Line:
  vtkSmartPointer< vtkLineSource > line = vtkSmartPointer< vtkLineSource >::New();
  line->SetPoint1( pos[0], pos[1], pos[2] );

  int index = m_LandmarkMap[ m_CurrentLandmarkMode ];
  if ( m_Landmarks[ index ].m_LastNode != NULL )
  {
    line->SetPoint2( m_Landmarks[ index ].m_LastNode->GetPosition()[0],
                     m_Landmarks[ index ].m_LastNode->GetPosition()[1],
                     m_Landmarks[ index ].m_LastNode->GetPosition()[2] );
  }
   
  vtkSmartPointer< vtkPolyDataMapper > lineMapper = vtkSmartPointer< vtkPolyDataMapper >::New();
  lineMapper->SetInput( line->GetOutput() );
 
  vtkSmartPointer< vtkActor > lineActor = vtkSmartPointer< vtkActor >::New();
  lineActor->SetMapper( lineMapper );
  lineActor->GetProperty()->SetLineWidth( 2.0 );

  // Customize line and node for current landmark mode.

  int id = m_Landmarks[ index ].GetNextID();
  sphereActor->GetProperty()->SetUserField1( m_Landmarks[ index ].m_LandmarkCodeNode );
  sphereActor->GetProperty()->SetUserField2( id );
  sphereActor->GetProperty()->SetColor( m_Landmarks[index].m_Color(0), 
                                        m_Landmarks[index].m_Color(1), 
                                        m_Landmarks[index].m_Color(2) );

  if ( m_Landmarks[ index ].m_LastNode != NULL )
  {
    lineActor->GetProperty()->SetColor( sphereActor->GetProperty()->GetColor() );
    lineActor->GetProperty()->SetUserField1( m_Landmarks[ index ].m_LandmarkCodeLine );
    lineActor->GetProperty()->SetUserField2( id );
    m_Ren->AddActor( lineActor );
  }

  m_Landmarks[ index ].m_LastNode = sphereActor;
    
}

void
Sabre3DInteractionMode::Purge()
{
  m_Landmarks[ m_LandmarkMap[RSF] ].Purge();
  m_Landmarks[ m_LandmarkMap[LSF] ].Purge();
  m_Landmarks[ m_LandmarkMap[RC] ].Purge();
  m_Landmarks[ m_LandmarkMap[LC] ].Purge();
  m_IsInitialized = false;
}

void 
Sabre3DInteractionMode::AddAcceptedPoint( Vector3d pos, LandmarkType landmark )
{
  if ( !m_IsInitialized )
    return;
  
  // Point.
  vtkSmartPointer< vtkSphereSource > sphere = vtkSmartPointer< vtkSphereSource >::New();
  sphere->SetRadius( 0.75 );

  vtkSmartPointer< vtkPolyDataMapper > sphereMapper = vtkSmartPointer< vtkPolyDataMapper >::New();
  sphereMapper->SetInput( sphere->GetOutput() );

  vtkSmartPointer< vtkActor > sphereActor = vtkSmartPointer< vtkActor >::New();
  sphereActor->SetMapper( sphereMapper );

  sphereActor->GetProperty()->SetInterpolationToFlat();
  sphereActor->GetProperty()->SetAmbient( 1.0 );
  sphereActor->GetProperty()->SetSpecular( 0.0 );
  sphereActor->GetProperty()->SetDiffuse( 0.0 );
  sphereActor->SetPosition( pos[0], pos[1], pos[2] );

  // Customize point for landmark.
  int index = m_LandmarkMap[ landmark ];
  sphereActor->GetProperty()->SetColor( m_Landmarks[ index ].m_Color(0), m_Landmarks[ index ].m_Color(1), m_Landmarks[ index ].m_Color(2) );
  sphereActor->GetProperty()->SetUserField1( m_Landmarks[ index ].m_LandmarkCodeNode + 20 );
  sphereActor->GetProperty()->SetUserField2( 0 );
      
  m_Ren->AddActor( sphereActor );

  m_RenWin->redraw();
}

void 
Sabre3DInteractionMode::SetLandmarkMode( LandmarkType mode )
{ 
  m_CurrentLandmarkMode = mode;
};

void 
Sabre3DInteractionMode::Disconnect()
{
  m_Landmarks[ m_LandmarkMap[ RSF ] ].m_LastNode = NULL;
  m_Landmarks[ m_LandmarkMap[ RSF ] ].m_LastAcceptedNode = NULL;
  m_Landmarks[ m_LandmarkMap[ LSF ] ].m_LastNode = NULL;
  m_Landmarks[ m_LandmarkMap[ LSF ] ].m_LastAcceptedNode = NULL;
  m_Landmarks[ m_LandmarkMap[ LC ] ].m_LastNode = NULL;
  m_Landmarks[ m_LandmarkMap[ LC ] ].m_LastAcceptedNode = NULL;
  m_Landmarks[ m_LandmarkMap[ RC ] ].m_LastNode = NULL;
  m_Landmarks[ m_LandmarkMap[ RC ] ].m_LastAcceptedNode = NULL;
}

void
Sabre3DInteractionMode::Undo()
{
  if ( !m_IsInitialized )
    return;

  int index = m_LandmarkMap[ m_CurrentLandmarkMode ];
  int id = -1;
  id = m_Landmarks[ index ].m_ID;
  if ( id == -1 ) return;

  // Search for landmarks using id #.
  vtkActorCollection* collection = m_Ren->GetActors();
  vtkActor* actor;
  collection->InitTraversal();
  actor = collection->GetNextItem();
  bool found = false;
  
  while ( actor != 0 )
  {
    if ( id > 2 )
    {
      // Look for 2nd last landmark node and set as last node.
      if ( ( actor->GetProperty()->GetUserField2() == (id - 1) ) 
        && ( actor->GetProperty()->GetUserField1() ==  m_CurrentLandmarkMode ) )
      {
        m_Landmarks[ index ].m_LastNode = actor;
      }
    }
    else
    {
      // If 2nd last landmark does not exist, set last node to NULL.
      m_Landmarks[ index ].m_LastNode = NULL;
    }

   // Look for last landmark node and record position for redo operations.
   if ( actor->GetProperty()->GetUserField1() == m_CurrentLandmarkMode && actor->GetProperty()->GetUserField2() == id )
     m_Landmarks[ index ].m_Undo.push_back( Vector3d( actor->GetCenter()[0], actor->GetCenter()[1], actor->GetCenter()[2] ) ); 

    // Look for last landmark node and line and remove.
    if ( actor->GetProperty()->GetUserField1() == m_CurrentLandmarkMode || actor->GetProperty()->GetUserField1() == (m_CurrentLandmarkMode + 10 )) 
      if ( actor->GetProperty()->GetUserField2() == id  )
      {       
        m_Ren->RemoveActor( actor );
        found = true;
      }

    // Iterate.
    actor = collection->GetNextActor();
  }

  // If undo is successful, reduce landmark ID # by one.
  if ( found )
    --m_Landmarks[ index ].m_ID;

  // Redraw.
  m_RenWin->redraw(); 
}


void
Sabre3DInteractionMode::Redo()
{
  if ( !m_IsInitialized )
    return;

  int index = m_LandmarkMap[ m_CurrentLandmarkMode ];

  if ( m_Landmarks[ index ].m_Undo.empty() )
    return;

  Vector3d pos = m_Landmarks[ index ].m_Undo.back();
  m_Landmarks[ index ].m_Undo.pop_back();
  AddPoint( pos );
 
  m_RenWin->redraw();
}

void Sabre3DInteractionMode::SetVisibility( bool value )
{
  if ( !m_IsInitialized )
    return;

 for ( int i = 0; i < m_Landmarks.size(); ++ i )
  {

    LandmarkType landmark = m_Landmarks[i].m_LandmarkCodeNode;

    // Find node positions.
    vtkActorCollection* collection = m_Ren->GetActors();
    vtkActor* actor;
    collection->InitTraversal();
    actor = collection->GetNextItem();

    while ( actor != 0 )
    {
      if ( value == true )
        actor->VisibilityOn();
      else
        actor->VisibilityOff();
      
      actor = collection->GetNextItem();
    }
  }
    m_RenWin->redraw();
}

void 
Sabre3DInteractionMode::Hide()
{
  SetVisibility( false );
}

void 
Sabre3DInteractionMode::Unhide()
{
  SetVisibility( true );
}

void 
Sabre3DInteractionMode::Accept( LabelImageWrapper* seg )
{
  if ( !m_IsInitialized )
    return;

  double xSpacing = m_ImgGrey->GetSpacing()[0];
  double ySpacing = m_ImgGrey->GetSpacing()[1];
  double zSpacing = m_ImgGrey->GetSpacing()[2];
 
  for ( int i = 0; i < m_Landmarks.size(); ++ i )
  {

    LandmarkType landmark = m_Landmarks[i].m_LandmarkCodeNode;

    // Find node positions.
    vtkActorCollection* collection = m_Ren->GetActors();
    vtkActor* actor;
    collection->InitTraversal();
    actor = collection->GetNextItem();

    std::list< Vector3d > landmarkNodes;

    while ( actor != 0 )
    {
      if ( actor->GetProperty()->GetUserField1() == landmark )
        landmarkNodes.push_back( Vector3d( actor->GetCenter() ) );

      actor = collection->GetNextItem();
    }

    NodeList::iterator it;
    for ( it = landmarkNodes.begin(); it != landmarkNodes.end(); it++ )
    {
      Vector3d point1 = *it;
      Vector3d point2;

      if ( it == landmarkNodes.begin() )
        if ( m_Landmarks[ m_LandmarkMap[landmark] ].m_LastAcceptedNode != NULL )      
        {
          point1 = *landmarkNodes.begin();
          point2 = Vector3d( m_Landmarks[ m_LandmarkMap[landmark] ].m_LastAcceptedNode->GetCenter()[0],
                             m_Landmarks[ m_LandmarkMap[landmark] ].m_LastAcceptedNode->GetCenter()[1],
                             m_Landmarks[ m_LandmarkMap[landmark] ].m_LastAcceptedNode->GetCenter()[2] );
             WriteLine3D( point1, point2, seg, landmark );
        }

      ++it;
      if ( it == landmarkNodes.end() )
        break;
      point2 = *it;
      --it;     
      WriteLine3D( point1, point2, seg, landmark );

         
          
    }
 
    int index = m_LandmarkMap[ landmark ];
    for ( it = m_Landmarks[ index ].m_Accepted.begin(); it != m_Landmarks[ index ].m_Accepted.end(); ++it )
      AddAcceptedPoint( Vector3d( it->get(0) * xSpacing, it->get(1) * ySpacing, it->get(2) * zSpacing ), landmark );


  }
  
  Reset();

  seg->GetImage()->Modified();

  m_RenWin->redraw();

}

void
Sabre3DInteractionMode::WriteLine3D( Vector3d point1, Vector3d point2, LabelImageWrapper* seg, LandmarkType value )
{
  if ( !m_IsInitialized )
    return;

  // Convert point1 and point 2 from world to voxel coordinates.
  int x1, y1, z1, x2, y2, z2;
  double xSpacing = m_ImgGrey->GetSpacing()[0];
  double ySpacing = m_ImgGrey->GetSpacing()[1];
  double zSpacing = m_ImgGrey->GetSpacing()[2];
 
  x1 = point1(0) / xSpacing; 
  y1 = point1(1) / ySpacing; 
  z1 = point1(2) / zSpacing;
  x2 = point2(0) / xSpacing;
  y2 = point2(1) / ySpacing; 
  z2 = point2(2) / zSpacing;

  unsigned int xSize = seg->GetSize()[0];
  unsigned int ySize = seg->GetSize()[1];
  unsigned int zSize = seg->GetSize()[2];

  // Make sure point the point is in bounds.
  if ( ( point1(0) < 0 || point1(0) >= xSize )  ||
       ( point2(0) < 0 || point2(0) >= xSize )  || 
       ( point1(1) < 0 || point1(1) >= ySize )  ||
       ( point2(1) < 0 || point2(1) >= ySize )  ||
       ( point1(2) < 0 || point1(2) >= zSize )  ||
       ( point2(2) < 0 || point2(2) >= zSize )    )
    return;

  // Use bresenham's algorithm to calculate line indices
  int d = (point1 - point2).inf_norm() + 1;
 
  int dx = x2 - x1;
  int dy = y2 - y1;
  int dz = z2 - z1;

  int ax = std::abs( dx ) * 2;
  int ay = std::abs( dy ) * 2;
  int az = std::abs( dz ) * 2;

  int sx = 0; int sy = 0; int sz = 0;

  if ( dx > 0 ) 
    sx = 1;
  if ( dx < 0 )
    sx = -1;
  if ( dy > 0 )
    sy = 1;
  if ( dy < 0 )
    sy = -1;
  if ( dz > 0 )
    sz = 1;
  if ( dz < 0 )
    sz = -1;

  int x = x1; int y = y1; int z = z1;

  std::list< Vector3d > output;
  int xd, yd, zd;

  if ( ax >= std::max(ay,az) ) // start x dominant
  { 
    yd = ay - ax/2;
    zd = az - ax/2;

    while ( true )
    {
      output.push_back( Vector3d( x, y, z  ));

      if ( x == x2 )
        break;
      
      if ( yd >= 0 )
      {
        y = y + sy;
        yd = yd - ax;
      }

      if ( zd >= 0 )
      {
        z = z + sz;
        zd = zd - ax;
      }

      x = x + sx;
      yd = yd + ay;
      zd = zd + az;
    }
  }  // end x dominant

  if ( ay >= std::max(ax,az) ) // start y dominant
  {
    xd = ax - ay/2;
    zd = az - ay/2;

    while ( true ) 
    {
      output.push_back( Vector3d( x, y, z ) );

      if ( y == y2 )
        break;

      if ( xd >= 0 )
      {
        x = x + sx;
        xd = xd - ay;
      }

      if ( zd >= 0 )
      {
        z = z + sz;
        zd = zd - ay;
      }

      y = y + sy;
      xd = xd + ax;
      zd = zd + az;
    }
  } // end y dominant

  if ( az >= std::max(ax,ay ) ) // start z dominant
  {
    xd = ax - az/2;
    yd = ay - az/2;

    while ( true )
    {
      output.push_back( Vector3d( x, y, z ) );

      if ( z == z2 )
        break;

      if ( xd >= 0 )
      {
        x = x + sx;
        xd = xd - az;
      }

      if ( yd >= 0 )
      {
        y = y + sy;
        yd = yd - az;
      }

      z = z + sz;
      xd = xd + ax;
      yd = yd + ay;
    }

  } // end z dominant

  // Write line indices to segmentation image and store in Accepted list.
  std::list< Vector3d >::iterator it;
  for ( it = output.begin(); it != output.end(); ++it )
  {
    LabelImageWrapper::ImageType::IndexType pixel;
    pixel[0] = it->get(0);
    pixel[1] = it->get(1);
    pixel[2] = it->get(2);
    seg->GetImage()->SetPixel( pixel, value );
    int index = m_LandmarkMap[ value ];
    m_Landmarks[ index ].m_Accepted.push_back( *it );
  }
}


void
Sabre3DInteractionMode::Reset()
{
  if ( !m_IsInitialized )
    return;

  vtkActorCollection* collection = m_Ren->GetActors();
  vtkActor* actor;

  collection->InitTraversal();
  actor = collection->GetNextItem();
  while ( actor != 0 )
  {
    if ( actor->GetProperty()->GetUserField1() < 20 )
      m_Ren->RemoveActor( actor );
    actor = collection->GetNextItem();
  }
  
 
  for ( int i=0; i < m_Landmarks.size(); ++i )
  {
    m_Landmarks[ i ].m_Undo.resize( 0 );
    //m_Landmarks[ i ].m_LastNode = NULL;
  }
  
  m_Landmarks[ m_LandmarkMap[ LSF ] ].m_LastAcceptedNode = m_Landmarks[ m_LandmarkMap[ LSF ] ].m_LastNode; 
  m_Landmarks[ m_LandmarkMap[ RSF ] ].m_LastAcceptedNode = m_Landmarks[ m_LandmarkMap[ RSF ] ].m_LastNode; 
  m_Landmarks[ m_LandmarkMap[ LC ] ].m_LastAcceptedNode = m_Landmarks[ m_LandmarkMap[ LC ] ].m_LastNode; 
  m_Landmarks[ m_LandmarkMap[ RC ] ].m_LastAcceptedNode = m_Landmarks[ m_LandmarkMap[ RC ] ].m_LastNode; 


  m_RenWin->redraw();
}

void
Sabre3DInteractionMode::DeleteLandmark( LabelImageWrapper* seg, LandmarkType landmark )
{
  if ( !m_IsInitialized )
    return;

  int index = m_LandmarkMap[ landmark ];

  typedef itk::ImageRegionIterator< LabelImageWrapper::ImageType > IteratorType;
  IteratorType it( seg->GetImage(), seg->GetImage()->GetLargestPossibleRegion() );

  for ( it.GoToBegin(); !it.IsAtEnd(); ++it )
  {
    if ( it.Get() == landmark )
      it.Set( 0 );
  }

  seg->GetImage()->Modified();

  m_Landmarks[ index ].m_Accepted.clear();

  vtkActorCollection* collection = m_Ren->GetActors();
  vtkActor* actor;

  collection->InitTraversal();
  actor = collection->GetNextItem();
  while ( actor != 0 )
  {
    if ( actor->GetProperty()->GetUserField1() == ( landmark + 20) )
      m_Ren->RemoveActor( actor );
    actor = collection->GetNextItem();
  }
  m_RenWin->redraw();
}


void
Sabre3DInteractionMode::OnLandmarkLoad( LabelImageWrapper* seg, GreyImageWrapper* grey )
{
  if ( !m_IsInitialized )
    return;

  typedef itk::ImageRegionIteratorWithIndex< LabelImageWrapper::ImageType > IteratorType;
  IteratorType it( seg->GetImage(), seg->GetImage()->GetLargestPossibleRegion() );
  
  double xSpacing = grey->GetImageBase()->GetSpacing()[0];
  double ySpacing = grey->GetImageBase()->GetSpacing()[1];
  double zSpacing = grey->GetImageBase()->GetSpacing()[2];

  for (it.GoToBegin(); !it.IsAtEnd(); ++it )
  {
    if ( it.Get() == 0 )
      continue;

    if ( it.Get() <= 4 )
      continue;

    LabelImageWrapper::ImageType::IndexType pixel = it.GetIndex();
    AddAcceptedPoint( Vector3d( pixel[0] * xSpacing, pixel[1] * ySpacing, pixel[2] * zSpacing  ), static_cast<LandmarkType>(it.Get()) );
  }
  m_RenWin->redraw();
  seg->GetImage()->Modified();
}

