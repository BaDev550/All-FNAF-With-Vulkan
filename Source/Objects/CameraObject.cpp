#include "gpch.h"
#include "CameraObject.h"
#include "Engine/Application.h"

void CameraObject::Update()
{
	_Handle.SetPerspectiveProjection(45.0f, Application::Get()->GetRenderer().GetAspectRatio(), 0.1f, 100.0f);
	_Handle.SetViewYXZ(GetTransform().position, GetTransform().rotation);
}