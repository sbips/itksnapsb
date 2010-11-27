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

#ifndef __AlignmentInteractionMode_h_
#define __AlignmentInteractionMode_h_

#include "SNAPCommon.h"
#include "CrosshairsInteractionMode.h"

/*
Modified crosshairs interactor for use with the ACPC alignment tool
- Scrolling through slices is disabled.
- Mouse actions trigger one additional effect:  The main image is resampled
  according to the rotation matrix specified in the ACPCAlignmentTool for 
  the 3 planes defined by the cursor location.
*/

class ACPCAlignmentTool;

class AlignmentInteractionMode : public CrosshairsInteractionMode
{
public:
  AlignmentInteractionMode( GenericSliceWindow *parent );
  virtual ~AlignmentInteractionMode() {};

  int OnMouseRelease(const FLTKEvent &event, const FLTKEvent &irisNotUsed(pressEvent));
  int OnMousePress(const FLTKEvent &event);
  int OnMouseDrag(const FLTKEvent &event, const FLTKEvent &irisNotUsed(pressEvent));
  int OnMouseWheel(const FLTKEvent &event);
  
  void UpdateAlignment();
  void SetACPCAlignmentTool( ACPCAlignmentTool* tool ) { m_ACPCAlignmentTool = tool; }
 

private:
  ACPCAlignmentTool* m_ACPCAlignmentTool;
};

#endif