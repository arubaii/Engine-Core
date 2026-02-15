#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include "scene_core/ecs/Entity.h"

struct RayHit
{
    bool Hit = false;
    float t = std::numeric_limits<float>::max(); // distance along ray
    glm::vec3 Position{};
    glm::vec3 Normal{};
    Entity entity{};
};

struct FrameContext
{
    RayHit LastHit;
    bool   HasHit = false;
};
