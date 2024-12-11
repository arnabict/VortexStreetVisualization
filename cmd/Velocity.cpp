#include "Velocity.hpp"
#include <vtkXMLImageDataReader.h>
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include "AmiraWriter.hpp"

namespace vispro
{
	void Velocity::Compute(const char* velocityPath, const char* velocityPathAm)
	{
		// read the file
		vtkNew<vtkXMLImageDataReader> reader;
		reader->SetFileName(velocityPath);
		reader->Update();
		vtkImageData* velocity = reader->GetOutput();

		// write the file
		AmiraWriter::WriteVectorField(velocityPathAm, "u", "v", "w", velocity);
	}
}