#pragma once

namespace vispro
{
	// Computes the vorticity of a given time step.
	class Vorticity
	{
	public:
		// receives the paths to the vtkImageData file of velocity (input) and the vorticity path (output)
		static void Compute(const char* velocityPath, const char* vorticityPath);
	};
}