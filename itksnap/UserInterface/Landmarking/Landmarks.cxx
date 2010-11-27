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
#include "Landmarks.h"
#include "GlobalState.h"
#include <iostream>
#include <fstream>

#include "itkAnalyzeObjectLabelMapImageIOFactory.h"
#include "itkAnalyzeObjectLabelMapImageIO.h"
#include "itkAnalyzeObjectMap.h"

#include "itkImageFileWriter.h"
#include "itkImageFileReader.h"
#include "itkCastImageFilter.h"
#include <itkImageRegionIteratorWithIndex.h>

#include "LabelImageWrapper.h"

#include "FL/fl_ask.H"

Landmarks
::Landmarks()
: m_SabreGridModified( false ), m_SabreTraceModified( false )
{
  // Create landmark vector 
  int numLandmarks = LandmarkTypeMAX + 1;
  m_AllLandmarks.resize( numLandmarks );
 
  // Initialize landmarks
  ClearAllLandmarks();

  // Set current landmark mode
  m_CurrentLandmarkMode = LandmarkTypeMIN;  
}


void
Landmarks
::LoadSabreGrid( const char* fn )
{
  std::ifstream fin( fn );
  
  // Check that the file can be opened for writing
  if( !fin.good() )
  {
    throw itk::ExceptionObject(__FILE__, __LINE__, "File does not exist or can not be opened");
  }

  // Clear existing grid landmarks
  ClearGridLandmarks();
  
  // Read in landmarks from text file
  std::string landmark;
  int landmarkCooridinate;  
  while ( !fin.eof() )
  {
    fin >> landmark;
 
    if ( landmark == "-ac" ) 
    {
      fin >> landmarkCooridinate;
      SetLandmark( AC, Vector3f( m_AllLandmarks[AC].axialIndex[0],landmarkCooridinate,m_AllLandmarks[AC].axialIndex[2]), false );
    }
    if ( landmark == "-pc" ) 
    {
      fin >> landmarkCooridinate;
      SetLandmark( PC, Vector3f( m_AllLandmarks[PC].axialIndex[0],landmarkCooridinate,m_AllLandmarks[PC].axialIndex[2]), false );
    }
    if ( landmark == "-ap" ) 
    {
      fin >> landmarkCooridinate;
      SetLandmark( AC, Vector3f( m_AllLandmarks[AC].axialIndex[0],m_AllLandmarks[AC].axialIndex[1],landmarkCooridinate), true );
      SetLandmark( PC, Vector3f( m_AllLandmarks[PC].axialIndex[0],m_AllLandmarks[PC].axialIndex[1],landmarkCooridinate), true );
    }
    if ( landmark == "-m" ) 
    {
      fin >> landmarkCooridinate;
      SetLandmark( M, Vector3f( landmarkCooridinate,0,0 ), false );
    }
    if ( landmark == "-lpron" ) 
    {
      fin >> landmarkCooridinate;
      SetLandmark( LPRON, Vector3f( 0,0,landmarkCooridinate), false );
    }
    if ( landmark == "-rpron" ) 
    {
      fin >> landmarkCooridinate;
      SetLandmark( RPRON, Vector3f( 0,0,landmarkCooridinate), false );
    }
    if ( landmark == "-ca" ) 
    {
      fin >> landmarkCooridinate;
      SetLandmark( CA, Vector3f( landmarkCooridinate,0,0 ), false );
    }
    if ( landmark == "-pe" ) 
    {
      fin >> landmarkCooridinate;
      SetLandmark( PE, Vector3f( 0,landmarkCooridinate,0 ), false );
    }
  }

  m_SabreGridModified = false;
}

