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


=========================================================================*/
#ifndef __Sabre3DTrackballMode_h_
#define __Sabre3DTrackballMode_h_

#include "vtkInteractorStyleTrackballCamera.h"

class VTK_RENDERING_EXPORT Sabre3DTrackballMode : public vtkInteractorStyleTrackballCamera
{
public:
  
  vtkTypeRevisionMacro( Sabre3DTrackballMode, vtkInteractorStyleTrackballCamera );
  void PrintSelf(ostream& os, vtkIndent indent) {};
  static Sabre3DTrackballMode* New( ) { return new Sabre3DTrackballMode(); }
  void   OnChar() {};

protected:

  Sabre3DTrackballMode() {};
  ~Sabre3DTrackballMode() {};



};

#endif