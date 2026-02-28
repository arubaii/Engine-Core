#include "EntityUtils.h"
#include "asset_core/AssetManager.h"
#include "asset_io/ModelImporter.h"
#include "renderer_core/MeshUtils.h"
#include "utils/Log.h"
#include "renderer_core/GLTexture2D.h"

namespace EntityUtils
{


	/**@param startsWith The entire string up to the index number, e.g. "Sphere "
	 */
	size_t GetMaxEntityIndex(Scene& scene, std::string startsWith)
	{
		entt::registry registry = std::move(scene.GetRegistry());
		auto view = scene.GetRegistry().view<TagComponent>();

		size_t startsWithLen = startsWith.length();
		size_t maxIndex = 0;

		for (auto e : view)
		{
			auto& tag  = registry.get<TagComponent>(e);
			std::string name = tag.Tag;

			if (!name.starts_with(startsWith))
				continue;

			std::string idx = name.substr(startsWithLen);

			// Ensure it's actually a number
			if (idx.empty() || !std::all_of(idx.begin(), idx.end(), ::isdigit))
				continue;

			size_t val = std::stoi(idx); // stoi converts a string to integer value
			maxIndex = std::max(maxIndex, val);
		}

		return maxIndex;
	}

	Entity DuplicateEntity(Scene& scene, Entity src)
	{
	    if (!src) return {};

		std::string srcTag = src.GetTag();

		std::string base = srcTag;
		{
    		int i = (int)base.size() - 1;

    		if (i >= 0 && base[i] == ']')
    		{
    			i--;
    			while (i >= 0 && std::isdigit((unsigned char)base[i])) i--;

    			const std::string needle = "[Copy ";
    			int start = i - (int)needle.size() + 1;

    			if (start >= 0 &&
					(i + 1) < (int)base.size() &&
					base.compare((size_t)start, needle.size(), needle) == 0 &&
					start > 0 && base[start - 1] == ' ')
    			{
    				base.erase((size_t)(start - 1));
    			}
    		}
		}

		int nextIndex = 1;
		{
    		const std::string prefix = base + " [Copy ";
	    	entt::registry registry = std::move(scene.GetRegistry());
    		auto view = registry.view<TagComponent>();

    		for (auto e : view)
    		{
    			const std::string& name = view.get<TagComponent>(e).Tag;
    			if (name.rfind(prefix, 0) != 0) continue;

    			size_t pos = prefix.size();
    			int val = 0;
    			bool any = false;

    			while (pos < name.size() && std::isdigit((unsigned char)name[pos]))
    			{
    				any = true;
    				val = val * 10 + (name[pos] - '0');
    				pos++;
    			}

    			if (!any) continue;
    			if (pos >= name.size() || name[pos] != ']') continue;
    			if (pos + 1 != name.size()) continue;

    			nextIndex = std::max(nextIndex, val + 1);
    		}
		}

		std::string newName = base + " [Copy " + std::to_string(nextIndex) + "]";
	    Entity dst = scene.CreateEntity(UUID(), newName);

		if (src.HasComponent<TransformComponent>())
		{
			dst.GetComponent<TransformComponent>() = src.GetComponent<TransformComponent>();
			dst.GetComponent<TransformComponent>().MarkDirty();
		}

		if (src.HasComponent<MeshComponent>())
			dst.AddComponent<MeshComponent>(src.GetComponent<MeshComponent>());

		if (src.HasComponent<MaterialComponent>())
			dst.AddComponent<MaterialComponent>(src.GetComponent<MaterialComponent>());

		if (src.HasComponent<LightComponent>())
			dst.AddComponent<LightComponent>(src.GetComponent<LightComponent>());

		if (dst.HasComponent<SelectedComponent>())
			dst.RemoveComponent<SelectedComponent>();

		return dst;
	}

	void SetEntityMaterial(Scene& scene, Entity e, AssetHandle matHandle)
	{
		if (!e) return;

		auto mat = AssetManager::GetAsset<MaterialAsset>(matHandle);
		if (!mat) return;

		if (!e.HasComponent<MaterialComponent>())
			e.AddComponent<MaterialComponent>();

		auto& mi = e.GetComponent<MaterialComponent>();
		mi.BaseMaterial = matHandle;
		mi.Desc = mat->Desc;
	}

	void SetModelMaterial(Scene& scene, Entity root, AssetHandle matHandle)
	{
		if (!root || !root.HasComponent<ModelRootComponent>()) return;

		auto mat = AssetManager::GetAsset<MaterialAsset>(matHandle);
		if (!mat) return;

		auto& mr = root.GetComponent<ModelRootComponent>();
		for (UUID id : mr.Parts)
		{
			Entity part = scene.GetEntityByID(id);
			if (!part) continue;

			if (!part.HasComponent<MaterialComponent>())
				part.AddComponent<MaterialComponent>();

			auto& matc = part.GetComponent<MaterialComponent>();
			matc.BaseMaterial = matHandle;
			matc.Desc = mat->Desc; // seed instance
		}
	}

