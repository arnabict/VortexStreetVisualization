#include "Streaklines.hpp"
#include "UnsteadyTracer.hpp"
#include <random>
#include <vtkTriangle.h>
#include <vtkPolyData.h>
#include <vtkPoints.h>
#include <vtkIdList.h>
#include <vtkXMLPolyDataWriter.h>


namespace vispro
{
	void Streaklines::Compute(const char* basePath, const Eigen::AlignedBox3d& seedBox, double stepSize, int particlesReleasedPerTimeStep)
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

			//// remove all the inactive particles
			//int pid = 0;
			//for (int ip = 0; ip < indomain.size(); ++ip) {
			//	if (indomain[ip] == 1) {
			//		particles[pid] = particles[ip];
			//		indomain[pid] = indomain[ip];
			//		pid++;
			//	}
			//}
			//particles.resize(pid);
			//indomain.resize(pid);

			// add new particles
			for (int ip = 0; ip < particlesReleasedPerTimeStep; ++ip) {
				double minz = clampedSeedBox.min().z();
				double maxz = clampedSeedBox.max().z();
				double t = (double)ip / particlesReleasedPerTimeStep;
				Eigen::Vector3d seed(-0.1, 0.1, minz + (maxz - minz) * t);
				particles.push_back(seed);
				indomain.push_back(1);
			}

			// store the particles
			vtkNew<vtkPoints> points;
			points->SetNumberOfPoints(particles.size());
			for (size_t ip = 0; ip < particles.size(); ++ip) {
				points->SetPoint(ip, particles[ip].data());
			}

			// form triangles
			vtkNew<vtkCellArray> cellArray;
			// ids->SetNumberOfIds(((particlesReleasedPerTimeStep - 1) * (iTime-1)) *6);
			for (int i = 0; i <= iTime - 1; ++i) {
				for (int j = 0; j < particlesReleasedPerTimeStep - 1; ++j) {
					int i10 = (i+1) * particlesReleasedPerTimeStep + j;
					int i00 = i * particlesReleasedPerTimeStep + j;
					int i11 = (i+1) * particlesReleasedPerTimeStep + (j+1);
					int i01 = i * particlesReleasedPerTimeStep + (j+1);

					vtkNew<vtkTriangle> triangle;
					triangle->GetPointIds()->SetId(0, i00);
					triangle->GetPointIds()->SetId(1, i10);
					triangle->GetPointIds()->SetId(2, i01);

					cellArray->InsertNextCell(triangle);

					vtkNew<vtkTriangle> triangle2;
					triangle2->GetPointIds()->SetId(0, i10);
					triangle2->GetPointIds()->SetId(1, i11);
					triangle2->GetPointIds()->SetId(2, i01);

					cellArray->InsertNextCell(triangle2);
				}
			}

			vtkNew<vtkPolyData> polyData;
			polyData->SetPoints(points);
			polyData->SetPolys(cellArray);
			//polyData->SetVerts(cellArray);
			char filename[256];
			sprintf(filename, "halfcylinder-streaklines-%.2f.vtp", startTime);
			polyData->Print(std::cout);
			vtkNew<vtkXMLPolyDataWriter> writer;
			writer->SetFileName((std::string(basePath) + filename).c_str());
			writer->SetInputData(polyData);
			writer->Update();

			// advect all particles to the next time step
			tracer.Flowmap(particles, indomain, stepSize, startTime, desc.TemporalSpacing);
		}
	}
}
