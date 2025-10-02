# DemoSceneWinRT

A C++/WinRT sample that demonstrates the **IntegerWorld** Arduino library running inside the `ArduinoWindowsHost` framework.  
The goal of this example is to show how an Arduino-style sketch can drive the same **3D demo scene** as real MCUs, hosted in a Windows UWP/XAML app with full Visual Studio tooling.

---

## Goals

- Use the [IntegerWorld](https://github.com/GitMoDu/IntegerWorld) library to render the same **3D scene** as IntegerWorld example.
- Demonstrate how Arduino sketches can be hosted on Windows while preserving `setup()`, `loop()`, and `serialEvent()` semantics.
- Provide a desktop environment for experimenting with **embedded-style rendering** and physics before deploying to constrained hardware.
- Integrate **VirtualPad** input for interactive control of the scene.
- Show how cooperative scheduling (via TaskScheduler) can drive rendering and input tasks in a predictable way.

---

## Dependencies

This demo builds on several Arduino libraries:

- **[IntegerWorld](https://github.com/GitMoDu/IntegerWorld)**  
  A C++11 embedded graphics framework for efficient 2D/3D rendering using integer math.  
  - Integer-only math for speed and low memory use  
  - Painter’s algorithm with fragment sorting  
  - Lighting and shading (emissive, diffuse, specular, metallic)  
  - Point, directional, and spotlight sources  
  - Triangle meshes, edge lines, and point clouds  

- **[VirtualPad](https://github.com/GitMoDu/VirtualPad)**  
  An abstracted controller API inspired by RetroArch’s RetroPad.  
  - Provides a consistent input model for buttons, joysticks, and sliders  
  - Supports mapping from analog or digital sources  
  - Designed for low RAM and fast state updates  
  - Integrates with TaskScheduler for periodic polling and dispatch  

- **[TaskScheduler](https://github.com/arkhipenko/TaskScheduler)** (dependency of both IntegerWorld and VirtualPad)  
  A cooperative multitasking library for Arduino and embedded systems.  
  - Periodic task execution with millisecond or microsecond resolution  
  - Event-driven task invocation  
  - Lightweight alternative to preemptive RTOS frameworks  
  - Used here to schedule rendering, input polling, and serial events  

---

## Project Structure

- `App.*` - Application bootstrap (C++/WinRT entry point).
- `MainPage.*` - XAML page hosting the scene and controls.
- `Host/ArduinoHost` - Arduino main host.
- `Scene/` - IntegerWorld demo scene and assets.