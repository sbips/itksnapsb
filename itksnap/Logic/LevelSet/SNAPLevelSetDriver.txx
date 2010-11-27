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
  Module:    $RCSfile: SNAPLevelSetDriver.txx,v $
  Language:  C++
  Date:      $Date: 2009/09/17 14:46:04 $
  Version:   $Revision: 1.5 $
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
#ifndef __SNAPLevelSetDriver_txx_
#define __SNAPLevelSetDriver_txx_

// Borland compiler is very lazy so we need to instantiate the template
//  by hand 
//#if defined(__BORLANDC__)
//#include <../../../SNAPBorlandDummyTypes.h>
//#endif

#include "SNAPLevelSetDriver.h"
#include "IRISVectorTypesToITKConversion.h"

#include "itkCommand.h"
#include "itkNarrowBandLevelSetImageFilter.h"
#include "itkDenseFiniteDifferenceImageFilter.h"
#include "LevelSetExtensionFilter.h"

#if defined(USE_ITK36_ITK38_SPARSEFIELD_BUGFIX)
#include "itkParallelSparseFieldLevelSetImageFilterBugFix.h"
#else 
#include "itkSparseFieldLevelSetImageFilter.h"
#endif

// Disable some windows debug length messages
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#pragma warning ( disable : 4503 )
#endif

using namespace itk;

// Create an inverting functor
class InvertFunctor {
public:
  unsigned char operator()(unsigned char input) { 
    return input == 0 ? 1 : 0; 
  }  
};

template<unsigned int VDimension>
SNAPLevelSetDriver<VDimension>
::SNAPLevelSetDriver(FloatImageType *init, FloatImageType *speed,
                     const SnakeParameters &sparms,
                     VectorImageType *externalAdvection)
{
  // Create the level set function
  m_LevelSetFunction = LevelSetFunctionType::New();

  // Pass the speed image to the function
  m_LevelSetFunction->SetSpeedImage(speed);  

  // Set the external advection if any
  if(externalAdvection)
    m_LevelSetFunction->SetAdvectionField(externalAdvection);

  // Remember the input and output images for later initialization
  m_InitializationImage = init;

  // Pass the parameters to the level set function
  AssignParametersToPhi(sparms,true);

  // Create the filter
  DoCreateLevelSetFilter();
}

template<unsigned int VDimension>
void 
SNAPLevelSetDriver<VDimension>
::AssignParametersToPhi(const SnakeParameters &p, bool irisNotUsed(firstTime))
{
  // Set up the level set function

  // The sign of the advection term is flipped in our equation
  m_LevelSetFunction->SetAdvectionWeight( - p.GetAdvectionWeight());
  m_LevelSetFunction->SetAdvectionSpeedExponent(p.GetAdvectionSpeedExponent());

  // The curvature exponent for traditional/legacy reasons has a +1 value.
  m_LevelSetFunction->SetCurvatureSpeedExponent(p.GetCurvatureSpeedExponent()+1);  
  m_LevelSetFunction->SetCurvatureWeight(p.GetCurvatureWeight());
  
  m_LevelSetFunction->SetPropagationWeight(p.GetPropagationWeight());
  m_LevelSetFunction->SetPropagationSpeedExponent(p.GetPropagationSpeedExponent());  
  m_LevelSetFunction->SetLaplacianSmoothingWeight(p.GetLaplacianWeight());
  m_LevelSetFunction->SetLaplacianSmoothingSpeedExponent(p.GetLaplacianSpeedExponent());  
  
  // We only need to recompute the internal images if the exponents to those
  // images have changed
  m_LevelSetFunction->CalculateInternalImages();
  
  // Call the initialize method
  typename LevelSetFunctionType::RadiusType radius;
  radius.Fill(1);
  m_LevelSetFunction->Initialize(radius);

  // Set the time step
  m_LevelSetFunction->SetTimeStepFactor(
    p.GetAutomaticTimeStep() ? 1.0 : p.GetTimeStepFactor());

  // Remember the parameters
  m_Parameters = p;
}

