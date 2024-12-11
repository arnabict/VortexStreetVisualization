#pragma once

namespace vispro
{
	// Computes the velocity magnitude of a given time step.
	class Magnitude
	{
	public:
		// receives the paths to the vtkImageData file of velocity (input) and the magnitude path (output)
		static void Compute(const char* velocityPath, const char* magnitudePath);
	};
}