#include "GameObject.h"
#include "Engine/Application.h"
#include "Graphics/Render/SceneRenderer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

void TransformComponent::CopyVec3PositionCoords(const glm::vec3& position)
{
	glfwSetClipboardString(Application::Get()->GetWindow().GetHandle(), glm::to_string(position).c_str());
	const char* text = glfwGetClipboardString(Application::Get()->GetWindow().GetHandle());
	LOG_INFO("Vec3 Positions are copied: {}", text);
}

DrawnableGameObject::DrawnableGameObject(SceneRenderer& renderer, const std::string& modelPath, gameobjectid_t id, const std::string& name)
	: GameObject(id, name), _Renderer(renderer)
{
	_DrawingHandle = &_Renderer.CreateDrawnable(*this, modelPath);
}