int 
Landmarks
::SaveSabreGrid( const char* filename )
{
  std::ofstream fout( filename );
  
  // Check that the file can be opened for writing.
  if( !fout.good() )
  {
    throw itk::ExceptionObject(__FILE__, __LINE__, "File can not be opened for writing");
  }

  // Counter for the number of landmarks written to file.
  int landmarksSaved = 0;

  // Save the landmarks required to generate the SABRE lobmask.
  if ( GetAP() != 0 ) { fout << "-ap " << GetAP() << " "; ++landmarksSaved; }
  if ( GetAC() != 0 ) { fout << "-ac " << GetAC() << " "; ++landmarksSaved; }
  if ( GetPC() != 0 ) { fout << "-pc " << GetPC() << " "; ++landmarksSaved; }
  if ( GetM() != 0 )  { fout << "-m "  << GetM()  << " "; ++landmarksSaved; }
  if ( GetRPRON() != 0 ) { fout << "-rpron " << GetLPRON() << " "; ++landmarksSaved; }
  if ( GetLPRON() != 0 ) { fout << "-lpron " << GetRPRON() << " "; ++landmarksSaved; }
  
  // Save secondary landmarks not used to generate the SABRE lobmask.
  if ( GetCA() != 0 ) { fout << "-ca " << GetCA() << " "; }
  if ( GetPE() != 0 ) { fout << "-pe " << GetPE() << " "; }
  
  fout.close();
  
  // Alert user if a required landmark was not written to file.
  if ( landmarksSaved != 6 )  
    fl_alert( "The grid file saved does not contain all 6 required landmarks.\n" );

  m_SabreGridModified = false;

  // Return the number of landmarks written to file.
  return landmarksSaved;
}

