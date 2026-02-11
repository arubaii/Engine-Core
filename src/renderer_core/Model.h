#pragma once
#include <vector>
#include "utils/SmartPtrs.h"
#include "Mesh.h"
#include "asset_io/ModelImporter.h"

class Model
{
public:
	Model(const std::string& path) { Load(path); }

	const std::vector<ModelImportData::Submesh>& GetSubmeshes() const { return m_Submeshes; }

private:
	void Load(const std::string& path)
	{
		ModelImportData data = ModelImporter::Import(path);
		m_Submeshes = std::move(data.Submeshes);
	}

private:
	std::vector<ModelImportData::Submesh> m_Submeshes;
};
