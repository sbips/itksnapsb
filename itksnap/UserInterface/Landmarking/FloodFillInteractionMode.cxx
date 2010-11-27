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
#include "SNAPCommon.h"
#include "UserInterfaceBase.h"
#include "GenericSliceWindow.h"
#include "IRISApplication.h"
#include "FloodFillInteractionMode.h"
#include "GenericImageData.h"
#include "SNAPAppearanceSettings.h"
#include "itkImageFileWriter.h"
#include "itkConnectedThresholdImageFilter.h"
#include "FloodFillLimitImage.h"
#include "FL/fl_ask.H"
#include "itkFloodFilledImageFunctionConditionalIterator.h"
#include "itkBinaryThresholdImageFunction.h"
#include "FloodFillBinaryThresholdImageFunction.h"

FloodFillInteractionMode
::FloodFillInteractionMode( GenericSliceWindow* parent )
: PaintbrushInteractionMode( parent )
{
  m_ConnectToLastLimitVoxel = 0;
  m_HideLimits = false;
  m_LimitMode = true;
  m_FloodFillLimits = false;
  m_LastLimitVoxel = Vector3f(0,0,0);
  m_Fill3D = false;
}  

void
FloodFillInteractionMode
::Initialize()
{
  SetupPaintbrush();
}

void
FloodFillInteractionMode
::SetupPaintbrush()
{
  PaintbrushSettings pbs = m_GlobalState->GetPaintbrushSettings();
  pbs.radius = 0.5 * 1;
  pbs.mode = PAINTBRUSH_RECTANGULAR;
  pbs.flat = true;
  pbs.chase = true;
  pbs.isotropic = true;
  m_GlobalState->SetPaintbrushSettings( pbs );
}
int
FloodFillInteractionMode
::OnKeyDown( const FLTKEvent &event )
{ 
  if ( event.Key == 32 ) 
  { 
    m_ConnectToLastLimitVoxel = 0;

    SNAPAppearanceSettings::Element &elt = m_ParentUI->GetAppearanceSettings()->GetUIElement(SNAPAppearanceSettings::PAINTBRUSH_OUTLINE);
    if ( elt.NormalColor != Vector3d( 0.3f, 1.0f, 0.3f ) )
    {
      elt.NormalColor = Vector3d( 0.3f, 1.0f, 0.3f );
      m_LimitMode = false;
    }
    else
    {
      elt.NormalColor = Vector3d( 1.0f, 0.3f, 0.3f );
      m_LimitMode = true;
    }
    
    m_ParentUI->RedrawWindows();
  }
  if ( event.Key == 'l' )
  {
    m_HideLimits = !m_HideLimits; 
    OnDraw(); 
    m_ParentUI->RedrawWindows();
  }
  return 0;
}

