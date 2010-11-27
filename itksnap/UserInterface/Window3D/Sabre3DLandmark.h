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
#ifndef __Sabre3DInteractionMode_h_
#define __Sabre3DInteractionMode_h_

#include "vtkInteractorStyleUser.h"
#include "vtkFlRenderWindowInteractor.h"
#include "vtkRenderer.h"
#include "vtkVolumePicker.h"
#include "GlobalState.h"
#include "GreyImageWrapper.h"

#include <iostream>
#include <list>
#include <map>
#include <vnl/vnl_vector_fixed.h>

class LabelImageWrapper;
class GreyImageWrapper;

class Sabre3DLandmark
{
public:
  typedef vnl_vector_fixed< double, 3 > Vector3d;
  typedef std::list< Vector3d >         NodeList;

  Sabre3DLandmark() {};
  Sabre3DLandmark( LandmarkType landmarkCodeNode, int landmarkCodeLine, bool lineRestriction, Vector3d color )
  {
    m_LineRestriction = lineRestriction;
    m_LandmarkCodeNode = landmarkCodeNode;
    m_LandmarkCodeLine = landmarkCodeLine;

    m_LastNode = NULL;
    m_LastAcceptedNode = NULL;
    m_SelectedLinePreNode = NULL;
    m_SelectedLinePostNode = NULL;
    m_ID = 1;
    m_Color[0] = color[0]; m_Color[1] = color[1]; m_Color[2] = color[2];
  }

  ~Sabre3DLandmark() {};

  int GetNextID() { return ++m_ID; }

public:

  vtkActor*                             m_LastNode;
  vtkActor*                             m_LastAcceptedNode;
  vtkActor*                             m_SelectedLinePreNode;
  vtkActor*                             m_SelectedLinePostNode;
  NodeList                              m_Undo;
  NodeList                              m_Accepted;
  int                                   m_ID;
  LandmarkType                          m_LandmarkCodeNode;
  int                                   m_LandmarkCodeLine;
  bool                                  m_LineRestriction;
  Vector3d                              m_Color;
};


class VTK_RENDERING_EXPORT Sabre3DInteractionMode : public vtkInteractorStyleUser
{
private:

  typedef vnl_vector_fixed< double, 3 > Vector3d;
  typedef std::vector< Sabre3DLandmark > Landmarks;
  typedef std::list< Vector3d > NodeList;
  typedef std::map< LandmarkType, int > LandmarkMap;

public:
  
  vtkTypeRevisionMacro( Sabre3DInteractionMode, vtkInteractorStyleUser );
  void PrintSelf(ostream& os, vtkIndent indent) {};
  static Sabre3DInteractionMode* New( ) { return new Sabre3DInteractionMode(); }

  void Initialize( vtkFlRenderWindowInteractor* renwin,
                   vtkRenderer* renderer,
                   vtkVolumePicker* picker,
                   GreyImageWrapper::ImagePointer grey );

  Vector3d GetPickPosition();

  void     AddPoint( Vector3d pos );
  void     AddAcceptedPoint( Vector3d pos, LandmarkType landmark );

  void     SetLandmarkMode( LandmarkType mode ); 

  void     Undo();
  void     Redo();

  void     Accept( LabelImageWrapper* seg );
  void     WriteLine3D( Vector3d point1, Vector3d point2, LabelImageWrapper* seg, LandmarkType value );
  void     Reset();

  void     DeleteLandmark( LabelImageWrapper* seg, LandmarkType landmark );

  void     OnLandmarkLoad( LabelImageWrapper* seg, GreyImageWrapper* grey );

  // Mouse responses
  void OnLeftButtonDown();
  void OnRightButtonDown() {};
  void OnMouseMove() {};


protected:

  Sabre3DInteractionMode();
  ~Sabre3DInteractionMode();


private:

  GreyImageWrapper::ImagePointer  m_ImgGrey;
  vtkFlRenderWindowInteractor*    m_RenWin;
  vtkRenderer*                    m_Ren;
  vtkVolumePicker*                m_Picker;


  LandmarkType m_CurrentLandmarkMode;
  bool         m_IsInitialized;
  int          m_NumberOfLandmarks;
  Landmarks    m_Landmarks;
  LandmarkMap  m_LandmarkMap;


};

#endif