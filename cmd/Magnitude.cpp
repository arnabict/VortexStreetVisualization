#include "Magnitude.hpp"
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkFloatArray.h>
#include "AmiraWriter.hpp"
#include "AmiraReader.hpp"

namespace vispro
{
	void Magnitude::Compute(const char* velocityPath, const char* magnitudePath)
	{
		// read the file
		vtkSmartPointer<vtkImageData> velocityImage = AmiraReader::ReadField(velocityPath, "velocity");
		vtkFloatArray* velocityArray = dynamic_cast<vtkFloatArray*>(velocityImage->GetPointData()->GetArray("velocity"));

		// allocate output field
		vtkNew<vtkImageData> magnitude;
		magnitude->SetDimensions(velocityImage->GetDimensions());
		magnitude->SetOrigin(velocityImage->GetOrigin());
		magnitude->SetSpacing(velocityImage->GetSpacing());
		vtkNew<vtkFloatArray> mArray;
		int64_t numPoints = (int64_t)magnitude->GetDimensions()[0] * magnitude->GetDimensions()[1] * magnitude->GetDimensions()[2];
		mArray->SetNumberOfComponents(1);
		mArray->SetNumberOfTuples(numPoints);
		mArray->SetName("magnitude");
		magnitude->GetPointData()->AddArray(mArray);
		
		// compute the field
#ifdef _DEBUG
#pragma omp parallel
#endif
		for (int64_t i = 0; i < numPoints; ++i) {
			Eigen::Vector3d vel(velocityArray->GetTuple3(i));
			double m = vel.norm();
			mArray->SetValue(i, (float)m);
		}

		// write the file
		AmiraWriter::WriteScalarField(magnitudePath, "magnitude", magnitude);
	}
}