# AI Coding Guide

## Project Overview

**gd2** ist ein Platzhaltername für ein 2D Jump'n'Run-Spiel. Die Abkürzung "gd2" steht für "George Decker 2", als inoffizieller Nachfolger
zu meinem früheren Projekt (siehe https://github.com/pfedick/DeckerGame). Das frühere Spiel nutzte Klemmbaustein-Optik und "Minifiguren" als Charaktere, inspiriert von klassischen Lego-Sets. Um rechtlichen Problemen mit der Marke "Lego" zu entgehen, wird das neue Projekt seine eigene visuelle Identität erhalten, mit neuen Charakteren, Umgebungen und Geschichten. Ziel ist es, das neue Spiel auf Steam und anderen Plattformen zu veröffentlichen.

### Genre
Metroidvania-ähnliches 2D Jump'n'Run mit Fokus auf Erkundung, Level-Design und interessanten Charakteren. Es wird Elemente von Plattforming, Rätseln, Action und Magie enthalten.

### Setting
Eine fantasievolle Welt mit einer Mischung aus mittelalterlichen, magischen und technologischen Elementen.

Weitere Informationen und Ideen zum Spiel sind in [concept.md](../concept.md) dokumentiert.

### Multiplayer
Das Spiel wird einen Koop-Modus für 2 Spieler bieten, in dem beide Spieler gleichzeitig die Welt erkunden können. Die Spieler können sich gegenseitig helfen, Rätsel lösen und Gegner bekämpfen. Zum Spielen müssen beide Spieler über ein lokales Netzwerk (auch VPN) verbunden sein (LAN). Um das Spielen zu zweit interessant zu machen, werden die Fähigkeiten, die der Spieler im Singleplayer-Modus erlernen kann, auf beide Charaktere aufgeteilt. Beispielsweise könnte ein Spieler die Fähigkeit erlernen, höher zu springen, während der andere die Fähigkeit erhält, schwere Objekte zu bewegen. Dadurch müssen die Spieler zusammenarbeiten, um bestimmte Bereiche zu erreichen oder Rätsel zu lösen, was den Koop-Modus spannend und herausfordernd macht. Die Fähigkeiten sind jedoch an bestimmten Punkten im Spiel übertragbar, so dass die Spieler auch im Koop-Modus die Möglichkeit haben, ihre Fähigkeiten zu tauschen und gemeinsam zu entscheiden, welche Fähigkeiten sie für die bevorstehenden Herausforderungen benötigen.

### Technik
Das Spiel wird in C++23 mit SDL3 und OpenGL/Vulkan/Metal (über SDL3 GPU API) entwickelt. Es nutzt die PPL7-Bibliothek für grundlegende Funktionen (Strings, Dateien, Grafix) und die PPLTK für die UI (Fenster, Widgets, Events). Alle Grafiken werden als 2D-Sprites mit Normal Maps für Beleuchtungseffekte gerendert. Das Ziel ist es, eine moderne, flexible Engine zu bauen, die auf verschiedenen Plattformen läuft und sowohl einfache als auch fortgeschrittene visuelle Effekte ermöglicht. Zielplattformen sind Windows und Linux, mit möglicher späterer Unterstützung für MacOS. Auch das SteamDeck soll unterstützt werden.

### Darstellung der Welt:
- Parallax-Scrolling mit mehreren Ebenen (Hintergrund, Midground, Vordergrund).
- 2D Deferred Lighting mit Normal Maps für dynamische Beleuchtung und Schatten.
- Depth Blur für atmosphärische Effekte auf entfernten Ebenen.
- Atmosphärische Effekte wie Nebel, Partikel, Wetter pro Ebene.
- Pixel-Shader- oder Grafik-Effekte für spezielle Aktionen (z.B. Sprinten, Magie).

## Design-Entscheidungen
- Die Welt basiert auf einem Raster von 0,5m x 0,5m großen Kacheln, welches 100 x 100 Pixeln bei einer Auflösung von 4k entspricht.
- Tiles entsprechen dieser Größe oder sind ein vielfaches davon (z.B. 1x1, 2x1, 2x2).
- Grafiken werden grundsätzlich für eine 4k-Auflösung vorgerendert, um eine hohe Detailgenauigkeit zu ermöglichen. Beim Rendern werden diese auf die tatsächliche Größe des Renderbuffers runterskaliert.
- Es gibt einen logischen Viewport, der immer eine Auflösung von 3840x2160 hat (4K). Alle Berechnungen für Positionen, Größen, Kollisionen, Geschwindigkeiten, etc. erfolgen in diesem logischen 4K-Raum.
- Die tatsächlichen Renderziele können niedriger sein (z.B. 1920x1080) für bessere Performance auf integrierten GPUs.
- In Lightwave 3D wird eine Orthogonale Kamera verwendet, die in ihrer horizontalen dem Raster der Welt entspricht, also 200 Pixel pro Meter
- Alle Grafiken werden mit Normal Maps erstellt, um dynamische Beleuchtung zu ermöglichen.
- Die Engine verwendet Pre-multiplied Alpha (PMA) für alle Texturen und Blending-Operationen, um saubere Transparenz und stabile Ergebnisse bei Effekten wie Blur zu gewährleisten.
- Der GameRenderer optimiert das Zeichnen von Sprites und Primitives, beachtet aber strikt den Painter's Algorithm für die Renderreihenfolge, um korrekte Überlagerungen zu gewährleisten, insbesondere bei semitransparenten Objekten.

## Roadmap & Derzeitiger Fokus
- Player-Mechaniken (Bewegung, Kollision, Kamera) (aus George Decker übernommen, Refakturierung steht noch aus)
- 2D Deferred Lighting (Global/Spot/Point Lights + Shadows + Normal Maps)
- Erstellung von Assets (Sprites, Normal Maps, Tilesets) in Lightwave 3D
- Festlegung eines konsistenten visuellen Stils und einer Farbpalette

### Abgeschlossene Schritte:
- Editor zum Erstellen von Level: Ein einfacher Editor wurde implementiert, mit dem man Level erstellen und bearbeiten kann. Er unterstützt das Platzieren von Kacheln, Sprites und Objekten, sowie das Speichern und Laden von Leveln.
- Parallax-Scrolling mit mehreren Ebenen: Es wurden mehrere Ebenen für den Hintergrund, Midground und Vordergrund erstellt, die sich mit unterschiedlichen Geschwindigkeiten bewegen, um einen Tiefeneffekt zu erzeugen.
- Gausscher Blur für entfernte Ebenen: Ein einfacher Gausscher Blur wurde implementiert, um entfernte Ebenen unscharf zu machen und so die Tiefenwirkung zu verstärken.
- Tiles, Sprites, Objekte und Animationen: Es wurden grundlegende Klassen für Tiles, Sprites und Objekte erstellt, die in den Leveln platziert werden können. Es gibt auch eine einfache Animationslogik für Sprites.
- Einfache Kamera, die dem Spieler folgt: Eine Kamera wurde implementiert, die dem Spielercharakter folgt und den sichtbaren Bereich der Welt steuert.
- Einfache Player-Mechaniken: Es wurden grundlegende Bewegungsmechaniken für den Spieler implementiert, einschließlich Laufen, Springen und Kollisionserkennung mit der Welt.
- Zusammenfassen der GPU-spezifischen Klassen und GPUBatcher in zentraler Klasse "GameRenderer". Die Renderlogik wurde vereinfacht, um die korrekte Reihenfolge von Sprites und Primitives zu gewährleisten, insbesondere bei semitransparenten Objekten. Es wird nun strikt das "Painter's Algorithm" für die Renderreihenfolge eingehalten. Metriken zur Renderleistung werden erfasst, um Engpässe zu identifizieren und gezielt zu optimieren.

### Next Focus: Player mechanics and camera handling
A player character will be added, which can be controlled by keyboard or gamepad.

The basic mechanics of walking, running, jumping and falling will be implemented in this step, also camera handling.

Player mechanics:
- left/right movement with acceleration and deceleration
- walking and running speed
- gravity and falling
- jumping with jump arc
- jump height and jump distance
- climbing ladders
- sliding on slopes
- collision detection with the level grid

Advanced jumping mechanics:
- Coyote time: after leaving a platform, the player can still jump for a short time (e.g. 0.2 seconds)
- Variable jump height: the longer the jump button is pressed, the higher the jump (up to a maximum)
- Wall jump: when touching a wall, the player can jump off the wall
- Double jump: the player can jump a second time while in the air
- Jump buffering: if the player presses the jump button shortly before landing, the jump will be executed immediately after landing
- Fast falling: gravity get's increased when the player start's to fall after a Jump


Camera handling:
- camera follows the player
- camera has a certain speed, it does not snap directly to the player position
- dead zone: a rectangle around the player, where the camera does not move when the player moves inside this rectangle
- camera boundaries: the camera does not move outside the level boundaries
- when running, the camera should look ahead in the movement direction, to give the player more visibility
- smooth camera movement: avoid sudden jumps, start slowly, accelerate, decelerate, stop smoothly
- parallax layers: the camera movement affects the parallax layers differently, creating a depth effect
- optional: camera shake effect for certain events (e.g. landing, explosions)
- optional: zoom in/out effect for certain events (e.g. sprinting, aiming)
- optional: cinematic camera movement for cutscenes or special events


### Future Focus: 2D Deferred Lighting Roadmap
(Context from session: Jan 13 2026 - User wants Global/Spot/Point lights + Shadows + Normal Maps)

**Phase 1: G-Buffer Setup (MRT)**
- Enable **Multiple Render Targets** in the sprite pipeline.
- Output 1: Albedo (Color)
- Output 2: Normal Map (RGB encoded XYZ).
- Requires updating `SpritePipeline` and `sprite.frag` to write to location 0 and 1.

**Phase 2: Light Accumulation Pass**
- Create new render target `render_target_light` (cleared with ambient color, e.g. dark blue/black).
- Use **Additive Blending** pipeline.
- Render "Light Quads" (geometry at light position) into this target.
- Shader logic: 
  - Read Normal Map from G-Buffer.
  - Calculate `dot(LightDir, Normal)` and Distance Attenuation.
  - Output light color intensity.

**Phase 3: Composite Pass**
- Final step: Multiply Albedo with Light Accumulation result.
- `FinalColor = Albedo * LightResult`.

**Phase 4: Shadows (Method A: Raymarching/2D Shadow Casting)**
- Create `OcclusionMap` (Solid pixels = 1.0, Air = 0.0).
- In the Light Shader: Perform simple 2D Raymarching from Pixel to LightPos using the OcclusionMap.
- If ray hits wall -> Light = 0.0 (Shadow).

---

## General AI Assistant Guidelines

**User Preferences**
- **Language**: German (Informal / "Du").
- **Edit Policy**:
- **Default**: Keine automatischen Änderungen anwenden. Code-Vorschläge als Snippets im Chat präsentieren, damit der Nutzer sie selbst implementieren kann (Learning by Doing).
- **Explizite Aufforderung**: Tools für Datei-Edits nur nutzen, wenn der Nutzer dies explizit wünscht (z. B. „Übernimm das für mich“ oder „Fix anwenden“).
- **Proaktives Angebot bei großen Änderungen**: Wenn eine Aufgabe Änderungen an mehr als **2 Dateien** oder insgesamt mehr als **100 Zeilen Code** erfordert, darfst du proaktiv anbieten, die Datei-Edits für den Nutzer durchzuführen.
- **Fortführung**: Reagiert der Nutzer nicht auf ein Angebot oder einen Snippet, wird davon ausgegangen, dass er die Änderungen selbst vorgenommen hat.

**Code generation philosophy**: Avoid generating large blocks of code the user doesn't understand. Instead:
- Explain concepts and patterns step-by-step
- Provide focused code snippets (minimal examples, not full implementations)
- Guide the user through problem-solving: ask clarifying questions, suggest approaches, let them write/adapt code
- Reference existing files/patterns in the codebase to learn from
- Prioritize teaching over fast delivery (Learning by Doing)

**When asked to implement features**:
- Start with architecture/design discussion
- Show a minimal skeleton or example
- Explain the "why" behind choices
- Let the user fill in details and adapt to their needs
- Review and refactor together, not in isolation

**Accuracy and verification**:
- Be honest about uncertainties: If you're not sure about something, say so clearly.
- If a solution is only a guess or hypothesis, communicate this immediately and clearly
- Whenever possible, verify assumptions through code analysis or tool usage before suggesting them
- Never invent API methods that don't exist; always verify against actual code, headers, or documentation
- Check parameters against documentation, function signatures, or existing usage patterns before proposing them
- Use available tools (grep_search, semantic_search, read_file, list_code_usages) to verify:
  - Whether a function/method exists in the codebase
  - What parameters it actually accepts
  - How it's used elsewhere in the project
  - What the actual API surface looks like (especially for SDL3, PPL7, PPLTK)
- When uncertain about SDL3 GPU API specifics, search the codebase for existing usage patterns or read relevant headers
- For PPL7/PPLTK APIs, reference the actual source files in pplib/include or ppltk/include to confirm method signatures


## Architecture & Key Files

### Core Structure
- **Entry Point**: [src/main.cpp](src/main.cpp) - Initializes `GPUContext`, `WindowManager_SDL3`, and starts `Game::run`.
- **Game Logic**: [src/game.cpp](src/game.cpp) / [src/game.h](src/game.h) - Main loop and game state.
- **GPU Layer**: `GPUContext` in [src/gpu.cpp](src/gpu.cpp) - Wraps `SDL_GPUDevice`, manages swapchain, texture uploads, and shutdown.
- **UI Toolkit (PPLTK)**: `ppltk/` - Custom widgets on top of SDL3.
  - Manager: `ppltk::WindowManager_SDL3` in [ppltk/src/WindowManager.cpp](ppltk/src/WindowManager.cpp) (Singleton).
  - Widgets: Tree structure in [ppltk/src/Widget.cpp](ppltk/src/Widget.cpp).
- **Core Library (PPL7)**: `pplib/` - Foundation types (`ppl7::String`, `Exception`, `Grafix`).

### Developer Workflow & Build
**Build System**: Autotools + Recursive Make (enforces C++23).
1. **Bootstrap**: `./genConfigure` (Root)
2. **Configure**: `./configure`
3. **Build All**: `make` (builds `pplib`, `ppltk`, and `gd2`)
   - **Game Only**: `make gd2`
   - **Libs Only**: `make ppl7`, `make ppl_toolkit`
   - **Clean**: `make clean` or `make cleanall`
4. **Platform**: `make mingw` (Windows deploy with DLLs).

**Dependencies**: SDL3, zlib, bzip2, pcre2, freetype, libpng, libjpeg, dav1d, assimp (checked in `configure.ac`).

### Code Conventions
- **Exceptions**: Use `throw ppl7::Exception("msg")` or `GPUException` for errors.
- **SDL3**: 
  - Prefer `SDL_GPUDevice` pipelines for game rendering.
- **Memory**: PPLTK widgets are owned by parents (`destroyChilds` handles cleanup).

## PPLTK Architecture & Integration
- **Window Manager**: Singleton `wm` handles events/redraws via `SDL_Window`.
- **Event Loop**: Events flow from SDL3 -> `WindowManager` -> `dispatchMouseEvent` -> Widgets.
- **Subclassing**: New widgets override `mouseDownEvent`, `paintEvent` (from `ppltk/src/Event.cpp`).
- **Redraw**: Call `needsRedraw()` on change; do not paint outside `paintEvent`.
- **Resources**: Fonts/Icons in `resources/res.h` (generated via `make res`).


## SDL3 GPU Architecture

**Renderer vs GPU path**: For gd2, **use SDL3's GPU API** (not SDL_Renderer) for all drawable sprites, tiles, and lighting passes. 
- **Implementation**: See the `GPUContext` class in [src/gpu.cpp](src/gpu.cpp) which wraps `SDL_CreateGPUDevice()` (Vulkan/SPIR-V preferred).
- **Strategy**: Commit to GPU-first: Pipelines, Command Buffers, Bind Groups.

**SDL3 Sources**: Include files can be found in "tmp/SDL3", which is a copy from the SDL3 source code.
Refer to `SDL_gpu.h` for API details. Key concepts:
- `SDL_GPUDevice`: Main GPU context (Vulkan/DirectX/Metal).
- `SDL_GPUTexture`: Explicit GPU textures (no more implicit `SDL_Texture`).
- `SDL_GPUCommandBuffer`: Record draw calls and resource bindings.


**Learning resources**: Excellent tutorials on SDL3 GPU API usage:
- [Getting Started with SDL3_GPU](https://glusoft.com/sdl3-tutorials/getting-started-sdl3_gpu/) - Basic setup and initialization
- [Display Triangle with SDL3_GPU](https://glusoft.com/sdl3-tutorials/display-triangle-sdl3_gpu/) - First rendering example
- [Display Texture with SDL3_GPU](https://glusoft.com/sdl3-tutorials/display-texture-sdl3_gpu/) - Texture loading and rendering
- [Sprite Batching with SDL3 GPU](https://glusoft.com/sdl3-tutorials/sprite-batching-sdl3-gpu/) - Efficient batch rendering for sprites
- [SDL3 GPU API Documentation](https://wiki.libsdl.org/SDL3/CategoryGPU) - Official API reference

**Texture loading workflow**:
- Create GPU textures: `SDL_CreateGPUTexture()` with `SDL_TEXTUREUSAGE_SAMPLING`.
- Upload via `SDL_UploadToGPUTexture()`.
- Offscreen render targets (e.g., for blur passes) use `SDL_TEXTUREUSAGE_RENDERTARGET`.
- Keep CPU copies only for streaming/dynamic updates.

**Minimal frame loop**:
```cpp
auto cmd = SDL_AcquireGPUCommandBuffer(device);
auto pass = SDL_BeginRenderPass(cmd, &renderPassDesc);
SDL_BindGPUGraphicsPipeline(pass, pipeline);
SDL_BindGPUVertexBuffers(pass, ...);
SDL_BindGPUFragmentSamplers(pass, ...);  // textures/samplers
SDL_DrawPrimitives(pass, vertexCount);
SDL_EndRenderPass(pass);
// Optionally: blur pass on offscreen texture
SDL_SubmitGPUCommandBuffer(cmd);
SDL_PresentGPUWindow(window);
```

## Lighting & Normal Mapping (Detailed Strategy)

**Architecture: 2D Deferred / Hybrid**
- **G-Buffer Pass**: Render all sprites (layers + entities) to MRT (Multiple Render Targets): 
  - Target 0: Albedo (RGB)
  - Target 1: Normal (RGB, encoded [0..1]) + optionally Roughness/Emissive/Occlusion in Alpha.
- **Light Accumulation Pass**:
  - Bind "G-Buffer Normal" as input sampler.
  - Clear LightMap with Ambient Color.
  - Draw Light Primitives (Quads covering the light radius) with **Additive Blending**.
  - **Shader**: Sample Normal at screen pos; `lighting = dot(normalize(LightPos - PixelPos), Normal) * attenuation`.
- **Composite Pass**:
  - `FinalColor = Albedo * LightMap`.

**Shadows (Method A: 2D Raymarching)**
- Requires an **Occlusion Map** (could be a separate low-res texture or the Alpha channel of the Normal Target).
- Light Shader steps uniform ray towards light source; checks occlusion pixels.
- Use `step_size` appropriate for pixel art resolution to avoid artifacts.

## Parallax Scrolling & Depth Blur

**Parallax layers**: Store each layer with a parallax factor (0.0 = fixed, 1.0 = camera depth). Offset layer's camera position by `camera.offset * parallax_factor` during rendering.

**Depth blur**:
1. Render far layers to offscreen color target (half-res to save bandwidth).
2. Apply separable Gaussian blur (horizontal pass → intermediate, vertical pass → final blur texture).
3. Composite blurred far layers + sharp near layers to main render target.
4. Use small kernels (5–9 tap) for performance; profile on target GPUs.

### Parallax Size Scaling (per-layer 1:1 on player layer)

- Goal: Keep sprites 1:1 on the main player layer; render smaller on far layers and larger on near layers.
- Data: add to each visual layer:
  - `parallax_factor` (scroll offset multiplier)
  - `scale` (size multiplier; 1.0 for player layer; <1.0 for far; >1.0 for near)
  - Optional `depth` to derive `scale` via a mapping.
- Suggested mappings:
  - Direct: `scale = layer.scale` (author-defined)
  - Depth-based: `scale = 1.0 / (1.0 + k * depth)` or `scale = mix(near_scale, far_scale, depth_norm)`
- Rendering transform per layer: `M_layer = T(camera_offset * parallax_factor) * S(scale)`; apply `M_layer` before the per-entity transform.
- Collision decoupling: collision/tile sizes stay constant in the `CollisionGrid`; only rendering uses `scale`.
- Assets and filtering:
  - Generate mipmaps for albedo and normal maps to avoid shimmer when downscaling.
  - Use appropriate min/mag filters (linear for modern look; nearest for retro).
  - Ensure normal map uses the same sampler and LOD policy as albedo.
- UI is unscaled: render ppltk HUD after world with `scale = 1.0`.

## Shader & Resource Binding

- Create one shader pipeline for lit sprites; separate simple pipeline for UI/unlit HUD.
- Use sampler objects: wrap clamp for sprites, wrap repeat for tiled backgrounds.
- Bind albedo + normal texture per draw or per layer in a `bind group`; add per-draw uniform (sprite rect, animation frame) if needed.
- Additive blend mode for glow/emissive passes.

## Coordinate Systems

- Choose consistent origin (NDC with Y up is standard) and bake into vertex shader.
- Keep normals in tangent space for flexibility.
- Ensure UV origin matches texture load expectations (top-left vs bottom-left) to avoid flipped normals.

## Screen Resolution & Scaling

**Strategy: Logical 4K / Physical Variable Rendering**
 
To balance simplicity and performance, the game uses a dual-resolution approach:
 
- **Logical Engine (4K)**: All game logic coordinates, physics, collision detection, and sprite placements are calculated in a fixed 3840x2160 (4K) coordinate system. This ensures consistent gameplay regardless of the actual rendering resolution.
- **Physical Rendering (Targeted, e.g., 1080p)**: The actual GPU render targets (G-Buffer, Lightmaps, Blur-Buffers) use a potentially lower resolution (e.g., 1920x1080) to significantly reduce the fill-rate and memory bandwidth requirements on integrated or entry-level GPUs.
 
- **Workflow**:
  1. All positions and sizes are passed to the `GPUBatcher` in 4K logical units.
  2. `GPUBatcher` uses projection matrices derived from both `logicalSize` and `renderSize` to map these 4K coordinates into the physical render target.
  3. The internal render targets are then scaled to the final window size (with letterboxing/pillarboxing maintaining a 16:9 ratio) by the `GameViewport` and `Level::draw` logic.
 
- **Rationale**: Optimization for integrated GPUs while keeping the development simplicity of a single coordinate space.
 
- **UI Rendering**: The UI (managed by PPLTK) is rendered at the native window resolution for maximum text clarity, overlayed on the scaled game world.

## Alpha & Blending

**Strategy: Global Pre-multiplied Alpha (PMA)**

To avoid dark borders during blurring, scaling, and transparency blending across offscreen layers, the entire engine uses a Pre-multiplied Alpha pipeline.

- **Assets**: Textures can be stored as Straight Alpha; the `sprite.frag` shader or the CPU loader converts them to PMA (`rgb * alpha`) before any blending occurs.
- **Pipelines**: All blend states use `src_color_blendfactor = ONE` and `dst_color_blendfactor = ONE_MINUS_SRC_ALPHA`.
- **Modulation**: Color modulation (tinting) is applied as a PMA multiplication in the `GPUBatcher`.

## Asset Resolution Strategy

When rendering high-res assets into a smaller physical buffer, image stability is key:

**High-res sprites with Mipmapping**
- Sprites are created at 4K resolution (or scaled from Lightwave assets).
- **Mipmaps (Pending)**: Every GPU texture should have calculated mip-levels and be generated via `SDL_GenerateMipmapsForGPUTexture` to prevent flickering/shimmering (aliasing) when downscaling 4K assets to 1080p.
- **Filtering (Pending)**: Use `SDL_GPU_SAMPLERMIPMAPMODE_LINEAR` and Anisotropic filtering (e.g. 8x).
- **Stability**: Currently, without mipmaps, downscaling to 1080p may cause shivering. Integer scaling or high-quality filtering is required.

**Avoid**: Rendering without mipmaps when logical resolution is higher than physical resolution.

## Migration Notes from SDL2 / DeckerGame

- **Update (Jan 2026/SDL 3.4+):** `SDL_Renderer` can now run on top of `SDL_GPUDevice`. Use `SDL_CreateRendererWithProperties` to share the context. This allows mixing high-level 2D drawing (UI) with low-level GPU pipelines (Game World).
- `SDL_Texture` (renderer) → `SDL_GPUTexture` (explicit GPU resource). No implicit blits; issue draw calls with bound data.
- Shaders are **explicit pipelines** from compiled bytecode (SPIR-V/DXIL/MSL depending on backend). Compile GLSL/HLSL offline or use SDL_shader helpers at runtime.
- Render targets are explicit: offscreen blur requires `SDL_GPUTexture` with `SDL_TEXTUREUSAGE_RENDERTARGET`.
- No `SDL_RenderCopyEx` or automatic scaling; write your own quad drawing with viewport/scissor if needed.

## pplib/ppltk Integration

- ppltk window/event loop already migrated to SDL3; ensure GPU device creation uses the ppltk window handle.
- Use pplib for string utilities, file I/O (asset loading), and the `grafix` namespace for CPU-side pixel manipulation if needed (e.g., generating height/normal maps on load).
- Keep ppltk UI separate from game GPU rendering: ppltk for menus, game loop for parallax/lighting.
## Audio Mixing & SDL3

**Architecture**:
- Maintain separate mix channels: music, sound effects, voice (or UI). Float-based mixing (32-bit) is cleaner than integer and reduces rounding artifacts in multi-channel blend.
- pplib provides audio decoders/encoders (MP3, Ogg, AIFF, Wave via `ppl7::audio` namespace) and ID3 tag reading; use these to load source files.

**SDL3 audio vs SDL2**:
- SDL2: audio callback with fixed format push/pull. SDL3: audio streams API is more flexible; create streams per logical channel or one stream per source, then mix in software before final device output.
- No more audio format conversion awkwardness; SDL3 handles resampling better.
- Consider using `SDL_OpenAudioDeviceStream()` for device output and `SDL_CreateAudioStream()` for source mixing pipelines.

**Float mixing strategy**:
- Load source (e.g., Ogg via pplib decoder) → resample to target rate (e.g., 48 kHz) → mix channels as float (accumulate with pan/volume per channel) → clamp to [-1.0, 1.0] → convert to output format (S16 or float depending on device).
- Keep accumulated samples as float to minimize precision loss; convert to S16/S32 only at device output if hardware is integer.
- Separate channel volumes and pans; sum all channels in a single mix pass per frame.

**Practical setup**:
- Create one output stream to the audio device (stereo, float or S16).
- Maintain ring buffers or streaming sources for music (with loop points), SFX (fire-and-forget), voice (streaming or queued).
- Each frame (or per audio callback if using one), read from sources, apply gain/pan per channel, sum, and submit to output stream.

**Performance**:
- Float mixing is CPU-cheap on modern hardware. Profile on target platforms (Windows mingw, Linux).
- Resample once at load time if possible, or use fast linear/cubic resampling per-frame.
- Consider limiting voice channels (e.g., max 4 simultaneous voices) to reduce CPU.

## World Architecture: Data & Draw Order

- Core model: keep one canonical grid for gameplay/collision and separate visual layers for rendering.
  - `CollisionGrid` (fixed raster): per-cell flags/types (solid, slope, ladder, ramp, platform, hazard). Not rendered; used by physics.
  - `TileLayers[]` (visual): 2–4 layers of tilemaps that reference atlas frames (albedo + normal). Each layer has `parallax_factor` and optional blur participation.
  - `Entities[]`: free-placed sprites/objects with components: Transform (x,y,z), Render (atlas frame/albedo+normal), Collider (optional), Interaction (optional).
  - `Lights[]`: point/ambient lights with position, color, radius, intensity; optionally associated to a render layer.

- Storage format (binary, chunk-based): Level Format is documented in [level.md](../level.md)

- Render ordering (per frame):
  1) Background parallax layers → offscreen target (optional downsample) → blur passes
  2) Mid/foreground tile layers (sharp) → main target
  3) Entities behind tiles (z < layer mid)
  4) Player and interactables
  5) Foreground entities and overlay tiles
  6) UI/HUD via ppltk (unlit pipeline)

- Batching strategy:
  - Group tiles/entities by atlas/material to minimize pipeline/bind switches.
  - Build per-layer render lists; sort by texture, then draw.
  - Use instancing for repeated tiles; push small per-draw uniforms (UV rect, transform).

- Parallax & blur:
  - Each visual layer carries `parallax_factor` (0.0 fixed → 1.0 camera). Far layers render to half-res offscreen and run separable Gaussian blur before compositing.

- Keep it simple:
  - One physics grid → less confusion; visuals decoupled from collision.
  - Entities carry `render_layer` and `z` for front/behind placement instead of duplicating structures per layer.



