#include "UndoSystem.h"
#include "scene_core/ecs/Components.h"
#include "io/Input.h"

UndoSystem::UndoSystem(entt::registry& registry,
                       RestoreFn restoreFn,
                       DeleteFn deleteFn,
                       LookupFn lookupFn)
    : m_Registry(registry),
      m_Restore(restoreFn),
      m_Delete(deleteFn),
      m_Lookup(lookupFn)
{
}

void UndoSystem::Push(Action action)
{
    m_UndoStack.push_back(std::move(action));
    m_RedoStack.clear();
}

void UndoSystem::Update(Input& input)
{

#ifdef __APPLE__
    bool ctrlOrCmd =
        input.IsKeyPressed(Key::LeftCmd) ||
        input.IsKeyPressed(Key::RightCmd);
#else
    bool ctrlOrCmd =
        input.IsKeyPressed(Key::LeftCtrl) ||
        input.IsKeyPressed(Key::RightCtrl);
#endif

    bool shift =
        input.IsKeyPressed(Key::LeftShift) ||
        input.IsKeyPressed(Key::RightShift);

    bool zOnce = input.IsKeyPressedOnce(Key::Z);

    if (ctrlOrCmd && zOnce)
    {
        if (shift) Redo();
        else       Undo();
    }
}

void UndoSystem::Undo()
{

    if (m_UndoStack.empty())
        return;

    Action a = std::move(m_UndoStack.back());
    m_UndoStack.pop_back();

    std::visit([&](auto& act)
    {
        ApplyUndo(act);
    }, a);

    m_RedoStack.push_back(std::move(a));

}

void UndoSystem::Redo()
{
    if (m_RedoStack.empty())
        return;

    Action a = std::move(m_RedoStack.back());
    m_RedoStack.pop_back();

    std::visit([&](auto& act)
    {
        ApplyRedo(act);
    }, a);

    m_UndoStack.push_back(std::move(a));
}

void UndoSystem::ApplyUndo(const MoveAction& a)
{
    auto view = m_Registry.view<IDComponent, TransformComponent>();

    for (auto e : view)
    {
        if (view.get<IDComponent>(e).ID == a.id)
        {
            auto& tc = view.get<TransformComponent>(e);
            tc.Translation = a.before;
            tc.MarkDirty();
            break;
        }
    }
}

void UndoSystem::ApplyRedo(const MoveAction& a)
{
    auto view = m_Registry.view<IDComponent, TransformComponent>();

    for (auto e : view)
    {
        if (view.get<IDComponent>(e).ID == a.id)
        {
            auto& tc = view.get<TransformComponent>(e);
            tc.Translation = a.after;
            tc.MarkDirty();
            break;
        }
    }
}

void UndoSystem::ApplyUndo(const CreateAction& a)
{
    Entity e = m_Lookup(a.snapshot.id);
    if (e)
        m_Delete(a.snapshot.id);
}

void UndoSystem::ApplyRedo(const CreateAction& a)
{
    m_Restore(a.snapshot);
}

void UndoSystem::ApplyUndo(const DeleteAction& a)
{
    m_Restore(a.snapshot);
}

void UndoSystem::ApplyRedo(const DeleteAction& a)
{
    m_Delete(a.snapshot.id);
}

void UndoSystem::ApplyUndo(const ModifyMaterialAction& a)
{
    Entity e = m_Lookup(a.id);
    if (e && e.HasComponent<MaterialComponent>())
        e.GetComponent<MaterialComponent>() = a.before;
}

void UndoSystem::ApplyRedo(const ModifyMaterialAction& a)
{
    Entity e = m_Lookup(a.id);
    if (e && e.HasComponent<MaterialComponent>())
        e.GetComponent<MaterialComponent>() = a.after;
}

void UndoSystem::ApplyUndo(const ModifyLightAction& a)
{
    Entity e = m_Lookup(a.id);
    if (e && e.HasComponent<LightComponent>())
        e.GetComponent<LightComponent>() = a.before;
}

void UndoSystem::ApplyRedo(const ModifyLightAction& a)
{
    Entity e = m_Lookup(a.id);
    if (e && e.HasComponent<LightComponent>())
        e.GetComponent<LightComponent>() = a.after;
}