void
Landmarks
::LoadSabreTrace( const char* fn, LabelImageWrapper* seg )
{
  typedef unsigned char ObjectPixelType;
  const   unsigned int  Dim = 3;

  typedef itk::Image< ObjectPixelType, Dim >       ObjectImageType;
  typedef itk::ImageFileReader< ObjectImageType >  ReaderType;

  // Register analyze object map factor
  itk::ObjectFactoryBase::RegisterFactory( itk::AnalyzeObjectLabelMapImageIOFactory::New() );

  // Create analyze object map IO
  itk::AnalyzeObjectLabelMapImageIO::Pointer io = itk::AnalyzeObjectLabelMapImageIO::New();

  // Create reader
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( fn );
  reader->SetImageIO( io );

  // Try image read
  try
  {
    reader->Update();
  }
  catch( itk::ExceptionObject &exc )
  {
    throw exc;
  }
  
  if ( reader->GetOutput()->GetLargestPossibleRegion().GetSize().operator []( 0 ) != seg->GetSize(). operator[]( 0 ) || 
       reader->GetOutput()->GetLargestPossibleRegion().GetSize().operator []( 1 ) != seg->GetSize(). operator[]( 1 ) ||
       reader->GetOutput()->GetLargestPossibleRegion().GetSize().operator []( 2 ) != seg->GetSize(). operator[]( 2 ) )
  {
    throw itk::ExceptionObject(__FILE__, __LINE__,"The size of the object map you are attempting to load does not match the size of the main image already loaded." );
  }
  
  // If tracing landmarks RSC or LSC are present in the object map, add to landmark vector
  // (allows users to relocate the landmark no differently than if they had placed it themselves)
  typedef itk::ImageRegionIteratorWithIndex< ObjectImageType > IteratorType;
  IteratorType it( reader->GetOutput(), reader->GetOutput()->GetLargestPossibleRegion() );

  int traceFoundRSC = 0;
  int traceFoundLSC = 0;

  for (it.GoToBegin(); !it.IsAtEnd(); ++it )
  {
    if ( it.Get() == RSC )
    {
      ObjectImageType::IndexType index = it.GetIndex();
      this->SetLandmark( RSC, Vector3f( index[0]+1, index[1]+1, index[2]+1 ), true );
      traceFoundRSC = 1;
    }
    if ( it.Get() == LSC )
    {
      ObjectImageType::IndexType index = it.GetIndex();
      this->SetLandmark( LSC, Vector3f( index[0]+1, index[1]+1, index[2]+1 ), true );
      traceFoundLSC = 1;
    }
    if ( traceFoundRSC + traceFoundLSC == 2 ) 
    { 
      break;
    }
  }

  //  Cast image to the internal image type
  typedef itk::CastImageFilter< ObjectImageType, LabelImageWrapper::ImageType > CastType;
  CastType::Pointer caster = CastType::New();
  caster->SetInput( reader->GetOutput() );
  caster->Update();

  // Update label image
  seg->SetImage( caster->GetOutput() );
  seg->GetImage()->Modified();

  m_SabreTraceModified = false;
}

 void
 Landmarks
 ::SaveSabreTrace( const char* fn, LabelImageWrapper* seg )
 { 
  typedef unsigned char       ObjectPixelType;
  const   unsigned int        Dimension = 3;
  
  typedef itk::Image< ObjectPixelType, Dimension >    ObjectImageType;
  typedef itk::ImageFileWriter< ObjectImageType >  WriterType;

  // Register analyze object map factor
  itk::ObjectFactoryBase::RegisterFactory( itk::AnalyzeObjectLabelMapImageIOFactory::New() );

  // Check for values greater than 255
  typedef itk::ImageRegionIterator< LabelImageWrapper::ImageType > ItType;
  ItType it( seg->GetImage(), seg->GetImage()->GetLargestPossibleRegion() );
  for ( it.GoToBegin(); !it.IsAtEnd(); ++it )
  {
    if ( it.Get() > 255 )
    {
      fl_alert("Can not save tracing because segmentation contains values greater than 255");
    }
  } 

  // Cast segmentation image to unsigned char
  typedef itk::CastImageFilter< LabelImageWrapper::ImageType, ObjectImageType > CastType;
  CastType::Pointer caster = CastType::New();
  caster->SetInput( seg->GetImage() );
  caster->Update();

  // Create object map from segmentation image
  itk::AnalyzeObjectMap< ObjectImageType >::Pointer objectMap = itk::AnalyzeObjectMap< ObjectImageType >::New();
  
  // Allocate object map (same size as segmentation image)
  ObjectImageType::SizeType size;
  for ( unsigned int i=0; i<3; ++i )
    size[i] = seg->GetSize().operator [](i);
  objectMap->SetRegions( size );
  objectMap->Allocate();

  // Add objects
  objectMap->AddObjectEntryBasedOnImagePixel( caster->GetOutput(), RSC, "rsc", 0,255,0);
  objectMap->AddObjectEntryBasedOnImagePixel( caster->GetOutput(), ROP, "rop", 255,0,0);
  objectMap->AddObjectEntryBasedOnImagePixel( caster->GetOutput(), LSC, "lsc", 255,192,10);
  objectMap->AddObjectEntryBasedOnImagePixel( caster->GetOutput(), LOP, "lop", 253,255,0);
  objectMap->AddObjectEntryBasedOnImagePixel( caster->GetOutput(), RSF, "rsf", 108,39,204);
  objectMap->AddObjectEntryBasedOnImagePixel( caster->GetOutput(), RC,  "rc",  255,35,112);
  objectMap->AddObjectEntryBasedOnImagePixel( caster->GetOutput(), LSF, "lsf", 255,0,255);
  objectMap->AddObjectEntryBasedOnImagePixel( caster->GetOutput(), LC,  "lc",  255,104,0);

  // Create analyze object map IO
  itk::AnalyzeObjectLabelMapImageIO::Pointer io = itk::AnalyzeObjectLabelMapImageIO::New();
	
  // Specify number of objects
  io->SetNumberOfObjects( 8 );  

  
  /** NOTE:  -the itkAnalyzeObject* files are community generated and not officially a part of ITK 
              and they need modification to be truly useful (for example, if object 1 and 3 are defined but 
              not object 2, the writer will crash, colors default to black, not exception safe, etc.)*/

  // Write output
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( fn );
  writer->SetImageIO( io );
  writer->SetInput( objectMap );

  // Try writing object map
  try
  {
    writer->Update();
  }
  catch( itk::ExceptionObject &exc )
  {
    throw exc;
  }


  m_SabreTraceModified = false;
 }

void
Landmarks
::SetLandmark(  LandmarkType landmark, Vector3f index, bool isDrawn )
{ 
   // Store landmark
  m_AllLandmarks[ landmark ].axialIndex = index;
  m_AllLandmarks[ landmark ].isDrawn = isDrawn; 

  // If change to sabre tracing landmark was made, set flag
  if ( landmark == RSC || landmark == LSC )
     m_SabreTraceModified = true;
  
  // If change to sabre grid landmark was made, set flag
  if ( landmark == AC || landmark == PC || landmark == M || landmark == LPRON || landmark == RPRON )
    m_SabreGridModified = true;

}

