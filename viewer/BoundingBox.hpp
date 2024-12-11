#pragma once

#include "Component.hpp"

class vtkPolyDataMapper;

namespace vispro
{
	// Displays the bounding box of the domain.
	class BoundingBox : public Component
	{
		Q_OBJECT
	public:
		// Default constructor.
		BoundingBox();
		// Destructor.
		virtual ~BoundingBox();

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

	private:
		// Copy-constructor is deleted.
		BoundingBox(const BoundingBox& other) = delete;

		// Maps the polydata.
		vtkSmartPointer<vtkPolyDataMapper> mPolyDataMapper;
	};
}