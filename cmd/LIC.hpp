#pragma once

#include <Eigen/Eigen>

class vtkImageData;

namespace vispro
{
	// Computes a line integral convolution (LIC).
	class LineIntegralConvolution
	{
	public:
		// receives the paths to the vtkImageData file of velocity (input) and the LIC path (output)
		static void Compute(const char* velocityPath, const char* licPath, double stepSize, int numAdvectionSteps);

	private:
		// Samples a given vector field and checks if the given point was inside given bounds.----
		static Eigen::Vector3d Sample(const Eigen::Vector3d& pos, bool& indomain, vtkImageData* velocity, const Eigen::AlignedBox3d& bounds);
		// Advects a particle to the next time step.
		static void Advect(Eigen::Vector3d& pos, bool& indomain, double stepSize, vtkImageData* velocity, const Eigen::AlignedBox3d& bounds);
	};
}