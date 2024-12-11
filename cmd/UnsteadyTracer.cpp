#include "UnsteadyTracer.hpp"
#include <vtkPointData.h>
#include <vtkImageData.h>
#include <vtkFloatArray.h>
#include "AmiraReader.hpp"
#include "Sampling.hpp"

namespace vispro
{
	UnsteadyTracer::TimeSeriesDescription::TimeSeriesDescription(double temporalSpacing, double startTime, int numTimeSteps) :
		TemporalSpacing(temporalSpacing), StartTime(startTime), NumTimeSteps(numTimeSteps) 
	{}

	UnsteadyTracer::UnsteadyTracer(const std::string& basePath) : mDesc(0.1, 0, 151), mHead(0), mBasePath(basePath)
	{
		mTime[0] = mTime[1] = mTime[2] = std::numeric_limits<double>::infinity();
		mData[0] = vtkSmartPointer<vtkImageData>::New();
		mData[1] = vtkSmartPointer<vtkImageData>::New();
		mData[2] = vtkSmartPointer<vtkImageData>::New();
		mBounds.setEmpty();
		bool success = AllocateVectorFieldsFromHeader((mBasePath + "halfcylinder-0.00.am").c_str());
		assert(success);
	}
	
	UnsteadyTracer::~UnsteadyTracer()
	{}

	const Eigen::AlignedBox3d& UnsteadyTracer::GetBounds() const { return mBounds; }
	const UnsteadyTracer::TimeSeriesDescription& UnsteadyTracer::GetDesc() const { return mDesc; }

	void UnsteadyTracer::Flowmap(std::vector<Eigen::Vector3d>& particles, std::vector<int>& inDomain, double stepSize, double startTime, double duration)
	{
		// nothing to do
		if (stepSize == 0) return;

		// make sure we are in the temporal domain
		if ((stepSize > 0 && (startTime < mDesc.StartTime || mDesc.StartTime + (mDesc.NumTimeSteps - 1.) * mDesc.TemporalSpacing < startTime + duration)) ||
			(stepSize < 0 && (startTime - duration < mDesc.StartTime || mDesc.StartTime + (mDesc.NumTimeSteps - 1.) * mDesc.TemporalSpacing < startTime))) {
			for (size_t i = 0; i < particles.size(); ++i)
				inDomain[i] = 0;
			return;
		}

		// check for each particle if it is in the spatial domain (1=true, 0=false)
		for (size_t i = 0; i < particles.size(); ++i)
			inDomain[i] = mBounds.contains(particles[i]) ? 1 : 0;

		// find out which three time steps to read at the beginning
		int t0 = std::min(std::max(0, (int)((startTime - mDesc.StartTime) / mDesc.TemporalSpacing)), mDesc.NumTimeSteps - 1);
		int t1 = std::min(std::max(0, t0 + (stepSize > 0 ? 1 : -1)), mDesc.NumTimeSteps - 1);
		int t2 = std::min(std::max(0, t0 + (stepSize > 0 ? 2 : -2)), mDesc.NumTimeSteps - 1);
		mTime[0] = mDesc.StartTime + t0 * mDesc.TemporalSpacing;
		mTime[1] = mDesc.StartTime + t1 * mDesc.TemporalSpacing;
		mTime[2] = mDesc.StartTime + t2 * mDesc.TemporalSpacing;

		// read the three time steps
		char filename[256];
		sprintf(filename, "halfcylinder-%.2f.am", mTime[0]);
		bool success = AmiraReader::ReadField((mBasePath + filename).c_str(), dynamic_cast<vtkFloatArray*>(mData[0]->GetPointData()->GetArray(0)));
		assert(success);
		sprintf(filename, "halfcylinder-%.2f.am", mTime[1]);
		success = AmiraReader::ReadField((mBasePath + filename).c_str(), dynamic_cast<vtkFloatArray*>(mData[1]->GetPointData()->GetArray(0)));
		assert(success);
		sprintf(filename, "halfcylinder-%.2f.am", mTime[2]);
		success = AmiraReader::ReadField((mBasePath + filename).c_str(), dynamic_cast<vtkFloatArray*>(mData[2]->GetPointData()->GetArray(0)));
		assert(success);

		mHead = 0;
		double time = startTime;
		if (stepSize > 0)	// forward integration
		{
			// while we have not reached the end time yet
			while (time < startTime + duration) {
				// perform steps until we reach the end or the central time step
				while (time < std::min(mTime[(mHead + 1) % 3], startTime + duration)) {
					double s = std::min(stepSize, std::min(mTime[(mHead + 1) % 3], startTime + duration) - time);
					Advect(particles, inDomain, time, s);
					time += s;
				}
				// if not yet at end, load next time step!
				if (time < startTime + duration) {
					// Compute the time of the next file to read
					mTime[mHead] = mDesc.StartTime + std::min(std::max(0, t0 + 3), mDesc.NumTimeSteps - 1) * mDesc.TemporalSpacing;
					sprintf(filename, "halfcylinder-%.2f.am", mTime[mHead]);
					bool success = AmiraReader::ReadField((mBasePath + filename).c_str(), dynamic_cast<vtkFloatArray*>(mData[mHead]->GetPointData()->GetArray(0)));
					assert(success);
					// move the head forward
					mHead = (mHead + 1) % 3;
					t0 += 1;
					time = mTime[mHead];	// set the time to the exact start time (to prevent numerical issues)
				}
			}
		}
		else // backward integration
		{
			// while we have not reached the end time yet
			while (time > startTime - duration) {
				// perform steps until we reach the end or the central time step
				while (time > std::max(mTime[(mHead + 1) % 3], startTime - duration)) {
					double s = std::max(stepSize, std::max(mTime[(mHead + 1) % 3], startTime - duration) - time);
					Advect(particles, inDomain, time, s);
					time += s;
				}
				// if not yet at end, load next time step!
				if (time > startTime - duration) {
					// Compute the time of the next file to read
					mTime[mHead] = mDesc.StartTime + std::min(std::max(0, t0 - 3), mDesc.NumTimeSteps - 1) * mDesc.TemporalSpacing;
					sprintf(filename, "halfcylinder-%.2f.am", mTime[mHead]);
					bool success = AmiraReader::ReadField((mBasePath + filename).c_str(), dynamic_cast<vtkFloatArray*>(mData[mHead]->GetPointData()->GetArray(0)));
					assert(success);
					// move the head forward
					mHead = (mHead + 1) % 3;
					t0 -= 1;
					time = mTime[mHead];	// set the time to the exact start time (to prevent numerical issues)
				}
			}
		}
	}

