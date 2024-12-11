#include "QDoubleSlider.hpp"

namespace vispro
{
	QDoubleSlider::QDoubleSlider(Qt::Orientation orientation, QWidget* parent) : QSlider(orientation, parent) {
		connect(this, SIGNAL(valueChanged(int)), this, SLOT(notifyValueChanged(int)));
		scaleFactor = 100;
	}

	double QDoubleSlider::doubleValue() { return value() / (double)intScaleFactor(); }
	
	int QDoubleSlider::intScaleFactor() { return scaleFactor; }

	void QDoubleSlider::setDoubleValue(double v) {
		int newvalue = v * (double)intScaleFactor();
		int curvalue = value();
		if (newvalue != curvalue)
			setValue(newvalue);
	}

	void QDoubleSlider::notifyValueChanged(int value) {
		double doubleValue = value / (double)intScaleFactor();
		emit doubleValueChanged(doubleValue);
	}
}