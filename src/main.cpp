#include "core/Application.h"



int main()
{
	ApplicationProperties appProps;
	appProps.Name = "App Name";
	appProps.WindowProps.Title = "Engine Core";
	appProps.WindowProps.MonitorSelected = 0; // 0 is the main monitor
	appProps.WindowProps.VSyncEnabled = false;

	Application app(appProps);
	app.Run();
}



/*

	TODO:
	- Refine Orbit Camera controls, e.g. SketchFab
		- Add interpolation curves to cursor hold movement
			- Click and throw with velocity
		- Click selects up close pivot
			- Add a recenter button
		- Add cursor hand grab cursor on click
		- Change cursor to hand when hovering overing over entity (easy)

	TODO: ============== BUGS ==============
	-
*/

/* Ownership:
		Application
		|---- Window  <--> IO (initializes window context and translates IO callbacks for scene reference)
		|---- Renderer
		|---- Scene
			    |---- Entities / Objects
			    |---- Camera

		Application
		   ↓
		 Scene --> Renderer
		   ↑
		 Window (queried, not owned)

		Application owns everything, scene tells the renderer what to render after querying window for input states.

	TODO =========================================================================================================
	1. Asset Management
	2. (Basic) Lighting System
	3. Serialization (save/load scene)
	4. (Basic) Physics & Collision
	5. Rudimentary GUI (refine progressively)
		- Debug panel
		- Drag-drop
			-- Primitives and Models
		- Hierarchy panel
			-- Select an entity in panel
			-- Delete / Duplicate
		- Inspector
			-- A panel that shows all components of the selected entity and lets you edit them, e.g. scale/position
		- Viewport (Rendered scene embedded in the GUI)
	6. Minimal Layer System
	7. Minimal Event System
	8. Tidy up -> Refine Abstractions -> Refactor

	Asset Management:
	|-- Asset Classes (Base class: (class Asset, public: AssetHandle Handle; virtual ~Asset() = default;) )
	|	-> |-- Texture
	|	-> |-- Mesh
	|	-> |-- Model
	|	-> |-- Shader
	|	-> |-- Audio
	|
	|-- Asset Registry (metadata)
	|	|-- UUID
	|	|-- Asset type
	|	|-- File path
	|	|-- Load state
	|
	|-- Asset Cache (live objects)
	|   |-- AssetLoader (Base class)
	|	-> |-- TextureLoader
	|	-> |-- MeshLoader
	|	-> |-- ShaderLoader (already have)
	|	-> |-- Audio Loader
	|	-> |-- ModelLoader
	|
	|-- Serializer / Deserializer
	|	|-- YAML / JSON
	|
	|-- File Watcher: enables for hot reloading (don't have to reload program when a file is changed)

	Asset paths are never identified by file paths at runtime, since files move, assets are renamed, etc.
		using AssetHandle = UUID;

	YAML EXAMPLE:

		Asset:
			Handle: 234823
			Type: Texture
			File: textures/wood.png

	API:

		AssetHandle LoadAsset(const std::filesystem::path&);
		template<typename T>
		Ref<T> GetAsset(AssetHandle);

		bool IsLoaded(AssetHandle);
		void ReloadAsset(AssetHandle);

*/