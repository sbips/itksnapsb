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
#include "FloodFillLimitImage.h"
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include "FL/fl_ask.H"

FloodFillLimitImage
::FloodFillLimitImage()
{
  m_Image = ImageType::New();
  m_IsInitialized = false;
  m_LastLimitVoxelList.resize( 3 );
  m_ConnectToPreviousNeeded = false;

}

FloodFillLimitImage
::~FloodFillLimitImage()
{

}

void
FloodFillLimitImage
::Initialize( ImageType::ConstPointer img )
{
  if ( m_IsInitialized ) return;

  ImageType::Pointer limitImage = ImageType::New();
  limitImage->SetRegions( img->GetLargestPossibleRegion() );
  limitImage->SetSpacing( img->GetSpacing() );
  limitImage->SetDirection( img->GetDirection() );
  limitImage->Allocate();
  limitImage->FillBuffer( 0 );
  m_Image = limitImage;

  m_IsInitialized = true;
}

void
FloodFillLimitImage
::Reset( ImageType::ConstPointer img )
{
  m_IsInitialized = false;
  Initialize( img );
}


void
FloodFillLimitImage
::SaveLimitImage( const char* fn )
{
  if ( !m_IsInitialized ) return;

  typedef itk::ImageFileWriter< ImageType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput( m_Image );
  writer->SetFileName( fn );
    
  // Try writing limit image
  try
  {
    writer->Update();
  }
  catch( itk::ExceptionObject &exc )
  {
    throw exc;
  }
}

void 
FloodFillLimitImage
::LoadLimitImage( const char* fn )
{

  typedef itk::ImageFileReader< ImageType > ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( fn );
  
  // Try reading limit image
  try
  {
    reader->Update();
  }
  catch( itk::ExceptionObject &exc )
  {
    throw exc;
  }

  if ( reader->GetOutput()->GetLargestPossibleRegion().GetSize() != 
       m_Image->GetLargestPossibleRegion().GetSize() )
  {
    fl_alert( "The dimensions of the limit image you are attempting to load\ndo not match the dimensions of the main image already loaded.\n" );
    return;
  }

  m_Image = reader->GetOutput();
}

void
FloodFillLimitImage
::UndoLastLimit( unsigned int plane, Vector3ui cursor )
{
  ImageType::IndexType pixelIndex;
  std::list< Vector3f >::iterator it;
  for ( it = m_LastLimitVoxelList[ plane ].begin(); it != m_LastLimitVoxelList[ plane ].end(); ++it )
  {
    pixelIndex[0] = it->operator[] (0);
    pixelIndex[1] = it->operator[] (1);
    pixelIndex[2] = it->operator[] (2);
    if ( pixelIndex[ plane ] != cursor[ plane ] ) return;
    m_Image->SetPixel( pixelIndex, 0 );
  }
  m_ConnectToPreviousNeeded = true;
}

void
FloodFillLimitImage
::RedoLastLimit( unsigned int plane, Vector3ui cursor )
{
  ImageType::IndexType pixelIndex;
  std::list< Vector3f >::iterator it;
  for ( it = m_LastLimitVoxelList[ plane ].begin(); it != m_LastLimitVoxelList[ plane ].end(); ++it )
  {
    pixelIndex[0] = it->operator[] (0);
    pixelIndex[1] = it->operator[] (1);
    pixelIndex[2] = it->operator[] (2);
    if ( pixelIndex[ plane ] != cursor[ plane ] ) return;
    m_Image->SetPixel( pixelIndex, 1 );
  }
  m_ConnectToPreviousNeeded = true;
}

void
FloodFillLimitImage
::ClearAllLimits()
{
  itk::ImageRegionIterator< ImageType > limitIt( m_Image, m_Image->GetLargestPossibleRegion() );
  for ( limitIt.GoToBegin(); !limitIt.IsAtEnd(); ++limitIt )
    limitIt.Set( 0 );
}

void
FloodFillLimitImage
::ClearLimitsOnSlice( unsigned int plane, Vector3ui cursor )
{  
  ImageType::SizeType size = m_Image->GetLargestPossibleRegion().GetSize();
  ImageType::IndexType start;

  if ( plane == 2 )
  {
    size[2] = 1;
    start[0] = 0;  start[1] = 0;  start[2] = cursor[2];
  }
  if ( plane == 0 )
  {
    size[0] = 1;

    start[0] = cursor[0];  start[1] = 0;  start[2] = 0;
  }
  if ( plane == 1 )
  {
    size[1] = 1;

    start[0] = 0;   start[1] = cursor[1];  start[2] = 0;
  } 
  
  ImageType::RegionType region;
  region.SetSize( size );
  region.SetIndex( start );
    
  itk::ImageRegionIterator< ImageType > it( m_Image, region );

  for ( it.GoToBegin(); !it.IsAtEnd(); ++it )
    it.Set( 0 );

}