#pragma once

#include "Component.hpp"
#include <vtkSmartPointer.h>

class vtkStreamTracer;
class vtkPointSource;
//class vtkCubeSource;
class vtkPolyDataMapper;
class vtkActor;

namespace vispro
{
    // Displays streamlines from a velocity field using VTK.
    class StreamLineVtk : public Component
    {
        Q_OBJECT
    public:
        // Default constructor.
        StreamLineVtk();
        // Destructor.
        virtual ~StreamLineVtk();

    protected:
        // Function to create the widget in.
        virtual void CreateWidget(QWidget* widget) override;
        // Function to create the actor.
        virtual vtkSmartPointer<vtkProp> CreateActor() override;

    public:
        // Sets the data of this component. In response, the component will update the VTK resources and the UI elements.
        virtual void SetData(Data* data) override;

    private slots:

        // Slot to handle double values
        void AdjustStreamlineProperty(double value);
        // Opens the color picker.
        void PickColor();

    private:
    
        Data* mData;
        //// Double subtraction value
        double mSliderValue;  // Default value for the subtraction is 1.0;

        // Copy-constructor is deleted.
        StreamLineVtk(const StreamLineVtk& other) = delete;

        // Stream tracer for generating the streamlines.
        vtkSmartPointer<vtkStreamTracer> mStreamTracer;
        // Mapper for the streamlines.
        vtkSmartPointer<vtkPolyDataMapper> mStreamlineMapper;
        // Actor for the streamlines.
        
        //vtkSmartPointer<vtkActor> mStreamlineActor;
        // Seed points for the streamlines.
        vtkSmartPointer<vtkPointSource> mPointSource;
        // Cube source for the streamlines.
        //vtkSmartPointer<vtkCubeSource> mCubeSource;
    };
}
