#pragma once
#include <unordered_map>
#include <type_traits>
#include <cassert>
#include <entt/entt.hpp>
#include "utils/SmartPtrs.h"
#include "utils/UUID.h"

#include "scene_core/ecs/EntitySnapshot.h"

#include "Renderer.h"
#include "Window.h"
#include "io/Input.h"
#include "scene_core/camera/FreeCameraController.h"
#include "scene_core/camera/OrbitCameraController.h"
#include "scene_core/camera/ThirdPersonCameraController.h"
#include "asset_io/Cursors.h"
#include "math/Intersect.h"
#include "renderer_core/Skybox.h"
#include "utils/Primitives.h"

#include "scene_core/systems/FrameContext.h"
#include "scene_core/systems/DragSystem.h"
#include "scene_core/systems/RaycastSystem.h"
#include "scene_core/systems/SelectionSystem.h"
#include "scene_core/systems/UndoSystem.h"
#include "scene_core/systems/CameraSystem.h"
#include "scene_core/systems/EditorCommandSystem.h"

class SelectionSystem;
class RaycastSystem;
class DragSystem;
class UndoSystem;
class CameraSystem;
class EditorCommandSystem;


struct CameraProps
{
	float Fov;
	float AspectRatio;
	float NearPlane;
	float FarPlane;
	float MoveSpeed;
};

// Refactor out later
enum class MATERIALS
{
	None,
	Glossy_Plastic,
	Matte_Plastic,
	Rubber,

	Shiny_Porcelain,
	Smooth_Porcelain,
	Jade_Porcelain,
	Fired_Clay,

	Aluminium,
	Chrome,
	Copper,
	Silver,
	Gold,

	Count
};

// const char* for ImGui
inline const char*GetMaterialAsString(MATERIALS m)
{
	switch (m)
	{
		case MATERIALS::None:             return "";

		case MATERIALS::Glossy_Plastic:   return "Glossy Plastic";
		case MATERIALS::Matte_Plastic:    return "Matte Plastic";
		case MATERIALS::Rubber:           return "Rubber";

		case MATERIALS::Shiny_Porcelain:  return "Shiny Porcelain";
		case MATERIALS::Smooth_Porcelain: return "Smooth Porcelain";
		case MATERIALS::Jade_Porcelain:   return "Jade Porcelain";
		case MATERIALS::Fired_Clay:       return "Fired Clay";

		case MATERIALS::Aluminium:        return "Aluminium";
		case MATERIALS::Chrome:           return "Chrome";
		case MATERIALS::Copper:           return "Copper";
		case MATERIALS::Silver:           return "Silver";
		case MATERIALS::Gold:             return "Gold";

		case MATERIALS::Count:            break;
	}

	return "Unknown";
}

inline const char* const* GetMaterialDropdownItems()
{
	static std::array<const char*, (size_t)MATERIALS::Count> items = []
	{
		std::array<const char*, (size_t)MATERIALS::Count> a{};
		for (int i = 0; i < (int)MATERIALS::Count; ++i)
			a[i] = GetMaterialAsString((MATERIALS)i);
		return a;
	}();
	return items.data();
}

inline int GetMaterialDropdownCount()
{
	return (int)MATERIALS::Count;
}

struct MaterialSpecs // From MaterialDesc
{
	glm::vec4 BaseColorFactor = glm::vec4(1.0f);
	float MetallicFactor  = 1.0f;
	float RoughnessFactor = 1.0f;
	float LightBoostFactor = 1.0f; // No effect on dialectrics
};

/** @note Materials should be more sophisticated later
 *
 */
inline MaterialSpecs GetMaterialType(MATERIALS m)
{
	switch (m)
	{
		//										         Color                 Metal  Rough  Light
		case MATERIALS::None:		     break;
		case MATERIALS::Glossy_Plastic:  return { {1.00f, 0.04f, 0.00f, 1.0f}, 0.0f, 0.08f };
		case MATERIALS::Matte_Plastic:   return { {0.04f, 0.04f, 0.04f, 1.0f}, 0.0f, 0.65f };
		case MATERIALS::Rubber:          return { {0.93f, 0.57f, 0.46f, 1.0f}, 0.0f, 0.90f };

		case MATERIALS::Shiny_Porcelain: return { {1.00f, 1.00f, 1.00f, 1.0f}, 0.0f, 0.10f };
		case MATERIALS::Smooth_Porcelain:return { {1.00f, 0.94f, 0.76f, 1.0f}, 0.0f, 0.38f };
		case MATERIALS::Jade_Porcelain:  return { {0.65f, 0.83f, 0.80f, 1.0f}, 0.0f, 0.18f };
		case MATERIALS::Fired_Clay:      return { {0.55f, 0.32f, 0.22f, 1.0f}, 0.0f, 0.78f };

		case MATERIALS::Aluminium:       return { {0.91f, 0.92f, 0.92f, 1.0f}, 1.0f, 0.55f, 2.55f };
		case MATERIALS::Chrome:          return { {0.95f, 0.95f, 0.95f, 1.0f}, 1.0f, 0.02f, 10000.01f };
		case MATERIALS::Copper:          return { {0.99f, 0.49f, 0.33f, 1.0f}, 1.0f, 0.20f, 195.1f };
		case MATERIALS::Silver:          return { {0.97f, 0.96f, 0.92f, 1.0f}, 1.0f, 0.12f, 1000.0f };
		case MATERIALS::Gold:            return { {1.00f, 0.77f, 0.34f, 1.0f}, 1.0f, 0.18f, 164.783f };


		case MATERIALS::Count:			 break;
	}

	return { {1,1,1,1}, 0.0f, 0.5f };
};



