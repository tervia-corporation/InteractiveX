# InteractiveX

<p align="center">
  <img src="https://img.shields.io/badge/version-r0.2-blue" />
  <img src="https://img.shields.io/badge/platform-Windows-lightgrey" />
  <img src="https://img.shields.io/badge/renderer-Direct3D9-purple" />
  <img src="https://img.shields.io/badge/language-C%20%2F%20C%2B%2B-orange" />
  <img src="https://img.shields.io/badge/license-open--source-green" />
  <img src="https://img.shields.io/badge/audio-miniaudio-blue" />
  <img src="https://img.shields.io/badge/video-pl__mpeg-blue" />
  <img src="https://img.shields.io/badge/graphics-Direct3D9-purple" />
</p>

<p align="center">
  Lightweight • Transparent • Full-source 2D Engine
</p>

---

## About

**InteractiveX** is a lightweight **2D game engine for Windows** built on top of **Direct3D 9**, written entirely in **C/C++**.

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

Current version **r0.2** includes a complete set of core systems for building 2D games.

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
- PNG / BMP image loading
- MPEG-1 video playback with audio sync

### Interface System

InteractiveX includes a lightweight GUI system inspired by **CSS-style customization**.

Components include:

- Buttons
- Labels
- Panels
- Sliders
- Layout system

All interface components support customizable:

- gradients
- sprites
- borders
- corner radius
- opacity
- font size

---

## Renderer

The renderer is built on **Direct3D 9** and provides:

- sprite rendering
- gradient primitives
- GUI drawing helpers
- world-space text rendering
- camera transformation
- font caching

---

## Debug Menu

InteractiveX ships with a **built-in debug menu** used to test engine systems.

The debug environment includes interactive demos for engine modules and visual debugging tools.

The debug menu is **disabled in release builds**.

---

## Dependencies

InteractiveX intentionally keeps dependencies minimal.

### Bundled

**miniaudio**

Single-header audio library used for music and sound effects.

License: MIT / Public Domain


**pl_mpeg**

Single-header MPEG-1 video and audio decoder used by the video player.

License: MIT

---

### Required (External)

**DirectX SDK — June 2010**

Provides Direct3D 9 headers and libraries required by the renderer.

Download:  
https://www.microsoft.com/en-us/download/details.aspx?id=6812

---

## Platform Support

| Component | Support |
|----------|--------|
| OS | Windows |
| Graphics | Direct3D 9 |
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