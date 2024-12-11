#pragma once

#include "Component.hpp"
#include <vtkSmartPointer.h>

class vtkStreamTracer;
class vtkPointSource;
class vtkPolyDataMapper;
class vtkActor;
class vtkImageData;

namespace vispro
{
    class StreakLineVtk : public Component
    {
        Q_OBJECT
    public:
        StreakLineVtk();
        virtual ~StreakLineVtk();

    protected:
        virtual void CreateWidget(QWidget* widget) override;
        virtual vtkSmartPointer<vtkProp> CreateActor() override;

    public:
        virtual void SetData(Data* data) override;

    private slots:
        void AdjustStreaklineProperty(double value);
        void PickColor();

    private:
        void UpdateStreaklinePipeline();

    private:
        Data* mData;
        double mSliderValue;  // Default value for adjusting the streakline properties

        vtkSmartPointer<vtkStreamTracer> mStreamTracer;
        vtkSmartPointer<vtkPolyDataMapper> mStreaklineMapper;
        vtkSmartPointer<vtkPointSource> mPointSource;
    };
}
