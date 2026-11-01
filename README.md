# Bicicleta-Estatica-Terapeutica

Firmware for the **ESP32-S3** therapeutic stationary bicycle controller (BLE HID Gamepad + Steering & Resistance Control), built with **ESP-IDF v5.3.2** using a containerized Docker CI/CD workflow.

---

## Project Structure

All build configuration, Docker files, and generated outputs are isolated inside `build/` to keep the repository root clean. Every module under `components/` and `apps/` follows the standard `include/` + `src/` structure:

```text
Bicicleta-Estatica-Terapeutica/
├── .github/workflows/ci.yml   # GitHub Actions CI pipeline
├── apps/
│   ├── comm_manager/          # Host communication & BLE HID report dispatcher
│   │   ├── include/
│   │   ├── src/
│   │   └── CMakeLists.txt
│   ├── data_manager/          # Sensor data aggregation, filtering & bike state mailbox
│   │   ├── include/
│   │   ├── src/
│   │   └── CMakeLists.txt
│   └── steering_wheel/        # Steering angle sampling & filtering app
│       ├── include/
│       ├── src/
│       └── CMakeLists.txt
├── components/
│   ├── adc/                   # ADC oneshot + calibration HAL wrapper
│   ├── bluetooth/             # Pure NimBLE HID Gamepad (GAP/GATT) middleware
│   └── ledc/                  # LEDC PWM HAL wrapper
├── main/
│   ├── main.c                 # Application entry point (app_main)
│   └── CMakeLists.txt
├── build/
│   ├── Dockerfile             # ESP-IDF v5.3.2 build image definition
│   ├── docker-compose.yml     # Local containerized build service
│   ├── sdkconfig.defaults     # Version-controlled ESP32-S3 SDK defaults
│   ├── .cache/                # Persistent ccache & component cache (gitignored)
│   └── out/                   # Compiled binaries, ELF & flash_args (gitignored)
├── CMakeLists.txt
└── Makefile                   # Unified build, flash, monitor & CI commands
```

---

## Prerequisites

### 1. Build Environment (Docker)
You do **not** need to install the ESP-IDF SDK locally to compile the project.
- **Docker** & **Docker Compose** (`docker compose`)
  - *Windows + WSL2:* Install [Docker Desktop for Windows](https://docs.docker.com/desktop/wsl/) and enable **WSL 2 integration** under *Settings → Resources → WSL Integration*, or install `docker-ce` directly inside WSL2.
- **GNU Make:**
  ```bash
  sudo apt update && sudo apt install -y make
  ```

### 2. Flashing & Serial Monitor Tools
- **On Windows + WSL2 (No `usbipd` required):**
  Because WSL2 can execute Windows `.exe` binaries directly (which have native access to Windows `COM` ports), the `Makefile` uses a Windows Python environment with `esptool` and `esp-idf-monitor` to flash and monitor over Windows `COMx` ports directly from your WSL terminal.
  - If you installed ESP-IDF on Windows via the Espressif installer, the `Makefile` uses `~/.espressif/python_env/.../python.exe` automatically.
  - If your Windows Python path differs, override `WIN_PYTHON`:
    ```bash
    make flash WIN_PYTHON="/mnt/c/Users/<you>/.espressif/python_env/idf5.5_py3.11_env/Scripts/python.exe"
    ```
- **On Native Linux:**
  Pass a Linux serial device (`PORT=/dev/ttyACM0` or `PORT=/dev/ttyUSB0`) and the `Makefile` will flash and monitor directly through the Docker container.

---

## Daily Workflow (`Makefile`)

### Build Firmware
Compiles the project inside the `espressif/idf:v5.3.2` container and writes artifacts (`biciver.bin`, `biciver.elf`, `flash_args`, `compile_commands.json`) to `build/out/`:
```bash
make build
```

### Flash Firmware
- **Auto-detect ESP32-S3 on Windows `COM` ports and flash:**
  ```bash
  make flash
  ```
- **Flash to a specific port (Windows `COMx` or Linux `/dev/tty*`):**
  ```bash
  make flash PORT=COM4
  # or on native Linux:
  make flash PORT=/dev/ttyACM0
  ```
- **Build in Docker and immediately flash in one step:**
  ```bash
  make build FLASH=1
  # or with an explicit port:
  make build PORT=COM4
  ```

### Serial Monitor (`esp_idf_monitor`)
Opens the ESP-IDF serial monitor attached to `build/out/biciver.elf` (automatically decodes crash backtraces and soft-resets the ESP32-S3 CPU core on connect so startup logs are never missed):
```bash
make monitor
# or specify a port explicitly:
make monitor PORT=COM4
```
- **Exit monitor:** `Ctrl+]`
- **Reboot ESP32-S3 CPU inside monitor (without dropping native USB):** `Ctrl+T` then `Ctrl+R`

> **Note on ESP32-S3 Native USB (`USB Serial/JTAG`):**
> When connected via the ESP32-S3's built-in USB port (`VID_303A&PID_1001`), pressing the physical `RST` button on the board resets the internal USB peripheral and temporarily disconnects the `COM` port from the host OS, causing generic serial monitors (PuTTY, Arduino Serial Monitor, etc.) to miss the first ~100 ms of boot logs. Always prefer `make monitor` (or toggle `RTS` from your terminal) to reset only the CPU core without dropping the USB connection.

### Other Useful Targets
| Command | Description |
| :--- | :--- |
| `make ci` | Runs the full clean CI pipeline (`fullclean` → regenerate `sdkconfig` from `build/sdkconfig.defaults` → `build` → `size-components`). |
| `make size` | Prints the RAM and Flash footprint breakdown per component. |
| `make menuconfig` | Opens the interactive ESP-IDF configuration menu in Docker. |
| `make clean` | Cleans build output in `build/out/`. |
| `make fullclean` | Deletes the entire `build/out/` directory. |
| `make shell` | Opens an interactive `bash` shell inside the ESP-IDF Docker container. |

---

## Adding or Modifying Components

Each component lives under `components/<name>/` (or `apps/<name>/`) with its own `include/` and `src/` folders:

```cmake
idf_component_register(
    SRCS "src/my_component.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_driver_gpio log
)
```

> **Tip:** ESP-IDF v5.x requires explicit component dependencies. If your public header (`include/*.h`) includes an ESP-IDF header (e.g., `driver/gpio.h`), add the corresponding ESP-IDF component (`esp_driver_gpio`) to `REQUIRES`. If only your `.c` file needs it, add it to `PRIV_REQUIRES`.
