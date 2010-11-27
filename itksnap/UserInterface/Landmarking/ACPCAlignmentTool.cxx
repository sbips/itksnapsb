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
#include "ACPCAlignmentTool.h"

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkResampleImageFilter.h>
#include <itkImageRegionIterator.h>
#include <itkNearestNeighborInterpolateImageFunction.h>

#include "FL/fl_ask.H"

ACPCAlignmentTool
::ACPCAlignmentTool()
{
  m_Image                = ImageType::New();
  m_Resampler            = ResampleType::New();
  m_LInterpolator        = LInterpolatorType::New();
  m_NInterpolator        = NInterpolatorType::New();
  m_CummulativeTransform = TransformType::New();

  m_IsInterpolate = false;
}

ACPCAlignmentTool
::~ACPCAlignmentTool()
{
}

void
ACPCAlignmentTool
::Initialize( ImageType::Pointer imgGrey )
{
  // Perform deep copy of imgGrey contents to m_Image

  ImageType::Pointer image = ImageType::New();

  ImageType::SizeType size = imgGrey->GetLargestPossibleRegion().GetSize();
  ImageType::IndexType start;
  start[0]=0;start[1]=0;start[2]=0;
  ImageType::RegionType region;
  region.SetSize( size );
  region.SetIndex( start );
  image->SetLargestPossibleRegion( region );
  image->SetBufferedRegion( region );
  image->SetRequestedRegion( region );
  image->SetDirection( imgGrey->GetDirection() );
  image->Allocate(); 

  typedef itk::ImageRegionIterator< ImageType > Iterator;
  Iterator it1( image, image->GetLargestPossibleRegion() );
  Iterator it2( imgGrey, imgGrey->GetLargestPossibleRegion() );

  while ( !it1.IsAtEnd() )
  {
    it1.Set(it2.Get());
    ++it1; ++it2;
  }

  m_Image = image;
  
};

void
ACPCAlignmentTool
::AlignmentChange( int degreesX, int degreesY, int degreesZ, 
                   int shiftX, int shiftY, int shiftZ,
                   Vector3ui cursor, 
                   ImageType::Pointer imgGrey )
{

  // Calculate transform by combining requested rotation and/or translation
  //  change with previous alignment change stored in m_CummulativeTransform. 

  const double pi = 4 * std::atan(1.0);

  TransformType::Pointer rtransform = TransformType::New();
  TransformType::Pointer ttransform = TransformType::New();

  ImageType::SpacingType spacing = m_Image->GetSpacing();
  ImageType::SizeType    size    = m_Image->GetLargestPossibleRegion().GetSize();

  TransformType::CenterType rcenter;
  rcenter[0] =   spacing[0] * size[0] / 2.0;
  rcenter[1] =  -spacing[1] * size[1] / 2.0; 
  rcenter[2] =   spacing[2] * size[2] / 2.0; 
  rtransform->SetCenter( rcenter );
  rtransform->SetRotation( degreesX * pi /180, degreesY * pi /180 , degreesZ * pi /180 );

  // Rotation transform:
  rtransform->Compose( m_CummulativeTransform, false );

  TransformType::TranslationType translation;
  translation[0] = shiftX * spacing[0];
  translation[1] = shiftY * spacing[1];
  translation[2] = shiftZ * spacing[2];

  TransformType::CenterType tcenter;
  tcenter[0] = spacing[0] * size[0] / 2.0;
  tcenter[1] = -spacing[1] * size[1] / 2.0; 
  tcenter[2] = spacing[2] * size[2] / 2.0; 
  ttransform->SetCenter( tcenter );
  
  // Translation transform:
  ttransform->SetTranslation( translation );

  // Final transform for resampling:
  ttransform->Compose( rtransform, false );

  // Configure resampler
  m_Resampler->SetInput( m_Image );
  m_Resampler->SetOutputParametersFromImage( m_Image );
  m_Resampler->SetDefaultPixelValue( 0 );
  m_Resampler->SetTransform( ttransform );

  if ( m_IsInterpolate )
     m_Resampler->SetInterpolator( m_LInterpolator );
  else
    m_Resampler->SetInterpolator( m_NInterpolator );

  // Save transform
  m_CummulativeTransform = ttransform;

  // Perform resampling in the 3 planes visible to user

  ImageType::IndexType start;
  ImageType::SizeType  subsize;;

  //================================================================================//
  // -- AXIAL -- //
  
  // Resample
  subsize = size; subsize[2] = 1; 
  start[0] = 0; start[1] = 0; start[2] = cursor[2]; 
  m_Resampler->SetSize( subsize );
  m_Resampler->SetOutputStartIndex( start );

  try
  {
    m_Resampler->UpdateLargestPossibleRegion();
  }
  catch ( itk::ExceptionObject &exc )
  {
    m_Resampler->ResetPipeline();
    throw exc;
  }

  start[0] = 0; start[1] = 0;  start[2] = cursor[2];
  subsize[0] = size[0]; subsize[1] = size[1];  subsize[2] = size[2]; 

  //  Copy resampled axial plane to imgGrey
  typedef itk::ImageRegionIterator< ImageType > IteratorType;
  IteratorType itO( imgGrey, m_Resampler->GetOutput()->GetLargestPossibleRegion() );
  IteratorType itR( m_Resampler->GetOutput(), m_Resampler->GetOutput()->GetLargestPossibleRegion() );
  while ( !itO.IsAtEnd() )
  {
    itO.Set( itR.Get() );
    ++itO; ++itR;
  }

  //================================================================================//
  // -- CORONAL -- //
  
  // Resample
  subsize = size; subsize[1] = 1; 
  start[0] = 0; start[1] = cursor[1]; start[2] = 0; 
  m_Resampler->SetSize( subsize );
  m_Resampler->SetOutputStartIndex( start );

  try
  {
    m_Resampler->UpdateLargestPossibleRegion();
  }
  catch ( itk::ExceptionObject &exc )
  {
    m_Resampler->ResetPipeline();
    throw exc;
  }

  start[0] = 0; start[1] = cursor[1]; start[2] = 0;
  subsize[0] = size[0]; subsize[1] = size[2];  subsize[2] = size[1];

  // Copy resampled coronal plane to imgGrey
  IteratorType itOc( imgGrey, m_Resampler->GetOutput()->GetLargestPossibleRegion() );
  IteratorType itRc( m_Resampler->GetOutput(), m_Resampler->GetOutput()->GetLargestPossibleRegion() );
  while ( !itOc.IsAtEnd() )
  {
    itOc.Set( itRc.Get() );
    ++itOc; ++itRc;
  }

  //================================================================================//
  // -- SAGITTAL -- //
  
  // Resample
  subsize = size; subsize[0] = 1; 
  start[0] = cursor[0]; start[1] = 0; start[2] = 0; 
  m_Resampler->SetSize( subsize );
  m_Resampler->SetOutputStartIndex( start );

  try
  {
    m_Resampler->UpdateLargestPossibleRegion();
  }
  catch ( itk::ExceptionObject &exc )
  {
    m_Resampler->ResetPipeline();
    throw exc;
  }

  // Copy resampled sagittal plane to imgGrey
  start[0] = cursor[0]; start[1] = 0; start[2] = 0;
  subsize[0] = size[1]; subsize[1] = size[2];  subsize[2] = size[0];

  IteratorType itOs( imgGrey, m_Resampler->GetOutput()->GetLargestPossibleRegion() );
  IteratorType itRs( m_Resampler->GetOutput(), m_Resampler->GetOutput()->GetLargestPossibleRegion() );
  while ( !itOs.IsAtEnd() )
  {
    itOs.Set( itRs.Get() );
    ++itOs; ++itRs;
  }
}

