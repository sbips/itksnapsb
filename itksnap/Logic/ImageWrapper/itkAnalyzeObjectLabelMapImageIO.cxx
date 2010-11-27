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
Module:    $RCSfile: itkAnalyzeObjectLabelMapImageIO.cxx,v $
Language:  C++
Date:      $Date: 2007/03/29 18:39:28 $
Version:   $Revision: 1.32 $

Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "itkAnalyzeObjectLabelMapImageIO.h"
#include "itkIOCommon.h"
#include "itkExceptionObject.h"
#include "itkByteSwapper.h"
#include "itkMetaDataObject.h"
#include "itkSpatialOrientationAdapter.h"
#include <itksys/SystemTools.hxx>
#include <vnl/vnl_math.h>
#include "itk_zlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>

namespace itk
{
  //Streaming not yet supported, so use the default base class to return the LargestPossibleRegion
#if _USE_STREAMABLE_REGION_FOR_AOLM
ImageIORegion AnalyzeObjectLabelMapImageIO
::GenerateStreamableReadRegionFromRequestedRegion( const ImageIORegion & requestedRegion ) const
{
  std::cout << "AnalyzeObjectLabelMapImageIO::GenerateStreamableReadRegionFromRequestedRegion() " << std::endl;
  std::cout << "RequestedRegion = " << requestedRegion << std::endl;
  return requestedRegion;
}
#endif

AnalyzeObjectLabelMapImageIO::AnalyzeObjectLabelMapImageIO()
{
  m_NumberOfObjects = 255;
}

AnalyzeObjectLabelMapImageIO::~AnalyzeObjectLabelMapImageIO()
{
}

void AnalyzeObjectLabelMapImageIO::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
}

bool AnalyzeObjectLabelMapImageIO::CanWriteFile(const char * FileNameToWrite)
{
    // This assumes that the final '.' in a file name is the delimiter
    // for the file's extension type
    std::string filename = FileNameToWrite;
    if(  filename == "" )
    {
    itkDebugMacro(<<"No filename specified.");
    return false;
    }
    const std::string::size_type it = filename.find_last_of( "." );
    // Now we have the index of the extension, make a new string
    // that extends from the first letter of the extension to the end of filename
    std::string fileExt( filename, it, filename.length() );
    if (fileExt != ".obj")
    {
      return false;
    }

    return true;
}


void AnalyzeObjectLabelMapImageIO::Read(void* buffer)
{  
     m_InputFileStream.open(m_FileName.c_str(), std::ios::binary | std::ios::in);
     m_InputFileStream.seekg( m_LocationOfFile);
    if ( ! m_InputFileStream.is_open())
    {
      itkDebugMacro(<< "Error: Could not open "<< m_FileName.c_str());
      throw itk::ExceptionObject(__FILE__, __LINE__, "File can not be opened for reading");
      exit(-1);
    }
    //TODO: Image spacing needs fixing.  Will need to look to see if a 
    //      .nii, .nii.gz, or a .hdr file
    //      exists for the same .obj file.
    //      If so, then read in the spacing for those images.

    //When this function decods the run length encoded raw data into an unsigned char volume
    //store the values into this structure.
    struct RunLengthStruct {
        unsigned char voxel_count;
        unsigned char voxel_value;
    };
    typedef struct RunLengthStruct RunLengthElement;
    RunLengthElement RunLengthArray[NumberOfRunLengthElementsPerRead];

    // The file consists of unsigned character pairs which represents the encoding of the data
    // The character pairs have the form of length, tag value.  Note also that the data in
    // Analyze object files are run length encoded a plane at a time.

    int index=0;
    int voxel_count_sum=0;
    unsigned char *tobuf = (unsigned char *)buffer;


    int VolumeSize;
    if(this->GetNumberOfDimensions() >3)
    {
      VolumeSize = this->GetDimensions(0) * this->GetDimensions(1) * this->GetDimensions(2) *this->GetDimensions(3);
    }
  else if(this->GetNumberOfDimensions() == 3)
    {
      VolumeSize = this->GetDimensions(0) * this->GetDimensions(1) * this->GetDimensions(2);
    }
  else if(this->GetNumberOfDimensions() == 2)
    {
      VolumeSize = this->GetDimensions(0) * this->GetDimensions(1);
    }
  else
    {
    VolumeSize = this->GetDimensions(0);
    }

  while (! m_InputFileStream.read(reinterpret_cast<char *>(RunLengthArray), sizeof(RunLengthElement)*NumberOfRunLengthElementsPerRead).eof())
  {
    for (int i = 0; i < NumberOfRunLengthElementsPerRead; i++)
    {
    if(RunLengthArray[i].voxel_count == 0)
      {
      itkDebugMacro(<<"Inside AnaylzeObjectLabelMap Invalid Length "<<(int)RunLengthArray[i].voxel_count<<std::endl);
      throw itk::ExceptionObject(__FILE__, __LINE__, "Invalid length");
      exit(-1);
      }
    for (int j = 0; j < RunLengthArray[i].voxel_count; j++)
      {
      tobuf[index] = RunLengthArray[i].voxel_value;
      index++;
      }
     voxel_count_sum += RunLengthArray[i].voxel_count;
 
     if ( index > VolumeSize )
        {
        itkDebugMacro(<<"BREAK!\n");
        throw itk::ExceptionObject(__FILE__, __LINE__, "Index greater than volume size");
        exit(-1);
        }
     }
  }

        
    if (index != VolumeSize)
    {
      itkDebugMacro(<< "Warning: Error decoding run-length encoding."<<std::endl);
      if(index < VolumeSize)
      {
        itkDebugMacro(<<"Warning: file underrun."<<std::endl);
      }
      else
      {
        itkDebugMacro(<<"Warning: file overrun."<<std::endl);
      }
      throw itk::ExceptionObject(__FILE__, __LINE__, "Error decoding run-length encoding");
      exit(-1);
    }

     m_InputFileStream.close();


}