	void UnsteadyTracer::Advect(std::vector<Eigen::Vector3d>& particles, std::vector<int>& inDomain, double time, double stepSize) const
	{
		int64_t numParticles = (int64_t)particles.size();
		for (int64_t i = 0; i < numParticles; ++i) 
		{
			// early out?
			int& indomain = inDomain[i];
			if (!indomain) continue;

			// numerical integration step
			Eigen::Vector3d& pos = particles[i];
			Eigen::Vector3d k1 = Sample(pos, time, indomain);
			if (!indomain) continue;
#if 0
			// fourth-order Runge-Kutta
			Eigen::Vector3d k2 = Sample(pos + 0.5 * stepSize * k1, time + 0.5 * stepSize, indomain);
			if (!indomain) continue;
			Eigen::Vector3d k3 = Sample(pos + 0.5 * stepSize * k2, time + 0.5 * stepSize, indomain);
			if (!indomain) continue;
			Eigen::Vector3d k4 = Sample(pos + stepSize * k3, time + stepSize, indomain);
			if (!indomain) continue;
			pos += stepSize * (k1 + 2 * k2 + 2 * k3 + k4) / (6.0);
#else
			// explicit euler
			pos += stepSize * k1;
#endif
		}
	}

	Eigen::Vector3d UnsteadyTracer::Sample(const Eigen::Vector3d& position, double time, int& inDomain) const
	{
		// is the sample inside the spatial domain?
		if (!mBounds.contains(position)) {
			inDomain = 0;
			return Eigen::Vector3d(0,0,0);
		}

		// determine which time steps to interpolate between
		int i0, i1;
		if (std::min(mTime[mHead], mTime[(mHead + 1) % 3]) <= time && time <= std::max(mTime[mHead], mTime[(mHead + 1) % 3])) {
			i0 = mHead;
			i1 = (mHead + 1) % 3;
		}
		else {
			assert(std::min(mTime[(mHead + 1) % 3], mTime[(mHead + 2) % 3]) <= time && time <= std::max(mTime[(mHead + 1) % 3], mTime[(mHead + 2) % 3]));
			i0 = (mHead + 1) % 3;
			i1 = (mHead + 2) % 3;
		}
		// read from vector field and interpolate
		Eigen::Vector3d v0 = Sampling::LinearSample3(position, mData[i0]);
		Eigen::Vector3d v1 = Sampling::LinearSample3(position, mData[i1]);
		double interp = (time - mTime[i0]) / (mTime[i1] - mTime[i0]);
		return v0 + (v1 - v0) * interp;
	}

	bool UnsteadyTracer::AllocateVectorFieldsFromHeader(const char* path)
	{
		// read the header
		Eigen::Vector3i resolution;
		Eigen::Vector3d spacing;
		int numComponents;
		bool success = AmiraReader::ReadHeader(path, mBounds, resolution, spacing, numComponents);
		assert(success);

		// allocate output field
		for (int i = 0; i < 3; ++i) {
			mData[i]->SetDimensions(resolution.data());
			mData[i]->SetOrigin(mBounds.min().data());
			mData[i]->SetSpacing(spacing.data());
			vtkNew<vtkFloatArray> mArray;
			int64_t numPoints = (int64_t)mData[i]->GetDimensions()[0] * mData[i]->GetDimensions()[1] * mData[i]->GetDimensions()[2];
			mArray->SetNumberOfComponents(3);
			mArray->SetNumberOfTuples(numPoints);
			mArray->SetName("velocity");
			mData[i]->GetPointData()->AddArray(mArray);
			mData[i]->GetPointData()->SetActiveScalars("velocity");
		}
		return true;
	}
}