	void BindMaterial(Scene& scene, const MaterialComponent& material, const Ref<Shader>& shader, int& slot)
	{
		const auto& desc = material.Desc;
		auto bindTex = [&](const char* uniform, AssetHandle h, GLuint fallback)
		{
			GLuint id = fallback;

			if (h != 0)
			{
				auto texAsset = AssetManager::GetAsset<TextureAsset>(h);
				if (texAsset && texAsset->Texture)
				{
					GLuint candidate = texAsset->Texture->GetID();
					if (candidate != 0 && glIsTexture(candidate))
						id = candidate;
				}
			}

			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_2D, id);
			shader->SetInt(uniform, slot);
			slot++;
		};

		bindTex("u_BaseColorTex",         desc.BaseColorTexture,         scene.m_WhiteTexture);
		bindTex("u_NormalTex",            desc.NormalTexture,            scene.m_FlatNormalTexture);
		bindTex("u_MetallicRoughnessTex", desc.MetallicRoughnessTexture, scene.m_WhiteTexture);
		bindTex("u_EmissiveTex",          desc.EmissiveTexture,          scene.m_WhiteTexture);
		bindTex("u_OcclusionTex",         desc.OcclusionTexture,         scene.m_WhiteTexture);

		shader->SetVec4 ("u_BaseColorFactor", desc.BaseColorFactor);
		shader->SetFloat("u_MetallicFactor",  desc.MetallicFactor);
		shader->SetFloat("u_RoughnessFactor", desc.RoughnessFactor);
		shader->SetVec3 ("u_EmissiveFactor",  desc.EmissiveFactor);
	}

	void DeleteSnapshotRecursive(Scene* scene, const EntitySnapshot& s)
	{
		// Delete children first
		for (const EntitySnapshot& ps : s.parts)
			DeleteSnapshotRecursive(scene, ps);

		// Then delete this entity
		Entity e = scene->GetEntityByID(s.id);
		if (e) scene->DeleteEntity(e);
	}

	Entity FindModelRootFromPart(Scene& scene, Entity part)
	{
		if (!part) return {};

		UUID partID = part.GetUUID();
		auto& registry = scene.GetRegistry();

		for (auto e : registry.view<ModelRootComponent>())
		{
			Entity root{ e, &registry };
			const auto& mr = root.GetComponent<ModelRootComponent>();

			for (UUID id : mr.Parts)
				if (id == partID)
					return root;
		}

		return {};
	}

	glm::vec3 ColorFromTemperature(float kelvin)
	{
		kelvin = glm::clamp(kelvin, 1000.0f, 40000.0f) / 100.0f;

		float r, g, b;

		// Red
		if (kelvin <= 66.0f)
			r = 255.0f;
		else {
			r = kelvin - 60.0f;
			r = 329.698727446f * pow(r, -0.1332047592f);
		}

		// Green
		if (kelvin <= 66.0f) {
			g = kelvin;
			g = 99.4708025861f * log(g) - 161.1195681661f;
		} else {
			g = kelvin - 60.0f;
			g = 288.1221695283f * pow(g, -0.0755148492f);
		}

		// Blue
		if (kelvin >= 66.0f)
			b = 255.0f;
		else if (kelvin <= 19.0f)
			b = 0.0f;
		else {
			b = kelvin - 10.0f;
			b = 138.5177312231f * log(b) - 305.0447927307f;
		}

		return glm::vec3(r, g, b) / 255.0f;
	}

	float ComputeEntityFloorY(Scene* scene, Entity e)
	{
		auto& tc = e.GetComponent<TransformComponent>();

		// Mesh select
		if (e.HasComponent<MeshComponent>())
		{
			auto& mc = e.GetComponent<MeshComponent>();
			if (!mc.MeshData)
				return tc.Translation.y;

			float minY = std::numeric_limits<float>::max();
			const auto& verts = mc.MeshData->Vertices;
			const glm::mat4& model = tc.GetCache().Model;

			for (auto& v : verts)
			{
				glm::vec3 w = glm::vec3(model * glm::vec4(v.Position, 1.0f));
				minY = std::min(minY, w.y);
			}

			return minY;
		}

		// Model select
		if (e.HasComponent<ModelRootComponent>())
		{
			float minY = std::numeric_limits<float>::max();
			auto& root = e.GetComponent<ModelRootComponent>();

			for (UUID id : root.Parts)
			{
				Entity part = scene->GetEntityByID(id);
				if (!part || !part.HasComponent<MeshComponent>())
					continue;

				minY = std::min(minY, ComputeEntityFloorY(scene, part));
			}

			if (minY != std::numeric_limits<float>::max())
				return minY;
		}

		return tc.Translation.y;
	}

	float ComputeXZRadius(Scene* scene, Entity e)
	{
		auto& tc = e.GetComponent<TransformComponent>();

		if (e.HasComponent<MeshComponent>())
		{
			auto& mc = e.GetComponent<MeshComponent>();
			if (!mc.MeshData)
				return 0.0f;

			auto& mesh = *mc.MeshData;
			glm::mat4 M = tc.GetCache().Model;

			float minX = FLT_MAX, maxX = -FLT_MAX;
			float minZ = FLT_MAX, maxZ = -FLT_MAX;

			for (auto& v : mesh.Vertices)
			{
				glm::vec4 world = M * glm::vec4(v.Position, 1.0f);
				minX = std::min(minX, world.x);
				maxX = std::max(maxX, world.x);
				minZ = std::min(minZ, world.z);
				maxZ = std::max(maxZ, world.z);
			}

			float extentX = 0.5f * (maxX - minX);
			float extentZ = 0.5f * (maxZ - minZ);

			return std::max(extentX, extentZ);
		}

		if (e.HasComponent<ModelRootComponent>())
		{
			float maxRadius = 0.0f;
			auto& root = e.GetComponent<ModelRootComponent>();

			for (UUID id : root.Parts)
			{
				Entity part = scene->GetEntityByID(id);
				if (!part)
					continue;

				maxRadius = std::max(maxRadius, ComputeXZRadius(scene, part));
			}

			return maxRadius;
		}

		return 0.0f;
	}

	float ComputeEntityRadius(Scene& scene, Entity entity)
	{
		// Single mesh entity
		if (entity.HasComponent<MeshComponent>() && !entity.HasComponent<ModelRootComponent>())
		{
			auto& mc = entity.GetComponent<MeshComponent>();
			if (mc.MeshData)
			{
				glm::vec3 center;
				float localRadius = MeshUtils::CalculateMeshRadius(*mc.MeshData, center);

				// Account for scale transformation
				auto& tc = entity.GetComponent<TransformComponent>();
				float maxScale = glm::max(glm::max(tc.Scale.x, tc.Scale.y), tc.Scale.z);

				return localRadius * maxScale;
			}
			return 5.0f;
		}

		// Multi-part model
		if (entity.HasComponent<ModelRootComponent>())
		{
			float maxRadius = 0.0f;
			auto& parts = entity.GetComponent<ModelRootComponent>().Parts;

			for (UUID id : parts)
			{
				Entity part = scene.GetEntityByID(id);
				if (!part || !part.HasComponent<MeshComponent>())
					continue;

				auto& mc = part.GetComponent<MeshComponent>();
				if (!mc.MeshData)
					continue;

				glm::vec3 meshCenter;
				float localRadius = MeshUtils::CalculateMeshRadius(*mc.MeshData, meshCenter);

				auto& tc = part.GetComponent<TransformComponent>();
				float maxScale = glm::max(glm::max(tc.Scale.x, tc.Scale.y), tc.Scale.z);

				maxRadius = glm::max(maxRadius, localRadius * maxScale);
			}

			return maxRadius > 0.0f ? maxRadius : 5.0f;
		}

		return 5.0f;
	}

	bool SnapshotContainsUUID(const EntitySnapshot& s, UUID id)
	{
		if (!id) return false;
		if (s.id == id) return true;

		for (const auto& p : s.parts)
			if (SnapshotContainsUUID(p, id))
				return true;

		return false;
	}

	glm::vec3 GetEntityForward(const TransformComponent& tc)
	{
		const glm::mat4& M = tc.GetCache().Model;

		glm::vec3 right   = glm::vec3(M[0]);
		glm::vec3 up      = glm::vec3(M[1]);
		glm::vec3 forward = -glm::vec3(M[2]);

		float r2 = glm::dot(right, right);
		float u2 = glm::dot(up, up);
		float f2 = glm::dot(forward, forward);

		if (r2 > 1e-8f) right   /= glm::sqrt(r2); else right   = glm::vec3(1.0f, 0.0f, 0.0f);
		if (u2 > 1e-8f) up      /= glm::sqrt(u2); else up      = glm::vec3(0.0f, 1.0f, 0.0f);
		if (f2 > 1e-8f) forward /= glm::sqrt(f2); else forward = glm::vec3(0.0f, 0.0f, -1.0f);

		forward = glm::normalize(glm::cross(right, up));
		return -forward;
	}

} // namespace EntityUtils