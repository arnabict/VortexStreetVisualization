#include "StreamLineVtk.hpp"
#include "Data.hpp"
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkPointSource.h>
#include <vtkStreamTracer.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageGaussianSmooth.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <qformlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include <iostream>
#include <vtkFloatArray.h>
#include "QDoubleSlider.hpp"
#include <Eigen/Dense>

namespace vispro
{
    StreamLineVtk::StreamLineVtk()
        : Component("StreamLineVtk"),
        mStreamTracer(nullptr),
        mStreamlineMapper(nullptr),
        mPointSource(nullptr),
        mSliderValue(0),
        mData(nullptr) // Initialize to null
    {}

    StreamLineVtk::~StreamLineVtk()
    {}

    void StreamLineVtk::CreateWidget(QWidget* widget)
    {
        QPushButton* colorButton = new QPushButton;
        double* color = vtkActor::SafeDownCast(mActor)->GetProperty()->GetColor();
        QColor col = QColor::fromRgbF(color[0], color[1], color[2]);
        QString qss = QString("background-color: %1").arg(col.name());
        colorButton->setStyleSheet(qss);
        connect(colorButton, &QPushButton::released, this, &StreamLineVtk::PickColor);

        // Label to show the slider value
        QLabel* valueLabel = new QLabel(QString::number(mSliderValue, 'f', 2));

        // Slider for subtraction value
        QSlider* subtractionSlider = new QSlider(Qt::Horizontal);
        subtractionSlider->setRange(0, 120);  // Range for 0.0 to 1.0 with 0.01 increments
        subtractionSlider->setSingleStep(1); // 0.01 step size
        subtractionSlider->setValue(static_cast<int>(mSliderValue * 100)); // Default value scaled to slider range

        connect(subtractionSlider, &QSlider::valueChanged, this, [this, valueLabel](int value) {
            double subtractionValue = static_cast<double>(value) / 100.0; // Scale to double
            AdjustStreamlineProperty(subtractionValue);
            valueLabel->setText(QString::number(subtractionValue, 'f', 2));
            });

        QFormLayout* layout = new QFormLayout;
        layout->addRow(new QLabel(tr("Color:")), colorButton);
        layout->addRow(new QLabel(tr("Change Value:")), subtractionSlider);
        layout->addRow(new QLabel(tr("Subtraction:")), valueLabel);
        widget->setLayout(layout);
    }

    void StreamLineVtk::AdjustStreamlineProperty(double value)
    {
        mSliderValue = value; // Update the slider value
        SetData(mData);       // Reprocess data with the new slider value
        emit RequestRender(); // Request an update to the render
    }

    vtkSmartPointer<vtkProp> StreamLineVtk::CreateActor()
    {
        mPointSource = vtkSmartPointer<vtkPointSource>::New();
        mPointSource->SetCenter(0.0, 0.0, 0.0);
        mPointSource->SetDistributionToUniform();
        mPointSource->SetRadius(2.0);
        mPointSource->SetNumberOfPoints(500);

        mStreamTracer = vtkSmartPointer<vtkStreamTracer>::New();      
        mStreamTracer->SetSourceConnection(mPointSource->GetOutputPort());
        mStreamTracer->SetIntegrationDirectionToBoth();
        mStreamTracer->SetMaximumPropagation(3000);
        mStreamTracer->SetMaximumNumberOfSteps(2000);

        mStreamlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mStreamlineMapper->SetInputConnection(mStreamTracer->GetOutputPort());

        vtkNew<vtkActor> actor;
        actor->SetMapper(mStreamlineMapper);
        actor->GetProperty()->SetColor(0.0, 0.33333333333, 0.0);
        return actor;
    }

    void StreamLineVtk::PickColor()
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

    //void StreamLineVtk::SetData(Data* data)
    //{
    //    mData = data;

    //    // Getting velocity data
    //    vtkImageData* velocityData = data->GetField(Data::EField::Velocity);
    //    if (velocityData == nullptr) {
    //        std::cerr << "Error: Velocity data is null." << std::endl;
    //        return;
    //    }

    //    vtkImageData* featureFlowData = data->GetField(Data::EField::FeatureFlow);
    //    featureFlowData->Print(std::cout);
    //    vtkDataArray* featureFlowArray = featureFlowData->GetPointData()->GetVectors();
    //    vtkFloatArray* featureFlowUArrayFloat = vtkFloatArray::SafeDownCast(featureFlowArray);

    //    // Gaussian smoother ********************
    //    //vtkSmartPointer<vtkImageGaussianSmooth> smoother = vtkSmartPointer<vtkImageGaussianSmooth>::New();
    //    //smoother->SetInputData(featureFlowData);

