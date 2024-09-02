
# File structure

* `src` - The source code of production binary.
  * `hw` - STM32 Cube project containing HW interface.
    It produces a final production binary.
  * `control` - The main controller source code.
* `simu` - The sources to simulate the system.
  * `app` - The simulation web application.
  * `model` - The mathematical model of the system that is controlled by this software.
    It is used to simulate the the system.
  * `wasm` - The WebAssembly interface for fully software simulation.
* `web` - The web GUI interface.
* `scripts` - The utility scripts.
