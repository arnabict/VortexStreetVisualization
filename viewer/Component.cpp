#include "Component.hpp"
#include <qgroupbox.h>
#include <vtkActor.h>

namespace vispro
{
	Component::Component(const std::string& name) : mName(name), mGroupBox(NULL), mActor(NULL) {}

	QWidget* Component::CreateWidget()
	{
		mGroupBox = new QGroupBox(mName.c_str());
		mGroupBox->setCheckable(true);
		mGroupBox->setChecked(true);
		connect(mGroupBox, &QGroupBox::clicked, this, &Component::SetVisible);

		CreateWidget(mGroupBox);
		return mGroupBox;
	}

	vtkSmartPointer<vtkProp> Component::CreateVTK()
	{
		mActor = CreateActor();
		return mActor;
	}

	void Component::SetVisible(bool visible)
	{
		mActor->SetVisibility(visible);
		mGroupBox->setChecked(visible);
		if (visible)
			emit RequestData();
		emit RequestRender();
	}

	bool Component::IsVisible() const { return mActor->GetVisibility(); }
}