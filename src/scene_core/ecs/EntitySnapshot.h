#pragma once
#include "Components.h"

struct EntitySnapshot
{
	UUID id;
	std::string tag;

	bool hasTransform = false;
	TransformComponent transform;

	bool hasMesh = false;
	MeshComponent mesh;

	bool hasLight = false;
	LightComponent light;

	bool hasMaterial = false;
	MaterialComponent material;

	bool hasModelRoot = false;
	std::vector<EntitySnapshot> parts; // Deep snapshot of parts
};
