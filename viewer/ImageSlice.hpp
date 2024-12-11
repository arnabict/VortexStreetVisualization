#pragma once

#include "Component.hpp"

class vtkImageMapToColors;
class vtkImageSliceMapper;
class vtkLookupTable;

namespace vispro
{
	class QDoubleSlider;

	// Displays a single slice of the data using color mapping.
	class ImageSlice : public Component
	{
		Q_OBJECT
	public:
		// Default constructor.
		ImageSlice();
		// Destructor.
		virtual ~ImageSlice();

		// Function to create the widget in.
		virtual void CreateWidget(QWidget* widget) override;
		// Function to create the actor.
		virtual vtkSmartPointer<vtkProp> CreateActor() override;
		// Sets the data of this component. In response, the component will update the VTK resources and the UI elements.
		virtual void SetData(Data* data) override;

	private slots:
		// Sets the lower end of the transfer function range.
		void SetMinColorValue(double minValue);
		// Sets the upper end of the transfer function range.
		void SetMaxColorValue(double maxValue);
		// Sets the position of the slice.
		void SetPosition(double position);
		// Sets the orientation of the slice.
		void SetOrientation(int orientation);

	private:
		// Copy-constructor is deleted.
		ImageSlice(const ImageSlice& other) = delete;

		// Helper function that creates and connects a slider with a given slot member function.
		QDoubleSlider* CreateDoubleSlider(double minValue, double maxValue, double initialValue, void (ImageSlice::* fn)(double)) const;

		// Sets a default color map.
		void SetColormapBlue2Red();

		// Maps the scalar field to color values.
		vtkSmartPointer<vtkImageMapToColors> mImageMapToColors;
		// Displays a single slice of the color data.
		vtkSmartPointer<vtkImageSliceMapper> mImageSliceMapper;
		// Lookup table
		vtkSmartPointer<vtkLookupTable> mLookupTable;

		// Value range that is mapped to color.
		double mColorRange[2];

		// Position of the slice in relative coordinates [0,1]
		double mSlicePosition;
		// Orientation of the slice.
		enum class EOrientation { X, Y, Z } mOrientation;
	};
}