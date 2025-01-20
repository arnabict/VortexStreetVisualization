#include "Particles.hpp"
#include "UnsteadyTracer.hpp"
#include <random>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkIdList.h>
#include <vtkXMLPolyDataWriter.h>

namespace vispro
{
	void Particles::Compute(const char* basePath, const Eigen::AlignedBox3d& seedBox, double stepSize, int particlesReleasedPerTimeStep)
	{
		UnsteadyTracer tracer(basePath);
		const UnsteadyTracer::TimeSeriesDescription& desc = tracer.GetDesc();
		Eigen::AlignedBox3d clampedSeedBox = seedBox.intersection(tracer.GetBounds());

		std::vector<Eigen::Vector3d> particles;
		std::vector<int> indomain;
		for (int iTime = 0; iTime < desc.NumTimeSteps; ++iTime)
		{
			// physical time for this time step
			float startTime = desc.StartTime + iTime * desc.TemporalSpacing;

			// remove all the inactive particles
			int pid = 0;
			for (int ip = 0; ip < indomain.size(); ++ip) {
				if (indomain[ip] == 1) {
					particles[pid] = particles[ip];
					indomain[pid] = indomain[ip];
					pid++;
				}
			}
			particles.resize(pid);
			indomain.resize(pid);

			// add new particles
			for (int ip = 0; ip < particlesReleasedPerTimeStep; ++ip) {
				particles.push_back(clampedSeedBox.sample());
				indomain.push_back(1);
			}

			// store the particles
			vtkNew<vtkPoints> points;
			vtkNew<vtkIdList> ids;
			points->SetNumberOfPoints(particles.size());
			ids->SetNumberOfIds(particles.size());
			for (size_t ip = 0; ip < particles.size(); ++ip) {
				points->SetPoint(ip, particles[ip].data());
				ids->SetId(ip, ip);
			}
			vtkNew<vtkCellArray> cellArray;
			cellArray->InsertNextCell(ids);
			vtkNew<vtkPolyData> polyData;
			polyData->SetPoints(points);
			polyData->SetVerts(cellArray);
			char filename[256];
			sprintf(filename, "halfcylinder-particles-%.2f.vtp", startTime);
			vtkNew<vtkXMLPolyDataWriter> writer;
			writer->SetFileName((std::string(basePath) + filename).c_str());
			writer->SetInputData(polyData);
			writer->Update();

			// advect all particles to the next time step
			tracer.Flowmap(particles, indomain, stepSize, startTime, desc.TemporalSpacing);
		}
	}
}