int
FloodFillInteractionMode
::OnMouseRelease( const FLTKEvent &event, const FLTKEvent &pressEvent )
{
  SNAPAppearanceSettings::Element &elt = m_ParentUI->GetAppearanceSettings()->GetUIElement(SNAPAppearanceSettings::PAINTBRUSH_OUTLINE);
  if ( elt.NormalColor == Vector3d( 0.3f, 1.0f, 0.3f ) )
  {
    m_LimitMode = false;
  }
  else
  {
    m_LimitMode = true;
  }

  // Get voxel indices and image axis
  Vector3f voxel = m_Parent->MapWindowToSlice( event.XSpace.extract( 2 ) );// + Vector3f( 0.5 ));

  int imageAxis  = m_Parent->GetSliceDirectionInImageSpace();

  // Convert voxel indices to axial plane (numbering starting from 1)
  Vector3f mappedVoxelAxialPlane = ConvertIndexToAxialPlane( voxel, imageAxis );

  // Check that voxel is valid
  Vector3f iSize = to_float(m_ParentUI->GetDriver()->GetCurrentImageData()->GetVolumeExtents()); 
  if ( mappedVoxelAxialPlane[0] <= 0 || mappedVoxelAxialPlane[1] <= 0 || mappedVoxelAxialPlane[2] <= 0 ) 
    return 1;
  if ( mappedVoxelAxialPlane[0] > iSize[0] || mappedVoxelAxialPlane[1] > iSize[1] || mappedVoxelAxialPlane[2] > iSize[2] ) 
    return 1;


  if ( m_GlobalState->GetFloodFillLimitImage()->GetConnectToPreviousNeeded() )
  {
    if ( m_ConnectToLastLimitVoxel > 2 )
    {
      m_LastLimitVoxel = m_GlobalState->GetFloodFillLimitImage()->m_LastLimitVoxelList[ imageAxis ].back();
    }
    else
    {
      m_ConnectToLastLimitVoxel = 0;
    }
     m_GlobalState->GetFloodFillLimitImage()->SetConnectToPreviousNeeded( false );
  }

/*
  Vector3ui cursorIndex = m_ParentUI->GetDriver()->GetCursorPosition();
  if ( cursorIndex[ imageAxis ] != m_LastLimitVoxel[ imageAxis ] )
  {
    m_ConnectToLastLimitVoxel = 0;
  }
*/

  // Add limit
  if ( m_LimitMode ) // Limit Mode
  {
    if ( pressEvent.Button == FL_LEFT_MOUSE && event.State & FL_SHIFT ) // Remove selected limit
    {
     m_FloodFillLimits = true;
     FloodFill( mappedVoxelAxialPlane - Vector3f(1) );
     m_FloodFillLimits = false;
     m_ConnectToLastLimitVoxel = 0;
    }
    else
    {
      if ( pressEvent.Button == FL_LEFT_MOUSE || pressEvent.Button == FL_RIGHT_MOUSE )
      {
        if ( m_ConnectToLastLimitVoxel ) 
        {
          BuildLimitLine( mappedVoxelAxialPlane - Vector3f(1), m_LastLimitVoxel ); // Add connecting point
        }
        else 
        {
          if ( pressEvent.Button == FL_LEFT_MOUSE )
          {
            BuildLimitLine( mappedVoxelAxialPlane - Vector3f(1), mappedVoxelAxialPlane - Vector3f(1) );  // Add start point
          }
        }
        m_LastLimitVoxel = mappedVoxelAxialPlane - Vector3f(1); 
        ++m_ConnectToLastLimitVoxel;
      }
      if ( pressEvent.Button == FL_RIGHT_MOUSE )
      {
         m_ConnectToLastLimitVoxel = 0; // Terminate limit
      }
    }
  }
  else // Flood Mode
  {
    if ( pressEvent.Button == FL_LEFT_MOUSE ) // Start flood fill
    {
       FloodFill( mappedVoxelAxialPlane - Vector3f(1) );
       m_ConnectToLastLimitVoxel = 0;
    }
    
    if ( pressEvent.Button == FL_RIGHT_MOUSE ) 
    {
    }
    
  }
  //m_ParentUI->OnFloodFillUpdate() 
  return 0;
}