void
ACPCAlignmentTool
::Resample()
{
  ResampleType::Pointer resampler = ResampleType::New();
  resampler->SetInput( m_Image );
  resampler->SetOutputParametersFromImage( m_Image );
  resampler->SetSize( m_Image->GetLargestPossibleRegion().GetSize() );
  resampler->SetDefaultPixelValue( 0 );
  resampler->SetTransform( m_CummulativeTransform );
  resampler->SetInterpolator( m_LInterpolator );
 
  try
  {
    resampler->UpdateLargestPossibleRegion();
  }
  catch ( itk::ExceptionObject &exc )
  {
    throw exc;
  }

  m_Image = resampler->GetOutput();
  SetIdentityTransform(); 
}

vnl_matrix< double >
ACPCAlignmentTool
::GetAnalyzeTransform()
{
  vnl_matrix< double > transform(4,4);  
  TransformType::MatrixType m        = m_CummulativeTransform->GetMatrix();
  TransformType::TranslationType t   = m_CummulativeTransform->GetTranslation();  
  ImageType::SpacingType spacing = m_Image->GetSpacing();
  
  // copy ITK transform to vnl matrix
  // (converting world cooridinates to voxel cooridinates for translation)
  transform[0][0] = m[0][0];
  transform[0][1] = m[0][1];
  transform[0][2] = m[0][2];
  transform[0][3] = t[0]/spacing[0]; // 
  transform[1][0] = m[1][0];
  transform[1][1] = m[1][1];
  transform[1][2] = m[1][2];
  transform[1][3] = t[1]/spacing[1]; //
  transform[2][0] = m[2][0];
  transform[2][1] = m[2][1];
  transform[2][2] = m[2][2];
  transform[2][3] = t[2]/spacing[2]; //
  transform[3][0] = 0;
  transform[3][1] = 0;
  transform[3][2] = 0;
  transform[3][3] = 1;

  // invert
  transform = vnl_matrix_inverse< double >( transform );
 
  // adjust signs
  transform[0][1] *= -1;
  transform[1][0] *= -1;
  transform[1][2] *= -1;
  transform[1][3] *= -1;
  transform[2][1] *= -1;

  // transpose
  transform = transform.transpose();

  return transform;
}


