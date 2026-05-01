# InteractiveX

<p align="center">
  <img src="https://img.shields.io/badge/version-r0.2-blue" />
  <img src="https://img.shields.io/badge/platform-Windows-lightgrey" />
  <img src="https://img.shields.io/badge/renderer-Direct3D9-purple" />
  <img src="https://img.shields.io/badge/language-C%20%2F%20C%2B%2B-orange" />
  <img src="https://img.shields.io/badge/license-open--source-green" />
  <img src="https://img.shields.io/badge/audio-miniaudio-blue" />
  <img src="https://img.shields.io/badge/video-pl__mpeg-blue" />
</p>

<p align="center">
  Lightweight • Transparent • Full‑source Real‑Time Engine
</p>

---

## About

**InteractiveX** is a lightweight **real‑time engine for Windows** built on top of **Direct3D 9**, written entirely in **C/C++**.

It can be used to build **games, graphical applications, developer tools, and other real‑time software**.

The engine focuses on **simplicity, transparency, and full control**.  
Instead of hiding systems behind layers of abstraction, InteractiveX exposes everything.

You are free to read, modify, replace, or extend any part of the engine.

> **The source is the documentation.**

---

## Philosophy

Many engines hide their internal systems behind complex frameworks and tooling.

InteractiveX follows a different philosophy:

- **No black boxes**
- **Readable engine code**
- **Minimal abstraction**
- **Modular architecture**
- **Full control over rendering and systems**

If you want to understand how something works — you can simply read the source.

---

## Features

Current version **r0.2** includes a complete set of core systems for building **real‑time applications and games**.

### Core Systems

- Scene management
- Input system (keyboard, mouse)
- XInput gamepad support
- Camera system (follow, zoom, shake)
- Entity system
- Tilemap rendering with collision
- Animation system with named clips
- Particle system
- Random generator (seeded)
- Binary save/load system
- Scene transitions
- 2D math utilities

### Media

- Audio playback (music + SFX)
- PNG and JPG image loading
- MPEG‑1 (.mpg/.mpeg) playback + MP4/OGV transcoding entrypoints (via ffmpeg to MPEG-1)

### Interface System

#### ISS (Interactive Style Sheet)

ISS is a CSS-inspired style file for UI layout/theme rules.

Supported now:
- selectors with block syntax
- `property: value;` declarations
- responsive units: `px`, `%`, `vw`, `vh`, `rem`
- positioning properties: `position`, `left`, `top`, `right`, `bottom`, `width`, `height`
- automatic vertical stack layout + alignment: `weight`, `align-x`, `align-y`
- outline/border: `outline-width`, `outline-color`
- gradients: `gradient-top`, `gradient-bottom`
- color properties via hex/name parsing (`#RRGGBB`, `#AARRGGBB`, `white`, `black`, `transparent`)
- background image via `background-image: url(...)`

Example:

```iss
#hud_panel {
  position: center;
  width: 40vw;
  height: 32vh;
}

#play_button {
  left: 6vw;
  bottom: 8vh;
  width: 18rem;
  height: 3rem;
}
```


InteractiveX includes a lightweight GUI system inspired by **CSS‑style customization**.

Components include:

- Buttons
- Labels
- Panels
- Sliders
- Layout system (scaled/responsive helpers)
- ISS style loader (.iss)

All interface components support customizable:

- gradients
- sprites
- borders
- corner radius
- opacity
- font size

---

## Renderer

The renderer is built on **Direct3D 9** and provides the real‑time graphics foundation of the engine:

- sprite rendering
- gradient primitives
- GUI drawing helpers
- world‑space text rendering
- camera transformation
- font caching

---

## Debug Menu

InteractiveX ships with a **built‑in debug menu** used to test engine systems.

The debug environment includes interactive demos for engine modules and visual debugging tools.

The debug menu is **disabled in release builds**.

---

## Dependencies

InteractiveX intentionally keeps dependencies minimal.

### Bundled

**miniaudio**  
Single‑header audio library used for music and sound effects.  
License: MIT / Public Domain

**pl_mpeg**  
Single‑header MPEG‑1 video and audio decoder used by the video player.  
License: MIT

---


### Build options (renderer backend)

Renderer build is now split into independent backend switches, with a profile preset for convenience:

> Platform rules:
> - **Windows**: DirectX 9/10/11/12 + Vulkan + OpenGL 3.1/4.3
> - **Linux**: Vulkan + OpenGL 3.1/4.3 (DirectX profiles are blocked)

- `-DIX_RENDERER_PROFILE=DX9`
- `-DIX_RENDERER_PROFILE=DX10`
- `-DIX_RENDERER_PROFILE=DX11`
- `-DIX_RENDERER_PROFILE=DX12`
- `-DIX_RENDERER_PROFILE=VULKAN`
- `-DIX_RENDERER_PROFILE=OPENGL31`
- `-DIX_RENDERER_PROFILE=OPENGL` (OpenGL 4.3 default)
- `-DIX_RENDERER_PROFILE=ALL` (DX9 + DX10 + DX11 + DX12 + Vulkan + OpenGL 3.1 + OpenGL 4.3)

You can also override individual flags:

- `-DIX_ENABLE_DX9=ON/OFF`
- `-DIX_ENABLE_DX10=ON/OFF`
- `-DIX_ENABLE_DX11=ON/OFF`
- `-DIX_ENABLE_DX12=ON/OFF`
- `-DIX_ENABLE_VULKAN=ON/OFF`
- `-DIX_ENABLE_OPENGL31=ON/OFF`
- `-DIX_ENABLE_OPENGL43=ON/OFF`

Example:

```bash
cmake -S . -B build -DIX_RENDERER_PROFILE=DX11
cmake --build build
```

### Required (External)

**DirectX SDK — June 2010**

Provides Direct3D 9 headers and libraries required by the renderer.

Download:  
https://www.microsoft.com/en-us/download/details.aspx?id=6812

---

## Platform Support

| Component | Support |
|----------|--------|
| OS | Windows / Linux |
| Graphics | Direct3D 9 / DirectX 10 / DirectX 11 / DirectX 12 / Vulkan / OpenGL 3.1 (Legacy) / OpenGL 4.3 |
| Compiler | MinGW-w64 / MSVC |
| Architecture | x86 / x64 |

Compatible with systems ranging from **Windows XP to Windows 11**.

---

## Status

InteractiveX is currently in **early development**.

Current version: **r0.2**

Core systems are implemented and actively tested through the debug environment.

---

## License

InteractiveX is open source.

See **LICENSE** for details.

---

<p align="center">
  Built for developers who want to understand their engine.
</p>


ISS sky example:
```iss
#huhlu_sky {
  sky-top: #223355;
  sky-bottom: #88AADD;
  background-image: url(assets/sky/hdri_sky.jpg);
}
```


Huhlu extensions:
- IXMaterial (PBR texture slots)
- Camera modes: Orbit/FPS/Free/Orthographic/Perspective + FOV/Exposure/Gamma
- Scene format `.ixs` (simple text)
- Simple GLTF/GLB loaders
- Area light + simple shadow factor support

Advanced render features (Huhlu postfx stage): Bloom, SSAO (screen-space approximation), Tonemap and Color Grading toggles.