class Scene
{
public:
	// void BindMaterial(const MaterialComponent& material, const Ref<Shader>& shader, int& slot);

	// void SetModelMaterial(Entity root, AssetHandle matHandle);

	Scene(Window& window, Input& input);

	void DrawScreenOverlays(const CameraComponent &cc, Renderer& renderer);

	void Update(float dt, Input& input);
	void Render(Renderer& renderer);

	void Shoot(glm::vec3 camPos, glm::vec3 camForward, float speed);


	// ============ Entity Configuration ============
	entt::registry& GetRegistry() { return m_Registry; }

	Window& GetWindow() { return m_Window; }
	const Window& GetWindow() const { return m_Window; }

	GLFWwindow* GetGLFWwindow() { return m_Window.GetGLFWwindow(); }
	GLFWwindow* GetGLFWwindow() const { return m_Window.GetGLFWwindow(); }

	Entity CreateEntity(UUID uuid = UUID(), const std::string& name = "Unnamed Entity");

	Entity CreateEntityForRestore(UUID uuid, const std::string &name);


	std::string GenerateCopyName(const std::string &base);

	Entity DuplicateEntity(Entity src);
	void   DeleteEntity   (Entity entity);

	void SetPrimaryCamera(Entity entity);

	Entity GetPrimaryCameraEntity(); // Useful for multiple cameras
	Entity GetEntityByName(const std::string& name);
	Entity GetEntityByID  (UUID id);
	UUID GetSelectedUUID() const { return m_SelectedUUID; }


	// size_t GetMaxEntityIndex(std::string startsWith);

	template<class T>
	void OnComponentAdded(Entity entity, T &component);

	Entity FindModelRootFromPart(Entity part);

	// void SetEntityMaterial(Entity e, AssetHandle matHandle);
	void ApplyMaterialToSelection(Entity selected, const Ref<MaterialAsset> &mat);
	void DrawGrid(const CameraComponent &cc, Renderer &renderer) const;
	void DrawLocalGrid(const CameraComponent& cc,
						  Renderer& renderer,
						  const glm::vec3& origin,
						  float height,
						  float size);

	void DrawSkybox(const CameraComponent &cc, Renderer &renderer) const;
	void DrawYAxis(const CameraComponent &cc, Renderer &renderer, Entity selected);

	void DrawOutlineDebugAll(const CameraComponent &cc, Renderer &renderer);

	void DrawOutline(const CameraComponent &cc, Renderer &renderer);


	bool Raycast(const Ray& ray, RayHit& outHit) const;

	Entity CreateEntityFromModel(const std::filesystem::path& path,
						  const std::string& rootName = "Unnamed Model");

	Ref<Shader> GetShader(UUID id);

	void SyncSubmeshes();

	glm::vec3 GetMainCameraPos();
	float	  GetMainCameraPitch();
	float	  GetMainCameraYaw();

	entt::entity GetSelectedEntity() const { return m_SelectedEntity; }
	void SetSelectedEntity(entt::entity e);



	// glm::vec3 ColorFromTemperature(float kelvin);

	// float ComputeEntityFloorY(Entity e);

	// float ComputeXZRadius(Entity e);

	void PushAction(Action a);

	void ApplyUndo(const MoveAction &a);
	void ApplyUndo(const CreateAction &a);
	void ApplyUndo(const DeleteAction& a);
	void ApplyRedo(const MoveAction &a);
	void ApplyRedo(const CreateAction &a);
	void ApplyRedo(const DeleteAction& a);

	EntitySnapshot SnapshotEntity(Entity e);
	Entity RestoreEntityFromSnapshot(const EntitySnapshot& s);


	void Undo();

	void Redo();

	CameraSystem& GetCameraSystem() { return *m_CameraSystem; }
	const CameraSystem& GetCameraSystem() const { return *m_CameraSystem; }
	DragSystem& GetDragSystem() { return *m_DragSystem; }

	UndoSystem& GetUndoSystem() { return *m_UndoSystem; }
	const UndoSystem& GetUndoSystem() const { return *m_UndoSystem; }

	SelectionSystem& GetSelectionSystem() { return *m_SelectionSystem; }
	const SelectionSystem& GetSelectionSystem() const { return *m_SelectionSystem; }



private:
	Window&   m_Window;
	CameraProps m_CameraProps;

	glm::vec2 m_Viewport;

	// ============ Camera Configuration ============

