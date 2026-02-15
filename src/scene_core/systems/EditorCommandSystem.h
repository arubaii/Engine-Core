#pragma once
#include <entt/entt.hpp>


class Scene;
class Input;
struct FrameContext;

class EditorCommandSystem
{
public:
	explicit EditorCommandSystem(Scene* scene);

	void Update(Input& input, FrameContext& ctx);

private:
	Scene* m_Scene = nullptr;
	entt::entity m_Hovered = entt::null;
};