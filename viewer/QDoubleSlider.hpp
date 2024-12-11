#pragma once

#include <qslider.h>

namespace vispro
{
	// Slider for the setting of double values. This is actually using a standard integer slider, but it divides the integer number by a "scaleFactor".
	class QDoubleSlider : public QSlider
	{
		Q_OBJECT

	public:
		// Constructor.
		QDoubleSlider(Qt::Orientation orientation = Qt::Orientation::Horizontal,
			QWidget* parent = 0);

		// Gets the value of the slider.
		double doubleValue();
		// Gets the internal scaling factor.
		int intScaleFactor();

		// Sets the value of the slider.
		void setDoubleValue(double v);

	Q_SIGNALS:
		// Signal that gets raised when the value of the slider changes.
		void doubleValueChanged(double value);

	public Q_SLOTS:
		// Slots that listens to changes to the slider by the user.
		void notifyValueChanged(int value);
	private:
		// The scaling factor that is used to convert from integer to double.
		int scaleFactor;
	};
}