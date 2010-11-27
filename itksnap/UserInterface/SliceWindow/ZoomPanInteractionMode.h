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
  Module:    $RCSfile: ZoomPanInteractionMode.h,v $
  Language:  C++
  Date:      $Date: 2007/12/30 04:05:29 $
  Version:   $Revision: 1.2 $
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
#ifndef __ZoomPanInteractionMode_h_
#define __ZoomPanInteractionMode_h_

#include "GenericSliceWindow.h"

/**
 * \class ZoomPanInteractionMode
 * \brief UI interaction mode that takes care of zooming and panning.
 *
 * \see GenericSliceWindow
 */
class ZoomPanInteractionMode : public GenericSliceWindow::EventHandler {
public:
  ZoomPanInteractionMode(GenericSliceWindow *parent);

  int OnMousePress(const FLTKEvent &event);
  int OnMouseRelease(const FLTKEvent &event, const FLTKEvent &pressEvent);    
  int OnMouseDrag(const FLTKEvent &event, const FLTKEvent &pressEvent);
  void OnDraw();
  
  // void OnDraw();
protected:
  /** The starting point for panning */
  Vector2f m_StartViewPosition;

  /** The starting zoom level */
  float m_StartViewZoom;

  /** Used to schedule UI repaint updates */
  bool m_NeedUIUpdateOnRepaint;
};

#endif // __ZoomPanInteractionMode_h_
