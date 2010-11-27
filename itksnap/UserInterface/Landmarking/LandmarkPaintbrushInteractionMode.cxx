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
#include "IRISApplication.h"
#include "GenericImageData.h"
#include "LandmarkPaintbrushInteractionMode.h"
#include "GenericSliceWindow.h"
#include "Landmarks.h"

LandmarkPaintbrushInteractionMode
::LandmarkPaintbrushInteractionMode( GenericSliceWindow* parent )
: PaintbrushInteractionMode( parent )
{
}

void
LandmarkPaintbrushInteractionMode
::ClearExistingLandmark( LandmarkType landmarkMode )
{
  // Get segmentation image data
  LabelImageWrapper *imgLabel = m_Driver->GetCurrentImageData()->GetSegmentation();

  // Get radius of paintbrush
  int radius = m_GlobalState->GetPaintbrushSettings().radius;

  // Remove any landmark voxels found in the 3D radius of the landmark index
  // (Done in 3D because although the landmark index is in the axial plane, if the paintbrush
  //  radius is > 1 and the landmak was placed in either the coronal or sagittal window, the
  //  landmark will extend into the axial z plane)
  for ( int x = -radius; x <= radius; ++x )
    for (int y = -radius; y <= radius; ++y )
      for (int z =- radius; z <= radius; ++z )
      {
        LabelImageWrapper::ImageType::IndexType pixelIndex;
        pixelIndex[0] = m_GlobalState->GetLandmarks()->GetLandmarkIndex( landmarkMode ).operator [](0) - 1 + x;
        pixelIndex[1] = m_GlobalState->GetLandmarks()->GetLandmarkIndex( landmarkMode ).operator [](1) - 1 + y;
        pixelIndex[2] = m_GlobalState->GetLandmarks()->GetLandmarkIndex( landmarkMode ).operator [](2) - 1 + z;
        
        if ( imgLabel->GetImage()->GetPixel(pixelIndex) == landmarkMode )
          imgLabel->GetImage()->SetPixel( pixelIndex, 0 );
      }     

  // Update
  imgLabel->GetImage()->Modified();
  m_ParentUI->OnPaintbrushPaint();
  m_ParentUI->RedrawWindows(); 
}

int
LandmarkPaintbrushInteractionMode
::OnMouseRelease( const FLTKEvent &event, const FLTKEvent &pressEvent )
{
  // Get current landmark
  LandmarkType landmarkMode = m_GlobalState->GetLandmarks()->GetCurrentLandmarkMode();

  // If landmark was already drawn, clear
  if ( m_GlobalState->GetLandmarks()->GetLandmarkIsDrawn( landmarkMode ) )
    ClearExistingLandmark( landmarkMode );

  if ( pressEvent.Button == FL_LEFT_MOUSE )
  {
    // Get voxel indices and image axis
    Vector3f mappedVoxel = m_Parent->MapWindowToSlice( event.XSpace.extract( 2 ) );
    int imageAxis = m_Parent->GetSliceDirectionInImageSpace();

    // Convert voxel indices to axial plane (numbering starting from 1)
    Vector3f mappedVoxelAxialPlane = ConvertIndexToAxialPlane( mappedVoxel, imageAxis );

    Vector3f iSize = to_float(m_ParentUI->GetDriver()->GetCurrentImageData()->GetVolumeExtents());
  
    if ( mappedVoxelAxialPlane[0] <= 1 || mappedVoxelAxialPlane[1] <= 1 || mappedVoxelAxialPlane[2] <= 1 ) 
      return 1;
    if ( mappedVoxelAxialPlane[0] > iSize[0] || mappedVoxelAxialPlane[1] > iSize[1] || mappedVoxelAxialPlane[2] > iSize[2] ) 
      return 1;


    // Store voxel indices in global state
    m_GlobalState->GetLandmarks()->SetLandmark(  landmarkMode, mappedVoxelAxialPlane, true );
   
    // Draw landmark
    PaintbrushInteractionMode::ComputeMousePosition( event.XSpace );
    PaintbrushInteractionMode::ApplyBrush( event );

    // Record the mouse event
    m_LastMouseEvent = event;

    // Update the UI
    m_ParentUI->UpdateSabreGridDisplay();

    // Intentionally do NOT store undo point
    //m_Parent->m_ParentUI->StoreUndoPoint("Drawing with paintbrush");
    
    return 0;
    
  }


  return 0;
}