bool AnalyzeObjectLabelMapImageIO::CanReadFile( const char* FileNameToRead )
{

    std::string filename = FileNameToRead;
    // This assumes that the final '.' in a file name is the delimiter
    // for the file's extension type
    const std::string::size_type it = filename.find_last_of( "." );
    // Now we have the index of the extension, make a new string
    // that extends from the first letter of the extension to the end of filename
    std::string fileExt( filename, it, filename.length() );
    if (fileExt != ".obj")
    {
      return false;
    }

    return true;
}

                              
    
void AnalyzeObjectLabelMapImageIO::ReadImageInformation()
{
    m_ComponentType = CHAR;
    m_PixelType = SCALAR;
    // Opening the file
    std::ifstream  m_InputFileStream;
     m_InputFileStream.open(m_FileName.c_str(), std::ios::binary | std::ios::in);
    if ( ! m_InputFileStream.is_open())
    {
      itkDebugMacro(<< "Error: Could not open: "<< m_FileName.c_str()<<std::endl);
      throw itk::ExceptionObject(__FILE__, __LINE__, "File can not be opened for reading");
      exit(-1);
    }

    // Reading the header, which contains the version number, the size, and the
    // number of objects
    bool NeedByteSwap=false;
    
    int header[6] = {1};
    if (  m_InputFileStream.read(reinterpret_cast<char *>(header),sizeof(int)*5).fail())
    {
      itkDebugMacro(<<"Error: Could not read header of "<<m_FileName.c_str()<<std::endl);
      throw itk::ExceptionObject(__FILE__, __LINE__, "Could not read header");
      exit(-1);
    }
    //Do byte swapping if necessary.
    if(header[0] == -1913442047|| header[0] ==1323699456 )    // Byte swapping needed (Number is byte swapped number of VERSION7)
    {
      NeedByteSwap = true;
      //NOTE: All analyze object maps should be big endian on disk in order to be valid
      //      The following function calls will swap the bytes when on little endian machines.
      itk::ByteSwapper<int>::SwapFromSystemToBigEndian(&(header[0]));
      itk::ByteSwapper<int>::SwapFromSystemToBigEndian(&(header[1]));
      itk::ByteSwapper<int>::SwapFromSystemToBigEndian(&(header[2]));
      itk::ByteSwapper<int>::SwapFromSystemToBigEndian(&(header[3]));
      itk::ByteSwapper<int>::SwapFromSystemToBigEndian(&(header[4]));
    }

    bool NeedBlendFactor = false;
    if(header[0] == VERSION7 )
    {
      if ( ( m_InputFileStream.read(reinterpret_cast<char *>(&(header[5])),sizeof(int)*1)).fail() )
      {
        itkDebugMacro(<<"Error: Could not read header of "<< m_FileName.c_str()<<std::endl);
        throw itk::ExceptionObject(__FILE__, __LINE__, "Could not read header");
        exit(-1);
      }

      if(NeedByteSwap)
      {
          itk::ByteSwapper<int>::SwapFromSystemToBigEndian(&(header[5]));
      }
      NeedBlendFactor = true;
    }
    //Now the file pointer is pointing to the image region
    
    if(header[5] >1 )
    {
    this->SetNumberOfDimensions(4);
    }
  else if(header[3] >1 )
    {
    this->SetNumberOfDimensions(3);
    }
  else if(header[2] >1 )
    {
    this->SetNumberOfDimensions(2);
    }
  else
    {
    this->SetNumberOfDimensions(1);
    }

  switch(this->GetNumberOfDimensions())
    {
    case 4:
      this->SetDimensions(3,header[5]);
      this->SetSpacing(3, 1);
    case 3:
      this->SetDimensions(2,header[3]);
      this->SetSpacing(2, 1);
    case 2:
      this->SetDimensions(1,header[2]);
      this->SetSpacing(1,1);
    case 1:
      this->SetDimensions(0, header[1]);
      this->SetSpacing(0,1);
    default:
      break;
    }


    std::vector<double> dirx(this->GetNumberOfDimensions(),0), diry(this->GetNumberOfDimensions(),0), dirz(this->GetNumberOfDimensions(),0);
    switch(this->GetNumberOfDimensions())
    {
    case 4:
      m_Origin[3] = 0;
    case 3:
      dirx[2] = 0;
      diry[2] = 0;
      dirz[2] = 1;

      dirz[1] = 0;

      dirz[0] = 0;

      m_Origin[2] = 0;
    case 2:
      dirx[1] = 0;
      diry[1] = 1;
      diry[0] = 0;

      m_Origin[1] = 0;
    case 1:
      dirx[0] = 1;
      m_Origin[0] = 0;
       
      break;
    default:
      itkDebugMacro(<<"Error:  Setting the steps has an error"<<std::endl);
      break;
    }

  this->SetDirection(0,dirx);
  if(this->GetNumberOfDimensions() > 1)
    {
    this->SetDirection(1,diry);
    }
  
  if(this->GetNumberOfDimensions() > 2)
    {
    this->SetDirection(2,dirz);
    }
    

  // Error checking the number of objects in the object file
    if ((header[4] < 1) || (header[4] > 256))
    {
    itkDebugMacro(<< "Error: Invalid number of object files.\n");
       m_InputFileStream.close();
      throw itk::ExceptionObject(__FILE__, __LINE__, "Invalid number of objects in file");
      exit(-1);
    }

    /*std::ofstream myfile;
    myfile.open("ReadFromFilePointer35.txt", myfile.app);*/
    itk::AnalyzeObjectEntryArrayType my_reference;
    (my_reference).resize(header[4]);
    for (int i = 0; i < header[4]; i++)
    {
      // Allocating a object to be created
      (my_reference)[i] = AnalyzeObjectEntry::New();
      (my_reference)[i]->ReadFromFilePointer( m_InputFileStream,NeedByteSwap, NeedBlendFactor);
      //(*my_reference)[i]->Print(myfile);
    }
    //myfile.close();
     m_LocationOfFile =  m_InputFileStream.tellg();
     m_InputFileStream.close();
    //Now fill out the MetaData
    MetaDataDictionary &thisDic=this->GetMetaDataDictionary();
    EncapsulateMetaData<std::string>(thisDic,ITK_OnDiskStorageTypeName,std::string(typeid(unsigned char).name()));
    EncapsulateMetaData<itk::AnalyzeObjectEntryArrayType>(thisDic,ANALYZE_OBJECT_LABEL_MAP_ENTRY_ARRAY,my_reference);
}

