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

#ifndef __LandmarkPaintbrushInteractionMode_h_
#define __LandmarkPaintbrushInteractionMode_h_

#include "SNAPCommon.h"
#include "PaintbrushInteractionMode.h"

class LandmarkPaintbrushInteractionMode : public PaintbrushInteractionMode
{
public:
  LandmarkPaintbrushInteractionMode( GenericSliceWindow* parent );
  virtual ~LandmarkPaintbrushInteractionMode() {}

  void ClearExistingLandmark( LandmarkType landmarkMode );
  
  int OnKeyDown( const FLTKEvent &event ) { return 0; }
  int OnMouseDrag( const FLTKEvent &event, const FLTKEvent & pressEvent ) { return 0; }
  int DragReleaseHandler( FLTKEvent &event, const FLTKEvent &pressEvent, bool drag ) { return 0; }
  int OnShortcut( const FLTKEvent &event )    { return 0; }
  int OnMousePress( const FLTKEvent &event )  { return 0; }
  int OnMouseRelease( const FLTKEvent &event, const FLTKEvent &pressEvent );

  //int OnMouseMotion( const FLTKEvent &event ) { return 0; }
  //int OnMouseEnter( const FLTKEvent &event )  { return 0; }
  //int OnMouseLeave( const FLTKEvent &event )  { return 0; }
  //void OnDraw() {};
};

#endif