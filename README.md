# Engine-Core

## Cloning the repository

This project uses the Git submodules: `ImGui`, `YAML`, `assimp`

Clone with:

```bash
git clone --recurse-submodules https://github.com/arubaii/Engine-Core.git
```
The scene adopts an ECS for entity management, as opposed to an object-oriented hierarchy. This allows for 
easy runtime entity configuration. 

The code architecture is informally documented in main.cpp.

Current Features 
- Model importing (all supported Assimp file types, e.g. `.obj`, `.fbx`, `.gltf`, `.glb`)
- Physically based rendering (still in progress)
- Material types
- Perspective free/orbit camera controls
- Undo and redo actions
- Duplicating and deleting entities in scene


Currently working on
- HDR skybox loader
- Better materials (e.g. clear coat, textured materials like rust or brushed aluminium)
- Robust GUI for model importing by file drag and drop
- Basic physics
- Basic audio engine
- Entity gizmos
- Scene serialization (reloading scene specs)

Showcase: 


[Watch the engine showcase](https://www.dropbox.com/scl/fi/utqbizelglxb73pumgol6/engine_showcase.mp4?rlkey=wkrip3kqnec3h1t62uzubdo3b&st=alp0ipyk&dl=0)