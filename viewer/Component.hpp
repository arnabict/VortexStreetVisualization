#pragma once

#include <qobject.h>
#include <vtkSmartPointer.h>

class QWidget;
class QGroupBox;
class vtkActor;
class vtkProp;

namespace vispro
{
	class Data;

	// Base class for a component.
	class Component : public QObject
	{
		Q_OBJECT

	public:
		// Constructor.
		Component(const std::string& name);

		// Function to create the widget.
		QWidget* CreateWidget();
		// Function to create the actor.
		vtkSmartPointer<vtkProp> CreateVTK();
		// Is this component currently enabled?
		bool IsVisible() const;

	protected:
		// Function that adds the UI components to the widget.
		virtual void CreateWidget(QWidget* widget) = 0;
		// Function to create the actor.
		virtual vtkSmartPointer<vtkProp> CreateActor() = 0;
		
	public:
		// Sets the data of this component. In response, the component will update the VTK resources and the UI elements.
		virtual void SetData(Data* data) = 0;

	public slots:
		// Sets the visibility of this component in the VTK render window.
		void SetVisible(bool visible);
	signals:
		// Signal that can be raised to request a re-rendering of the viewport, for example when a parameter on the UI was changed.
		void RequestRender();
	signals:
		// Signal that can be raised to request a SetData() call with the current data.
		void RequestData();

	private:
		// Name of the component.
		std::string mName;
		// Group box that holds the UI elements.
		QGroupBox* mGroupBox;

	protected:
		// Actor of this component
		vtkSmartPointer<vtkProp> mActor;
	};
}