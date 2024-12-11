#include <qapplication.h>
#include "MainWindow.hpp"
#include <Windows.h>

#ifdef __APPLE__
#include <QApplication>
#include <QGLFormat>
#include "QVTKOpenGLWidget.h"
#endif

// ---------------------------------------
int main(int argc, char* argv[])
{
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
	printf("Debugging Window:\n");

	if (argc < 2) {
		// Pass the data base path to the data as command line argument!
		// e.g., D:/halfcylinder3d-Re320_vti/
		return -1;
	}

#ifdef __APPLE__
	QSurfaceFormat::setDefaultFormat(QVTKOpenGLStereoWidget::defaultFormat());
#endif

	// create the application
	QApplication app(argc, argv);

	// create the main window
	vispro::MainWindow window(argv[1]);

	// show the window and run the application
	window.show();
	return app.exec();
}
