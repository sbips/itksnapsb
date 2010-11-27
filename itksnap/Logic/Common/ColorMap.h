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
  Module:    $RCSfile: ColorMap.h,v $
  Language:  C++
  Date:      $Date: 2009/09/16 20:03:13 $
  Version:   $Revision: 1.6 $
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

#ifndef __ColorMap_h_
#define __ColorMap_h_

#include "itkRGBAPixel.h"
#include "Registry.h"
#include <vector>

/**
 * \class ColorMap
 * \brief This class provides a representation of color maps. 
 * A color map is a mapping from the interval [0,1] to the RGBA space. The
 * mapping is piecewise continuous. Where it is continuous, it is linear.
 *
 * The color map is represented as an ordered list of points. Each point has
 * an index value (between 0 and 1). CMPoints may be continuous (i.e., the color
 * map is continuous at the point) and discontinuous. A continuous point has 
 * one RGBA value, and discontinuous points have two values. In between points,
 * the color map is interpolated linearly.
 */
class ColorMap 
{
  public:

    /** RGBA type */
    typedef unsigned char EltType;
    typedef itk::RGBAPixel<EltType> RGBAType;

    /** Type of point */
    enum CMPointType { CONTINUOUS, DISCONTINUOUS };

    /** System presets */
    enum SystemPreset {
      COLORMAP_GREY = 0, COLORMAP_RED, COLORMAP_GREEN, COLORMAP_BLUE, 
      COLORMAP_HOT, COLORMAP_COOL, COLORMAP_SPRING, COLORMAP_SUMMER, 
      COLORMAP_AUTUMN, COLORMAP_WINTER, COLORMAP_COPPER, COLORMAP_HSV, 
      COLORMAP_JET, COLORMAP_BWR, COLORMAP_RWB, COLORMAP_SIZE
    };

    /** CMPoint representation */
    class CMPoint 
    {
      public:
      
        double m_Index;
        CMPointType m_Type;
        RGBAType m_RGBA[2];

        bool operator==(const CMPoint& rhs) const;

        CMPoint();
      
        // Continuous point
        CMPoint(double, EltType r, EltType g, EltType b, EltType a);

        // CMPoint with alpha discontinuity
        CMPoint(double, EltType r, EltType g, EltType b, EltType a1, EltType a2);

        // CMPoint with full discontinuity
        CMPoint(double, 
          EltType r1, EltType g1, EltType b1, EltType a1, 
          EltType r2, EltType g2, EltType b2, EltType a2);

        // CMPoint copy constructor
        CMPoint(const CMPoint& rhs);
    };

    /** 
     * This method maps a value to an RGBA tuple. Values less than 0 and greater
     * than one are accepted, and are mapped to 0 and 1, respecitively. 
     */
    RGBAType MapIndexToRGBA(double j) const;

    /** This method initializes the color map to one of the system presets */
    void SetToSystemPreset(SystemPreset preset);

    size_t GetNumberOfCMPoints()
      { return m_CMPoints.size(); }

    CMPoint GetCMPoint(size_t j)
      { return m_CMPoints[j]; }

    void UpdateCMPoint(size_t j, const CMPoint &p)
      { m_CMPoints[j] = p; this->UpdateInterpolants(); }

    void DeleteCMPoint(size_t j)
      { m_CMPoints.erase(m_CMPoints.begin() + j); this->UpdateInterpolants(); }

    /** 
     * This method inserts a new control point, which is interpolated
     * using the values of the existing points
     */
    size_t InsertInterpolatedCMPoint(double j);

    bool operator==(const ColorMap& rhs) const;

    SystemPreset GetSystemPreset() const
      { return m_CMPreset; }

    ColorMap();

    ColorMap (SystemPreset preset);

    ColorMap (const ColorMap& rhs);

    void PrintSelf();

    void SaveToRegistry(Registry &reg);
    void LoadFromRegistry(Registry &reg);

  private:

    typedef std::vector<CMPoint> CMPointList;
    typedef CMPointList::iterator CMPointIterator;
    typedef CMPointList::const_iterator CMPointConstIterator;

    // Array of points, which must always be sorted by the index
    CMPointList m_CMPoints;

    // Preset
    SystemPreset m_CMPreset;

    // Recomputes internal interpolation terms
    void UpdateInterpolants();

    struct InterpolantData 
      { float slope[4], intercept[4]; };
    typedef std::vector<InterpolantData> InterpolantVector;
    InterpolantVector m_Interpolants;
};


#endif 