/**
 *
 */
void
AnalyzeObjectLabelMapImageIO
::WriteImageInformation(void)
{
  itkDebugMacro(<<"I am in the writeimageinformaton"<<std::endl);
  std::string tempfilename = this->GetFileName();
  // Opening the file
    std::ofstream outputFileStream;
    outputFileStream.open(tempfilename.c_str(), std::ios::binary | std::ios::out | std::ios::trunc);
    if ( !outputFileStream.is_open())
    {
      itkDebugMacro(<<"Error: Could not open "<< tempfilename.c_str()<<std::endl);
      throw itk::ExceptionObject(__FILE__, __LINE__, "File can not be opened for writing");
      exit(-1);
    }
   
    int header[5] = {1,1,1,1,1};
    header[0]=VERSION6;
    switch(this->GetNumberOfDimensions())
    {
		case 3:
        header[3] = this->GetDimensions(2);
    case 2:
        header[2] = this->GetDimensions(1);
    case 1:
        header[1] = this->GetDimensions(0);
        break;
    default:
        itkDebugMacro(<<"There is an error in writing the header for the Dimensions");
        throw itk::ExceptionObject(__FILE__, __LINE__, "Error writing dimensions to header");
        exit(-1);
    }

    header[4] = m_NumberOfObjects + 1;

    //All object maps are written in BigEndian format as required by the AnalyzeObjectMap documentation.
    bool NeedByteSwap=itk::ByteSwapper<int>::SystemIsLittleEndian();
    if(NeedByteSwap)
    {
    itk::ByteSwapper<int>::SwapFromSystemToBigEndian(&(header[0]));
    itk::ByteSwapper<int>::SwapFromSystemToBigEndian(&(header[1]));
    itk::ByteSwapper<int>::SwapFromSystemToBigEndian(&(header[2]));
    itk::ByteSwapper<int>::SwapFromSystemToBigEndian(&(header[3]));
    itk::ByteSwapper<int>::SwapFromSystemToBigEndian(&(header[4]));
    //itk::ByteSwapper<int>::SwapFromSystemToBigEndian(&(header[5]));
    }

    // Writing the header, which contains the version number, the size, and the
    // number of objects
    if(outputFileStream.write(reinterpret_cast<char *>(header), sizeof(int)*5).fail())
    {
      itkDebugMacro(<<"Error: Could not write header of "<< tempfilename.c_str()<<std::endl);
      throw itk::ExceptionObject(__FILE__, __LINE__, "Error writing info to header");
      exit(-1);
    }

    itk::AnalyzeObjectEntryArrayType my_reference;
    itk::ExposeMetaData<itk::AnalyzeObjectEntryArrayType>(this->GetMetaDataDictionary(),ANALYZE_OBJECT_LABEL_MAP_ENTRY_ARRAY, my_reference);

    for (unsigned int i = 0; i < m_NumberOfObjects + 1; i++)
      {
      // Using a temporary so that the object file is always written in BIG_ENDIAN mode but does
      // not affect the current object itself
      AnalyzeObjectEntry::Pointer ObjectWrite = AnalyzeObjectEntry::New();

      ObjectWrite->SetName( my_reference[i]->GetName() );
      ObjectWrite->SetEndRed( my_reference[i]->GetEndRed() );
      ObjectWrite->SetEndGreen( my_reference[i]->GetEndGreen() );
      ObjectWrite->SetEndBlue( my_reference[i]->GetEndBlue() );    
      if ( i == 0 )
      {
        ObjectWrite->SetShades( 64 );
        ObjectWrite->SetEndRed( 255 );
        ObjectWrite->SetEndGreen( 255 );
        ObjectWrite->SetEndBlue( 255 );
      }
      else
      {
        ObjectWrite->SetShades( 16 );
      }
            

      if (NeedByteSwap == true)
        {
        ObjectWrite->SwapObjectEndedness();
        }
      ObjectWrite->Write(outputFileStream); 

      }
    

  outputFileStream.close();
}


