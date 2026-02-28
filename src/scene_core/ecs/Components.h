#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>


#include "asset_core/AssetTypes.h"
#include "../camera/PerspectiveCamera.h"
#include "scene_core/camera/FreeCameraController.h"
#include "utils/UUID.h"
#include "utils/SmartPtrs.h"


struct TransformCache
{
	glm::mat4 Model{1.0f};
	glm::mat4 InvModel{1.0f};
	glm::mat3 NormalMat{1.0f};
	bool Dirty = true;
};

struct TransformComponent
{
	glm::vec3 Translation {0.0f, 0.0f, 0.0f};
	glm::vec3 Rotation    {0.0f, 0.0f, 0.0f}; // { pitch, yaw, roll }
	glm::vec3 Scale       {1.0f, 1.0f, 1.0f};

	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;
	TransformComponent(const glm::vec3& translation)
		: Translation(translation) {}

	const glm::vec3& GetPosition() const { return Translation; }
	const glm::vec3& GetRotation() const { return Rotation; }
	glm::mat4 GetModelMatrix() const
	{
		glm::mat4 T = glm::translate(glm::mat4(1.0f), Translation);
		glm::mat4 R = glm::toMat4(glm::quat(Rotation));
		glm::mat4 S = glm::scale(glm::mat4(1.0f), Scale);

		if (!Cache.Dirty)
			Cache.Dirty = true;

		return T * R * S;
	}

	void SetScale(const glm::vec3& s) { Scale = s; MarkDirty(); }
	void SetTranslation(const glm::vec3& t) { Translation = t; MarkDirty(); }
	void SetRotation(const glm::vec3& r) { Rotation = r; MarkDirty(); }


	void MarkDirty() { Cache.Dirty = true; }


	const TransformCache& GetCache() const
	{
		if (Cache.Dirty)
		{
			Cache.Model = GetModelMatrix();
			Cache.InvModel = glm::inverse(Cache.Model);
			Cache.NormalMat = glm::transpose(glm::inverse(glm::mat3(Cache.Model)));

			Cache.Dirty = false;
		}
		return Cache;
	}

private:
	// So Scene::Raycast (const) can update it
	mutable TransformCache Cache;
};



struct IDComponent
{
	UUID ID;
	IDComponent() = default;
	IDComponent(UUID uuid) : ID(uuid) {}
};

struct TagComponent
{
	std::string Tag;
};

struct ScreenComponent // Screen space; UI entities
{
	uint8_t _ = 0;
};


struct MeshComponent
{
	Ref<Mesh> MeshData;
	Ref<MaterialAsset> Material;

	// Lazy-computed cached bounds
	mutable glm::vec3 BoundsCenter{0.0f};
	mutable float BoundsRadius = 0.0f;
	mutable bool BoundsCalculated = false;

	glm::vec4 BaseColor = glm::vec4(1.0); // Fallback
	bool UseNormalColors = false;

	glm::vec3 BasisRotation{0.0f, 0.0f, 0.0f};

};


struct ModelComponent
{
	Ref<ModelAsset> Model;
};

struct ModelRootComponent
{
	UUID RootID;
	std::vector<UUID> Parts;   // entities of each submesh
	glm::vec3 LastTranslation{0.0f};
	glm::vec3 LastRotation{0.0f};
	glm::vec3 LastScale{1.0f};
};

struct ModelPartComponent
{
	UUID RootID;
};

struct MaterialComponent
{
	AssetHandle BaseMaterial = UUID(0);
	MaterialDesc Desc;
};

struct TextureComponent
{
	uint8_t _ = 0;
};

struct SelectedComponent
{
	uint8_t _ = 0;
};

// TODO: Add multiple light types later, e.g. Point, Diffuse, Flashlight
struct LightComponent
{
	float Luminosity = 100.0f; // Lumens
	float Temperature = 100000; // Kelvin (1000 - 200000), 0 means ignore
	glm::vec3 TintColor = glm::vec3(1.0f);
	bool HideLight = false;
};

struct BulletComponent
{

	glm::vec3 Velocity;
};

struct WireframeComponent
{
	uint8_t _ = 0;
};

struct CameraComponent
{
	PerspectiveCamera Camera;
	float Fov;
	float AspectRatio;
	float NearPlane;
	float FarPlane;

	float MoveSpeed;

	bool Primary = false; // Denotes the primary camera in the scene


	CameraComponent(float Fov, float AspectRatio, float NearPlane, float FarPlane, float MoveSpeed)
	: Camera(Fov, AspectRatio, NearPlane, FarPlane), MoveSpeed(MoveSpeed) {}

};
