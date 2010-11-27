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
#include "AlignmentInteractionMode.h"
#include "ACPCAlignmentTool.h"
#include "UserInterfaceBase.h"
#include "IRISApplication.h"
#include "IRISImageData.h"

AlignmentInteractionMode
::AlignmentInteractionMode( GenericSliceWindow *parent )
: CrosshairsInteractionMode( parent )
{
  m_ACPCAlignmentTool = NULL;
}

int
AlignmentInteractionMode
::OnMouseRelease(const FLTKEvent &event, const FLTKEvent &irisNotUsed(pressEvent))
{
  CrosshairsInteractionMode::OnMouseRelease( event, event );
  if ( m_ACPCAlignmentTool != NULL )
    UpdateAlignment();
  return 0;
}

int
AlignmentInteractionMode
::OnMouseDrag(const FLTKEvent &event, const FLTKEvent &irisNotUsed(pressEvent) )
{
  CrosshairsInteractionMode::OnMouseDrag( event, event );
  if ( m_ACPCAlignmentTool != NULL )
    UpdateAlignment();
  return 0;
}

int
AlignmentInteractionMode
::OnMousePress(const FLTKEvent &event)
{
  CrosshairsInteractionMode::OnMousePress( event );
  if ( m_ACPCAlignmentTool != NULL )
    UpdateAlignment();
  return 0;
}

int 
AlignmentInteractionMode
::OnMouseWheel(const FLTKEvent &event)
{
  // Disable scrolling
  return 1;
}

void
AlignmentInteractionMode
::UpdateAlignment()
{
  // Get cursor position
  Vector3ui cursor = m_ParentUI->GetDriver()->GetCursorPosition();

  // Resample main image for the 3 planes defined by the cursor location
  m_ACPCAlignmentTool->AlignmentChange(0,0,0,0,0,0,cursor,m_Driver->GetIRISImageData()->GetGrey()->GetImage() );
  
  // Trigger redraw of main image
  m_Driver->GetIRISImageData()->GetGrey()->SetImage( m_Driver->GetIRISImageData()->GetGrey()->GetImage() );
}