    //    //double stdDev[3] = { 2.0, 2.0, 2.0 };
    //    //smoother->SetStandardDeviations(stdDev);
    //    ////smoother->SetStandardDeviation(1.0);
    //    //smoother->SetRadiusFactors(1.5, 1.5, 1.5);
    //    //smoother->Update();

    //    //vtkImageData* smoothedFeatureFlowData = smoother->GetOutput();
    //    //smoothedFeatureFlowData->Print(std::cout);
    //    //vtkDataArray* smoothedFeatureFlowArray = smoothedFeatureFlowData->GetPointData()->GetVectors();
    //    //vtkFloatArray* smoothedFeatureFlowUArrayFloat = vtkFloatArray::SafeDownCast(smoothedFeatureFlowArray);

    //    //if (!smoothedFeatureFlowUArrayFloat) {
    //    //    std::cerr << "Error: Smoothed Feature Flow array is not vtkFloatArray or is null." << std::endl;
    //    //    return;
    //    //}
    //    // ****************************************

    //    int dimensions[3];
    //    velocityData->GetDimensions(dimensions);

    //    double origin[3];
    //    velocityData->GetOrigin(origin);

    //    double spacing[3];
    //    velocityData->GetSpacing(spacing);

    //    vtkDataArray* velocityArray = velocityData->GetPointData()->GetVectors();
    //    if (velocityArray == nullptr) {
    //        std::cerr << "Error: No velocity vector data found in the field." << std::endl;
    //        return;
    //    }

    //    vtkFloatArray* velocityArrayFloat = vtkFloatArray::SafeDownCast(velocityArray);

    //    vtkNew<vtkImageData> field;
    //    field->SetDimensions(dimensions);
    //    field->SetOrigin(origin);
    //    field->SetSpacing(spacing);

    //    vtkNew<vtkFloatArray> mNum;
    //    // Calculate total number of grid points in field
    //    int64_t numPoints = (int64_t)field->GetDimensions()[0] 
    //        * field->GetDimensions()[1] 
    //        * field->GetDimensions()[2];
    //    mNum->SetNumberOfComponents(velocityArray->GetNumberOfComponents());
    //    mNum->SetNumberOfTuples(velocityArray->GetNumberOfTuples());
    //    mNum->SetName(velocityArray->GetName());

    //    // Attach velocityArray to grid
    //    field->GetPointData()->AddArray(mNum);
    //    field->GetPointData()->SetActiveVectors(velocityArray->GetName());

    //    /*std::cout << mNum->GetNumberOfTuples();
    //    std::cout << numPoints;*/

    //    for (int z = 0; z < dimensions[2]; ++z) {
    //        for (int y = 0; y < dimensions[1]; ++y) {
    //            for (int x = 0; x < dimensions[0]; ++x) {
    //                int linear = x + dimensions[0] * (y + dimensions[1] * z);
    //                double* point = velocityArrayFloat->GetTuple3(linear);
    //                double* f = featureFlowUArrayFloat->GetTuple3(linear);
    //                mNum->SetTuple3(linear,
    //                    point[0] - f[0], // Adjust using slider
    //                    point[1] - f[1],
    //                    point[2] - f[2]);
    //            }
    //        }
    //    }

    //    mStreamTracer->SetInputData(field);
    //    mStreamTracer->Update();

    //    /*field->Print(std::cout);
    //    velocityData->Print(std::cout);*/
    //    //mStreamTracer->GetOutput()->Print(std::cout);
    //    //mStreamlineActor->GetProperty()->SetColor(0.0, 1.0, 0.0);

    //    emit RequestRender();
    //}

