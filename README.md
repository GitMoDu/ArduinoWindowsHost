# ArduinoWindowsHost

Lightweight host framework for running Arduino-style `setup()`/`loop()` code on Windows. Provides a `LoopHost` base class and a small thread manager (`HostThreadManager` / `TemplateHostManager`) to run hosts on a background thread while exposing start/stop lifecycle hooks.

## Key pieces

- `LoopHost` — base class for Arduino-like hosts (override `OnStart`, `OnRun`, `OnStop`).
- `HostThreadManager.hpp` / `TemplateHostManager<T>` — manages host lifetime and thread spawning/joining.
- Designed for C++14 and Visual Studio 2022.

## Installation

1. Install the NuGet package (example):
   - Using the Package Manager Console:
     ```
     Install-Package ArduinoWindowsHost
     ```
   - Or use __Manage NuGet Packages__ on your project in Visual Studio.

2. Locate the package files that were copied into your project's packages directory. The NuGet install will place the class/header files under your project's package root (for example: `packages/ArduinoWindowsHost.<version>/content` or `packages/ArduinoWindowsHost.<version>/include`). Note the exact relative path from your `.vcxproj`.

3. Add that relative path to your project include directories so the compiler can find the headers:
   - Preferred: Open __Project Properties__ -> __Configuration Properties__ -> __C/C++__ -> __Additional Include Directories__ and add the relative path (for example `..\packages\ArduinoWindowsHost.1.0.0\include` or `$(ProjectDir)packages\ArduinoWindowsHost.1.0.0\include`).
   - Alternatively you can set it under __Project Properties__ -> __Configuration Properties__ -> __VC++ Directories__ -> __Include Directories__.

4. Example path (adjust version and exact subfolder as needed):
   - If headers were copied to `packages\ArduinoWindowsHost.1.0.0\include` relative to the project:
     - Add `..\packages\ArduinoWindowsHost.1.0.0\include` to __Additional Include Directories__.

Tips:
- If you want a stable reference without hard-coding the version, define a user macro (Project Properties -> __Configuration Properties__ -> __User Macros__) such as `ArduinoWindowsHostDir` and set it to the package folder, then use `$(ArduinoWindowsHostDir)\include` in __Additional Include Directories__.
- If the package uses a different layout, point your include directory to whichever folder contains the `.hpp`/`.h` files.


## NuGet packaging

- The repository includes a packaging helper script `NugetPack.cmd` at the project root.

- The script runs the exact NuGet pack command below to create the package and place output in `.\artifacts`:

- To create the package from the project root:
  1. Open a shell in the project root (for example the __Developer Command Prompt for VS 2022__ or PowerShell).
  2. Run the packaging script:
     - PowerShell:
       ```
       .\NugetPack.cmd
       ```
     - CMD:
       ```
       NugetPack.cmd
       ```
  3. The script executes the command shown above and produces a `.nupkg` file under `.\artifacts`.

Notes:
- The script requires `nuget` to be available for the command to run (for example `nuget.exe` on __PATH__ or otherwise accessible). If you prefer to run NuGet directly instead of the script, run the exact `nuget pack` command shown above from the project root.
