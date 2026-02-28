#include "RaycastSystem.h"
#include "math/Intersect.h"
#include "scene_core/ecs/Components.h"
#include "math/Ray.h"
#include "renderer_core/Mesh.h"
#include "renderer_core/MeshUtils.h"


RaycastSystem::RaycastSystem(entt::registry& registry)
	: m_Registry(registry)
{
}

void RaycastSystem::Update(Input& input,
                           CameraComponent& camera,
                           Window& window,
                           FrameContext& ctx)
{
    ctx.LastHit = {};

    if (input.IsInUI())
        return;

    glm::vec2 mousePos = input.GetViewportMousePos(window);
    Ray ray = camera.Camera.GetRayFromScreen(mousePos, window.GetLogicalViewport());

    RayHit hit;
    if (Raycast(ray, hit))
    {
        ctx.LastHit = hit;
        ctx.HasHit = true;
    }

}

bool RaycastSystem::RayIntersectsSphere(const Ray& ray, const glm::vec3& center, float radius, float& tOut)
{

    glm::vec3 oc = ray.Origin - center;

    float b = glm::dot(oc, ray.Direction);
    float c = glm::dot(oc, oc) - radius * radius;

    float disc = b * b - c;
    if (disc < 0.0f) return false;

    float s = sqrt(disc);
    float t0 = -b - s;
    float t1 = -b + s;

    float t = (t0 > 0.0f) ? t0 : t1;
    if (t <= 0.0f) return false;

    tOut = t;
    return true;
}

bool RaycastSystem::Raycast(const Ray& ray, RayHit& outHit)
{
    bool hitAnything = false;

    auto view = m_Registry.view<TransformComponent, MeshComponent>();

    for (auto e : view)
    {
        const auto& tc = view.get<TransformComponent>(e);
        const auto& mc = view.get<MeshComponent>(e);

        if (!mc.MeshData)
            continue;

        // Cached bounds
        GPUMesh& gpu = MeshRendererCache::GetOrCreate(*mc.MeshData);

        if (!gpu.BoundsReady)
        {
            glm::vec3 c;
            float r = MeshUtils::CalculateMeshRadius(*mc.MeshData, c);
            gpu.LocalBoundsCenter = c;
            gpu.LocalBoundsRadius = r;
            gpu.BoundsReady = true;
        }

        /// World-space sphere early-out
        const auto& X = tc.GetCache();
        const glm::mat4& model = X.Model;
        const glm::mat4& invModel = X.InvModel;
        const glm::mat3& normalMat = X.NormalMat;

        glm::vec3 centerWorld =
            glm::vec3(model * glm::vec4(gpu.LocalBoundsCenter, 1.0f));

        float radiusWorld =
            gpu.LocalBoundsRadius *
            glm::max(glm::max(tc.Scale.x, tc.Scale.y), tc.Scale.z);

        float tSphereWorld = 0.0f;

        if (!RayIntersectsSphere(ray, centerWorld, radiusWorld, tSphereWorld))
            continue;

        if (tSphereWorld >= outHit.t)
            continue;

        // Local ray
        Ray localRay;
        localRay.Origin =
            glm::vec3(invModel * glm::vec4(ray.Origin, 1.0f));
        localRay.Direction =
            glm::normalize(glm::vec3(invModel *
                                     glm::vec4(ray.Direction, 0.0f)));

        const auto& verts = mc.MeshData->Vertices;
        const auto& inds  = mc.MeshData->Indices;

        for (size_t i = 0; i < inds.size(); i += 3)
        {
            glm::vec3 v0 = verts[inds[i+0]].Position;
            glm::vec3 v1 = verts[inds[i+1]].Position;
            glm::vec3 v2 = verts[inds[i+2]].Position;

            float tLocal;
            glm::vec3 nLocal;

            if (RayIntersectsTriangle(localRay, v0, v1, v2, tLocal, nLocal))
            {
                // Local hit point
                glm::vec3 pLocal = localRay.At(tLocal);
                // World hit point
                glm::vec3 pWorld = glm::vec3(model * glm::vec4(pLocal, 1.0f));
                // Compute world t along world ray so we can compare across entities
                float tWorld = glm::dot(pWorld - ray.Origin,
                               glm::normalize(ray.Direction));

                if (tWorld > 0.0f && tWorld < outHit.t)
                {
                    outHit.Hit = true;
                    outHit.t = tWorld;
                    outHit.Position = pWorld;
                    outHit.Normal =
                        glm::normalize(normalMat * nLocal);
                    outHit.entity = Entity (e, &m_Registry);

                    hitAnything = true;
                }
            }
        }
    }

    return hitAnything;
}
