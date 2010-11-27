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

#ifndef __FloodFillLimitImage_h_
#define __FloodFillLimitImage_h_

#include "GreyImageWrapper.h"

class FloodFillLimitImage
{
private:
  typedef GreyImageWrapper::ImageType ImageType;

public:
  typedef std::vector< std::list< Vector3f > > LimitListType;

public:
  FloodFillLimitImage();
  ~FloodFillLimitImage();

  void Initialize( ImageType::ConstPointer img );
  void Reset( ImageType::ConstPointer img );

  void SaveLimitImage( const char* fn );
  void LoadLimitImage( const char* fn );

  void UndoLastLimit( unsigned int plane, Vector3ui cursor );
  void RedoLastLimit( unsigned int plane, Vector3ui cursor );

  void ClearAllLimits();
  void ClearLimitsOnSlice( unsigned int plane, Vector3ui cursor );

  irisGetMacro(Image, ImageType::Pointer);
  
  LimitListType m_LastLimitVoxelList;

  irisSetMacro( ConnectToPreviousNeeded, bool );
  irisGetMacro( ConnectToPreviousNeeded, bool );

private:
  ImageType::Pointer m_Image;
  bool m_IsInitialized;
  bool m_ConnectToPreviousNeeded;

};

#endif