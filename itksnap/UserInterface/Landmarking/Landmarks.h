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


#ifndef __Landmarks_h_
#define __Landmarks_h_

#include "SNAPCommon.h"
#include "GlobalState.h"

#include <vector>

class LabelImageWrapper;


/**
 * \class Landmarks
 * \Logic for user placed landmarks.
 *
 * Each landmarks consists of a Vector3f specifying its axial cooridinate 
 * in image space and a bool indicating whether it has been drawn or not.
 *
 * Landmarks AC, PC, PE, CA, M, LRPRON, RPRON (1x1 voxel in size) are used
 * to generate the SABRE grid file.  AC, PC, M, LPRON and RPRON are required 
 * for the SABRE grid file.  CA is used only to guide the placement of M.  
 * PE is used only to determine the coronal slice for the placement of LPRON
 * and RPRON. 
 * 
 * Landmarks RSC and LSC (2x2 voxels in size ) are used to generate the SABRE 
 * object map tracing, along with ROP and LOP.  However, ROP and LOP are not 
 * represented within this class because they are line landmarks, not point 
 * landmarks.
 */

class Landmarks
{
public:
  Landmarks();
  ~Landmarks() {}
  
  //       Get the axial index for the requested landmark
  Vector3f GetLandmarkIndex(   LandmarkType landmark ) { return m_AllLandmarks[ landmark ].axialIndex; }

  //       Get the drawn state for the requested landmark
  bool     GetLandmarkIsDrawn( LandmarkType landmark ) { return m_AllLandmarks[ landmark ].isDrawn; }

  //       Load/Save the Sabre grid text file
  void     LoadSabreGrid( const char* fn );
  int      SaveSabreGrid( const char* fn );
  
  //       Load/Save the Sabre tracing object map
  void     LoadSabreTrace( const char* fn, LabelImageWrapper* seg );
  void     SaveSabreTrace( const char* fn, LabelImageWrapper* seg );
 
  //       Set the location of the landmark and specify whether it has been drawn
  void     SetLandmark( LandmarkType landmark, Vector3f index, bool isDrawn );

  //       Clears all landmarks from the landmark vector 
  void     ClearAllLandmarks();

  //       Clears grid landmarks from the landmark vector
  void     ClearGridLandmarks();

  //       Remove AC, PC, PE, CA, M, LPRON, RPRON from the label image
  void     RemoveGridLandmarks( LabelImageWrapper *imgLabel );

  //       Clears AC, PC, PE, CA, M, LPRON, RPRON from the landmark vector
  //       AND removes them from the label image
  void     ClearAndRemoveGridLandmarks( LabelImageWrapper *imgLabel );

  //       Returns true if user has placed AC, PC, M, LPRON or RPRON
  bool     SabreGridModified();

  //       Returns true if user has placed RSC or LSC
  //       (Note:  Does not return information about trace landmarks ROP or LOP placed using the line tool)
  bool     SabreTraceModified();
       
  //       Return relevant SABRE X,Y or Z cooridinate for each landmark
  //       (The AC,PC,M,LPRON,RPRON cooridinates are needed for SABRE grid file, the 
  //       remaining are required to determine the AC,PC,M,LPRON,RPRON locations.)
  float    GetAC();    // Anterior Commissure
  float    GetPC();    // Posterior Commisure
  float    GetPE();    // Posterior Edge
  float    GetCA();    // Cererbal Aquaduct
  float    GetAP();    // ACPC slice
  float    GetPON();   // LPRON/RPRON slice
  float    GetM();     // Midline slice
  float    GetLPRON(); // Left parietal occipital notch
  float    GetRPRON(); // Right parieital occipital notch

  irisSetMacro(CurrentLandmarkMode, LandmarkType );
  irisGetMacro(CurrentLandmarkMode, LandmarkType );

  
private:
  struct Landmark
  {
    Vector3f axialIndex;
    bool     isDrawn;
  };

  std::vector< Landmark > m_AllLandmarks;

  LandmarkType m_CurrentLandmarkMode;

  bool m_SabreGridModified;
  bool m_SabreTraceModified;
};

#endif