void
ACPCAlignmentTool
::SetIdentityTransform()
{
  m_CummulativeTransform->SetIdentity();
}

void
ACPCAlignmentTool
::SetInverseTransform()
{
  m_CummulativeTransform->GetInverse( m_CummulativeTransform );
}

void 
ACPCAlignmentTool
::SaveTransform( const char* fn )
{
  std::ofstream fout( fn );
  
  // Check that the file can be opened for writing.
  if( !fout.good() )
  {
    throw itk::ExceptionObject(__FILE__, __LINE__, "File can not be opened for writing");
  }

  // Get transform in analyze format 
  // (i.e. the format used by the analyze oblique sections module)
  vnl_matrix< double > matrix(4,4);
  matrix = this->GetAnalyzeTransform();

  // Save transform
  fout.precision( 8 );
  fout.setf( ios::fixed, ios::floatfield );
  
  for ( int i = 0; i < 4; ++i )
  {
    for ( int j = 0; j < 4; ++j )
    {
      fout << matrix[i][j] << " ";
    }
    fout << std::endl;
  }
  fout.close();
}

void ACPCAlignmentTool
::LoadTransform( const char* fn )
{
  std::ifstream fin( fn );
  
  // Check that the file can be opened for writing
  if( !fin.good() )
  {
    throw itk::ExceptionObject(__FILE__, __LINE__, "File does not exist or can not be opened.");
  }
  
  // Load Matrix (analyze format)
  vnl_matrix<float> vnlmatrix(4,4); 
  vnlmatrix.set_identity();
   
  vnl_matrix<float>::iterator it;
  it = vnlmatrix.begin();
 
  int counter  = 0;
  while ( !fin.eof() )
  {
    if ( counter > 16 )
      break;

    fin >> *it;
    ++it;
    ++counter;
  }

  // Notify if the loaded matrix contains less than 16 entries
  if ( counter < 16 )
  {
    throw itk::ExceptionObject(__FILE__, __LINE__, "File does not contain the 16 entries required to define an alignment change.");
  }

  // Convert analyze transform to itk euler transform (a to d below)
  ImageType::SpacingType spacing = m_Image->GetSpacing();

  // a) Transpose
  vnlmatrix = vnlmatrix.transpose();

  // b) Adjust signs
  vnlmatrix[0][1] *= -1;
  vnlmatrix[1][0] *= -1;
  vnlmatrix[1][2] *= -1;
  vnlmatrix[1][3] *= -1;
  vnlmatrix[2][1] *= -1;  
  
  // c) Invert
  vnlmatrix = vnl_matrix_inverse< float >(vnlmatrix).inverse();
  
  // d) Convert translation parameters from voxel to world cooridinates
  for ( int i = 0; i < 3; ++ i )
  {
    vnlmatrix[i][3] *= spacing[i];
  }

  // Create new transform
  /* Unfortunately, we can not use the SetMatrix function and directly update the matrix.
     Instead we must calculate the Euler angles from rotation matrix, and update the transform
     by updating the rotation angles.  Attempts to directly update the matrix failed because
     IsMatrixOrthogonal returned false at the precision required by the transform class. */

  // Calculate Euler angles from the transform matrix (vnlmatrix)
  float angleX, angleY, angleZ;

  if( m_CummulativeTransform->GetComputeZYX() )
  {
    angleY = -vcl_asin(vnlmatrix[2][0]);
    double C = vcl_cos(angleY);
    if(vcl_fabs(C)>0.00005)
    {
      double x = vnlmatrix[2][2] / C;
      double y = vnlmatrix[2][1] / C;
      angleX = vcl_atan2(y,x);
      x = vnlmatrix[0][0] / C;
      y = vnlmatrix[1][0] / C;
      angleZ = vcl_atan2(y,x);
    }
   else
    {
      angleX = 0;
      double x = vnlmatrix[1][1];
      double y = -vnlmatrix[0][1];
      angleZ = vcl_atan2(y,x);
    }
  }
  else
  {
    angleX = vcl_asin(vnlmatrix[2][1]);
    double A = vcl_cos(angleX);
    if(vcl_fabs(A)>0.00005)
    {
      double x = vnlmatrix[2][2] / A;
      double y = -vnlmatrix[2][0] / A;
      angleY = vcl_atan2(y,x);

      x = vnlmatrix[1][1] / A;
      y = -vnlmatrix[0][1] / A;
      angleZ = vcl_atan2(y,x);
  }
  else
    {
    angleZ = 0;
    double x = vnlmatrix[0][0];
    double y = vnlmatrix[1][0];
    angleY = vcl_atan2(y,x);
    }
  }

  // Apply Euler angles to the transform
  m_CummulativeTransform->SetRotation( angleX, angleY, angleZ );

  // Apply translation to the transform
  TransformType::TranslationType translation;
  translation[0] = vnlmatrix[0][3];
  translation[1] = vnlmatrix[1][3];
  translation[2] = vnlmatrix[2][3];
  m_CummulativeTransform->SetTranslation( translation ); 
}

