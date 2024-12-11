#include "VolrenVtk.hpp"
#include "Data.hpp"
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkImageData.h>
#include <vtkOpenGLGPUVolumeRayCastMapper.h>
#include <vtkVolume.h>
#include <vtkVolumeProperty.h>
#include <qwidget.h>
#include <qformlayout.h>
#include <qlabel.h>
#include "QDoubleSlider.hpp"
#include <vtkColorTransferFunction.h>
#include <vtkPiecewiseFunction.h>

namespace vispro
{
    VolrenVtk::VolrenVtk() :
        Component("VolrenVtk"),
        mVolren(NULL), mColorTransferFunction(NULL), mOpacityTransferFunction(NULL)
    {
        mColorRange[0] = 10;
        mColorRange[1] = 0;
        mOpacityRange[0] = 0;
        mOpacityRange[1] = 2.5;
        mExtinction = 0.99;
    }
    VolrenVtk::~VolrenVtk() {}

    void VolrenVtk::CreateWidget(QWidget* widget)
    {
        // create slider for setting the value ranges
        QDoubleSlider* colorMinSlider = CreateDoubleSlider(0, 10, mColorRange[0], &VolrenVtk::SetMinColorValue);
        QDoubleSlider* colorMaxSlider = CreateDoubleSlider(0, 10, mColorRange[1], &VolrenVtk::SetMaxColorValue);
        QDoubleSlider* opacityMinSlider = CreateDoubleSlider(0, 10, mOpacityRange[0], &VolrenVtk::SetMinOpacityValue);
        QDoubleSlider* opacityMaxSlider = CreateDoubleSlider(0, 10, mOpacityRange[1], &VolrenVtk::SetMaxOpacityValue);
        QDoubleSlider* extinctionSlider = CreateDoubleSlider(0, 0.999, mExtinction, &VolrenVtk::SetExtinctionValue);

        QFormLayout* layout = new QFormLayout;
        layout->addRow(new QLabel(tr("Color min:")), colorMinSlider);
        layout->addRow(new QLabel(tr("Color max:")), colorMaxSlider);
        layout->addRow(new QLabel(tr("Opacity min:")), opacityMinSlider);
        layout->addRow(new QLabel(tr("Opacity max:")), opacityMaxSlider);
        layout->addRow(new QLabel(tr("Extinction:")), extinctionSlider);
        widget->setLayout(layout);
    }

    QDoubleSlider* VolrenVtk::CreateDoubleSlider(double minValue, double maxValue, double initialValue, void (VolrenVtk::*fn)(double)) const {
        QDoubleSlider* slider = new QDoubleSlider;
        slider->setMinimum(minValue * slider->intScaleFactor());    // minimal value on slider
        slider->setMaximum(maxValue * slider->intScaleFactor());    // maximal value on slider
        slider->setDoubleValue(initialValue);
        connect(slider, &QDoubleSlider::doubleValueChanged, this, fn);
        return slider;
    }

    vtkSmartPointer<vtkProp> VolrenVtk::CreateActor()
    {
        mVolren = vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper>::New();
        mColorTransferFunction = vtkSmartPointer<vtkColorTransferFunction>::New();
        mOpacityTransferFunction = vtkSmartPointer<vtkPiecewiseFunction>::New();

        SetColormapBlue2Red();

        // assign transfer function to volume properties
        vtkSmartPointer<vtkVolumeProperty> volumeProperty = vtkSmartPointer<vtkVolumeProperty>::New();
        volumeProperty->SetColor(mColorTransferFunction);
        volumeProperty->SetScalarOpacity(mOpacityTransferFunction);

        // create volume actor and assign mapper and properties
        vtkSmartPointer<vtkVolume> volume = vtkSmartPointer<vtkVolume>::New();
        volume->SetMapper(mVolren);
        volume->SetProperty(volumeProperty);
        return volume;
    }

    // Sets a default color map.
    void VolrenVtk::SetColormapBlue2Red()
    {
        mColorTransferFunction->RemoveAllPoints();
        mOpacityTransferFunction->RemoveAllPoints();
        int numPoints = 256;
        double prev_xc = std::numeric_limits<double>::max();
        double prev_xo = std::numeric_limits<double>::max();
        for (int i = 0; i < numPoints; ++i)
        {
            double t = i / (double)(numPoints - 1);
            double xc = mColorRange[0] + t * (mColorRange[1] - mColorRange[0]);
            if (std::abs(prev_xc - xc) > 1E-2) {
                double r = std::min(std::max(0.0, (((5.0048 * t + -8.0915) * t + 1.1657) * t + 1.4380) * t + 0.6639), 1.0);
                double g = std::min(std::max(0.0, (((7.4158 * t + -15.9415) * t + 7.4696) * t + 1.2767) * t + -0.0013), 1.0);
                double b = std::min(std::max(0.0, (((6.1246 * t + -16.2287) * t + 11.9910) * t + -1.4886) * t + 0.1685), 1.0);
                mColorTransferFunction->AddRGBPoint(xc, r, g, b);
                prev_xc = xc;
            }
            double xo = mOpacityRange[0] + t * (mOpacityRange[1] - mOpacityRange[0]);
            if (std::abs(prev_xo - xo) > 1E-2) {
                mOpacityTransferFunction->AddPoint(xo, t * mExtinction);
                prev_xo = xo;
            }
        }
        // make sure the last point has full exinction.
        mOpacityTransferFunction->AddPoint(prev_xo, mExtinction);
    }

    void VolrenVtk::SetData(Data* data)
    {
        vtkImageData* imageData = data->GetField(Data::EField::Vorticity);
        if (imageData == nullptr) return;
        mVolren->SetInputData(imageData);
        mVolren->Update();
    }

    void VolrenVtk::SetMinColorValue(double minValue) {
        mColorRange[0] = minValue;
        SetColormapBlue2Red();
        emit RequestRender();
    }
    void VolrenVtk::SetMaxColorValue(double maxValue) {
        mColorRange[1] = maxValue;
        SetColormapBlue2Red();
        emit RequestRender();
    }

    void VolrenVtk::SetMinOpacityValue(double minValue) {
        mOpacityRange[0] = minValue;
        SetColormapBlue2Red();
        emit RequestRender();
    }
    void VolrenVtk::SetMaxOpacityValue(double maxValue) {
        mOpacityRange[1] = maxValue;
        SetColormapBlue2Red();
        emit RequestRender();
    }

    void VolrenVtk::SetExtinctionValue(double scale) {
        mExtinction = scale;
        SetColormapBlue2Red();
        emit RequestRender();
    }
}