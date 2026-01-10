# Shader Compilation

Die Shader müssen von GLSL zu SPIR-V kompiliert werden, bevor sie von SDL3 GPU API verwendet werden können.

## Voraussetzungen

- **glslc** (Teil des Vulkan SDK)
  - Download: https://vulkan.lunarg.com/sdk/home
  - Oder installieren über Package Manager:
    - Windows: `choco install vulkan-sdk`
    - Linux: `sudo apt install vulkan-tools` oder `sudo pacman -S vulkan-tools`
    - FreeBSD: `pkg install vulkan-tools`

## Kompilierung

### Windows (PowerShell)

```powershell
cd res\shaders
glslc sprite.vert -o sprite.vert.spv
glslc sprite.frag -o sprite.frag.spv
```

### Linux / FreeBSD

```bash
cd res/shaders
glslc sprite.vert -o sprite.vert.spv
glslc sprite.frag -o sprite.frag.spv
```

## Automatische Kompilierung

Für die Zukunft können wir das Makefile erweitern:

```makefile
# In Makefile
SHADERS = res/shaders/sprite.vert.spv res/shaders/sprite.frag.spv

%.spv: %
	glslc $< -o $@

shaders: $(SHADERS)

all: shaders gd2
```

## Shader-Dateien

- `sprite.vert` - Vertex Shader für Sprite-Rendering
- `sprite.frag` - Fragment Shader für Sprite-Rendering
- `*.spv` - Kompilierte SPIR-V Binärdateien (werden von SDL3 GPU geladen)

## Hinweise

- Die `.spv` Dateien sollten **nicht** ins Git eingecheckt werden (siehe `.gitignore`)
- Bei Änderungen an den GLSL-Dateien müssen die Shader neu kompiliert werden
- Fehler beim Kompilieren werden von `glslc` ausgegeben
