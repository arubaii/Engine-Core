#include "SelectionSystem.h"
#include "scene_core/ecs/Components.h"
#include "io/Input.h"

SelectionSystem::SelectionSystem(Scene* scene)
	: m_Scene(scene)
{
}

void SelectionSystem::Update(Input& input, FrameContext& ctx)
{
	auto& registry = m_Scene->GetRegistry();

	if (!input.IsMousePressedOnce(Mouse::Left))
		return;

	if (input.IsMouseCapturedByUI())
		return;


	entt::entity clicked = entt::null;

	if (ctx.HasHit)
	{
		clicked = ctx.LastHit.entity;

		if (registry.all_of<ModelPartComponent>(clicked))
		{
			auto& part = registry.get<ModelPartComponent>(clicked);
			Entity root = m_Scene->GetEntityByID(part.RootID);
			if (root && registry.valid((entt::entity)root))
				clicked = (entt::entity)root;
		}
	}

	entt::entity currentlySelected = entt::null;

	auto view = registry.view<SelectedComponent>();
	for (auto e : view)
	{
		currentlySelected = e;
		break;
	}

	if (clicked != entt::null && clicked == currentlySelected)
	{
		registry.remove<SelectedComponent>(clicked);
		m_Selected = entt::null;
		m_SelectedUUID = {};
		return;
	}

	if (currentlySelected != entt::null)
		registry.remove<SelectedComponent>(currentlySelected);

	if (clicked != entt::null)
	{
		registry.emplace_or_replace<SelectedComponent>(clicked);
		m_Selected = clicked;
		m_SelectedUUID = registry.get<IDComponent>(clicked).ID;
	}
}


Entity SelectionSystem::GetSelectedEntity()
{
	auto& registry = m_Scene->GetRegistry();

	auto view = registry.view<SelectedComponent>();
	for (auto e : view)
		return Entity{ e, &registry };

	return {};
}


void SelectionSystem::SetSelectedEntity(entt::entity e)
{
	auto& registry = m_Scene->GetRegistry();

	// Remove old SelectedComponent
	if (m_Selected != entt::null &&
		registry.valid(m_Selected) &&
		registry.all_of<SelectedComponent>(m_Selected))
	{
		registry.remove<SelectedComponent>(m_Selected);
	}

	m_Selected = entt::null;
	m_SelectedUUID = {};

	if (e == entt::null ||
		!registry.valid(e) ||
		!registry.all_of<IDComponent>(e))
	{
		return;
	}

	if (!registry.all_of<SelectedComponent>(e))
		registry.emplace<SelectedComponent>(e);

	m_Selected = e;
	m_SelectedUUID = registry.get<IDComponent>(e).ID;
}

void SelectionSystem::ClearSelection(Entity e)
{
	if (e.HasComponent<SelectedComponent>())
		e.RemoveComponent<SelectedComponent>();
}