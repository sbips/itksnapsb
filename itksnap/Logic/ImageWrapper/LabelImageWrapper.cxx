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
  Module:    $RCSfile: LabelImageWrapper.cxx,v $
  Language:  C++
  Date:      $Date: 2009/08/29 23:18:42 $
  Version:   $Revision: 1.7 $
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
#include "ImageWrapper.txx"
#include "ScalarImageWrapper.txx"
#include "LabelImageWrapper.h"
#include "ColorLabel.h"
#include "ColorLabelTable.h"

// Create an instance of ImageWrapper of appropriate type
template class ImageWrapper<LabelType>;
template class ScalarImageWrapper<LabelType>;

LabelImageWrapper
::LabelImageWrapper()
{
  // Instantiate the filters
  for(unsigned int i=0;i<3;i++) 
  {
    m_RGBAFilter[i] = RGBAFilterType::New();
    m_RGBAFilter[i]->SetInput(m_Slicer[i]->GetOutput());
  }

  SetLabelColorTable(NULL);
}

LabelImageWrapper
::LabelImageWrapper(const LabelImageWrapper &source)
: ScalarImageWrapper<LabelType>(source)
{
  // Instantiate the filters
  for(unsigned int i=0;i<3;i++) 
  {
    m_RGBAFilter[i] = RGBAFilterType::New();
    m_RGBAFilter[i]->SetInput(m_Slicer[i]->GetOutput());
  }

  // Initialize the color table as well
  SetLabelColorTable(source.GetLabelColorTable());
}

LabelImageWrapper
::~LabelImageWrapper()
{
  for (size_t i = 0; i < 3; ++i)
    {
    m_RGBAFilter[i] = NULL;
    }
}

ColorLabelTable *
LabelImageWrapper
::GetLabelColorTable() const
{
  return m_RGBAFilter[0]->GetColorTable();
}

void 
LabelImageWrapper
::SetLabelColorTable(ColorLabelTable *table) 
{
  // Set the new table
  for(unsigned int i=0;i<3;i++) 
    m_RGBAFilter[i]->SetColorTable(table);
}

void 
LabelImageWrapper
::UpdateColorMappingCache() 
{
  // Better have a color table
  assert(GetLabelColorTable());

  // Dirty the intensity filters
  for(unsigned int i=0;i<3;i++)
    m_RGBAFilter[i]->Modified();  
}

LabelImageWrapper::DisplaySlicePointer
LabelImageWrapper
::GetDisplaySlice(unsigned int dim) const
{
  return m_RGBAFilter[dim]->GetOutput();
}

/**
 * This definition is needed to use RGBA pixels for compilation
 */
typedef itk::RGBAPixel<unsigned char> ColorPixel;

namespace itk {

template<>
class NumericTraits<ColorPixel>
{
public:
  typedef ColorPixel ValueType;
  typedef ColorPixel PrintType;
  typedef ColorPixel AbsType;
  typedef ColorPixel AccumulateType;
  static const ColorPixel Zero;
  static const ColorPixel One;

  static ColorPixel NonpositiveMin() { return Zero; }
  static bool IsPositive(ColorPixel val) { return true; }
  static bool IsNonpositive(ColorPixel val) { return false; }
  static bool IsNegative(ColorPixel val) { return false; }
  static bool IsNonnegative(ColorPixel val) {return true; }
private:

  static const unsigned char ZeroArray[4];
  static const unsigned char OneArray[4];
};

} // End of namespace

const unsigned char itk::NumericTraits<ColorPixel>::ZeroArray[4] = {0,0,0,0};
const ColorPixel itk::NumericTraits<ColorPixel>::Zero = 
  ColorPixel(itk::NumericTraits<ColorPixel>::ZeroArray);

const unsigned char itk::NumericTraits<ColorPixel>::OneArray[4] = {1,1,1,1};
const ColorPixel itk::NumericTraits<ColorPixel>::One = 
  ColorPixel(itk::NumericTraits<ColorPixel>::OneArray);
