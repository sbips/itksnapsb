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
  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: RecursiveInteractionMode.h,v $
  Language:  C++
  Date:      $Date: 2006/12/02 04:22:21 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2003 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#ifndef __RecursiveInteractionMode_h_
#define __RecursiveInteractionMode_h_

#include "InteractionModeClient.h"
#include <list>

// Forward reference to FLTK Canvas
class FLTKCanvas;

/**
 * \class RecursiveInteractionMode
 * \brief An interaction mode to which more interaction modes can be added
 * This class is an interaction mode and an interaction mode client, so that
 * you can create a tree of interaction modes. The only thing to remember is 
 * that if you override one of the OnXXX event methods, you need to call 
 * Superclass::OnXXX in order for the child events to be invoked
 */
class RecursiveInteractionMode : public InteractionMode, public InteractionModeClient
{
public:
  // Typedef for the iterator
  typedef std::list<InteractionMode *>::iterator ModeIterator;

  // All the events
  virtual int OnMousePress(const FLTKEvent &event);
  virtual int OnMouseRelease(const FLTKEvent &event, const FLTKEvent &pressEvent);
  virtual int OnMouseDrag(const FLTKEvent &event, const FLTKEvent &pressEvent);
  virtual int OnMouseEnter(const FLTKEvent &event);
  virtual int OnMouseLeave(const FLTKEvent &event);
  virtual int OnMouseMotion(const FLTKEvent &event);
  virtual int OnMouseWheel(const FLTKEvent &event);
  virtual int OnKeyDown(const FLTKEvent &event);
  virtual int OnKeyUp(const FLTKEvent &event);
  virtual int OnShortcut(const FLTKEvent &event);
  virtual int OnOtherEvent(const FLTKEvent &event);
  virtual void OnDraw();

  // Constructor, takes the target FLTK canvas as the parameter
  RecursiveInteractionMode(FLTKCanvas *canvas)
    : InteractionMode(canvas) {}

  // Virtual destructor
  virtual ~RecursiveInteractionMode() {}
};

#endif // __RecursiveInteractionMode_h_
