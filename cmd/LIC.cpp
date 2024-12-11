#include "LIC.hpp"
#include "AmiraReader.hpp"
#include "AmiraWriter.hpp"
#include "Sampling.hpp"
#include <vtkImageData.h>
#include <vtkSmartPointer.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <random>

namespace vispro
{
	void LineIntegralConvolution::Compute(const char* velocityPath, const char* licPath, double stepSize, int numAdvectionSteps)
	{
		// read the file
		vtkSmartPointer<vtkImageData> velocityImage = AmiraReader::ReadField(velocityPath, "velocity");
		int* res = velocityImage->GetDimensions();

		// create noise field
		vtkNew<vtkImageData> noiseImage;
		noiseImage->SetDimensions(velocityImage->GetDimensions());
		noiseImage->SetOrigin(velocityImage->GetOrigin());
		noiseImage->SetSpacing(velocityImage->GetSpacing());
		vtkNew<vtkFloatArray> noiseArray;
		noiseArray->SetNumberOfComponents(1);
		noiseArray->SetNumberOfTuples((int64_t)res[0] * res[1] * res[2]);
		noiseArray->SetName("noise");
		noiseImage->GetPointData()->AddArray(noiseArray);
		std::default_random_engine rng;
		std::uniform_real_distribution<double> rnd;
		for (int64_t i = 0; i < res[0] * res[1] * res[2]; ++i)
			noiseArray->SetTuple1(i, rnd(rng));

		// allocate output field
		vtkNew<vtkImageData> licImage;
		licImage->SetDimensions(velocityImage->GetDimensions());
		licImage->SetOrigin(velocityImage->GetOrigin());
		licImage->SetSpacing(velocityImage->GetSpacing());
		vtkNew<vtkFloatArray> licArray;
		licArray->SetNumberOfComponents(1);
		licArray->SetNumberOfTuples((int64_t)res[0] * res[1] * res[2]);
		licArray->SetName("lic");
		licImage->GetPointData()->AddArray(licArray);

		// compute the field
		Eigen::Vector3d origin(licImage->GetOrigin());
		Eigen::Vector3d spacing(licImage->GetSpacing());
		Eigen::AlignedBox3d bounds(origin, origin + spacing.cwiseProduct(Eigen::Vector3d(res[0] - 1, res[1] - 1, res[2] - 1)));
		for (int iz = 0; iz < res[2]; ++iz) {
			for (int iy = 0; iy < res[1]; ++iy) {
				for (int ix = 0; ix < res[0]; ++ix) {
					double sum = 0, count = 0;
					// forward tracing
					Eigen::Vector3d pos = origin + Eigen::Vector3d(ix, iy, iz).cwiseProduct(spacing);
					bool indomain = true;
					for (int istep = 0; istep < numAdvectionSteps; ++istep) {
						Eigen::Vector3d prevPos = pos;
						Advect(pos, indomain, stepSize, velocityImage, bounds);
						if (indomain) {
							double weight = (pos - prevPos).norm();
							sum += Sampling::LinearSample1(pos, noiseImage) * weight;
							count += weight;
						}
						else break;
					}
					// backward tracing
					pos = origin + Eigen::Vector3d(ix, iy, iz).cwiseProduct(spacing);
					indomain = true;
					for (int istep = 0; istep < numAdvectionSteps; ++istep) {
						Eigen::Vector3d prevPos = pos;
						Advect(pos, indomain, -stepSize, velocityImage, bounds);
						if (indomain) {
							double weight = (pos - prevPos).norm();
							sum += Sampling::LinearSample1(pos, noiseImage) * weight;
							count += weight;
						}
						else break;
					}
					// compute weighted average
					if (count > 0)
						sum /= count;
					licArray->SetValue(((int64_t)iz * res[1] + iy) * res[0] + ix, sum);
				}
			}
		}

		// write the file
		AmiraWriter::WriteScalarField(licPath, "lic", licImage);
	}

	Eigen::Vector3d LineIntegralConvolution::Sample(const Eigen::Vector3d& pos, bool& indomain, vtkImageData* velocity, const Eigen::AlignedBox3d& bounds) {
		indomain &= bounds.contains(pos);
		if (indomain)
			return Sampling::LinearSample3(pos, velocity);
		else return Eigen::Vector3d(0, 0, 0);
	}

	void LineIntegralConvolution::Advect(Eigen::Vector3d& pos, bool& indomain, double stepSize, vtkImageData* velocity, const Eigen::AlignedBox3d& bounds) {
		Eigen::Vector3d k1 = Sample(pos, indomain, velocity, bounds);
		if (!indomain) return;
#if 0
		// fourth-order Runge-Kutta
		Eigen::Vector3d k2 = Sample(pos + 0.5 * stepSize * k1, indomain, velocity, bounds);
		if (!indomain) return;
		Eigen::Vector3d k3 = Sample(pos + 0.5 * stepSize * k2, indomain, velocity, bounds);
		if (!indomain) return;
		Eigen::Vector3d k4 = Sample(pos + stepSize * k3, indomain, velocity, bounds);
		if (!indomain) return;
		pos += stepSize * (k1 + 2 * k2 + 2 * k3 + k4) / (6.0);
#else
		// explicit euler
		pos += stepSize * k1;
#endif
	}
}