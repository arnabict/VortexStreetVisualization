#include "StreakLineVtk.hpp"
#include "Data.hpp"
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkPointSource.h>
#include <vtkStreamTracer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <qformlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include <vtkFloatArray.h>
#include "QDoubleSlider.hpp"

namespace vispro
{
    StreakLineVtk::StreakLineVtk()
        : Component("StreakLineVtk"),
        mStreamTracer(nullptr),
        mStreaklineMapper(nullptr),
        mPointSource(nullptr),
        mSliderValue(0.0),
        mData(nullptr) // Initialize to null
    {
    }

    StreakLineVtk::~StreakLineVtk()
    {
    }

    void StreakLineVtk::CreateWidget(QWidget* widget)
    {
        QPushButton* colorButton = new QPushButton;
        double* color = vtkActor::SafeDownCast(mActor)->GetProperty()->GetColor();
        QColor col = QColor::fromRgbF(color[0], color[1], color[2]);
        QString qss = QString("background-color: %1").arg(col.name());
        colorButton->setStyleSheet(qss);
        connect(colorButton, &QPushButton::released, this, &StreakLineVtk::PickColor);

        // Slider for adjusting streakline properties
        QSlider* subtractionSlider = new QSlider(Qt::Horizontal);
        subtractionSlider->setRange(0, 100);  // Range for 0.0 to 1.0 with 0.01 increments
        subtractionSlider->setSingleStep(1); // 0.01 step size
        subtractionSlider->setValue(static_cast<int>(mSliderValue * 100)); // Default value scaled to slider range

        connect(subtractionSlider, &QSlider::valueChanged, this, [this](int value) {
            double subtractionValue = static_cast<double>(value) / 100.0; // Scale to double
            AdjustStreaklineProperty(subtractionValue);
            });

        // Layout for the widget
        QFormLayout* layout = new QFormLayout;
        layout->addRow(new QLabel(tr("Streakline Color:")), colorButton);
        layout->addRow(new QLabel(tr("Adjust Streakline Value:")), subtractionSlider);
        widget->setLayout(layout);
    }

    void StreakLineVtk::AdjustStreaklineProperty(double value)
    {
        mSliderValue = value; // Update the slider value
        SetData(mData);       // Reprocess data with the new slider value
        emit RequestRender(); // Request an update to the render
    }

    vtkSmartPointer<vtkProp> StreakLineVtk::CreateActor()
    {
        mPointSource = vtkSmartPointer<vtkPointSource>::New();
        mPointSource->SetCenter(0.0, 0.0, 0.0);
        mPointSource->SetRadius(2);
        mPointSource->SetNumberOfPoints(100);

        mStreamTracer = vtkSmartPointer<vtkStreamTracer>::New();
        mStreamTracer->SetSourceConnection(mPointSource->GetOutputPort());
        mStreamTracer->SetIntegrationDirectionToForward(); // Important for streaklines
        mStreamTracer->SetMaximumPropagation(1000);
        mStreamTracer->SetMaximumNumberOfSteps(500);

        mStreaklineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mStreaklineMapper->SetInputConnection(mStreamTracer->GetOutputPort());

        vtkNew<vtkActor> actor;
        actor->SetMapper(mStreaklineMapper);
        actor->GetProperty()->SetColor(0.5, 0.5, 0.5);  // Initial color
        mActor = actor; // Ensure mActor is set properly
        return actor;
    }

    void StreakLineVtk::SetData(Data* data)
    {
        mData = data;

        vtkImageData* velocityData = data->GetField(Data::EField::Velocity);
     
        if (velocityData == nullptr) {
            std::cerr << "Error: Velocity data is null." << std::endl;
            return;
        }

        vtkDataArray* velocityArray = velocityData->GetPointData()->GetVectors();
        if (velocityArray == nullptr) {
            std::cerr << "Error: No velocity vector data found in the field." << std::endl;
            return;
        }

        vtkFloatArray* velocityArrayFloat = vtkFloatArray::SafeDownCast(velocityArray);

        int dimensions[3];
        velocityData->GetDimensions(dimensions);

        double origin[3];
        velocityData->GetOrigin(origin);

        double spacing[3];
        velocityData->GetSpacing(spacing);

        vtkNew<vtkImageData> field;
        field->SetDimensions(dimensions);
        field->SetOrigin(origin);
        field->SetSpacing(spacing);

        vtkNew<vtkFloatArray> mNum;
        int64_t numPoints = (int64_t)field->GetDimensions()[0] * field->GetDimensions()[1] * field->GetDimensions()[2];
        mNum->SetNumberOfComponents(velocityArray->GetNumberOfComponents());
        mNum->SetNumberOfTuples(velocityArray->GetNumberOfTuples());
        mNum->SetName(velocityArray->GetName());
        field->GetPointData()->AddArray(mNum);
        field->GetPointData()->SetActiveVectors(velocityArray->GetName());

        // Adjust velocity field based on slider value (subtraction)
        for (int z = 0; z < dimensions[2]; ++z) {
            for (int y = 0; y < dimensions[1]; ++y) {
                for (int x = 0; x < dimensions[0]; ++x) {
                    double* point = velocityArrayFloat->GetTuple3(x + dimensions[0] * (y + dimensions[1] * z));
                    mNum->SetTuple3(x + dimensions[0] * (y + dimensions[1] * z),
                        point[0] - mSliderValue, // Adjust using slider (modify X component)
                        point[1],                // Keep Y component
                        point[2]);               // Keep Z component
                }
            }
        }


        // Set the updated velocity field for streakline generation
        mStreamTracer->SetInputData(field);
        mStreamTracer->Update();  // Ensure that the streaklines are recalculated
        emit RequestRender();     // Trigger a render update
    }

    void StreakLineVtk::PickColor()
    {
        double* color = vtkActor::SafeDownCast(mActor)->GetProperty()->GetColor();
        QColor col = QColorDialog::getColor(QColor::fromRgbF(color[0], color[1], color[2]));
        if (col.isValid())
        {
            QString qss = QString("background-color: %1").arg(col.name());
            QPushButton* button = dynamic_cast<QPushButton*>(sender());
            button->setStyleSheet(qss);
            vtkActor::SafeDownCast(mActor)->GetProperty()->SetColor(col.redF(), col.greenF(), col.blueF());
            emit RequestRender();
        }
    }
}