	float MOVE_PLANE_Y = 0.0f;



	friend class Entity;
	entt::registry m_Registry;
	std::unordered_map<UUID, entt::entity> m_EntityMap; // May be useful for serialization
	std::unordered_map<std::string, entt::entity> m_EntityNameMap;

	// Click and hold to move around
	entt::entity m_DraggedEntity = entt::null;
	bool         m_IsDragging = false;
	glm::vec3 m_DragOffset{0.0f};

	unsigned int m_InfiniteGridVAO = 0;
	unsigned int m_GridVAO = 0;

	// TODO: Move later
	Ref<Shader> m_InfiniteGridShader;
	Ref<Shader> m_GridShader;
	Ref<Shader> m_BasisShader;
	Ref<Shader> m_BaseShader;
	Ref<Shader> m_CrosshairShader;
	Ref<Shader> m_LightShader;
	Ref<Shader> m_PhongShader;
	Ref<Shader> m_OutlineShader;
	Ref<Shader> m_SkyboxShader;
	Ref<Shader> m_PBRShader;
	Ref<Shader> m_MaskShader;
	Ref<Shader> m_OutlinePostShader;
	Ref<Shader> m_DepthOnlyShader;

	VertexBufferLayout m_CrosshairLayout;
	VertexBuffer m_CrosshairVB;
	VertexArray m_CrosshairVA;

	glm::vec3 CROSSHAIR_COLOR = glm::vec3(1.0f);

	Skybox m_Sky;
	VertexArray        m_SkyboxVA;
	VertexBuffer       m_SkyboxVB;
	VertexBufferLayout m_SkyboxLayout;
	GLCubemap          m_SkyboxCubemap;
	bool               m_SkyboxLoaded = false;
	int m_Width = 0;
	int m_Height = 0;
	int m_MaxMip = 0;

	entt::entity m_SelectedEntity = entt::null;
	UUID m_SelectedUUID = {};
	GLuint m_HDRSkyboxCubemap = 0;

public:


	static constexpr glm::vec3 DefaultCameraPosition{ 154.23f, 122.63f, 99.62f };
	static constexpr float     DefaultPitch = -35.42f;
	static constexpr float     DefaultYaw   = -169.88f;

	bool m_GeometrySelect = false;
	float m_GeometryScale = 1.0f;

	// Assigned by UI query, keep one source of truth there
	bool m_ShowGrid;
	bool m_ShowAxes;
	bool m_ShowSkybox;
	bool m_ShowCrosshair;
	bool m_ShowLights;

	enum class DragConstraint { None, X, Y, Z, XZ };

	bool m_DragDistanceLocked = false;
	float m_LockedDragDistance = 0.0f;
	glm::vec3 m_InitialDragPos{0.0f};
	glm::vec3 m_FinalDragPos{0.0f};
	glm::vec2 m_InitialMousePos{0.0f};
	DragConstraint m_DragConstraint = DragConstraint::None;

	bool m_DragAffectsVertical = true;
	bool m_DragAffectsXZ = false;





	bool m_WasHovering = false;
	bool m_CursorEnabled = false;

	GLuint m_WhiteTexture = 0;
	GLuint m_FlatNormalTexture = 0;

	float m_LightRadius = 1.0f;

	struct SceneLight
	{
		glm::vec3 Position;
		float Luminosity;
		glm::vec3 Color;
		float Radius;
	};

	std::vector<SceneLight> m_Lights;

	bool m_ShouldDrawLocalGrid = false;
	entt::entity m_LocalGridID = entt::null;



	std::vector<Action> m_Undo;
	std::vector<Action> m_Redo;

	// raycast cache
	glm::vec2 m_LastRayMouse = {-1.0f, -1.0f};
	glm::vec3 m_LastCamPos   = {FLT_MAX, FLT_MAX, FLT_MAX};
	float     m_LastCamYaw   = FLT_MAX;
	float     m_LastCamPitch = FLT_MAX;

	RayHit    m_LastRayHit{};
	bool      m_LastHasHit = false;

	GLuint m_IrradianceMap = 0;   // GL_TEXTURE_CUBE_MAP
	GLuint m_PrefilterMap  = 0;   // GL_TEXTURE_CUBE_MAP (mipmapped)
	GLuint m_BRDFLUTTex    = 0;   // GL_TEXTURE_2D

	Mesh m_SphereMesh = PRIMITIVES::GenerateSphere();
	Mesh m_BulletMesh = PRIMITIVES::GenerateSphere(8, 8);

	bool m_EnablePhysics;

	Scope<CameraSystem>    		m_CameraSystem;
	Scope<RaycastSystem>   		m_RaycastSystem;
	Scope<SelectionSystem> 		m_SelectionSystem;
	Scope<DragSystem>      		m_DragSystem;
	Scope<UndoSystem>			m_UndoSystem;
	Scope<EditorCommandSystem>  m_EditorCommandSystem;

	bool m_OutlinesInitialized = false;
	int  m_LastOutlineW = 0;
	int  m_LastOutlineH = 0;
};

