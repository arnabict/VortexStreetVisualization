#pragma once

#include "Component.hpp"

class vtkSphereSource;
class vtkGlyph3D;
class vtkPolyDataMapper;

namespace vispro
{
	// Displays a set of particles using spheres.
	class Particles : public Component
	{
		Q_OBJECT
	public:
		// Default constructor.
		Particles();
		// Destructor.
		virtual ~Particles();

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
		// Sets an radius of the spheres.
		void SetRadius(double radius);

	private:
		// Copy-constructor is deleted.
		Particles(const Particles& other) = delete;

		// Source for creating spheres.
		vtkSmartPointer<vtkSphereSource> mSphereSource;
		// Glyphs for rendering of the particles.
		vtkSmartPointer<vtkGlyph3D> mGlyphs;
		// Maps the polydata.
		vtkSmartPointer<vtkPolyDataMapper> mPolyDataMapper;
	};
}