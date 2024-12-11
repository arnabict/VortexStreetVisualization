#pragma once

#include <vector>
#include <Eigen/Eigen>
#include <vtkSmartPointer.h>

class vtkImageData;
class vtkFloatArray;

namespace vispro
{
	class UnsteadyTracer
	{
	public:
		// Describes a time series with uniform temporal spacing of time steps.
		struct TimeSeriesDescription {
			TimeSeriesDescription(double temporalSpacing, double startTime, int numTimeSteps);
			double TemporalSpacing;	// temporal distance between two time steps
			double StartTime;		// start time of the sequence
			int NumTimeSteps;		// number of time steps in the sequence
		};

		// Constructor.
		UnsteadyTracer(const std::string& basePath);
		// Destructor.
		~UnsteadyTracer();

		// Traces a set of particles from a start time for a certain target duration. The particle set is modified and will store the target positions in the end.
		void Flowmap(std::vector<Eigen::Vector3d>& particles, std::vector<int>& inDomain, double stepSize, double startTime, double duration);

		// Gets the bounding box of the domain
		const Eigen::AlignedBox3d& GetBounds() const;
		// Gets general parameters about the time series.
		const TimeSeriesDescription& GetDesc() const;

	private:
		// Delete the copy-constructor.
		UnsteadyTracer(const UnsteadyTracer& other) = delete;

		// Advects a set of particles for one integration step, starting at time "time". The necesary data is assumed to be present in memory already.
		void Advect(std::vector<Eigen::Vector3d>& particles, std::vector<int>& inDomain, double time, double stepSize) const;
		// Samples the velocity for a certain particle and assumes that the necessary data is in memory.
		Eigen::Vector3d Sample(const Eigen::Vector3d& position, double time, int& inDomain) const;
		// Reads the header of a given vector field to initialize the vtkImageData objects in the ring buffer.
		bool AllocateVectorFieldsFromHeader(const char* path);
		// Physical time of a time step in the ring buffer.
		double mTime[3];
		// Vector field data of a time step in the ring buffer.
		vtkSmartPointer<vtkImageData> mData[3];
		// Bounding box of the domain
		Eigen::AlignedBox3d mBounds;
		// Head index in the ring buffer
		int mHead;
		// General parameters about the time series.
		const TimeSeriesDescription mDesc;
		// Base path to the data set.
		std::string mBasePath;
	};
}