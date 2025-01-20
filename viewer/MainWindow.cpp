#include "MainWindow.hpp"
#include <QtWidgets>
#include <vtkRenderer.h>
#include <vtkNamedColors.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <QVTKOpenGLNativeWidget.h>
#include "Data.hpp"
#include "Component.hpp"
#include <qscrollarea.h>
#include "BoundingBox.hpp"
#include "IsosurfaceVtk.hpp"
#include "VolrenVtk.hpp"
#include "ImageSlice.hpp"
#include "Particles.hpp"
#include "StreakLineVtk.hpp"
#include "VolrenShader.hpp"
#include "StreamLineVtk.hpp"
#include "PathLineVtk.hpp"

namespace vispro
{
	MainWindow::MainWindow(const char* basePath)
	{
		CreateActions();
		CreateMenus();
		CreateData(basePath);
		CreateComponents();
		CreateVTK();
		CreateUI();
	}

	MainWindow::~MainWindow() {}

	void MainWindow::HandleAbout() {
		QMessageBox::about(this, tr("Info"), tr("This is the demo framework of the Interactive Visualization Project @ FAU Erlangen-Nuernberg"));
	}

	void MainWindow::HandleRequestRender() {
		mRenderWindow->Render();
	}

	void MainWindow::HandleDataChanged() {
		Data* data = dynamic_cast<Data*>(sender());
		if (!data) return;
		for (auto& component : mComponents)
			if (component->IsVisible())
				component->SetData(data);
		HandleRequestRender();
	}

	void MainWindow::HandleRequestData() {
		Component* component = dynamic_cast<Component*>(sender());
		if (!component) return;
		component->SetData(mData.get());
	}

	void MainWindow::CreateActions()
	{
		mAboutAct = new QAction(tr("&About"), this);
		mAboutAct->setStatusTip(tr("Show the application's info box"));
		connect(mAboutAct, &QAction::triggered, this, &MainWindow::HandleAbout);
	}

	void MainWindow::CreateMenus()
	{
		mHelpMenu = menuBar()->addMenu(tr("&Help"));
		mHelpMenu->addAction(mAboutAct);
	}

	void MainWindow::CreateData(const char* basePath)
	{
		mData = std::unique_ptr<Data>(new Data(basePath));
		connect(mData.get(), &Data::DataChanged, this, &MainWindow::HandleDataChanged);
	}

	void MainWindow::CreateVTK()
	{
		mRenderer = vtkSmartPointer<vtkRenderer>::New();
		vtkNew<vtkNamedColors> colors;
		mRenderer->SetBackground(colors->GetColor3d("White").GetData());
		mSceneWidget = new QVTKOpenGLNativeWidget;
		mRenderWindow = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
		mSceneWidget->setRenderWindow(mRenderWindow);
		mRenderWindow->AddRenderer(mRenderer);
		mRenderWindow->SetWindowName("RenderWindow");
		for (auto& component : mComponents) {
			mRenderer->AddActor(component->CreateVTK());
			component->SetData(mData.get());
		}
	}

	void MainWindow::CreateComponents()
	{
		// Add all visualization components here!
		mComponents.push_back(std::make_unique<BoundingBox>());
		mComponents.push_back(std::make_unique<IsosurfaceVtk>());
		mComponents.push_back(std::make_unique<PathLineVtk>());
		mComponents.push_back(std::make_unique<StreamLineVtk>());
		mComponents.push_back(std::make_unique<StreakLineVtk>());
		mComponents.push_back(std::make_unique<Particles>());
		mComponents.push_back(std::make_unique<VolrenVtk>());
		//mComponents.push_back(std::make_unique<VolrenShader>());
		//mComponents.push_back(std::make_unique<ImageSlice>());
	}

	void MainWindow::CreateUI()
	{
		// place a widget in the window
		QWidget* widget = new QWidget;
		setCentralWidget(widget);

		QWidget* options = new QWidget;
		QVBoxLayout* verticalOptionsLayout = new QVBoxLayout;
		options->setLayout(verticalOptionsLayout);
		verticalOptionsLayout->addWidget(mData->GetWidget());
		for (auto& component : mComponents) {
			verticalOptionsLayout->addWidget(component->CreateWidget());
			connect(component.get(), &Component::RequestRender, this, &MainWindow::HandleRequestRender);
			connect(component.get(), &Component::RequestData, this, &MainWindow::HandleRequestData);
		}

		QScrollArea* scrollArea = new QScrollArea;
		scrollArea->setWidget(options);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea->setMaximumWidth(options->width());
		scrollArea->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

		QSplitter* horizontalSplitter = new QSplitter;
		horizontalSplitter->addWidget(scrollArea);
		horizontalSplitter->addWidget(mSceneWidget);

		QVBoxLayout* verticalLayout = new QVBoxLayout;
		widget->setLayout(verticalLayout);
		verticalLayout->addWidget(horizontalSplitter);
		verticalLayout->addWidget(mData->GetTimeSliderWidget());

		/*QString message = tr("Example message.");
		statusBar()->showMessage(message);*/

		setWindowTitle(tr("Vortex Street Visualization"));
		setMinimumSize(800, 600);
		showMaximized();

		setAcceptDrops(true);
	}
	
}