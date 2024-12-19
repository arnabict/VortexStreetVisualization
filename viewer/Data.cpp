#include "Data.hpp"
#include <qslider.h>
#include <qgroupbox.h>
#include <qformlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qvariant.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkImageData.h>
#include <vtkPolyData.h>
#include "AmiraReader.hpp"

namespace vispro
{
	Data::Data(const char* basePath) : mBasePath(basePath)
	{
		// Allolate data containers.
		mFieldData.resize(NumFields);			// number of fields
		mFieldEnabled.resize(NumFields, false);	// number of fields
		for (size_t i = 0; i < mFieldData.size(); ++i)
			mFieldData[i] = vtkSmartPointer<vtkImageData>::New();
		mParticleData = vtkSmartPointer<vtkXMLPolyDataReader>::New();

		mFieldEnabled[(int)EField::Velocity] = true;
		//mFieldEnabled[(int)EField::Magnitude] = true;
		mFieldEnabled[(int)EField::Vorticity] = true;
		//mFieldEnabled[(int)EField::LIC] = true;
		//mFieldEnabled[(int)EField::FTLE] = true;

		mParticleEnabled = true;

		// create the time slider UI
		QSlider* slider = new QSlider(Qt::Orientation::Horizontal);
		slider->setFocusPolicy(Qt::StrongFocus);
		slider->setTickPosition(QSlider::TicksBothSides);
		slider->setTickInterval(10);
		slider->setSingleStep(1);
		slider->setMinimum(0);
		slider->setMaximum(150);
		mTimeSliderWidget = slider;
		connect(slider, &QSlider::valueChanged, this, &Data::SetTime);

		mBounds[0] = mBounds[2] = mBounds[4] = 0;
		mBounds[1] = mBounds[3] = mBounds[5] = 1;

		std::string fieldNames[NumFields];
		fieldNames[(int)EField::Velocity] = "Velocity";
		fieldNames[(int)EField::Magnitude] = "Magnitude";
		fieldNames[(int)EField::Vorticity] = "Vorticity";
		fieldNames[(int)EField::LIC] = "LIC";
		fieldNames[(int)EField::FTLE] = "FTLE";

		// create the data field options UI
		QGroupBox* groupBox = new QGroupBox("Data");
		QFormLayout* layout = new QFormLayout;
		for (int iField = 0; iField < NumFields; ++iField) {
			QCheckBox* checkBox = new QCheckBox;
			checkBox->setChecked(mFieldEnabled[iField]);
			checkBox->setProperty("id", QVariant(iField));
			connect(checkBox, &QCheckBox::stateChanged, this, &Data::CheckedChanged);
			layout->addRow(new QLabel(tr((std::string("Read ") + fieldNames[iField] + ":").c_str())), checkBox);
		}
		{
			QCheckBox* checkBox = new QCheckBox;
			checkBox->setChecked(mParticleEnabled);
			connect(checkBox, &QCheckBox::stateChanged, this, &Data::CheckedParticlesChanged);
			layout->addRow(new QLabel(tr("Read Particles:")), checkBox);
		}
		groupBox->setLayout(layout);
		mWidget = groupBox;

		SetTime(0);
	}

	Data::~Data()
	{}
	
	void Data::SetTime(int time) 
	{
		for (int iField = 0; iField < NumFields; ++iField) {
			if (mFieldEnabled[iField]) {
				char filename[256];
				switch (iField) {
				case (int)EField::Velocity:
					sprintf(filename, "halfcylinder-%.2f.am", time * 0.1);
					break;
				case (int)EField::Magnitude:
					sprintf(filename, "halfcylinder-magnitude-%.2f.am", time * 0.1);
					break;
				case (int)EField::Vorticity:
					sprintf(filename, "halfcylinder-vorticity-%.2f.am", time * 0.1);
					break;
				case (int)EField::LIC:
					sprintf(filename, "halfcylinder-lic-%.2f.am", time * 0.1);
					break;
				case (int)EField::FTLE:
					sprintf(filename, "halfcylinder-ftle-%.2f.am", time * 0.1);
					break;
				default: 
					throw std::logic_error("path for field not yet implemented!");
				}
				auto result = AmiraReader::ReadField((mBasePath + filename).c_str(), "field");
				if (result != nullptr) {
					mFieldData[iField] = result;
					for (int i = 0; i < 6; i++)
						mBounds[i] = result->GetBounds()[i];
				}
			}
		}
		if (mParticleEnabled) {
			char filename[256];
			//sprintf(filename, "halfcylinder-particles-%.2f.vtp", time * 0.1);
			sprintf(filename, "halfcylinder-streaklines-%.2f.vtp", time * 0.1);
			mParticleData->SetFileName((mBasePath + filename).c_str());
			mParticleData->Update();
		}
		emit DataChanged();
	}

	void Data::CheckedChanged(int state) {
		mFieldEnabled[sender()->property("id").toInt()] = (bool)state;
	}

	void Data::CheckedParticlesChanged(int state) {
		mParticleEnabled = (bool)state;
	}

	vtkImageData* Data::GetField(const EField& field) {
		if (mFieldEnabled[(int)field])
			return mFieldData[(int)field];
		else return nullptr;
	}

	vtkPolyData* Data::GetParticles() {
		if (mParticleEnabled)
			return mParticleData->GetOutput();
		else return nullptr;
	}

	void Data::SetEnableField(const EField& field, bool request) {
		mFieldEnabled[(int)field] = request;
	}

	bool Data::GetEnableField(const EField& field) {
		return mFieldEnabled[(int)field];
	}

	QWidget* Data::GetWidget() const { return mWidget; }
	QWidget* Data::GetTimeSliderWidget() const { return mTimeSliderWidget; }
	const double* Data::GetBounds() const { return mBounds; }
}