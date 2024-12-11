#pragma once

#include "Component.hpp"

class vtkOpenGLGPUVolumeRayCastMapper;
class vtkColorTransferFunction;
class vtkPiecewiseFunction;

namespace vispro
{
	class QDoubleSlider;

	// Displays a volume rendering of a scalar field, computed using custom shaders.
	class VolrenShader : public Component
	{
		Q_OBJECT
	public:
		// Default constructor.
		VolrenShader();
		// Destructor.
		virtual ~VolrenShader();

	protected:
		// Function to create the widget in.
		virtual void CreateWidget(QWidget* widget) override;
		// Function to create the actor.
		virtual vtkSmartPointer<vtkProp> CreateActor() override;

	public:
		// Sets the data of this component. In response, the component will update the VTK resources and the UI elements.
		virtual void SetData(Data* data) override;

	private slots:
		// Sets the lower end of the transfer function range.
		void SetMinColorValue(double minValue);
		// Sets the upper end of the transfer function range.
		void SetMaxColorValue(double maxValue);

		// Sets the lower end of the transfer function range.
		void SetMinOpacityValue(double minValue);
		// Sets the upper end of the transfer function range.
		void SetMaxOpacityValue(double maxValue);

		// Sets the extinction scaling factor.
		void SetExtinctionValue(double scale);

		// Sets the boolean flag that turns the shadow computation on or off.
		void SetShadowEnabledValue(int state);

	private:
		// Copy-constructor is deleted.
		VolrenShader(const VolrenShader& other) = delete;

		// Helper function that creates and connects a slider with a given slot member function.
		QDoubleSlider* CreateDoubleSlider(double minValue, double maxValue, double initialValue, void (VolrenShader::* fn)(double)) const;

		// Sets a default color map.
		void SetColormapBlue2Red();

		// Transfer function for color.
		vtkSmartPointer<vtkColorTransferFunction> mColorTransferFunction;
		// Transfer function for opacity.
		vtkSmartPointer<vtkPiecewiseFunction> mOpacityTransferFunction;
		// OpenGL volume renderer.
		vtkSmartPointer<vtkOpenGLGPUVolumeRayCastMapper> mVolren;

		// Value range that is mapped to color.
		double mColorRange[2];
		// Value range that is mapped to opacity.
		double mOpacityRange[2];
		// Scaling factor of the extinction.
		double mExtinction;

		// Boolean flag {0,1} that turns shadow computations on or off.
		int mShadowEnabled;
	};
}