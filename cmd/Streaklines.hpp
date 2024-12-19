#pragma once

#include <Eigen/Eigen>

namespace vispro
{
	// Class that advects particles.
	class Streaklines
	{
	public:
		// Receives the seed region, the numerical integration step size and the number of particles to release each time step.
		static void Compute(const char* basePath, const Eigen::AlignedBox3d& seedBox, double stepSize, int particlesReleasedPerTimeStep);
	};
}
