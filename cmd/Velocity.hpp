#pragma once

namespace vispro
{
	// Writes the velocity of a given time step in the Amira format.
	class Velocity
	{
	public:
		// receives the paths to the vtkImageData file of velocity (input) and converts it to the Amira format (output)
		static void Compute(const char* velocityPath, const char* velocityPathAm);
	};
}