void
FloodFillInteractionMode
::BuildLimitLine(Vector3f start, Vector3f end )
{
  int imageAxis         = m_Parent->GetSliceDirectionInImageSpace();
  m_GlobalState->GetFloodFillLimitImage()->m_LastLimitVoxelList[ imageAxis ].clear();

  LimitImageType::SizeType size;
  size = m_GlobalState->GetFloodFillLimitImage()->GetImage()->GetLargestPossibleRegion().GetSize();

  double x0, y0, z0, x1, y1, z1;
  if ( imageAxis == 0 ) //Sagittal
  {
    x0 = start[2]; y0 = start[1]; z0 = start[0];
    x1 = end[2];   y1 = end[1];   z1 = end[0];
  }
  if ( imageAxis == 1 ) //Coronal
  {
    x0 = start[0]; y0 = start[2]; z0 = start[1];
    x1 = end[0];   y1 = end[2];   z1 = end[1];
  }
  if ( imageAxis == 2 ) // Axial
  {
    x0 = start[0]; y0 = start[1]; z0 = start[2];
    x1 = end[0];   y1 = end[1];   z1 = end[2];
  }

  // if slice has changed, draw point
  if ( z0 != z1 ) { x1 = x0; y1 = y0; z1 = z0; }
  
  bool steep = std::abs(y1-y0) > std::abs(x1-x0);
  if ( steep )
  {
    std::swap(x0,y0);
    std::swap(x1,y1);
  }
  if ( x0 > x1 )
  {
    std::swap(x0,x1);
    std::swap(y0,y1);
  }
  int deltax = x1 - x0;
  int deltay = std::abs(y1-y0);
  int error = deltax / 2;
  int ystep;
  int y = y0;
  if ( y0 < y1 )
    ystep = 1;
  else
    ystep = -1;

  LimitImageType::IndexType pixelIndex;
  
  for ( int x=x0; x<x1; ++x )
  {
    if ( steep )
    {
      if ( imageAxis == 0 ) // Sagittal
      {  pixelIndex[0] = z0; pixelIndex[1] = x; pixelIndex[2] = y; }
      if ( imageAxis == 1 ) // Coronal
      {  pixelIndex[0] = y; pixelIndex[1] = z0; pixelIndex[2] = x; }
      if ( imageAxis == 2 ) // Axial
      {  pixelIndex[0] = y; pixelIndex[1] = x; pixelIndex[2] = z0; }
    }
    else
    {
      if ( imageAxis == 0 ) // Sagittal
      {  pixelIndex[0] = z0; pixelIndex[1] = y; pixelIndex[2] = x; }
      if ( imageAxis == 1 ) // Coronal
      {  pixelIndex[0] = x; pixelIndex[1] = z0; pixelIndex[2] = y; }
      if ( imageAxis == 2 ) // Axial
      {  pixelIndex[0] = x; pixelIndex[1] = y;  pixelIndex[2] = z0; }
    }
    m_GlobalState->GetFloodFillLimitImage()->GetImage()->SetPixel( pixelIndex, 1 );
    m_GlobalState->GetFloodFillLimitImage()->m_LastLimitVoxelList[ imageAxis ].push_back( Vector3f( pixelIndex[0], pixelIndex[1], pixelIndex[2] ) );
 
    error = error - deltay;
    if ( error < 0 )
    {
      y = y + ystep;
      error = error + deltax;
    }
  }

  // Add endpoints (sometimes necessary, e.g. for adjacent start and end voxels)
  // start
  pixelIndex[0] = start[0]; pixelIndex[1] = start[1]; pixelIndex[2] = start[2];
  m_GlobalState->GetFloodFillLimitImage()->GetImage()->SetPixel( pixelIndex, 1 );
  m_GlobalState->GetFloodFillLimitImage()->m_LastLimitVoxelList[ imageAxis ].push_back( Vector3f( pixelIndex[0], pixelIndex[1], pixelIndex[2] ) );
  // end
  pixelIndex[0] = end[0]; pixelIndex[1] = end[1]; pixelIndex[2] = end[2];
  m_GlobalState->GetFloodFillLimitImage()->GetImage()->SetPixel( pixelIndex, 1 );
  m_GlobalState->GetFloodFillLimitImage()->m_LastLimitVoxelList[ imageAxis ].push_back( Vector3f( pixelIndex[0], pixelIndex[1], pixelIndex[2] ) );
}

void
FloodFillInteractionMode
::DrawLimits()
{
  if ( m_HideLimits ) return;

  int imageAxis         = m_Parent->GetSliceDirectionInImageSpace();
  Vector3ui cursorIndex = m_ParentUI->GetDriver()->GetCursorPosition();
  
  LimitImageType::SizeType size;
  size = m_GlobalState->GetFloodFillLimitImage()->GetImage()->GetLargestPossibleRegion().GetSize();
  
  LimitImageType::IndexType start;
  
  if ( imageAxis == 2 )
  {
    size[2] = 1;
    start[0] = 0;  start[1] = 0;  start[2] = cursorIndex[2];
  }
  if ( imageAxis == 0 )
  {
    size[0] = 1;

    start[0] = cursorIndex[0];  start[1] = 0;  start[2] = 0;
  }
  if ( imageAxis == 1 )
  {
    size[1] = 1;

    start[0] = 0;   start[1] = cursorIndex[1];  start[2] = 0;
  } 
  
  LimitImageType::RegionType region;
  region.SetSize( size );
  region.SetIndex( start );
    
  glColor3f( 1.0f, 0.3f, 0.3f);

  itk::ImageRegionIterator< LimitImageType > it( m_GlobalState->GetFloodFillLimitImage()->GetImage(), region );
  for ( it.GoToBegin(); !it.IsAtEnd(); ++it )
  {
    if ( it.Get() )
    {
      glBegin( GL_QUADS );
      LimitImageType::IndexType index = it.GetIndex();

      if ( imageAxis == 0 ) // Sagittal
      {
        unsigned int x = index[0];
        unsigned int y = index[1];
        unsigned int z = index[2];
        index[0] = size[1] - y - 1;
        index[1] = z;
        index[2] = x;
      }
      else if ( imageAxis == 1 ) // Coronal
      {
        unsigned int x = index[0];
        unsigned int y = index[1];
        unsigned int z = index[2];
        index[0] = x;
        index[1] = z;
        index[2] = y;
      }

      glVertex2d( index[0], index[1] + 1 );
      glVertex2d( index[0] + 1, index[1] + 1 );
      glVertex2d( index[0] + 1, index[1] );
      glVertex2d( index[0], index[1] );
      glEnd();
    }
  }
}