    void StreamLineVtk::SetData(Data* data)
    {
        mData = data;

        vtkImageData* velocityData = data->GetField(Data::EField::Velocity);
        if (!velocityData) {
            std::cerr << "Error: Velocity data is null." << std::endl;
            return;
        }

        vtkImageData* featureFlowData = data->GetField(Data::EField::FeatureFlow);
        if (!featureFlowData) {
            std::cerr << "Error: Feature flow data is null." << std::endl;
            return;
        }

        vtkDataArray* featureFlowArray = featureFlowData->GetPointData()->GetVectors();
        if (!featureFlowArray || featureFlowArray->GetNumberOfComponents() != 3) {
            std::cerr << "Error: Feature flow data must be a 3-component vector field." << std::endl;
            return;
        }

        vtkNew<vtkFloatArray> uComponentArray;
        vtkNew<vtkFloatArray> vComponentArray;
        vtkNew<vtkFloatArray> wComponentArray;

        uComponentArray->SetNumberOfComponents(1);
        vComponentArray->SetNumberOfComponents(1);
        wComponentArray->SetNumberOfComponents(1);

        uComponentArray->SetNumberOfTuples(featureFlowArray->GetNumberOfTuples());
        vComponentArray->SetNumberOfTuples(featureFlowArray->GetNumberOfTuples());
        wComponentArray->SetNumberOfTuples(featureFlowArray->GetNumberOfTuples());

        for (vtkIdType i = 0; i < featureFlowArray->GetNumberOfTuples(); ++i) {
            double vector[3];
            featureFlowArray->GetTuple(i, vector);
            uComponentArray->SetTuple1(i, vector[0]);
            vComponentArray->SetTuple1(i, vector[1]);
            wComponentArray->SetTuple1(i, vector[2]);
        }

        vtkNew<vtkImageGaussianSmooth> gaussianSmoothU;
        vtkNew<vtkImageGaussianSmooth> gaussianSmoothV;
        vtkNew<vtkImageGaussianSmooth> gaussianSmoothW;

        vtkNew<vtkImageData> uImageData;
        vtkNew<vtkImageData> vImageData;
        vtkNew<vtkImageData> wImageData;

        uImageData->DeepCopy(featureFlowData);
        vImageData->DeepCopy(featureFlowData);
        wImageData->DeepCopy(featureFlowData);

        uImageData->GetPointData()->SetScalars(uComponentArray);
        vImageData->GetPointData()->SetScalars(vComponentArray);
        wImageData->GetPointData()->SetScalars(wComponentArray);

        gaussianSmoothU->SetInputData(uImageData);
        gaussianSmoothV->SetInputData(vImageData);
        gaussianSmoothW->SetInputData(wImageData);

        gaussianSmoothU->SetStandardDeviation(3.0);
        gaussianSmoothV->SetStandardDeviation(3.0);
        gaussianSmoothW->SetStandardDeviation(3.0);

        gaussianSmoothU->SetRadiusFactors(1.5, 1.5, 1.5);
        gaussianSmoothV->SetRadiusFactors(1.5, 1.5, 1.5);
        gaussianSmoothW->SetRadiusFactors(1.5, 1.5, 1.5);

        gaussianSmoothU->Update();
        gaussianSmoothV->Update();
        gaussianSmoothW->Update();

        vtkDataArray* smoothedUArray = gaussianSmoothU->GetOutput()->GetPointData()->GetScalars();
        vtkDataArray* smoothedVArray = gaussianSmoothV->GetOutput()->GetPointData()->GetScalars();
        vtkDataArray* smoothedWArray = gaussianSmoothW->GetOutput()->GetPointData()->GetScalars();

        vtkNew<vtkFloatArray> smoothedFeatureFlowArray;
        smoothedFeatureFlowArray->SetNumberOfComponents(3);
        smoothedFeatureFlowArray->SetNumberOfTuples(featureFlowArray->GetNumberOfTuples());

        for (vtkIdType i = 0; i < smoothedFeatureFlowArray->GetNumberOfTuples(); ++i) {
            double smoothedVector[3] = {
                smoothedUArray->GetTuple1(i),
                smoothedVArray->GetTuple1(i),
                smoothedWArray->GetTuple1(i)
            };
            smoothedFeatureFlowArray->SetTuple(i, smoothedVector);
        }

        smoothedFeatureFlowArray->Print(std::cout);

        int dimensions[3];
        velocityData->GetDimensions(dimensions);

        double origin[3];
        velocityData->GetOrigin(origin);

        double spacing[3];
        velocityData->GetSpacing(spacing);

        vtkDataArray* velocityArray = velocityData->GetPointData()->GetVectors();
        if (!velocityArray) {
            std::cerr << "Error: No velocity vector data found in the field." << std::endl;
            return;
        }

        vtkFloatArray* velocityArrayFloat = vtkFloatArray::SafeDownCast(velocityArray);

        vtkNew<vtkImageData> field;
        field->SetDimensions(dimensions);
        field->SetOrigin(origin);
        field->SetSpacing(spacing);

        vtkNew<vtkFloatArray> mNum;
        mNum->SetNumberOfComponents(velocityArray->GetNumberOfComponents());
        mNum->SetNumberOfTuples(velocityArray->GetNumberOfTuples());
        mNum->SetName(velocityArray->GetName());

        field->GetPointData()->AddArray(mNum);
        field->GetPointData()->SetActiveVectors(velocityArray->GetName());

        for (int z = 0; z < dimensions[2]; ++z) {
            for (int y = 0; y < dimensions[1]; ++y) {
                for (int x = 0; x < dimensions[0]; ++x) {
                    int linear = x + dimensions[0] * (y + dimensions[1] * z);
                    double* point = velocityArrayFloat->GetTuple3(linear);
                    double* f = smoothedFeatureFlowArray->GetTuple3(linear);
                    mNum->SetTuple3(linear,
                        point[0] - f[0],
                        point[1] - f[1],
                        point[2] - f[2]);
                }
            }
        }

        mStreamTracer->SetInputData(field);
        mStreamTracer->Update();

        emit RequestRender();
    }
}
