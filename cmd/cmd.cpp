#include <iostream>
#include <string>
#include "Magnitude.hpp"
#include "Velocity.hpp"
#include "Vorticity.hpp"
#include "Particles.hpp"
#include "Streaklines.hpp"
#include "FeatureFlow.hpp"
#include "LIC.hpp"
#include "FTLE.hpp"
#include <Windows.h>

static const int num_time_steps = 151;

void ComputeVelocity(const std::string& basePath) {
	// for each time step
	for (int time = 0; time < num_time_steps; ++time) {
		char filenameIn[256];
		char filenameOut[256];
		sprintf(filenameIn, "halfcylinder-%.2f.vti", time * 0.1);
		sprintf(filenameOut, "halfcylinder-%.2f.am", time * 0.1);
		vispro::Velocity::Compute((basePath + filenameIn).c_str(), (basePath + filenameOut).c_str());
		std::cout << "\rVelocity: " << (time + 1) << " / " << num_time_steps;
	}
}

void ComputeMagnitude(const std::string& basePath) {
	// for each time step
	for (int time = 0; time < num_time_steps; ++time) {
		char filenameIn[256];
		char filenameOut[256];
		sprintf(filenameIn, "halfcylinder-%.2f.am", time * 0.1);
		sprintf(filenameOut, "halfcylinder-magnitude-%.2f.am", time * 0.1);
		vispro::Magnitude::Compute((basePath + filenameIn).c_str(), (basePath + filenameOut).c_str());
		std::cout << "\rMagnitude: " << (time + 1) << " / " << num_time_steps;
	}
}

void ComputeVorticity(const std::string& basePath) {
	// for each time step
	for (int time = 0; time < num_time_steps; ++time) {
		char filenameIn[256];
		char filenameOut[256];
		sprintf(filenameIn, "halfcylinder-%.2f.am", time * 0.1);
		sprintf(filenameOut, "halfcylinder-vorticity-%.2f.am", time * 0.1);
		vispro::Vorticity::Compute((basePath + filenameIn).c_str(), (basePath + filenameOut).c_str());
		std::cout << "\rVorticity: " << (time + 1) << " / " << num_time_steps;
	}
}

void ComputeParticles(const std::string& basePath) {
	Eigen::AlignedBox3d seeds(Eigen::Vector3d(-0.5, -0.5, -0.5), Eigen::Vector3d(0.5, 0.5, 0.5));
	vispro::Particles::Compute(basePath.c_str(), seeds,
		0.05,
		20);
}

void ComputeStreaklines(const std::string& basePath) {
	Eigen::AlignedBox3d seeds(Eigen::Vector3d(-0.5, -0.5, -0.5), Eigen::Vector3d(0.5, 0.5, 0.5));
	vispro::Streaklines::Compute(basePath.c_str(), seeds,
		0.05,
		20);
}

void ComputeFeatureFlow(const std::string& basePath) {
	for (int time = 0; time < num_time_steps; ++time) {
		char filenameIn1[256];
		char filenameIn2[256];
		char filenameIn3[256];
		char filenameOut[256];
		sprintf(filenameIn1, "halfcylinder-%.2f.am", std::max(0, time - 1) * 0.1);
		sprintf(filenameIn2, "halfcylinder-%.2f.am", time * 0.1);
		sprintf(filenameIn3, "halfcylinder-%.2f.am", std::min(time + 1, num_time_steps - 1) * 0.1);
		sprintf(filenameOut, "halfcylinder-featureflow-%.2f.am", time * 0.1);
		int deltaSteps = 2;
		if (time == 0 || time == num_time_steps - 1)
			deltaSteps = 1;

		vispro::FeatureFlow::Compute(
			(basePath + filenameIn1).c_str(),
			(basePath + filenameIn2).c_str(),
			(basePath + filenameIn3).c_str(),
			deltaSteps,
			(basePath + filenameOut).c_str());

		std::cout << "\rFeature Flow: " << (time + 1) << " / " << num_time_steps;
	}
	std::cout << std::endl;
}

void ComputeLIC(const std::string& basePath) {
	// for each time step
	for (int time = 0; time < num_time_steps; ++time) {
		char filenameIn[256];
		char filenameOut[256];
		sprintf(filenameIn, "halfcylinder-%.2f.am", time * 0.1);
		sprintf(filenameOut, "halfcylinder-lic-%.2f.am", time * 0.1);
		vispro::LineIntegralConvolution::Compute((basePath + filenameIn).c_str(), (basePath + filenameOut).c_str(),
			0.01,	// integration step size
			20);	// number of integration steps
		std::cout << "\rLIC: " << (time + 1) << " / " << num_time_steps;
	}
}

void ComputeFTLE(const std::string& basePath) {
	// for each time step
	for (int time = 50; time < 60; ++time)
	{
		char filenameOut[256];
		sprintf(filenameOut, "halfcylinder-ftle-%.2f.am", time * 0.1);
		vispro::FTLE::Compute(basePath.c_str(), (basePath + filenameOut).c_str(),
			Eigen::Vector3i(640, 240, 80),		// grid resolution
			-0.01,		// integration step size
			time * 0.1,	// start time 
			2.0);		// integration duration
		std::cout << "\rFTLE: " << (time + 1) << " / " << num_time_steps;
	}
}

int main(int argc, char* argv[])
{
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
	printf("Debugging Window:\n");

	if (argc < 2) {
		// Pass the data base path to the data as command line argument!
		// e.g., D:/halfcylinder3d-Re320_vti/
		return -1;
	}

	//Computation
	//ComputeVelocity(argv[1]);
	//ComputeMagnitude(argv[1]);
	//ComputeVorticity(argv[1]);
	//ComputeParticles(argv[1]);
	//ComputeStreaklines(argv[1]);
	ComputeFeatureFlow(argv[1]);
	//ComputeLIC(argv[1]);
	//ComputeFTLE(argv[1]);


	return 0;
}