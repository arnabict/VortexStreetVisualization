#include "IsosurfaceVtk.hpp"
#include "Data.hpp"
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkImageData.h>
#include <vtkFlyingEdges3D.h>
#include <qwidget.h>
#include <qformlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include "QDoubleSlider.hpp"

namespace vispro
{
    IsosurfaceVtk::IsosurfaceVtk() :
        Component("IsosurfaceVtk"),
        mPolyDataMapper(NULL), mMarchingCubes(NULL)
    {}
    IsosurfaceVtk::~IsosurfaceVtk() {}

    void IsosurfaceVtk::CreateWidget(QWidget* widget)
    {
        // create button and a color dialog
        QPushButton* colorButton = new QPushButton;
        double* color = vtkActor::SafeDownCast(mActor)->GetProperty()->GetColor();
        QColor col = QColor::fromRgbF(color[0], color[1], color[2]);
        QString qss = QString("background-color: %1").arg(col.name());
        colorButton->setStyleSheet(qss);
        connect(colorButton, &QPushButton::released, this, &IsosurfaceVtk::PickColor);

        // create slider for setting the isovalue
        QDoubleSlider* isoSlider = new QDoubleSlider;
        isoSlider->setMinimum(0);                               // minimal isovalue on slider
        isoSlider->setMaximum(5 * isoSlider->intScaleFactor()); // maximal isovalue on slider
        isoSlider->setDoubleValue(mMarchingCubes->GetValue(0));
        connect(isoSlider, &QDoubleSlider::doubleValueChanged, this, &IsosurfaceVtk::SetIsovalue);

        QFormLayout* layout = new QFormLayout;
        layout->addRow(new QLabel(tr("Color:")), colorButton);
        layout->addRow(new QLabel(tr("Isovalue:")), isoSlider);
        widget->setLayout(layout);
    }

    vtkSmartPointer<vtkProp> IsosurfaceVtk::CreateActor()
    {
        mMarchingCubes = vtkSmartPointer<vtkFlyingEdges3D>::New();
        mMarchingCubes->SetValue(0, 1.5);                   // initial threshold
        mMarchingCubes->ComputeScalarsOff();

        mPolyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        mPolyDataMapper->SetInputConnection(mMarchingCubes->GetOutputPort());

        vtkNew<vtkActor> actor;
        actor->SetMapper(mPolyDataMapper);
        actor->GetProperty()->SetColor(0.5, 0.5, 0.5);     // initial color
        return actor;
    }

    void IsosurfaceVtk::SetData(Data* data)
    {
        vtkImageData* imageData = data->GetField(Data::EField::Vorticity);
        if (imageData == nullptr) return;
        mMarchingCubes->SetInputData(imageData);
        mMarchingCubes->Update();
    }

    void IsosurfaceVtk::PickColor()
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

    void IsosurfaceVtk::SetIsovalue(double isovalue)
    {
        mMarchingCubes->SetValue(0, isovalue);
        mMarchingCubes->Update();
        emit RequestRender();
    }
}