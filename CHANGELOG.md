# Changelog

All notable changes to **InteractiveX** are documented in this file.

The format loosely follows **Keep a Changelog** principles.

---

## [Unreleased]

### Build

- Added DirectX 11, DirectX 12 and Vulkan backend compile support flags and probes (`engine/renderer_dx11.*`, `engine/renderer_dx12.*`, `engine/renderer_vulkan.*`).
- Added Linux backend gating: Linux supports Vulkan/OpenGL backends, while DirectX backends remain Windows-only.
- Split renderer build into cleaner, per-backend switches using:
  - `IX_RENDERER_PROFILE=DX9|DX10|DX11|DX12|VULKAN|OPENGL31|OPENGL|ALL`
  - granular flags `IX_ENABLE_DX9`, `IX_ENABLE_DX10`, `IX_ENABLE_DX11`, `IX_ENABLE_DX12`, `IX_ENABLE_VULKAN`, `IX_ENABLE_OPENGL31`, `IX_ENABLE_OPENGL43`
- Separated DirectX 9 probe into its own file (`engine/renderer_dx9.*`) to avoid mixing all backend choices in one place.
- Added explicit audio helpers for MP3/OGG playback (`MediaAudio_MusicPlayMP3/OGG`, `MediaAudio_SFXPlayMP3/OGG`).
- Added JPG loading API (`Image_LoadJPG`) alongside existing PNG loading path.
- Added MP4/OGV transcode-open path (`VideoPlayer_OpenMP4`/`VideoPlayer_OpenOGV`) using ffmpeg to MPEG-1 temp files when direct decode is unavailable.
- Huhlu upgraded with basic lighting model (directional, point, spot), area light and simple shadow factor; improved ASCII FBX support including UV/normal parsing and texture path extraction (`RelativeFilename`); plus IXMaterial (PBR slots), intermediate camera modes (Orbit/FPS/Free/Ortho/Perspective), FOV/exposure/gamma, `.ixs` scene format, and simple GLTF/GLB loader.
- Expanded `.iss` style loader with responsive unit resolution (`px`, `%`, `vw`, `vh`, `rem`), positioning rules (`position`, `left`, `top`, `right`, `bottom`, `width`, `height`), color parsing, `background-image` extraction, auto layout (vertical stack), alignment, outline/border, gradients, and `height`/`weight` box sizing.
- Added responsive/scaled layout helpers (`Layout_BeginVerticalScaled`, reference/viewport scaling API) to reduce resolution-specific UI breakage.

---

## [r0.2] — 2026-03-16

First public release of **InteractiveX**.

This version introduces the core architecture of the engine and its real‑time systems, including rendering, audio, GUI, and the integrated debug environment.

---

## Engine Core

Added the fundamental systems required to build **real‑time applications and games**:

- Engine loop and subsystem initialization
- Scene management system
- Keyboard and mouse input handling
- XInput gamepad support
- High‑resolution frame timer
- Camera system (follow, zoom, shake)
- Entity system with collision helpers
- Tilemap rendering with AABB collision
- Animation system with named clips and callbacks
- Particle emitter system
- Binary save/load system
- Scene transition system (fade and wipe)
- Seeded random generator
- 2D math utilities
- Asset path resolution system

---

## Media

Added multimedia support:

- Audio playback system (music + SFX pool) using **miniaudio**
- PNG image loading via **GDI+**
- MPEG‑1 video playback using **pl_mpeg**
- Built‑in splash screen system

---

## Rendering

Direct3D9 renderer including:

- Sprite rendering
- Gradient and primitive drawing helpers
- GUI rendering utilities
- World‑space text rendering
- CPU pixel buffer rendering
- Debug overlay system
- Automatic device reset on window resize

---

## Interface System

Introduced a lightweight GUI framework with CSS‑style customization.

Components include:

- Button
- Label
- Panel
- Slider
- Layout system
- Image widget

Supports customizable:

- gradients
- borders
- sprites
- opacity
- corner radius
- font styling

---

## Debug Environment

InteractiveX ships with a **built‑in debug menu** used to test engine systems.

Includes interactive demos for:

- rendering
- input
- audio
- tilemaps
- animation
- particles
- math utilities
- transitions
- video playback
- save/load system
- gamepad input

The debug environment is compiled only when the `engine/debug` directory is included.

---

## Dependencies

InteractiveX intentionally keeps external dependencies minimal.

### Bundled

| Library | Purpose |
|-------|--------|
| **miniaudio** | Audio playback |
| **pl_mpeg** | MPEG‑1 video decoding |

### External Requirement

| Library | Purpose |
|-------|--------|
| **DirectX SDK (June 2010)** | Direct3D9 headers and libraries |

Download:  
https://www.microsoft.com/en-us/download/details.aspx?id=6812

---

**Initial release of the InteractiveX engine.**
