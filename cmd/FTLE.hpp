#pragma once

#include <Eigen/Eigen>

namespace vispro
{
	// Class that computes the finite-time Lyapunov exponent.
	class FTLE
	{
	public:
		// Receives the output path of the *.am files, as well the desired grid resolution and numerical integration parameters.
		static void Compute(const char* basePath, const char* ftlePath, const Eigen::Vector3i& resolution, double stepSize, double startTime, double duration);
	};
}