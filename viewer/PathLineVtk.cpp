#include "PathLineVtk.hpp"
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
#include <qslider.h>
#include <iostream>
#include <vtkFloatArray.h>

namespace vispro {

    PathLineVtk::PathLineVtk()
        : Component("PathLineVtk"),
        mStreamTracer(nullptr),
        mPointSource(nullptr),
        mPathlineMapper(nullptr),
        mData(nullptr) {
    }

    PathLineVtk::~PathLineVtk() {}

    void PathLineVtk::CreateWidget(QWidget* widget) {
        QFormLayout* layout = new QFormLayout;
        layout->addRow(new QLabel(tr("Pathline Visualization")));
        widget->setLayout(layout);
    }

    vtkSmartPointer<vtkProp> PathLineVtk::CreateActor() {
        // Seed points for pathlines.
        mPointSource = vtkSmartPointer<vtkPointSource>::New();
        mPointSource->SetCenter(0.0, 0.0, 0.0);
        mPointSource->SetRadius(2.0);
        mPointSource->SetNumberOfPoints(100);

        // Stream tracer for pathlines.
        mStreamTracer = vtkSmartPointer<vtkStreamTracer>::New();
        mStreamTracer->SetSourceConnection(mPointSource->GetOutputPort());
        mStreamTracer->SetIntegrationDirectionToForward();
        mStreamTracer->SetMaximumPropagation(1000);
        mStreamTracer->SetMaximumNumberOfSteps(500);

        // Mapper for pathlines.
        mPathlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mPathlineMapper->SetInputConnection(mStreamTracer->GetOutputPort());

        vtkNew<vtkActor> actor;
        actor->SetMapper(mPathlineMapper);
        actor->GetProperty()->SetColor(0.0, 0.0, 1.0); // Blue pathlines.
        return actor;
    }

    void PathLineVtk::SetData(Data* data) {
        mData = data;
        UpdatePathlinePipeline();
        emit RequestRender();
    }

    void PathLineVtk::UpdatePathlinePipeline() {
        if (!mData) {
            std::cerr << "Error: No data available for pathlines." << std::endl;
            return;
        }

        vtkImageData* velocityData = mData->GetField(Data::EField::Velocity);
        if (!velocityData) {
            std::cerr << "Error: Velocity data is null." << std::endl;
            return;
        }

        vtkDataArray* velocityArray = velocityData->GetPointData()->GetVectors();
        if (!velocityArray) {
            std::cerr << "Error: No velocity vector data found." << std::endl;
            return;
        }

        vtkFloatArray* velocityArrayFloat = vtkFloatArray::SafeDownCast(velocityArray);

        vtkNew<vtkImageData> timeStepData;
        timeStepData->DeepCopy(velocityData);

        for (int t = 0; t < 10; ++t) { // Simulate 10 time steps
            vtkDataArray* velocityStepArray = timeStepData->GetPointData()->GetVectors();
            for (vtkIdType i = 0; i < velocityStepArray->GetNumberOfTuples(); ++i) {
                double* vec = velocityArrayFloat->GetTuple3(i);
                velocityStepArray->SetTuple3(i, vec[0], vec[1] + t * 0.1, vec[2]); // Simulate temporal variation.
            }

            mStreamTracer->SetInputData(timeStepData);
            mStreamTracer->Update();
        }
    }

} // namespace vispro
