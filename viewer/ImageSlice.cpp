#include "ImageSlice.hpp"
#include "Data.hpp"
#include <vtkImageMapToColors.h>
#include <vtkImageSlice.h>
#include <vtkImageSliceMapper.h>
#include <vtkLookupTable.h>
#include <vtkImageData.h>
#include <qwidget.h>
#include <qformlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include "QDoubleSlider.hpp"

namespace vispro
{
    ImageSlice::ImageSlice() :
        Component("ImageSlice"),
        mImageMapToColors(NULL),
        mImageSliceMapper(NULL),
        mLookupTable(NULL),
        mSlicePosition(0),
        mOrientation(EOrientation::X)
    {
        mColorRange[0] = 10;
        mColorRange[1] = 0;
    }
    ImageSlice::~ImageSlice() {}

    void ImageSlice::CreateWidget(QWidget* widget)
    {
        // create slider for setting the value ranges
        QDoubleSlider* colorMinSlider = CreateDoubleSlider(0, 10, mColorRange[0], &ImageSlice::SetMinColorValue);
        QDoubleSlider* colorMaxSlider = CreateDoubleSlider(0, 10, mColorRange[1], &ImageSlice::SetMaxColorValue);

        // create combobox for selecting the orientation
        QComboBox* orientationBox = new QComboBox;
        orientationBox->addItem("X");
        orientationBox->addItem("Y");
        orientationBox->addItem("Z");
        connect(orientationBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ImageSlice::SetOrientation);

        // create slider for setting the slice position
        QDoubleSlider* positionSlider = new QDoubleSlider;
        positionSlider->setMinimum(0);                               // minimal isovalue on slider
        positionSlider->setMaximum(1 * positionSlider->intScaleFactor()); // maximal isovalue on slider
        positionSlider->setDoubleValue(mSlicePosition);
        connect(positionSlider, &QDoubleSlider::doubleValueChanged, this, &ImageSlice::SetPosition);

        QFormLayout* layout = new QFormLayout;
        layout->addRow(new QLabel(tr("Color min:")), colorMinSlider);
        layout->addRow(new QLabel(tr("Color max:")), colorMaxSlider);
        layout->addRow(new QLabel(tr("Orientation:")), orientationBox);
        layout->addRow(new QLabel(tr("Position:")), positionSlider);
        widget->setLayout(layout);
    }

    QDoubleSlider* ImageSlice::CreateDoubleSlider(double minValue, double maxValue, double initialValue, void (ImageSlice::* fn)(double)) const {
        QDoubleSlider* slider = new QDoubleSlider;
        slider->setMinimum(minValue * slider->intScaleFactor());    // minimal value on slider
        slider->setMaximum(maxValue * slider->intScaleFactor());    // maximal value on slider
        slider->setDoubleValue(initialValue);
        connect(slider, &QDoubleSlider::doubleValueChanged, this, fn);
        return slider;
    }

    vtkSmartPointer<vtkProp> ImageSlice::CreateActor()
    {
        // Create a lookup table
        mLookupTable = vtkSmartPointer<vtkLookupTable>::New();
        SetColormapBlue2Red();

        // Maps scalar values to colors
        mImageMapToColors = vtkSmartPointer<vtkImageMapToColors>::New();
        mImageMapToColors->SetLookupTable(mLookupTable);
        mImageMapToColors->SetInputData(vtkSmartPointer<vtkImageData>::New());

        // Displays the slice
        mImageSliceMapper = vtkSmartPointer<vtkImageSliceMapper>::New();
        mImageSliceMapper->SetInputConnection(mImageMapToColors->GetOutputPort());
        mImageSliceMapper->SetOrientation((int)mOrientation);
        mImageSliceMapper->SetSliceNumber(mSlicePosition);

        // Construct the actor
        vtkNew<vtkImageSlice> actor;
        actor->SetMapper(mImageSliceMapper);
        return actor;
    }

    void ImageSlice::SetColormapBlue2Red()
    {
        const int numPoints = 256;
        mLookupTable->SetNumberOfTableValues(numPoints);
        mLookupTable->SetRange(std::min(mColorRange[0], mColorRange[1]), std::max(mColorRange[0], mColorRange[1]));
        for (int i = 0; i < numPoints; ++i) {
            double t = i / (double)(numPoints - 1);
            if (mColorRange[0] > mColorRange[1]) t = 1 - t;
            double r = std::min(std::max(0.0, (((5.0048 * t + -8.0915) * t + 1.1657) * t + 1.4380) * t + 0.6639), 1.0);
            double g = std::min(std::max(0.0, (((7.4158 * t + -15.9415) * t + 7.4696) * t + 1.2767) * t + -0.0013), 1.0);
            double b = std::min(std::max(0.0, (((6.1246 * t + -16.2287) * t + 11.9910) * t + -1.4886) * t + 0.1685), 1.0);
            mLookupTable->SetTableValue(i, r, g, b);
        }
        mLookupTable->Build();
    }

    void ImageSlice::SetData(Data* data) {
        vtkImageData* imageData = data->GetField(Data::EField::FTLE);
        if (imageData == nullptr) return;
        mImageMapToColors->SetInputData(imageData);
        mImageMapToColors->Update();
    }

    void ImageSlice::SetMinColorValue(double minValue) {
        mColorRange[0] = minValue;
        SetColormapBlue2Red();
        emit RequestRender();
    }
    void ImageSlice::SetMaxColorValue(double maxValue) {
        mColorRange[1] = maxValue;
        SetColormapBlue2Red();
        emit RequestRender();
    }

    void ImageSlice::SetPosition(double position) {
        vtkImageData* imageData = dynamic_cast<vtkImageData*>(mImageMapToColors->GetInput());
        if (!imageData) return;
        int numSlices = imageData->GetDimensions()[(int)mOrientation];
        int slice = std::min(std::max(0, (int)(position * numSlices)), numSlices - 1);
        mImageSliceMapper->SetSliceNumber(slice);
        emit RequestRender();
    }

    void ImageSlice::SetOrientation(int orientation) {
        mOrientation = (EOrientation)orientation;
        mImageSliceMapper->SetOrientation(orientation);
        emit RequestRender();
    }
}