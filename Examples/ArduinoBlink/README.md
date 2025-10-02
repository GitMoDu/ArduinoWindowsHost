# ArduinoBlink (C++/WinRT + Arduino Host Emulation)

A Windows (UWP / C++/WinRT) sample that hosts an Arduino-style sketch on the desktop using the `ArduinoWindowsHost` framework.

It provides:

- An emulated loop / setup / serialEvent environment (`LoopHost` subclass)
- A visual representation of an Arduino Uno board with live LED indicators (Power, Built-In, TX, RX)
- An integrated serial monitor (input + output, auto-scroll, newline handling)
- Start/Stop and Reset controls to manage the hosted sketch lifecycle

---

## Features

- Implements classic Arduino `setup()`, `loop()`, and `serialEvent()` in `ArduinoHost`
- Blinks the built-in LED and prints alternating Tick/Tock messages
- Simulates serial RX/TX activity LEDs
- Manual serial input with optional auto newline and clear-on-send
- Reset recreates the host and re-runs `setup()`

---

## Project Structure

- **Application bootstrap**: `MainPage.xaml/.cpp` (C++/WinRT)
- **UI**: `MainPage` with board rendering, serial console, and control logic
- **Core Arduino logic**: `Host/ArduinoHost.hpp`

---

## UI Controls

| Control                  | Purpose                                      |
|---------------------------|----------------------------------------------|
| Start / Stop              | Starts or stops the emulated host loop       |
| Reset (red circle)        | Recreates the host (runs `setup()` again)    |
| Serial input + Send       | Sends text to `Serial` (optionally newline)  |
| Clear on send             | Clears input after transmit                  |
| Newline                   | Appends `\n` or sends newline when empty     |
| Auto Scroll               | Keeps output scrolled to bottom              |
| Delete Button             | Clears output (or flushes while running)     |

**LED Indicators**

- Built-In: Mirrors `LED_BUILTIN` state
- TX / RX: Brief flash on serial transmit / receive
- Power: Host active

---

## Build Prerequisites

- Visual Studio 2022
- C++/WinRT workload (Individual components > C++ (v143) / Windows 10+ SDK)
- Windows 10/11 SDK (matching your target)
- NuGet restore (if `ArduinoWindowsHost` is delivered via package)

**Steps:**

1. Open the solution
2. Build (x86/x64/ARM as needed)
3. Deploy
4. Press **Start**

---

## Extending the Sketch

Add more behavior inside `Host/ArduinoHost.hpp`:

- Additional pins: call `pinMode(pin, OUTPUT)` then `digitalWrite`
- Serial protocol: parse input in `serialEvent()` instead of discarding
- Timing: avoid long `delay()` calls for responsiveness; prefer `millis()` patterns

Example: echo received characters.

---

## Troubleshooting

- **No output**: Ensure you pressed Start
- **LEDs not updating**: Rendering hook attaches only when running
- **Serial input disabled**: Host must be running
- **Build errors about namespaces**:
  - Confirm `ArduinoWindowsHost` is referenced correctly
  - Confirm C++/WinRT VS extension is installed

---

## Modifying the UI

- Update `MainPage.xaml` for layout changes
- LED visibility is toggled each frame in `onRendering(...)`
- Serial auto-scroll handled by `SerialTxAdapter` (see usage in `updateArduinoHostState`)

---

## Adapting for Your Own Sketch

1. Replace logic inside `ArduinoHost::setup()` and `loop()`
2. Keep `Serial.begin(...)`
3. Add any state fields you need
4. Rebuild (no app code changes required unless you add new UI)

---

## License

MIT

---

## Attribution

Uses a board image in `Assets/ArduinoUno.png`  
Creative Commons license: https://commons.wikimedia.org/wiki/File:Arduino_Logo.svg