/**
 *
 */
void
AnalyzeObjectLabelMapImageIO

::Write( const void* buffer)
{

  if(this->GetComponentType() != UCHAR)
  {
    std::cerr<<"Error: The pixel type needs to be an unsigned char."<<std::endl;
    throw itk::ExceptionObject(__FILE__, __LINE__, "Pixel type must be unsigned char");
    exit(-1);
  }
  this->WriteImageInformation();
  std::string tempfilename = this->GetFileName();
  // Opening the file
    std::ofstream outputFileStream;
    outputFileStream.open(tempfilename.c_str(), std::ios::binary | std::ios::out | std::ios::app);
    if ( !outputFileStream.is_open())
    {
      itkDebugMacro(<<"Error: Could not open "<< tempfilename.c_str()<<std::endl);
      throw itk::ExceptionObject(__FILE__, __LINE__, "File can not be opened for writing");
      exit(-1);
    }
    itk::AnalyzeObjectEntryArrayType my_reference;
    itk::ExposeMetaData<itk::AnalyzeObjectEntryArrayType>(this->GetMetaDataDictionary(),ANALYZE_OBJECT_LABEL_MAP_ENTRY_ARRAY, my_reference);
  // Encoding the run length encoded raw data into an unsigned char volume
    int VolumeSize;
    if(this->GetNumberOfDimensions() >3)
    {
      VolumeSize = this->GetDimensions(0) * this->GetDimensions(1) * this->GetDimensions(2) *this->GetDimensions(3);
    }
  else if(this->GetNumberOfDimensions() == 3)
    {
      VolumeSize = this->GetDimensions(0) * this->GetDimensions(1) * this->GetDimensions(2);
    }
  else if(this->GetNumberOfDimensions() == 2)
    {
      VolumeSize = this->GetDimensions(0) * this->GetDimensions(1);
    }
  else
    {
    VolumeSize = this->GetDimensions(0);
    }
  int PlaneSize;
  if(this->GetNumberOfDimensions() > 1)
    {
    PlaneSize = this->GetDimensions(0)*this->GetDimensions(1);
    }
  else
    {
    PlaneSize = this->GetDimensions(0);
    }
  int bufferindex=0;
  int planeindex=0;
  int runlength=0;
  unsigned char CurrentObjIndex=0;
  const  int buffer_size=16384;      //NOTE: This is probably overkill, but it will work
  unsigned char bufferObjectMap[buffer_size] = {0};

  unsigned char *bufferChar = (unsigned char *)buffer;

  for(int i=0;i<VolumeSize;i++)
    {
      if (runlength==0)
      {
          CurrentObjIndex = bufferChar[i];
          runlength=1;
      }
      else
      {
        if (CurrentObjIndex==bufferChar[i])
        {
          runlength++;
          if (runlength==255)
          {
            bufferObjectMap[bufferindex]=runlength;
            bufferObjectMap[bufferindex+1]=CurrentObjIndex;
            bufferindex += 2;
            runlength=0;
          }
        }
        else
        {
          bufferObjectMap[bufferindex]=runlength;
          bufferObjectMap[bufferindex+1]=CurrentObjIndex;
          bufferindex += 2;
          CurrentObjIndex=bufferChar[i];
          runlength=1;
        }
      }

      planeindex++;
      if (planeindex==PlaneSize)
      {
        // Just finished with a plane of data, so encode it
        planeindex=0;
        if (runlength!=0)
        {
          bufferObjectMap[bufferindex]=runlength;
          bufferObjectMap[bufferindex+1]=CurrentObjIndex;
          bufferindex += 2;
          runlength=0;
        }
      }
      if (bufferindex==buffer_size)
      {
        // buffer full
        if (outputFileStream.write(reinterpret_cast<char *>(bufferObjectMap), buffer_size).fail())
        {
            itkDebugMacro(<<"error: could not write buffer"<<std::endl);
            throw itk::ExceptionObject(__FILE__, __LINE__, "Can not write buffer");
            exit(-1);
        }
        bufferindex=0;
      }

    }
    if (bufferindex!=0)
    {
      if (runlength!=0)
      {
        bufferObjectMap[bufferindex]=runlength;
        bufferObjectMap[bufferindex+1]=CurrentObjIndex;
        bufferindex += 2;
      }
      if(outputFileStream.write(reinterpret_cast<char *>(bufferObjectMap), bufferindex).fail())
      {
      itkDebugMacro(<<"error: could not write buffer"<<std::endl);
      throw itk::ExceptionObject(__FILE__, __LINE__, "Can not write buffer");
      exit(-1);
      }
    }
}

} // end namespace itk
