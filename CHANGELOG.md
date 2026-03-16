# Changelog

All notable changes to **InteractiveX** are documented in this file.

The format loosely follows **Keep a Changelog** principles.

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
