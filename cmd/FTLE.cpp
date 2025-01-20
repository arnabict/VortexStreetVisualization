#include "FTLE.hpp"
#include "UnsteadyTracer.hpp"
#include <vtkImageData.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include "AmiraWriter.hpp"

namespace vispro
{
	void FTLE::Compute(const char* basePath, const char* ftlePath, const Eigen::Vector3i& resolution, double stepSize, double startTime, double duration)
	{
		// allocate the tracer, which reads the header of the data set
		UnsteadyTracer tracer(basePath);
		Eigen::AlignedBox3d bounds = tracer.GetBounds();
		Eigen::Vector3d spacing(
			(bounds.max()[0] - bounds.min()[0]) / (resolution[0] - 1.),
			(bounds.max()[1] - bounds.min()[1]) / (resolution[1] - 1.),
			(bounds.max()[2] - bounds.min()[2]) / (resolution[2] - 1.));

		// allocate output field
		vtkNew<vtkImageData> ftle;
		ftle->SetDimensions(resolution.data());
		ftle->SetOrigin(bounds.min().data());
		ftle->SetSpacing(spacing.data());
		vtkNew<vtkFloatArray> mArray;
		int64_t numPoints = (int64_t)resolution.prod();
		mArray->SetNumberOfComponents(1);
		mArray->SetNumberOfTuples(numPoints);
		mArray->SetName("ftle");
		ftle->GetPointData()->AddArray(mArray);

		if (mArray->GetNumberOfTuples() != numPoints) {
			std::cerr << "Expected: " << numPoints << "Found: " << mArray->GetNumberOfTuples();
			return;
		}
		else {
			std::cout << numPoints;
		}

		// create the seed points on a regular grid
		std::vector<Eigen::Vector3d> particles(numPoints);
		std::vector<int> inDomain(numPoints, 1);
		for (int iz = 0; iz < resolution[2]; ++iz)
			for (int iy = 0; iy < resolution[1]; ++iy)
				for (int ix = 0; ix < resolution[0]; ++ix)
				{
					Eigen::Vector3d pos = bounds.min() + Eigen::Vector3d(ix, iy, iz).cwiseProduct(spacing);
					particles[((int64_t)iz * resolution.y() + iy) * resolution.x() + ix] = pos;
				}

		// trace the particles
		tracer.Flowmap(particles, inDomain, stepSize, startTime, duration);

		// compute the FTLE values
		for (int iz = 0; iz < resolution[2]; ++iz) {
			for (int iy = 0; iy < resolution[1]; ++iy) {
				for (int ix = 0; ix < resolution[0]; ++ix) {
					// get indices for the neighbors (clamp to not leave the domain)
					int ix0 = std::max(0, ix - 1);
					int ix1 = std::min(ix + 1, resolution[0] - 1);
					int iy0 = std::max(0, iy - 1);
					int iy1 = std::min(iy + 1, resolution[1] - 1);
					int iz0 = std::max(0, iz - 1);
					int iz1 = std::min(iz + 1, resolution[2] - 1);

					double ftle = 0;
					if (inDomain[((int64_t)iz * resolution[1] + iy) * resolution[0] + ix0] &&
						inDomain[((int64_t)iz * resolution[1] + iy) * resolution[0] + ix1] &&
						inDomain[((int64_t)iz * resolution[1] + iy0) * resolution[0] + ix] &&
						inDomain[((int64_t)iz * resolution[1] + iy1) * resolution[0] + ix] &&
						inDomain[((int64_t)iz0 * resolution[1] + iy) * resolution[0] + ix] &&
						inDomain[((int64_t)iz1 * resolution[1] + iy) * resolution[0] + ix])
					{
						// compute flow map gradient via finite differences
						Eigen::Vector3d phix = (particles[((int64_t)iz * resolution[1] + iy) * resolution[0] + ix1] - particles[((int64_t)iz * resolution[1] + iy) * resolution[0] + ix0]) / (((int64_t)ix1 - ix0) * spacing[0]);
						Eigen::Vector3d phiy = (particles[((int64_t)iz * resolution[1] + iy1) * resolution[0] + ix] - particles[((int64_t)iz * resolution[1] + iy0) * resolution[0] + ix]) / (((int64_t)iy1 - iy0) * spacing[1]);
						Eigen::Vector3d phiz = (particles[((int64_t)iz1 * resolution[1] + iy) * resolution[0] + ix] - particles[((int64_t)iz0 * resolution[1] + iy) * resolution[0] + ix]) / (((int64_t)iz1 - iz0) * spacing[2]);
						Eigen::Matrix3d phiT;
						phiT << phix, phiy, phiz;
						phiT.transposeInPlace();
						
						// compute FTLE
						double lambda_max = (phiT.transpose() * phiT).eigenvalues().real().maxCoeff();
						ftle = 1. / duration * std::log(std::sqrt(lambda_max));
					}
					mArray->SetValue(((int64_t)iz * resolution[1] + iy) * resolution[0] + ix, ftle);
				}
			}
		}

		// write the result to file
		AmiraWriter::WriteScalarField(ftlePath, "ftle", ftle);
	}
}