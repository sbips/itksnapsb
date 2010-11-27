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

#include "GuidedMeshIO.h"
#include "vtkPolyDataWriter.h"
#include "vtkSTLWriter.h"
#include "vtkBYUWriter.h"
#include "vtkTriangleFilter.h"
#include "itkExceptionObject.h"

GuidedMeshIO
::GuidedMeshIO()
{
  m_EnumFileFormat.AddPair(FORMAT_VTK, "VTK Mesh");
  m_EnumFileFormat.AddPair(FORMAT_BYU, "BYU Mesh");
  m_EnumFileFormat.AddPair(FORMAT_STL, "STL Mesh"); 
  m_EnumFileFormat.AddPair(FORMAT_COUNT, "INVALID FORMAT");
}


GuidedMeshIO::FileFormat 
GuidedMeshIO
::GetFileFormat(Registry &folder, FileFormat dflt)
{
  return folder.Entry("Format").GetEnum(m_EnumFileFormat, dflt);  
}

void GuidedMeshIO
::SetFileFormat(Registry &folder, FileFormat format)
{
  folder.Entry("Format").PutEnum(m_EnumFileFormat, format);
}

void
GuidedMeshIO
::SaveMesh(const char *FileName, Registry &folder, vtkPolyData *mesh)
{
  // Read the format specification from the registry folder
  FileFormat format = GetFileFormat(folder);

  // Create the appropriate mesh writer for the format
  if(format == FORMAT_VTK)
    {
    vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
    writer->SetInput(mesh);
    writer->SetFileName(FileName);
    writer->Update();
    writer->Delete();
    }
  else if(format == FORMAT_STL)
    {
    vtkTriangleFilter *tri = vtkTriangleFilter::New();
    vtkSTLWriter *writer = vtkSTLWriter::New();
    tri->SetInput(mesh);
    writer->SetInput(tri->GetOutput());
    writer->SetFileName(FileName);
    writer->Update();
    writer->Delete();
    tri->Delete();
    }
  else if(format == FORMAT_BYU)
    {
    vtkTriangleFilter *tri = vtkTriangleFilter::New();
    vtkBYUWriter *writer = vtkBYUWriter::New();
    tri->SetInput(mesh);
    writer->SetInput(tri->GetOutput());
    writer->SetFileName(FileName);
    writer->Update();
    writer->Delete();
    tri->Delete();
    }
  else 
    throw itk::ExceptionObject("Illegal format specified for saving image");
}
