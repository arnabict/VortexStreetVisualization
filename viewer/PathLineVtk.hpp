#pragma once

#include "Component.hpp"
#include <vtkSmartPointer.h>

class vtkStreamTracer;
class vtkPointSource;
class vtkPolyDataMapper;
class vtkActor;
class vtkImageData;

namespace vispro {

    // Displays pathlines using vtkStreamTracer.
    class PathLineVtk : public Component {
        Q_OBJECT
    public:
        PathLineVtk();
        virtual ~PathLineVtk();

    protected:
        // Create the UI widget.
        virtual void CreateWidget(QWidget* widget) override;
        // Create the actor for the pathlines.
        virtual vtkSmartPointer<vtkProp> CreateActor() override;

    public:
        // Set data for the component.
        virtual void SetData(Data* data) override;

    private:
        void UpdatePathlinePipeline();

    private:
        // Stream tracer for generating pathlines.
        vtkSmartPointer<vtkStreamTracer> mStreamTracer;
        // Point source for seed points.
        vtkSmartPointer<vtkPointSource> mPointSource;
        // Mapper for pathlines.
        vtkSmartPointer<vtkPolyDataMapper> mPathlineMapper;

        // The velocity field data.
        Data* mData;
    };

} // namespace vispro