template<unsigned int VDimension>
void
SNAPLevelSetDriver<VDimension>
::DoCreateLevelSetFilter()
{
  // In this method we have the flexibility to create a level set filter
  // of any ITK solver type.  This way, we can plug in different solvers:
  // NarrowBand, ParallelSparseField, even Dense.  
  if(m_Parameters.GetSolver() == SnakeParameters::PARALLEL_SPARSE_FIELD_SOLVER)
    {
    // Define an extension to the appropriate filter class
#if defined(USE_ITK36_ITK38_SPARSEFIELD_BUGFIX)
    typedef ParallelSparseFieldLevelSetImageFilterBugFix<
      FloatImageType, FloatImageType> LevelSetFilterType;
#else
    typedef SparseFieldLevelSetImageFilter<
      FloatImageType, FloatImageType> LevelSetFilterType;
#endif

    typedef typename LevelSetFilterType::Pointer LevelSetFilterPointer;
    LevelSetFilterPointer filter = LevelSetFilterType::New();

    // Cast this specific filter down to the lowest common denominator that is
    // a filter
    m_LevelSetFilter = filter.GetPointer();

    // Perform the special configuration tasks on the filter
    filter->SetInput(m_InitializationImage);
    filter->SetNumberOfLayers(3);
    filter->SetIsoSurfaceValue(0.0f);
    filter->SetDifferenceFunction(m_LevelSetFunction);
    }

  else if(m_Parameters.GetSolver() == SnakeParameters::NARROW_BAND_SOLVER)
    {
    // Define an extension to the appropriate filter class
    typedef itk::NarrowBandLevelSetImageFilter<
      FloatImageType,FloatImageType,float,FloatImageType> LevelSetFilterType;
    typedef LevelSetExtensionFilter<LevelSetFilterType> ExtensionFilter;
    typename ExtensionFilter::Pointer filter = ExtensionFilter::New();

    // Cast this specific filter down to the lowest common denominator that is
    // a filter
    m_LevelSetFilter = filter.GetPointer();

    // Perform the special configuration tasks on the filter
    filter->SetSegmentationFunction(m_LevelSetFunction);
    filter->SetInput(m_InitializationImage);
    filter->SetNarrowBandTotalRadius(5);
    filter->SetNarrowBandInnerRadius(3);
    filter->SetFeatureImage(m_LevelSetFunction->GetSpeedImage());  
    }

  else if(m_Parameters.GetSolver() == SnakeParameters::DENSE_SOLVER)
    {
    // Define an extension to the appropriate filter class
    typedef DenseFiniteDifferenceImageFilter<
      FloatImageType,FloatImageType> LevelSetFilterType;
    typedef LevelSetExtensionFilter<LevelSetFilterType> ExtensionFilter;
    typename ExtensionFilter::Pointer filter = ExtensionFilter::New();
    
    // Cast this specific filter down to the lowest common denominator that is
    // a filter
    m_LevelSetFilter = filter.GetPointer();

    // Perform the special configuration tasks on the filter
    filter->SetInput(m_InitializationImage);
    filter->SetDifferenceFunction(m_LevelSetFunction);
    }

  else
    {
    throw itk::ExceptionObject(__FILE__,__LINE__,"Unknown level set solver requested");
    }

  // This code is common to all filters. It causes the filter to initialize
  // the necessary memory and sets the iteration counter to 0
  m_LevelSetFilter->SetManualReinitialization(true);
  m_LevelSetFilter->SetNumberOfIterations(0);
  
  // Update the largest possible region. The slicer may be changing the 
  // requested region on this image, so it's important that we always 
  // update the entire image
  m_LevelSetFilter->UpdateLargestPossibleRegion();
}

template<unsigned int VDimension>
void
SNAPLevelSetDriver<VDimension>
::Restart()
{ 
  // Tell the filter to reinitialize next time that an update will 
  // be performed, and set the number of iterations to 0
  m_LevelSetFilter->SetStateToUninitialized();
  m_LevelSetFilter->SetNumberOfIterations(0);

  // Update the largest possible region. The slicer may be changing the 
  // requested region on this image, so it's important that we always 
  // update the entire image
  m_LevelSetFilter->UpdateLargestPossibleRegion();
}

template<unsigned int VDimension>
void 
SNAPLevelSetDriver<VDimension>
::Run(unsigned int nIterations)
{
  // Increment the number of iterations 
  unsigned int nElapsed = m_LevelSetFilter->GetElapsedIterations();
  m_LevelSetFilter->SetNumberOfIterations(nElapsed + nIterations);
  
  // Update the largest possible region. The slicer may be changing the 
  // requested region on this image, so it's important that we always 
  // update the entire image
  m_LevelSetFilter->UpdateLargestPossibleRegion();
}

template<unsigned int VDimension>
typename SNAPLevelSetDriver<VDimension>::FloatImageType * 
SNAPLevelSetDriver<VDimension>
::GetCurrentState()
{
  // Fix the spacing of the level set filter's output (huh?)
  m_LevelSetFilter->GetOutput()->SetDirection(m_InitializationImage->GetDirection());
  m_LevelSetFilter->GetOutput()->SetSpacing(m_InitializationImage->GetSpacing());
  m_LevelSetFilter->GetOutput()->SetOrigin(m_InitializationImage->GetOrigin());

  // Return the filter's output
  return m_LevelSetFilter->GetOutput();
}

template<unsigned int VDimension>
unsigned int
SNAPLevelSetDriver<VDimension>
::GetElapsedIterations() const
{
  return m_LevelSetFilter->GetElapsedIterations();
}

template<unsigned int VDimension>
void 
SNAPLevelSetDriver<VDimension>
::CleanUp()
{
  // Basically, the filter is finished, and we can finally return 
  // from running the filter.  Let's clear the level set and the 
  // function to free memory
  m_LevelSetFilter = NULL;
  m_LevelSetFunction = NULL;
}

template<unsigned int VDimension>
void
SNAPLevelSetDriver<VDimension>
::SetSnakeParameters(const SnakeParameters &sparms)
{
  // Parameter setting can be destructive or passive.  If the solver has 
  // has changed, then it's destructive, otherwise it's passive
  bool destructive = sparms.GetSolver() != m_Parameters.GetSolver();

  // First of all, pass the parameters to the phi function, which may or
  // may not cause it to recompute it's images
  AssignParametersToPhi(sparms,false);

  // Create a new level set filter
  if(destructive)
    {
    DoCreateLevelSetFilter();
    }
}

#endif

