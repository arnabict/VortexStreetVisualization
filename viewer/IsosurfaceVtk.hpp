#pragma once

#include "Component.hpp"

class vtkActor;
class vtkFlyingEdges3D;
class vtkPolyDataMapper;

namespace vispro
{
	// Displays the isosurface of a scalar field, computed using VTK algorithms.
	class IsosurfaceVtk : public Component
	{
		Q_OBJECT
	public:
		// Default constructor.
		IsosurfaceVtk();
		// Destructor.
		virtual ~IsosurfaceVtk();

	protected:
		// Function to create the widget in.
		virtual void CreateWidget(QWidget* widget) override;
		// Function to create the actor.
		virtual vtkSmartPointer<vtkProp> CreateActor() override;

	public:
		// Sets the data of this component. In response, the component will update the VTK resources and the UI elements.
		virtual void SetData(Data* data) override;

	private slots:
		// Opens the color picker.
		void PickColor();
		// Sets an isovalue.
		void SetIsovalue(double isovalue);

	private:
		// Copy-constructor is deleted.
		IsosurfaceVtk(const IsosurfaceVtk& other) = delete;

		// Marching cubes implementation
		vtkSmartPointer<vtkFlyingEdges3D> mMarchingCubes;
		// Maps the polydata.
		vtkSmartPointer<vtkPolyDataMapper> mPolyDataMapper;
	};
}