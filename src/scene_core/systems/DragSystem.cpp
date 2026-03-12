#include "DragSystem.h"
#include "scene_core/ecs/Components.h"
#include "FrameContext.h"
#include "core/Window.h"
#include "io/Input.h"
#include <glm/gtc/matrix_transform.hpp>

#include "core/Application.h"
#include "core/Application.h"
#include "core/Scene.h"
#include "GLcommon.h"

#include "SelectionSystem.h"
#include "scene_core/ecs/EntityUtils.h"


#include "utils/WebCursor.h"


DragSystem::DragSystem(Scene* scene)
    : m_Scene(scene)
{
}

void DragSystem::SetDragMode(Input& input, bool vertical, bool xz)
{

    if (!input.IsActionActive(InputAction::EntityDrag))
    {
        DragAffectsVertical = false;
        DragAffectsXZ = false;
    }
    else
    {
        DragAffectsVertical = vertical;
        DragAffectsXZ = xz;
    }
}

void DragSystem::Update(float dt,
                        Input& input,
                        CameraComponent& camera,
                        FrameContext& ctx)
{
    auto& registry = m_Scene->GetRegistry();
    Window& window = m_Scene->GetWindow();
    glm::vec2 viewport = window.GetViewport();
    GLFWwindow* glfwWindow   = m_Scene->GetGLFWwindow();



    bool inOrbit = m_Scene->GetCameraSystem().GetActiveControllerIndex() ==
                   m_Scene->GetCameraSystem().ORBIT_CONTROLLER_INDEX;



    bool primaryClick = input.IsActionActive(InputAction::PrimaryClick);
    bool secondaryClick = input.IsActionActive(InputAction::SecondaryClick);


    // Geometry Drag & Drop
    if (!IsDragging &&
        input.IsActionActive(InputAction::EntityDrag) &&
        ctx.HasHit)
    {
        entt::entity raw = ctx.LastHit.entity;

        if (registry.all_of<ModelPartComponent>(raw))
        {
            auto& part = registry.get<ModelPartComponent>(raw);
            Entity root = m_Scene->GetEntityByID(part.RootID);
            if (root)
                raw = (entt::entity)root;
        }

        m_DraggedEntity = raw;

        Entity dragged{ m_DraggedEntity, &registry };

        // If not selected, select it first
        if (!dragged.HasComponent<SelectedComponent>())
        {
            m_Scene->GetSelectionSystem().SetSelectedEntity(m_DraggedEntity);
        }

        if (!inOrbit)
            IsDragging = true;

        m_DragOffset = dragged.GetPosition() - ctx.LastHit.Position;
    }



    if (IsDragging &&
        primaryClick &&
        m_DraggedEntity != entt::null)
    {
        auto& tc = registry.get<TransformComponent>(m_DraggedEntity);

        if (!m_DragDistanceLocked)
        {
            glm::vec3 camPos = camera.Camera.GetPosition();
            m_LockedDragDistance = glm::length(tc.Translation - camPos);
            m_LockedDragDistance = glm::clamp(m_LockedDragDistance, 2.0f, MAX_DRAG_DISTANCE);


            m_InitialDragPos = tc.Translation;
            m_InitialMousePos = input.GetViewportMousePos(window);
            m_FinalDragPos = m_InitialDragPos;

            m_DragDistanceLocked = true;
        }

        glm::vec2 mouseDelta = input.GetViewportMousePos(window) - m_InitialMousePos;
        glm::vec3 newPos = m_InitialDragPos;


        float fovScale =
            glm::tan(glm::radians(
                camera.Camera.GetFOV() * 0.5f));

        float sensitivity =
            (m_LockedDragDistance * fovScale) /
            (viewport.y * 0.5f);

        if (DragAffectsVertical)
        {
            glfwSetCursor(glfwWindow, m_Cursors.ResizeUpDown);

            newPos.y -= mouseDelta.y * sensitivity;
        }
        else if (DragAffectsXZ)
        {
            glm::vec3 camRight =
                camera.Camera.GetRightVector();

            glm::vec3 camForward =
                glm::normalize(glm::vec3(
                    camera.Camera.GetForwardVector().x,
                    0.0f,
                    camera.Camera.GetForwardVector().z));

            newPos += camRight * mouseDelta.x * sensitivity;
            newPos += camForward * (-mouseDelta.y) * sensitivity;

            Entity dragged{ m_DraggedEntity, &registry };

            m_LocalGridHeight = EntityUtils::ComputeEntityFloorY(m_Scene, dragged);
            m_LocalGridSize   = EntityUtils::ComputeXZRadius(m_Scene, dragged);
            m_LocalGridOrigin = glm::vec3(newPos.x,
                                          m_LocalGridHeight,
                                          newPos.z);
        }

        tc.Translation = newPos;
        tc.MarkDirty();

        m_FinalDragPos = newPos;
    }


    // End Drag
    if (IsDragging &&
        !primaryClick)
    {
        if (m_InitialDragPos != m_FinalDragPos)
        {
            UUID id = registry.get<IDComponent>(m_DraggedEntity).ID;

            MoveAction action;
            action.id = id;
            action.before = m_InitialDragPos;
            action.after  = m_FinalDragPos;

            m_Scene->GetUndoSystem().Push(action);
        }

        IsDragging = false;
        m_DraggedEntity = entt::null;
        m_DragDistanceLocked = false;
    }



    // Set cursor
    if (!input.IsInUI())
    {
        m_IsControllingCursor = true;

        if (IsDragging)
        {
            if (DragAffectsVertical)
            {
#ifdef __EMSCRIPTEN__
                SetCanvasCursor("ns-resize");
#else
                glfwSetCursor(glfwWindow, m_Cursors.ResizeUpDown);
#endif
            }

            if (DragAffectsXZ)
            {
#ifdef __EMSCRIPTEN__
                SetCanvasCursor("move");
#else
                glfwSetCursor(glfwWindow, m_Cursors.Move);
#endif
            }
        }
        else if (ctx.HasHit && !inOrbit)
        {
#ifdef __EMSCRIPTEN__
            SetCanvasCursor("grab");
#else
            glfwSetCursor(glfwWindow, m_Cursors.OpenHand);
#endif
        }
        else if (inOrbit)
        {
            if (primaryClick || secondaryClick)
            {
#ifdef __EMSCRIPTEN__
                SetCanvasCursor("grabbing");
#else
                glfwSetCursor(glfwWindow, m_Cursors.GrabHand);
#endif
            }
            else if (ctx.HasHit && !primaryClick)
            {
#ifdef __EMSCRIPTEN__
                SetCanvasCursor("grab");
#else
                glfwSetCursor(glfwWindow, m_Cursors.OpenHand);
#endif
            }
            else
            {
#ifdef __EMSCRIPTEN__
                SetCanvasCursor("pointer");
#else
                glfwSetCursor(glfwWindow, m_Cursors.PointHand);
#endif
            }
        }
        else
        {
#ifdef __EMSCRIPTEN__
            SetCanvasCursor("default");
#else
            glfwSetCursor(glfwWindow, m_Cursors.Arrow);
#endif
        }
    }
    else
        m_IsControllingCursor = false;
}