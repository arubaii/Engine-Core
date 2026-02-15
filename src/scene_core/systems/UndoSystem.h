#pragma once
#include <entt/entt.hpp>
#include <vector>
#include <variant>
#include <functional>
#include "scene_core/ecs/Entity.h"
#include "scene_core/ecs/EntitySnapshot.h"

class Input;

struct MoveAction
{
	UUID id;
	glm::vec3 before;
	glm::vec3 after;
};

struct ModifyMaterialAction
{
	UUID id;
	MaterialComponent before;
	MaterialComponent after;
};

struct ModifyLightAction
{
	UUID id;
	LightComponent before;
	LightComponent after;
};

struct CreateAction
{
	EntitySnapshot snapshot;
};

struct DeleteAction
{
	EntitySnapshot snapshot;
};

using Action = std::variant<
	MoveAction,
	CreateAction,
	DeleteAction,
	ModifyMaterialAction,
	ModifyLightAction
>;



class UndoSystem
{
public:
	using RestoreFn = std::function<Entity(const EntitySnapshot&)>;
	using DeleteFn = std::function<void(UUID)>;
	using LookupFn = std::function<Entity(UUID)>;


	UndoSystem(entt::registry& registry,
	           RestoreFn restoreFn,
	           DeleteFn deleteFn,
	           LookupFn lookupFn);

	void Update(Input& input);

	void Push(Action action);

	void Undo();
	void Redo();

private:
	entt::registry& m_Registry;

	RestoreFn m_Restore;
	DeleteFn  m_Delete;
	LookupFn  m_Lookup;

	std::vector<Action> m_UndoStack;
	std::vector<Action> m_RedoStack;

	void ApplyUndo(const MoveAction& a);
	void ApplyRedo(const MoveAction& a);

	void ApplyUndo(const CreateAction& a);
	void ApplyRedo(const CreateAction& a);

	void ApplyUndo(const DeleteAction& a);
	void ApplyRedo(const DeleteAction& a);

	void ApplyUndo(const ModifyMaterialAction& a);
	void ApplyRedo(const ModifyMaterialAction& a);

	void ApplyUndo(const ModifyLightAction& a);
	void ApplyRedo(const ModifyLightAction& a);
};