void
Landmarks
::ClearAllLandmarks()
{
  for ( int i = 0; i < m_AllLandmarks.size(); ++i )
  {
    m_AllLandmarks[i].axialIndex[0] = 0;
    m_AllLandmarks[i].axialIndex[1] = 0;
    m_AllLandmarks[i].axialIndex[2] = 0;
    m_AllLandmarks[i].isDrawn = false;
  }
  m_SabreGridModified  = false;
  m_SabreTraceModified = false;
}

void 
Landmarks
::ClearGridLandmarks()
{
  int numGridLandmarks = 7;
  int gridLandmarks[] = {AC,PC,PE,CA,M,LPRON,RPRON};

  for ( int i = 0; i < numGridLandmarks; ++ i )
  {
    m_AllLandmarks[gridLandmarks[i]].axialIndex[0] = 0;
    m_AllLandmarks[gridLandmarks[i]].axialIndex[1] = 0;
    m_AllLandmarks[gridLandmarks[i]].axialIndex[2] = 0;
    m_AllLandmarks[gridLandmarks[i]].isDrawn = false;
  }
  m_SabreGridModified = false;
}

void 
Landmarks
::RemoveGridLandmarks( LabelImageWrapper *imgLabel )
{
  int numGridLandmarks = 7;
  int gridLandmarks[] = {AC,PC,PE,CA,M,LPRON,RPRON};

  for ( int i = 0; i < numGridLandmarks; ++ i )
  {
    if ( m_AllLandmarks[ gridLandmarks[i] ].isDrawn )
    {
      LabelImageWrapper::ImageType::IndexType pixelIndex;
      pixelIndex[0] = m_AllLandmarks[gridLandmarks[i]].axialIndex[0] - 1;
      pixelIndex[1] = m_AllLandmarks[gridLandmarks[i]].axialIndex[1] - 1;
      pixelIndex[2] = m_AllLandmarks[gridLandmarks[i]].axialIndex[2] - 1;
      imgLabel->GetImage()->SetPixel( pixelIndex, 0 );
    }
  }

  imgLabel->GetImage()->Modified();
}

void
Landmarks
::ClearAndRemoveGridLandmarks( LabelImageWrapper *imgLabel )
{
  ClearGridLandmarks();

  RemoveGridLandmarks( imgLabel );

}

bool
Landmarks
::SabreGridModified()
{
  return m_SabreGridModified;
}

bool
Landmarks
::SabreTraceModified()
{
  return m_SabreTraceModified;
}


float
Landmarks
::GetAC()
{
  return m_AllLandmarks[ AC ].axialIndex[1]; 
}

float
Landmarks
::GetPC()
{
 return m_AllLandmarks[ PC ].axialIndex[1]; 
}

float
Landmarks
::GetPE()
{
  return m_AllLandmarks[ PE ].axialIndex[1]; 
}

float
Landmarks
::GetCA()
{
  return m_AllLandmarks[ CA ].axialIndex[0]; 
}

float
Landmarks
::GetAP()
{
  return std::max( m_AllLandmarks[ AC ].axialIndex[2], m_AllLandmarks[ PC ].axialIndex[2] );
}

float 
Landmarks
::GetPON()
{
  if ( m_AllLandmarks[ PE ].isDrawn && m_AllLandmarks[ PC ].isDrawn )
    return (int) ( GetPC() - ( GetPC() - GetPE() ) / 2 + 0.5 );
  else
    return 0;
}

float
Landmarks
::GetM()
{
  return m_AllLandmarks[ M ].axialIndex[0];
}
 
float
Landmarks
::GetLPRON()
{
  return m_AllLandmarks[ LPRON ].axialIndex[2];
}
 
float
Landmarks
::GetRPRON()
{
  return m_AllLandmarks[ RPRON ].axialIndex[2];
}
 

