#include "BoundingBox.hpp"
#include "Data.hpp"
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkImageData.h>
#include <qwidget.h>
#include <Eigen/Eigen>
#include <qbuttongroup.h>
#include <qformlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcolor.h>
#include <qcolordialog.h>

namespace vispro
{
	BoundingBox::BoundingBox() : 
        Component("BoundingBox"),
        mPolyDataMapper(NULL) 
    {}
	BoundingBox::~BoundingBox() {}

	void BoundingBox::CreateWidget(QWidget* widget)
	{
        // create button and a color dialog
        QPushButton* colorButton = new QPushButton;
        double* color = vtkActor::SafeDownCast(mActor)->GetProperty()->GetColor();
        QColor col = QColor::fromRgbF(color[0], color[1], color[2]);
        QString qss = QString("background-color: %1").arg(col.name());
        colorButton->setStyleSheet(qss);
        QFormLayout* layout = new QFormLayout;
        layout->addRow(new QLabel(tr("Box color:")), colorButton);
        widget->setLayout(layout);
        connect(colorButton, &QPushButton::released, this, &BoundingBox::PickColor);
	}

	vtkSmartPointer<vtkProp> BoundingBox::CreateActor()
	{
        mPolyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        vtkNew<vtkActor> actor;
        actor->SetMapper(mPolyDataMapper);
        actor->GetProperty()->SetColor(0.3, 0.3, 0.3);   // initial color
        return actor;
	}

	void BoundingBox::SetData(Data* data)
	{
        // get the bounding box
        const double* bounds = data->GetBounds();

        // create the vertex data
        std::vector<Eigen::Vector3d> pts;
        for (int i = 0; i < 2; ++i)
            for (int j = 2; j < 4; ++j)
                for (int k = 4; k < 6; ++k)
                    pts.push_back(Eigen::Vector3d(bounds[i], bounds[j], bounds[k]));
        vtkNew<vtkPolyData> boundsPolydata;
        vtkNew<vtkPoints> boundsPoints;
        for (int i = 0; i < 8; ++i)
            boundsPoints->InsertNextPoint(pts[i].data());
        boundsPolydata->SetPoints(boundsPoints);

        // create the index data
        vtkNew<vtkCellArray> cells;
        vtkIdType cell[2] = { 0, 1 };
        cells->InsertNextCell(2, cell);
        cell[0] = 0; cell[1] = 2;
        cells->InsertNextCell(2, cell);
        cell[0] = 3; cell[1] = 2;
        cells->InsertNextCell(2, cell);
        cell[0] = 3; cell[1] = 1;
        cells->InsertNextCell(2, cell);
        cell[0] = 4; cell[1] = 5;
        cells->InsertNextCell(2, cell);
        cell[0] = 4; cell[1] = 6;
        cells->InsertNextCell(2, cell);
        cell[0] = 7; cell[1] = 5;
        cells->InsertNextCell(2, cell);
        cell[0] = 7; cell[1] = 6;
        cells->InsertNextCell(2, cell);
        cell[0] = 1; cell[1] = 5;
        cells->InsertNextCell(2, cell);
        cell[0] = 0; cell[1] = 4;
        cells->InsertNextCell(2, cell);
        cell[0] = 2; cell[1] = 6;
        cells->InsertNextCell(2, cell);
        cell[0] = 3; cell[1] = 7;
        cells->InsertNextCell(2, cell);
        boundsPolydata->SetLines(cells);

        // assign the polydata to the mapper
        mPolyDataMapper->SetInputData(boundsPolydata);
	}

    void BoundingBox::PickColor()
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