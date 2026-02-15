#pragma once
#include <entt/entt.hpp>
#include "FrameContext.h"
#include "core/Scene.h"

class Input;


class SelectionSystem
{
public:
	SelectionSystem(Scene* scene);

	void Update(Input& input, FrameContext& ctx);

	Entity GetSelectedEntity();

	entt::entity GetSelected() const { return m_Selected; }

	void ClearSelection(Entity e);
	void SetSelectedEntity(entt::entity e);
private:

private:
	Scene* m_Scene;
	entt::entity m_Selected = entt::null;
	UUID m_SelectedUUID = {};
};
