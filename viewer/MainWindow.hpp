#pragma once

#include <qmainwindow.h>
#include <vtkSmartPointer.h>

class QAction;
class QMenu;
class QSplitter;
class QVTKOpenGLNativeWidget;
class vtkRenderer;
class vtkGenericOpenGLRenderWindow;

namespace vispro
{
	class Data;
	class Component;

	class MainWindow : public QMainWindow
	{
		Q_OBJECT

	public:
		explicit MainWindow(const char* basePath);
		virtual ~MainWindow();

	private slots:
		void HandleAbout();
		void HandleRequestRender();	// Listens to components that might want the scene to be rerendered.
		void HandleRequestData();	// Listens to components that might want to receive the data anew.
		void HandleDataChanged();	// Slot for signals when the data has changed.

	private:
		void CreateActions();
		void CreateMenus();
		void CreateData(const char* basePath);
		void CreateComponents();
		void CreateVTK();
		void CreateUI();

		// menus
		QMenu* mHelpMenu;
		QAction* mAboutAct;

		// data to visualize
		std::unique_ptr<Data> mData;
		// visualization components
		std::vector<std::unique_ptr<Component>> mComponents;

		// VTK rendering
		QVTKOpenGLNativeWidget* mSceneWidget;
		vtkSmartPointer<vtkRenderer> mRenderer;
		vtkSmartPointer<vtkGenericOpenGLRenderWindow> mRenderWindow;
	};
}