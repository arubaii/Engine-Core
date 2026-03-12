#include "EditorCommandSystem.h"
#include "core/Scene.h"
#include "scene_core/ecs/Entity.h"
#include "scene_core/ecs/EntitySnapshot.h"
#include "scene_core/systems/UndoSystem.h"
#include "io/Input.h"

EditorCommandSystem::EditorCommandSystem(Scene* scene)
	: m_Scene(scene)
{
}

void EditorCommandSystem::Update(Input& input, FrameContext& ctx)
{
	if (input.IsMouseCapturedByUI())
		return;

	if (!ctx.HasHit)
		return;

	auto& registry = m_Scene->GetRegistry();

	entt::entity raw = ctx.LastHit.entity;
	if (raw == entt::null || !registry.valid(raw))
		return;

	Entity hovered{ raw, &registry };
	if (!hovered)
		return;


	// If hovering a part, promote to root
	if (hovered.HasComponent<ModelPartComponent>())
	{
		auto rootID = hovered.GetComponent<ModelPartComponent>().RootID;
		Entity root = m_Scene->GetEntityByID(rootID);
		if (!root)
			return;
		hovered = root;
	}

	if (!hovered || !hovered.HasComponent<IDComponent>())
		return;

	// Delete
	if (input.IsActionActiveOnce(InputAction::EntityDelete))
	{
		EntitySnapshot snapshot = m_Scene->SnapshotEntity(hovered);

		DeleteAction action;
		action.snapshot = snapshot;

		m_Scene->DeleteEntity(hovered);
		m_Scene->GetUndoSystem().Push(action);

		return;
	}

	// Duplicate
	if (input.IsActionActiveOnce(InputAction::EntityDuplicate))
	{
		Entity duplicate = m_Scene->DuplicateEntity(hovered);
		if (!duplicate)
			return;

		// Always snapshot the model root, never a part
		Entity root = duplicate;

		if (root.HasComponent<ModelPartComponent>())
		{
			auto& part = root.GetComponent<ModelPartComponent>();
			root = m_Scene->GetEntityByID(part.RootID);
			if (!root)
				return;
		}

		EntitySnapshot snapshot = m_Scene->SnapshotEntity(root);

		CreateAction action;
		action.snapshot = snapshot;

		m_Scene->GetUndoSystem().Push(action);
	}
}