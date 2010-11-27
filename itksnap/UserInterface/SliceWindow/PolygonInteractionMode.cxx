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
  Module:    $RCSfile: PolygonInteractionMode.cxx,v $
  Language:  C++
  Date:      $Date: 2009/09/17 20:28:39 $
  Version:   $Revision: 1.7 $
  Copyright (c) 2007 Paul A. Yushkevich
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#include "PolygonInteractionMode.h"

#include "GlobalState.h"
#include "PolygonDrawing.h"
#include "UserInterfaceBase.h"
#include "IRISApplication.h"

PolygonInteractionMode
::PolygonInteractionMode(GenericSliceWindow *parent)
: GenericSliceWindow::EventHandler(parent), m_Filled( true ), m_Closed( true )
{
  m_Drawing = new PolygonDrawing();
}

PolygonInteractionMode
::~PolygonInteractionMode()
{
  delete m_Drawing;
}

int
PolygonInteractionMode
::OnEitherEvent(const FLTKEvent &event, 
                const FLTKEvent &irisNotUsed(pressEvent))
{

  // We'll need these shorthands
  int id = m_Parent->m_Id;

  // Check for enter key
  if(Fl::test_shortcut(FL_Enter))
    {
    m_Parent->m_ParentUI->OnAcceptPolygonAction(id);
    return 1;
    }
  else if(Fl::test_shortcut(FL_Delete))
    {
    m_Parent->m_ParentUI->OnDeletePolygonSelectedAction(id);
    return 1;
    }
  else if(Fl::test_shortcut(FL_Insert))
    {
    m_Parent->m_ParentUI->OnInsertIntoPolygonSelectedAction(id);
    return 1;
    }
  else if(Fl::test_shortcut('v' | FL_CTRL))
    {
    m_Parent->m_ParentUI->OnPastePolygonAction(id);
    return 1;
    }

  // Pass through events that are irrelevant
  if(event.SoftButton != FL_LEFT_MOUSE 
    && event.SoftButton != FL_RIGHT_MOUSE
    && event.Key != ' ')
    return 0;

#ifdef DRAWING_LOCK
  if (!m_GlobalState->GetDrawingLock(id)) break;
#endif /* DRAWING_LOCK */

  // Compute the dimension of a pixel on the screen
  Vector2f pixelSize = GetPixelSizeVector();

  // Masquerade keypress events as mouse-clicks at the cursor position
  if(event.Key == ' ') 
    {
    // Map the cursor position into slice coordinates
    Vector3f xEvent = m_Parent->MapImageToSlice(
      to_float(m_Driver->GetCursorPosition()));

    // Get the event state based on shift-ctrl
    int fakeButton = (event.State & FL_SHIFT) ? FL_RIGHT_MOUSE : FL_LEFT_MOUSE;

    // Handle the event
    m_Drawing->Handle(FL_PUSH, fakeButton, xEvent(0), xEvent(1),
                      pixelSize(0),pixelSize(1));
    }
  else
    {
    // Map the event into slice coordinates 
    Vector3f xEvent = m_Parent->MapWindowToSlice(event.XSpace.extract(2));

    // Handle the event
    m_Drawing->Handle(event.Id,event.SoftButton,xEvent(0),xEvent(1),
                      pixelSize(0),pixelSize(1));
    }

#ifdef DRAWING_LOCK
  m_GlobalState->ReleaseDrawingLock(id);
#endif /* DRAWING_LOCK */
  
  // Update the display
  m_Parent->GetCanvas()->redraw();
  
  // Let the parent UI know that the polygon state has changed
  m_ParentUI->OnPolygonStateUpdate(id);

  // Even though no action may have been performed, we don't want other handlers
  // to get the left and right mouse button events
  return 1;
}

Vector2f 
PolygonInteractionMode
::GetPixelSizeVector()
{
  Vector3f x = 
    m_Parent->MapWindowToSlice(Vector2f(1.0f)) - 
    m_Parent->MapWindowToSlice(Vector2f(0.0f));

  return Vector2f(x[0],x[1]);
}

                          
void
PolygonInteractionMode
::OnDraw()
{
  // Compute the dimension of a pixel on the screen
  Vector2f pixelSize = GetPixelSizeVector();

  // Call the poly's draw method
  m_Drawing->Draw(pixelSize(0),pixelSize(1));
}

