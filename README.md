# Interactive 3D Urban Scene with Dynamic Lighting & Photorealistic Rendering

A real-time 3D interactive urban environment simulation developed in C++ and OpenGL 3.3 (Programmable Pipeline). The project features advanced real-time rendering techniques including shadow mapping with PCF anti-aliasing, Blinn-Phong multi-light shading, dynamic fog, custom vehicle physics, and OBB (Oriented Bounding Box) collision detection.

## 🌟 Key Features

* **Advanced Lighting Engine (Blinn-Phong):** Supports a global directional light source (Sun) alongside 4 dynamic spotlight sources (vehicle headlights and alternating police strobe lights).
* **High-Resolution Shadow Mapping:** Implemented two-pass depth rendering with a 5x5 Percentage Closer Filtering (PCF) matrix to eliminate shadow aliasing and produce soft edges.
* **Oriented Bounding Box (OBB) Collisions:** Precise 3D collision detection algorithm between movable vehicles and rotated city buildings using local coordinate transformation.
* **NFS-Style Third-Person Camera:** Interpolated smooth camera movement using linear transformation (`glm::mix`) that fluidly tracks the active vehicle's forward vector.
* **Modular Vehicle Mechanics:** Vehicles are decomposed into independent sub-objects (chassis and 4 individual wheels) allowing realistic wheel rotation synced with speed.
* **Atmospheric Effects:** Exponential distance-based fog rendering (`mix(fogColor, finalColor, fogFactor)`) and interactive day/night sun cycle controls.
* **Spatial Audio:** Integrated 3D sound effects and vehicle-specific audio profiles using the irrKlang library.

## 🛠️ Tech Stack & Dependencies

* **Language:** C++17
* **Graphics API:** OpenGL 3.3 (Core Profile)
* **Windowing & Input:** GLFW
* **Math Library:** GLM (OpenGL Mathematics)
* **3D Model Loader:** `tiny_obj_loader`
* **Texture Loader:** `stb_image`
* **Audio Engine:** irrKlang 3D Audio Library

## 🎮 Controls & Manual

| Key | Action | Description |
| :--- | :--- | :--- |
| **W, A, S, D** | Movement | Drive selected vehicle / Move camera (Free Cam mode) |
| **Mouse** | Orientation | Rotate camera view |
| **1** | Camera Mode | Toggle Third-Person (NFS-style) camera |
| **2, 3, 4** | Select Vehicle | **2:** BMW M3 \| **3:** Porsche 911 \| **4:** Police Cruiser |
| **5, 6, 7** | Render Modes | **5:** Solid \| **6:** Wireframe \| **7:** Polygonal |
| **8, 9** | Fog Control | **8:** Decrease fog density \| **9:** Increase fog density |
| **Arrow Keys** | Sun Position | Adjust global directional light vector (real-time shadows) |
| **M / N** | Mouse Cursor | **M:** Enable cursor \| **N:** Disable cursor |

## 🚀 Building & Running

1. Clone the repository:
   ```bash
   git clone [https://github.com/your-username/3d-urban-scene-opengl.git](https://github.com/your-username/3d-urban-scene-opengl.git)
