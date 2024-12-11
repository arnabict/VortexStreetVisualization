#pragma once

#include <vtkSmartPointer.h>
#include <vector>
#include <qobject.h>

class QWidget;
class vtkImageData;
class vtkPolyData;
class vtkXMLPolyDataReader;

namespace vispro
{
	// Stores the data of a time step
	class Data : public QObject
	{
		Q_OBJECT
	public:
		// Enumeration of all fields in the data.
		enum class EField {
			Velocity,
			Magnitude,
			Vorticity,
			LIC,
			FTLE
		};
		// Number of fields
		static const int NumFields = 5;

		// Constructor.
		Data(const char* basePath);
		// Destructor.
		~Data();

		// Sets the time step which triggers loading of requested fields.
		void SetTime(int time);

		// Gets a specific field.
		vtkImageData* GetField(const EField& field);
		// Gets the particle data.
		vtkPolyData* GetParticles();

		// Enables a field, which triggers data loading.
		void SetEnableField(const EField& field, bool request);
		// Gets whether a field is enabled.
		bool GetEnableField(const EField& field);
		// Gets the widget to manipulate the fields to read.
		QWidget* GetWidget() const;
		// Gets the widget to manipulate the time.
		QWidget* GetTimeSliderWidget() const;
		// Gets the bounding box of the domain.
		const double* GetBounds() const;

	signals:
		// Signal that the data has changed.
		void DataChanged();

	private slots:
		// Slot for listening to checkbox clicks.
		void CheckedChanged(int state);
		// Slot for listening to checkbox clicks.
		void CheckedParticlesChanged(int state);

	private:
		// Delete copy-constructor.
		Data(const Data& data) = delete;

		// Base path to the data.
		std::string mBasePath;
		// Vector that contains a flag for each field to indicate whether it is currently loaded. Use EField as index.
		std::vector<bool> mFieldEnabled;
		// Flag that indicates whether particle data is read.
		bool mParticleEnabled;
		// Vector that contains the fields of the current time step. Use EField as index.
		std::vector<vtkSmartPointer<vtkImageData>> mFieldData;
		// Reader for the particle data.
		vtkSmartPointer<vtkXMLPolyDataReader> mParticleData;
		// Bounds of the domain (xmin,xmax, ymin,ymax, zmin,zmax).
		double mBounds[6];

		// Widget to manipulate the fields to read.
		QWidget* mWidget;
		// Widget to manipulate the time.
		QWidget* mTimeSliderWidget;
	};
}