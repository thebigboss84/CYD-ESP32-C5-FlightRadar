# CYD ESP32-C5 FlightRadar & Aerospace Dashboard

An advanced, real-time aerospace radar and multi-instrument avionics dashboard built specifically for the **RockBase NM-CYD-C5** (ESP32-C5 dual-band Wi-Fi 6, 16MB Flash, 8MB PSRAM, 320x240 ST7789 display, XPT2046 touch).

Inspired by [Flight-CYD-ESP32-Radar](https://github.com/Coreymillia/Flight-CYD-ESP32-Radar) and [AirESP32ace](https://github.com/vmalis/AirESP32ace), this project brings tactical airspace visualization, live aircraft tracking, satellite telemetry, and space mission monitoring together into an interactive, standalone desktop instrument.

---

## Features

### 1. Tactical Flight Radar
* **Live ADS-B Airspace Tracking**: Fetches real-time flight vectors via the OpenSky Network within a customizable radius (e.g. 50 km - 150 km) around your home city.
* **Modern Avionics Scope**: Pitch-black tactical scope, circular bezel with 45-degree radial ticks, luminous cardinal headings (`N` in cyan, `S`/`E`/`W` in teal), and home bullseye.
* **Aircraft Chevrons & True Heading**: Renders aircraft as precision mini-silhouettes with illuminated cores, swept wings, and 7-pixel velocity vectors indicating actual flight track. Ground targets appear as distinct blips.
* **Altitude Color Coding**:
  * **> 20,000 ft** (High Altitude / Cruise) - Amber / Yellow
  * **10,000 - 20,000 ft** (Mid Altitude) - Green
  * **< 10,000 ft** (Approach / Departure) - Cyan
  * **Ground** (Taxi / Parked) - Slate / White
* **Smart De-Cluttered Callouts**: Uncluttered view limiting callout tags to the **top 3 closest airborne aircraft** using dark anti-glare badges connected by leader lines.
* **Corner HUD Avionics**: Displays live range, target count, nearest target distance/callsign, and altitude legend at a glance.

### 2. Flight Directory & Route Inspector
* **Scrollable Flight Roster**: View all aircraft in range sorted by proximity, displaying Callsign, ICAO hex, Altitude, Ground Speed, Distance, and Bearing.
* **Interactive Touch Selection**: Tap any aircraft in the list to trigger automatic origin and destination airport route lookups (e.g. `LAX -> JFK`).
* **Route Detail Card**: Detailed aircraft model, carrier, elevation, and climb/descent rate.

### 3. Live Local Weather & 4-Day Forecast
* Powered by Open-Meteo (no API key required).
* Current temperature, "feels like" temperature, relative humidity, wind speed, and direction compass.
* 4-day extended outlook with high/low forecast cards and weather condition glyphs.

### 4. ISS Orbit Tracker
* Live International Space Station tracking updated in real time via `wheretheiss.at`.
* Displays latitude, longitude, orbital altitude (~420 km), velocity (~27,600 km/h), footprint distance, and compass heading from your home location.
* Orbital daylight / eclipse visibility status.

### 5. SpaceX Rocket Launch Telemetry
* Live countdown timer and mission telemetry for upcoming SpaceX launches (Falcon 9, Falcon Heavy, Starship).
* Real-time seconds countdown, rocket type, launch pad location, and mission description via Launch Library 2.

### 6. On-Device Captive AP Configuration Portal
* **Zero Hardcoded Secrets**: At initial boot (or by tapping `[SET]` in the header bar), the device broadcasts an access point named `CYD-AirRadar-Setup`.
* Connect via phone or PC to configure:
  * Wi-Fi SSID and Password
  * Custom City Name (e.g., Rancho Cucamonga)
  * Custom Latitude & Longitude coordinates
  * Radar detection radius (km)
* Settings are permanently saved to ESP32 non-volatile storage (NVS) across reboots.

### 7. 12-Hour Local Clock with Automatic DST
* Automatically synchronizes local time via NTP (`pool.ntp.org`) with POSIX timezones and automatic Daylight Saving Time handling for your selected location.

---

## Hardware: RockBase NM-CYD-C5

The **RockBase NM-CYD-C5** is an ESP32-C5 development board featuring a RISC-V 32-bit single-core processor, dual-band Wi-Fi 6 (2.4 GHz and 5 GHz), 16MB Flash, 8MB PSRAM, a 320x240 ST7789 TFT display, and XPT2046 resistive touch.

### Pinout Configuration

| Function | Pin | Notes |
| :--- | :--- | :--- |
| **TFT CS** | `GPIO 3` | Display SPI Chip Select |
| **TFT DC / RS** | `GPIO 4` | Display Data/Command |
| **TFT RST** | `GPIO 5` | Display Reset |
| **TFT Backlight** | `GPIO 8` | Active HIGH (PWM capable) |
| **SPI MOSI** | `GPIO 7` | Shared between Display & Touch |
| **SPI SCK** | `GPIO 6` | Shared between Display & Touch |
| **SPI MISO** | `GPIO 2` | Shared with Touch |
| **Touch CS** | `GPIO 1` | XPT2046 Chip Select |
| **Touch IRQ** | `GPIO -1` | Polled |
| **Status LED** | `GPIO 27` | On-board LED |
| **Boot Button** | `GPIO 28` | ESP32-C5 Strapping / Boot pin |

---

## Installation & Build

### Prerequisites
* [PlatformIO IDE](https://platformio.org/) (VS Code extension or CLI)
* ESP-IDF / Arduino-ESP32 with ESP32-C5 support

### Build & Flash via PlatformIO

```bash
# Clone the repository
git clone https://github.com/thebigboss84/CYD-ESP32-C5-FlightRadar.git
cd CYD-ESP32-C5-FlightRadar

# Build firmware
pio run

# Flash to device (replace COM6 with your port)
pio run -t upload --upload-port COM6
```

> **IMPORTANT (ESP32-C5 Bootloader Offset)**:
> The ESP32-C5 ROM bootloader expects the secondary bootloader at offset **`0x2000`** (unlike legacy ESP32 at `0x1000` or `0x0000`). If flashing manually via `esptool.py`, use:
> ```bash
> python -m esptool --chip esp32c5 --port COM6 --baud 460800 write_flash 0x2000 .pio/build/cyd_c5/bootloader.bin 0x8000 .pio/build/cyd_c5/partitions.bin 0xe000 boot_app0.bin 0x10000 .pio/build/cyd_c5/firmware.bin
> ```

---

## APIs Used

All external APIs used in this firmware are completely free and require no personal accounts or API keys:

1. **[OpenSky Network](https://opensky-network.org/)**: Live ADS-B state vectors.
2. **[FlightAware / OpenSky Flight Routes](https://hexdb.io/)**: Aircraft route and carrier metadata.
3. **[Open-Meteo](https://open-meteo.com/)**: Global weather forecasts and meteorological observations.
4. **[WhereTheISS.at](https://wheretheiss.at/)**: Real-time ISS orbital coordinates.
5. **[The Space Devs / Launch Library 2](https://thespacedevs.com/llapi)**: Upcoming rocket launches and orbital schedules.

---

## License & Credits

Distributed under the MIT License.

Special thanks to:
* [Coreymillia/Flight-CYD-ESP32-Radar](https://github.com/Coreymillia/Flight-CYD-ESP32-Radar)
* [vmalis/AirESP32ace](https://github.com/vmalis/AirESP32ace)
