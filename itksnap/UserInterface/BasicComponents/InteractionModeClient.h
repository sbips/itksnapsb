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
  Module:    $RCSfile: InteractionModeClient.h,v $
  Language:  C++
  Date:      $Date: 2006/12/02 04:22:21 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2003 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#ifndef __InteractionModeClient_h_
#define __InteractionModeClient_h_

#include "InteractionMode.h"
#include <list>

/**
 * \class InteractionModeClient
 * \brief An abstract class that can pass on Fltk events to interaction modes
 */
class InteractionModeClient
{
public:
  /**
   * Push an interaction mode onto the stack of modes.  Mode becomes first to 
   * receive events.  The events that it does not receive are passed on to the
   * next mode on the stack.
   */
  void PushInteractionMode(InteractionMode *mode);

  /**
   * Pop the last interaction mode off the stack
   */
  InteractionMode *PopInteractionMode();

  /**
   * Get the top interaction mode on the stack
   */
  InteractionMode *GetTopInteractionMode();

  /**
   * See if the interaction mode is in the stack
   */
  bool IsInteractionModeAdded(InteractionMode *target);

  /**
   * Remove all interaction modes
   */
  void ClearInteractionStack();

  /** 
   * Set the interaction stack to consist of just one interaction mode. This is 
   * equivalent to calling ClearInteractionStack() followed by PushInteractionMode()
   */
  void SetSingleInteractionMode(InteractionMode *mode);

  /**
   * Get the number of interaction modes on the stack
   */
  unsigned int GetInteractionModeCount();

  // Virtual destructor
  virtual ~InteractionModeClient() {}

protected:
  /**
   * This method should be called to draw the interactors (call their OnDraw methods 
   * in a sequence from bottom to top)
   */
  void FireInteractionDrawEvent();

  // The stack of interaction modes
  std::list<InteractionMode *> m_Interactors;
};

#endif // __InteractionModeClient_h_

