#include "Particles.hpp"
#include "Data.hpp"
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkGlyph3D.h>
#include <vtkSphereSource.h>
#include <qwidget.h>
#include <qformlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcolor.h>
#include <qcolordialog.h>
#include "QDoubleSlider.hpp"

namespace vispro
{
    Particles::Particles() :
        Component("Particles"),
        mPolyDataMapper(NULL), mGlyphs(NULL), mSphereSource(NULL)
    {}
    Particles::~Particles() {}

    void Particles::CreateWidget(QWidget* widget)
    {
        // create button and a color dialog
        QPushButton* colorButton = new QPushButton;
        double* color = vtkActor::SafeDownCast(mActor)->GetProperty()->GetColor();
        QColor col = QColor::fromRgbF(color[0], color[1], color[2]);
        QString qss = QString("background-color: %1").arg(col.name());
        colorButton->setStyleSheet(qss);
        connect(colorButton, &QPushButton::released, this, &Particles::PickColor);

        // create slider for setting the radius of the spheres
        QDoubleSlider* radiusSlider = new QDoubleSlider;
        radiusSlider->setMinimum(0);                               // minimal value on slider
        radiusSlider->setMaximum(0.1 * radiusSlider->intScaleFactor()); // maximal value on slider
        radiusSlider->setDoubleValue(mSphereSource->GetRadius());
        connect(radiusSlider, &QDoubleSlider::doubleValueChanged, this, &Particles::SetRadius);

        QFormLayout* layout = new QFormLayout;
        layout->addRow(new QLabel(tr("Color:")), colorButton);
        layout->addRow(new QLabel(tr("Radius:")), radiusSlider);
        widget->setLayout(layout);
    }

    vtkSmartPointer<vtkProp> Particles::CreateActor()
    {
        mSphereSource = vtkSmartPointer<vtkSphereSource>::New();
        mSphereSource->SetRadius(0.02);

        mGlyphs = vtkSmartPointer<vtkGlyph3D>::New();
        mGlyphs->SetSourceConnection(mSphereSource->GetOutputPort());

        mPolyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        //mPolyDataMapper->SetInputConnection(mGlyphs->GetOutputPort());
        mPolyDataMapper->SetInputData(mGlyphs->GetOutput());
        mPolyDataMapper->Update();


        vtkNew<vtkActor> actor;
        actor->SetMapper(mPolyDataMapper);
        actor->GetProperty()->SetColor(0.9, 0.9, 0.5);     // initial color
        return actor;
    }

    void Particles::SetData(Data* data)
    {
        vtkPolyData* pointData = data->GetParticles();
        if (pointData == nullptr) return;
        /*mGlyphs->SetInputData(pointData);
        mGlyphs->Update();*/
        mPolyDataMapper->SetInputData(pointData);
        mPolyDataMapper ->Update();
    }

    void Particles::PickColor()
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

    void Particles::SetRadius(double radius)
    {
        mSphereSource->SetRadius(radius);
        mSphereSource->Update();
        emit RequestRender();
    }
}