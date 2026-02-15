#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include "asset_io/Cursors.h"

struct FrameContext;
class Input;
class Window;
struct CameraComponent;
class Scene;


class DragSystem
{
public:
	explicit DragSystem(Scene* scene);

	void Update(float dt,
				Input& input,
				CameraComponent& camera,
				FrameContext& ctx);

	void SetDragMode(Input& input, bool vertical, bool xz);

	float GetLocalGridHeight()		const { return m_LocalGridHeight; }
	float GetLocalGridSize()		const { return m_LocalGridSize; }
	glm::vec3 GetLocalGridOrigin()  const { return m_LocalGridOrigin; }

public:
	// Used in renderer loop
	bool DragAffectsVertical = true;
	bool DragAffectsXZ = false;
	bool IsDragging = false;

private:
	Scene* m_Scene = nullptr;

	EditorCursors m_Cursors;
	// Drag state

	entt::entity m_DraggedEntity = entt::null;

	glm::vec3 m_DragOffset;

	glm::vec3 m_InitialDragPos{};
	glm::vec2 m_InitialMousePos{};
	glm::vec3 m_FinalDragPos{};

	float m_LockedDragDistance = 0.0f;
	bool m_DragDistanceLocked = false;

	bool m_WasHovering = false;

	static constexpr float MAX_DRAG_DISTANCE = 250.0f;

	glm::vec3 m_LocalGridOrigin = glm::vec3(0.0f);
	float m_LocalGridHeight = 0.0f;
	float m_LocalGridSize = 0.0f;

};
