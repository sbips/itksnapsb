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
  Module:    $RCSfile: InteractionModeClient.cxx,v $
  Language:  C++
  Date:      $Date: 2007/12/30 04:05:16 $
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
#include "InteractionModeClient.h"
#include <algorithm>
#include <list>

void                            
InteractionModeClient
::PushInteractionMode(InteractionMode *mode)
{
  m_Interactors.push_front(mode);
}

InteractionMode *
InteractionModeClient
::PopInteractionMode() 
{
  InteractionMode *lastMode = m_Interactors.front();
  m_Interactors.pop_front();
  return lastMode;
}

void 
InteractionModeClient
::ClearInteractionStack() 
{
  m_Interactors.clear();
} 

void InteractionModeClient
::SetSingleInteractionMode(InteractionMode *mode)
{
  this->ClearInteractionStack();
  this->PushInteractionMode(mode);
}

unsigned int 
InteractionModeClient
::GetInteractionModeCount()
{
  return m_Interactors.size();
}

InteractionMode *
InteractionModeClient
::GetTopInteractionMode()
{
  return m_Interactors.front();
}

bool
InteractionModeClient
::IsInteractionModeAdded(InteractionMode *target)
{
  return 
    std::find(m_Interactors.begin(), m_Interactors.end(), target) !=
      m_Interactors.end();
}

void 
InteractionModeClient
::FireInteractionDrawEvent()
{
  // Propagate the drawing event through the stack
  typedef std::list<InteractionMode *>::iterator ModeIterator; 
  for (ModeIterator it = m_Interactors.begin(); it!=m_Interactors.end();it++)
    {
    InteractionMode *mode = *it;
    mode->OnDraw();
    }
}

