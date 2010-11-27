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

#ifndef __FloodFillInteractionMode_h_
#define __FloodFillInteractionMode_h_

#include "SNAPCommon.h"
#include "PaintbrushInteractionMode.h"
#include "GreyImageWrapper.h"

class UserInterfaceLogic;

class FloodFillInteractionMode : public PaintbrushInteractionMode
{
public:
  FloodFillInteractionMode( GenericSliceWindow* parent );
  virtual ~FloodFillInteractionMode() {}

  int OnKeyDown( const FLTKEvent &event );
  int OnMouseDrag( const FLTKEvent &event, const FLTKEvent & pressEvent ) { return 0; }
  int DragReleaseHandler( FLTKEvent &event, const FLTKEvent &pressEvent, bool drag ) { return 0; }
  int OnShortcut( const FLTKEvent &event )    { return 0; }
  int OnMousePress( const FLTKEvent &event )  { return 0; }

  int OnMouseRelease( const FLTKEvent &event, const FLTKEvent &pressEvent );
  void OnDraw();

  //int OnMouseMotion( const FLTKEvent &event ) { return 0; }
  int OnMouseEnter( const FLTKEvent &event )  { SetupPaintbrush(); return PaintbrushInteractionMode::OnMouseEnter( event ); }
  //int OnMouseLeave( const FLTKEvent &event )  { return 0; }

  void Initialize();
  void SetupPaintbrush();

  void DrawLimits();
  
  void HideLimits( bool value );
  void BuildLimitLine(Vector3f start, Vector3f end );  
  
  void FloodFill( Vector3f seed ); 

  irisSetMacro(Fill3D, bool);
  irisGetMacro(Fill3D, bool);
    
  irisSetMacro(ConnectToLastLimitVoxel, bool);
  irisGetMacro(ConnectToLastLimitVoxel, bool);

private:
 
  typedef GreyImageWrapper::ImageType LimitImageType;  

  Vector3f  m_LastLimitVoxel;

  unsigned int m_ConnectToLastLimitVoxel;

  bool     m_LimitMode;
  bool     m_HideLimits;
  bool     m_FloodFillLimits;
  bool     m_Fill3D;

  std::list< Vector3f > m_LastLimitVoxelList;
  };

#endif