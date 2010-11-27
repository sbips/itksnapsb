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
#include "vtkSmartPointer.h"

#include <iostream>
#include <list>
#include <map>
#include <vnl/vnl_vector_fixed.h>

class LabelImageWrapper;
class GreyImageWrapper;
 
// Simple struct encapsulating essential landmark properties
struct Sabre3DLandmark
{
  // TYPEDEFS
  
  typedef vnl_vector_fixed< double, 3 > Vector3d;
  typedef std::list< Vector3d >         NodeList;

  // FUNCTIONS

  // Default constructor provided for use with arrays and lists
  Sabre3DLandmark() {};

  // Standard constructor (Note: lineRestriction = true denotes that the line should
  // not turn back on itself, i.e. the central sulci)
  Sabre3DLandmark( LandmarkType landmarkCodeNode, int landmarkCodeLine, bool lineRestriction, Vector3d color )
  {
    m_ID = 1;
    m_LineRestriction = lineRestriction;
    m_Color[0] = color[0]; 
    m_Color[1] = color[1]; 
    m_Color[2] = color[2];
    m_LastNode = NULL;
    m_LastAcceptedNode = NULL;
    m_LandmarkCodeNode = landmarkCodeNode;
    m_LandmarkCodeLine = landmarkCodeLine;
  }

  ~Sabre3DLandmark() 
  { 
  };


  int GetNextID() { return ++m_ID; }

  // MEMBERS

  int                                   m_ID;
  bool                                  m_LineRestriction;
  Vector3d                              m_Color;
  vtkSmartPointer< vtkActor >           m_LastNode;
  vtkSmartPointer< vtkActor >           m_LastAcceptedNode;
  LandmarkType                          m_LandmarkCodeNode;
  int                                   m_LandmarkCodeLine;

  NodeList                              m_Undo;
  NodeList                              m_Accepted;
  
  void Purge()
  {
    m_ID = 1;
    if ( m_LastNode != NULL )
      m_LastNode->Delete();
    if ( m_LastAcceptedNode != NULL )
      m_LastAcceptedNode->Delete();
    m_Undo.clear();
    m_Accepted.clear();
  };

};


class VTK_RENDERING_EXPORT Sabre3DInteractionMode : public vtkInteractorStyleUser
{
private:

  typedef vnl_vector_fixed< double, 3 >  Vector3d;
  typedef std::vector< Sabre3DLandmark > Landmarks;
  typedef std::list< Vector3d >          NodeList;
  typedef std::map< LandmarkType, int >  LandmarkMap;

public:
  
  vtkTypeRevisionMacro( Sabre3DInteractionMode, vtkInteractorStyleUser );
  void PrintSelf(ostream& os, vtkIndent indent) {};
  static Sabre3DInteractionMode* New( ) { return new Sabre3DInteractionMode(); }

  // Interactor must be initialized before use.
  void Initialize( vtkFlRenderWindowInteractor* renwin,
                   vtkRenderer* renderer,
                   vtkVolumePicker* picker,
                   GreyImageWrapper::ImagePointer grey );

  // Set landmark mode (RSF/LSF/RC/LC).
  void     SetLandmarkMode( LandmarkType mode ); 

  // Undo last node.
  void     Undo();
  
  // Redo last node.
  void     Redo();

  // Hide all lines and nodes.
  void     Hide();
  
  // Show all lines and nodes.
  void     Unhide();

  // Accept all landmarks (i.e. convert node positions to lines and modify segmentation).
  void     Accept( LabelImageWrapper* seg );
  
  // Delete accepted landmark.
  void     DeleteLandmark( LabelImageWrapper* seg, LandmarkType landmark );

  // Load landmarks.
  void     OnLandmarkLoad( LabelImageWrapper* seg, GreyImageWrapper* grey );

  // Mouse responses.
  void OnLeftButtonDown();

  void     Disconnect();

  // Not implemented
  void 	OnLeftButtonUp () {}
  void 	OnMiddleButtonDown () {}
  void 	OnMiddleButtonUp () {}
  void 	OnRightButtonDown () {}
  void 	OnRightButtonUp () {}
  void 	OnChar () {}
  void 	OnKeyPress () {} 
  void 	OnKeyRelease () {}

  void Purge();

private:

  // Get picked position in volume cooridinates.
  Vector3d GetPickPosition();

  // Set visibility of nodes and lines.
  void     SetVisibility( bool value );

  // Compute line from node positions.
  void     WriteLine3D( Vector3d point1, Vector3d point2, LabelImageWrapper* seg, LandmarkType value );
  
  // Add user defined point.
  void     AddPoint( Vector3d pos );

  // Add point from existing object map.
  void     AddAcceptedPoint( Vector3d pos, LandmarkType landmark );

  // Remove lines and nodes after accept action.
  void     Reset();


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