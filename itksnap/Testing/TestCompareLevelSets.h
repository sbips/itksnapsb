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
   
  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: TestCompareLevelSets.h,v $
  Language:  C++
  Date:      $Date: 2006/12/02 04:22:20 $
  Version:   $Revision: 1.1 $
  Copyright (c) 2003 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.
=========================================================================*/
#ifndef __TestCompareLevelSets_h_
#define __TestCompareLevelSets_h_

#include "SNAPCommon.h"
#include "TestBase.h"

/**
 * This class is used to test the functionality in the ImageWrapper class
 */
class TestCompareLevelSets : public TestBaseOneImage<GreyType>
{
public:
  typedef TestBaseOneImage<GreyType> Superclass;
  
  void PrintUsage();
  void Run();
  void RunExperiment();
    
  const char *GetTestName() 
  { 
    return "CompareLevelSets"; 
  }
  
  const char *GetDescription()
  { 
    return "Compare level set methods"; 
  }

  virtual void ConfigureCommandLineParser(CommandLineArgumentParser &parser)
  {
    Superclass::ConfigureCommandLineParser(parser);
    parser.AddOption("config",1);
    parser.AddOption("generate",0);
  }

private:
  // Progress callback
  void IterationCallback(itk::Object *object, const itk::EventObject &event);
  
  // Interior extraction functor
  class InteriorFunctor { 
  public:
    unsigned char operator()(float input) { 
      return input <= 0.0f ? 255 : 0; 
    }  
  };

  // Image type
  typedef itk::Image<float,3> FloatImageType;

  // Compute volume overlap as (A int B) / (A union B)
  float ComputeOverlapDistance(FloatImageType *i1,FloatImageType *i2);
};

#endif // __TestCompareLevelSets_h_