void 
FloodFillInteractionMode
::HideLimits( bool value )
{
   m_HideLimits = value;
}

void
FloodFillInteractionMode
::OnDraw()
{
  PaintbrushInteractionMode::OnDraw();
  DrawLimits();
}



void 
FloodFillInteractionMode
::FloodFill( Vector3f seed )
{
  // Get the flood fill properties
  LabelType new_color = m_GlobalState->GetDrawingColorLabel();
  LabelType old_color = m_GlobalState->GetOverWriteColorLabel();
  CoverageModeType mode = m_GlobalState->GetCoverageMode();
 
  // Get seed pixel
  LimitImageType::IndexType pixelIndex;
  pixelIndex[0] = seed[0]; pixelIndex[1] = seed[1]; pixelIndex[2] = seed[2];
    
  // Get slice for flood fill
  LabelImageWrapper::ImageType::SizeType size = m_GlobalState->GetFloodFillLimitImage()->GetImage()->GetLargestPossibleRegion().GetSize();  
  LabelImageWrapper::ImageType::IndexType start;

  unsigned int xmax, ymax;

  int imageAxis         = m_Parent->GetSliceDirectionInImageSpace();
  if ( imageAxis == 2 ) // Axial
  {
    size[2] = 1;
    start[0] = 0;  start[1] = 0;  start[2] = seed[2];
    xmax = size[0]; ymax = size[1];
  }
  if ( imageAxis == 0 )// Sagittal
  {
    size[0] = 1;
    start[0] = seed[0];  start[1] = 0;  start[2] = 0;
    xmax = size[1]; ymax = size[2];
  }
  if ( imageAxis == 1 )
  {
    size[1] = 1;
    start[0] = 0;   start[1] = seed[1];  start[2] = 0;
    xmax = size[0]; ymax = size[2];
  } 


  if ( m_Fill3D )
  {
    LabelImageWrapper::ImageType::Pointer tempSeg = LabelImageWrapper::ImageType::New();

    tempSeg = m_ParentUI->GetDriver()->GetCurrentImageData()->GetSegmentation()->GetImage();
    
    typedef FloodFillBinaryThresholdImageFunction< LabelImageWrapper::ImageType, LimitImageType, float > FunctionType;
   
    FunctionType::Pointer function = FunctionType::New();
    function->SetInputImage( tempSeg );
    function->SetLimitImage( m_GlobalState->GetFloodFillLimitImage()->GetImage() );

    if ( mode == PAINT_OVER_ONE )
    {
      function->AddFillValue( old_color );
    }
    else if ( mode == PAINT_OVER_ALL )
    {
      function->FillAll( true );
    }
    else if ( mode == PAINT_OVER_COLORS )
    {
      for ( int i=0; i < 255; ++ i )
      {
        if ( this->m_Driver->GetColorLabelTable()->GetColorLabel( i ).IsVisible() )
        {
          function->AddFillValue( i );
        }
      }
    }
   
    typedef itk::FloodFilledImageFunctionConditionalIterator< LabelImageWrapper::ImageType, FunctionType > IteratorType;

    start[0] = seed[0]; start[1] = seed[1]; start[2] = seed[2];
    IteratorType it( tempSeg, function, start );
          
    it.GoToBegin();
    while ( !it.IsAtEnd() )
    {
      it.Set( new_color );
      ++it;
    }    
  }
  else
  {
    LabelImageWrapper::ImageType::Pointer tempSeg = LabelImageWrapper::ImageType::New();
    LimitImageType::Pointer tempLimit = LimitImageType::New();

    LabelImageWrapper::ImageType::RegionType region;
    region.SetSize( size );
    region.SetIndex( start );
    
    LabelImageWrapper::ImageType::SizeType subsize;  
    LabelImageWrapper::ImageType::IndexType substart;
    subsize[0] = xmax; subsize[1] = ymax; subsize[2] = 1;
    substart[0] = 0; substart[1] = 0; substart[2] = 0;
    
    LabelImageWrapper::ImageType::RegionType subregion;
    subregion.SetSize( subsize );
    subregion.SetIndex( substart );
   
    tempSeg->SetRegions( subregion );
    tempSeg->SetDirection( m_ParentUI->GetDriver()->GetCurrentImageData()->GetSegmentation()->GetImage()->GetDirection() );
    tempSeg->SetSpacing( m_ParentUI->GetDriver()->GetCurrentImageData()->GetSegmentation()->GetImage()->GetSpacing() );
    tempSeg->Allocate();
  
    typedef itk::ImageRegionIterator< LabelImageWrapper::ImageType > SegIteratorType;
    SegIteratorType segit( m_ParentUI->GetDriver()->GetCurrentImageData()->GetSegmentation()->GetImage(), region );
    SegIteratorType tempsegit( tempSeg, tempSeg->GetLargestPossibleRegion() );

    segit.GoToBegin();
    tempsegit.GoToBegin();
    while ( !segit.IsAtEnd() )
    {
      tempsegit.Set( segit.Get() );
      ++segit; ++tempsegit;
    }
    
    tempLimit->SetRegions( subregion );
    tempLimit->SetDirection( m_ParentUI->GetDriver()->GetCurrentImageData()->GetSegmentation()->GetImage()->GetDirection() );
    tempLimit->SetSpacing( m_ParentUI->GetDriver()->GetCurrentImageData()->GetSegmentation()->GetImage()->GetSpacing() );
    tempLimit->Allocate();

    typedef itk::ImageRegionIterator< LimitImageType > LimitIteratorType;
    LimitIteratorType limitit( m_GlobalState->GetFloodFillLimitImage()->GetImage(), region );
    LimitIteratorType templimitit( tempLimit, tempLimit->GetLargestPossibleRegion() );

    limitit.GoToBegin();
    templimitit.GoToBegin();
    while ( !limitit.IsAtEnd() )
    {
      templimitit.Set( limitit.Get() );
      ++limitit; ++templimitit;
    }
    
    typedef FloodFillBinaryThresholdImageFunction< LabelImageWrapper::ImageType, LimitImageType, float > FunctionType;

    FunctionType::Pointer function = FunctionType::New();
    function->SetInputImage( tempSeg );
    function->SetLimitImage( tempLimit );

    if ( mode == PAINT_OVER_ONE )
    {
      function->AddFillValue( old_color );
    }
    else if ( mode == PAINT_OVER_ALL )
    {
      function->FillAll( true );
    }
    else if ( mode == PAINT_OVER_COLORS )
    {
      for ( int i=0; i < 255; ++ i )
      {
        if ( this->m_Driver->GetColorLabelTable()->GetColorLabel( i ).IsVisible() )
        {
          function->AddFillValue( i );
        }
      }
    }
   
    typedef itk::FloodFilledImageFunctionConditionalIterator< LabelImageWrapper::ImageType, FunctionType > IteratorType;
     
    if ( imageAxis == 1 ) // Coronal
    {
      start[0] = seed[0];
      start[1] = seed[2];
      start[2] = 0;
    }
    if ( imageAxis == 0 ) // Sagittal
    {
      start[0] = seed[1];
      start[1] = seed[2];
      start[2] = 0;
    }
    if ( imageAxis == 2 ) // Axial 
    {
      start[0] = seed[0];
      start[1] = seed[1];
      start[2] = 0;
    }
    
    IteratorType it( tempSeg, function, start );
        
    it.GoToBegin();
    while ( !it.IsAtEnd() )
    {
      it.Set( new_color );
      ++it;
    }

    segit.GoToBegin();
    tempsegit.GoToBegin();
    while ( !segit.IsAtEnd() )
    {
      segit.Set( tempsegit.Get() );
      ++segit; ++tempsegit;
    }

  }
  // Store undo point
  m_Parent->m_ParentUI->StoreUndoPoint("Flood Filling");
  
  // Update UI
  m_ParentUI->GetDriver()->GetCurrentImageData()->GetSegmentation()->GetImage()->Modified();
  m_ParentUI->OnPaintbrushPaint();
  m_ParentUI->RedrawWindows();
}

