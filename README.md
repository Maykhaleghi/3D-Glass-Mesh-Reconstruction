# 3D Glass Mesh Reconstruction

A computer graphics project based on the **Meshes** programming project from [The Graphics Codex](https://graphicscodex.com/projects/meshes/index.html). The project focuses on reconstructing a 3D drinking glass as a procedural mesh and exporting it in the OFF format.

The glass reconstruction, reference image, and general surface-of-revolution approach are based on the Graphics Codex exercise. The main extension developed in this project is the use of **non-uniform Catmull–Rom interpolation** to generate additional points along the glass profile before constructing the final mesh. This produces a denser and smoother version of the original glass.

## Project Structure

~~~text
codex-meshes/
├── src/
│   └── mesh_generator.cpp
│
├── models/
│   ├── glass-reconstructed.off
│   └── glass-smoothed.off
│
├── refrences/
│   └── glass-refrence.png
│
├── results/
│   ├── Initial glass.png
│   └── Smoothed glass.png
│
└── exercises/
    ├── cube/
    │   ├── cube.cpp
    │   └── cube.off
    │
    └── cylinder/
        └── cylinder.off
~~~

## Contents

### `src/`

- **`mesh_generator.cpp`** — Main C++ implementation for generating the smoothed glass mesh. It defines the original outer and inner glass profiles, interpolates them using non-uniform Catmull–Rom interpolation, and generates the final 3D surface.

### `models/`

- **`glass-reconstructed.off`** — Initial reconstructed glass mesh.
- **`glass-smoothed.off`** — Refined glass mesh generated from the interpolated profiles.

The smoothed model is generated using **128 angular slices** and **500 height samples**, resulting in a significantly denser mesh than the original reconstruction.

### `refrences/`

- **`glass-refrence.png`** — Reference image used for the glass reconstruction. This reference and the glass reconstruction exercise originate from the Graphics Codex Meshes project.

### `results/`

- **`Initial glass.png`** — Visualization of the initial reconstructed glass.
- **`Smoothed glass.png`** — Visualization of the final smoothed glass.

These images show the difference between the original profile-based mesh and the refined version.

### `exercises/`

Contains the supporting mesh exercises from the Graphics Codex project:

- **`exercises/cube/`** — Cube mesh exercise, including its C++ implementation and OFF model.
- **`exercises/cylinder/`** — Cylinder mesh exercise and OFF model.

These exercises provided practice with the mesh representation and procedural geometry used before working on the glass.

## Smoothing Extension

The original glass profile is represented by a limited number of points. In this project, **non-uniform Catmull–Rom interpolation** is applied to the outer and inner profiles to generate additional points between the original profile points.

The interpolated profiles are then revolved around the vertical axis to create the final glass mesh. The result is a smoother and substantially denser representation of the same reconstructed glass.

## Reference

This project is based on the **Meshes** programming project from *The Graphics Codex*:

[Graphics Codex — Meshes Project](https://graphicscodex.com/projects/meshes/index.html)

The Graphics Codex project provides the original drinking-glass exercise, reference material, and solid-of-revolution approach. The **Catmull–Rom interpolation and profile-smoothing extension were developed independently for this project**.

**Bachelor's Project — Computer Graphics**
