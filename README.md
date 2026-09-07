# CYD ESP32-C5 FlightRadar & Aerospace Dashboard

An advanced, real-time aerospace radar and multi-instrument avionics dashboard built specifically for the **RockBase NM-CYD-C5** (ESP32-C5 dual-band Wi-Fi 6, 16MB Flash, 8MB PSRAM, 320x240 ST7789 display, XPT2046 touch, onboard WS2812 RGB LED on GPIO 27, and GY-GPS6MV2 GPS module).

Inspired by [Flight-CYD-ESP32-Radar](https://github.com/Coreymillia/Flight-CYD-ESP32-Radar) and [AirESP32ace](https://github.com/vmalis/AirESP32ace), this project brings tactical airspace visualization, live aircraft tracking, satellite telemetry, and space mission monitoring together into an interactive, standalone desktop instrument.

---

## Features

### 1. Tactical Flight Radar
* **Live ADS-B Airspace Tracking**: Fetches real-time flight vectors via the OpenSky Network within a customizable radius (e.g. 25 km - 150 km) around your coordinates.
* **Interactive Touch Radar Zoom**: Tap the top-left **`RNG`** card to instantly cycle through tactical zoom ranges:
  * **25 km**: Airport terminal approach & final glide path
  * **50 km**: Metro regional airspace
  * **100 km**: Extended regional airspace
  * **150 km**: Enroute high-altitude cruise corridor
* **Tactical Traffic Filters**: Tap the **`FLT`** card to toggle traffic filters:
  * **ALL**: Complete commercial and civil airspace
  * **MILITARY**: Heavy transports (C-17, C-130, KC-135, KC-46), fighters, and USAF/USN/DoD callsigns
  * **POLICE / HELO**: Local law enforcement (LAPD, LASD, CHP), medical helicopters, CalFire, and rotorcraft
* **Aviation Emergency "Squawk 7700" Interceptor**:
  * Automatically detects emergency transponder squawk codes:
    * `7700`: General In-Flight Emergency / Mayday
    * `7600`: Lost Radio Communications
    * `7500`: Unlawful Interference / Hijacking
  * Renders a high-priority red alert banner and reticle targeting the distressed aircraft.
  * Triggers an intense 10 Hz red emergency strobe on the onboard WS2812 LED beacon.
* **Modern Avionics Scope**: Pitch-black tactical scope, circular bezel with 45-degree radial ticks, luminous cardinal headings (`N` in cyan, `S`/`E`/`W` in teal), and home GPS bullseye.
* **Aircraft Chevrons & True Heading**: Renders aircraft as precision mini-silhouettes with illuminated cores, swept wings, and velocity vectors indicating actual flight track. Helicopters appear with tactical crosshair markers.
* **Altitude Color Coding**:
  * **> 20,000 ft** (High Altitude / Cruise) - Cyan
  * **10,000 - 20,000 ft** (Mid Altitude) - Emerald Green
  * **< 10,000 ft** (Approach / Departure) - Amber Yellow
  * **Ground** (Taxi / Parked) - Slate Gray
* **Smart De-Cluttered Callouts**: Limits callout tags to the **top 3 closest airborne aircraft** using anti-glare badges connected by leader lines.

### 2. WS2812 RGB LED Aerospace Beacon (GPIO 27)
Multi-state priority-driven hardware indicator using the onboard WS2812 RGB LED:
* **Priority 1 (Red Strobe @ 10 Hz)**: Active in-flight emergency squawk (`7700`, `7600`, `7500`).
* **Priority 2 (Rapid Amber Strobe @ 4 Hz)**: Active SpaceX liftoff visible in sky (T-0 to T+10m).
* **Priority 3 (Pulsing Amber Breath)**: SpaceX launch countdown (T-15m to T-0).
* **Priority 4 (Interstellar Purple Breath)**: ISS visible overhead pass (< 800 km in daylight).
* **Priority 5 (Double White Wingtip Strobe)**: Aircraft passing directly overhead (< 6 km, 2.2s period).
* **Standby**: Ambient dim tactical breathing glow.

### 3. Flight Directory & Route Inspector
* **Scrollable Flight Roster**: View all aircraft in range sorted by proximity, displaying Callsign, ICAO hex, Altitude, Ground Speed, Distance, Bearing, and transponder squawk.
* **Automatic Route Discovery**: Queries flight databases in the background to show departure and arrival airports (e.g., `Denver (DEN) -> Ontario (ONT)`).
* **Interactive Touch Selection**: Tap any flight in the list to reveal full route cards and climb/descent vertical speed rates.

### 4. Live Local Weather & 4-Day Forecast
* Powered by Open-Meteo (100% free, no API key required).
* Current temperature, "feels like" temperature, relative humidity, wind speed, and direction compass.
* 4-day extended outlook with high/low forecast cards and weather condition glyphs.

### 5. ISS Orbit Tracker & "Look Up Now!" Guide
* Real-time orbital coordinates updated via `wheretheiss.at`.
* Displays latitude, longitude, altitude (~420 km), velocity (~27,600 km/h), and distance.
* **Live Horizon Elevation & Sightline**: Calculates real-time elevation angle above the horizon. Displays a prominent **"LOOK UP! ISS VISIBLE AT XXX deg (Elev +XX deg)"** banner whenever the station is above the horizon in sunlight.

### 6. SpaceX Launch Telemetry & Sky Sightlines
* Live countdown timer and mission telemetry for upcoming SpaceX launches (Falcon 9, Falcon Heavy, Starship) via Launch Library 2.
* **Visible Sky Tracker**: Computes distance, compass bearing, and line-of-sight from user coordinates to Vandenberg SFB (SLC-4E) and Cape Canaveral.
* Prominently flags **"LOOK OUTSIDE NOW! 281 km WNW - VISIBLE IN SKY!"** when a launch occurs within visual range.

### 7. Hardware GPS Module Integration
* Connect an external **GY-GPS6MV2 (u-blox NEO-6M)** to the NM-CYD-C5 LP-UART header:
  * **GPS TX -> ESP32-C5 GPIO 4 (RX)**
  * **GPS RX -> ESP32-C5 GPIO 5 (TX)**
  * **GPS VCC -> 3.3V / 5V**
  * **GPS GND -> GND**
* Seamlessly detects 3D satellite locks and automatically synchronizes radar and weather coordinates dynamically as you move.

### 8. On-Device Captive AP Configuration Portal
* **Zero Hardcoded Secrets**: At initial boot (or by tapping `[SET]` in the header bar), the device broadcasts an access point named `CYD-AirRadar-Setup`.
* Connect via phone or PC to configure Wi-Fi credentials, custom city name, coordinates, and default radar range. Saved permanently to ESP32 NVS flash.

---

## Hardware: RockBase NM-CYD-C5 Pinout

| Function | Pin | Notes |
| :--- | :--- | :--- |
| **TFT CS** | `GPIO 3` | Display SPI Chip Select |
| **TFT DC / RS** | `GPIO 4` | Display Data/Command (TFT) |
| **TFT RST** | `GPIO 5` | Display Reset (TFT) |
| **TFT Backlight** | `GPIO 8` | Active HIGH (PWM capable) |
| **SPI MOSI** | `GPIO 7` | Shared between Display & Touch |
| **SPI SCK** | `GPIO 6` | Shared between Display & Touch |
| **SPI MISO** | `GPIO 2` | Shared with Touch |
| **Touch CS** | `GPIO 1` | XPT2046 Chip Select |
| **GPS UART RX** | `GPIO 4` | P5 Header Pin 1 |
| **GPS UART TX** | `GPIO 5` | P5 Header Pin 2 |
| **RGB WS2812** | `GPIO 27` | Built-in RGB status beacon |
| **Boot Button** | `GPIO 28` | ESP32-C5 Strapping / Boot pin |

---


### 9. Interactive City & GPS Airspace Selector
* **Dedicated `CITY` View**: Tap the **`CITY`** tab in the bottom footer navigation to enter the interactive airspace selector.
* **Live GPS Diagnostics**: Displays real-time satellite telemetry from the connected GY-GPS6MV2 module (3D Fix state, active satellite count, HDOP precision, and live latitude/longitude).
* **Instant Airspace Switching (10 Touch Cards)**:
  * **`[GPS] Live Fix`**: Dynamically locks to your live GPS coordinates as you move.
  * **`[HOME] Rancho Cucamonga`**: Restores your configured custom home coordinates from NVS storage.
  * **Major Global & Aerospace Hubs**:
    * **Los Angeles, CA** (34.05°N, 118.24°W)
    * **San Francisco, CA** (37.77°N, 122.42°W)
    * **Cape Canaveral, FL** (28.39°N, 80.61°W - SpaceX / NASA)
    * **New York, NY** (40.71°N, 74.01°W - JFK / LGA)
    * **London, UK** (51.51°N, 0.13°W - Heathrow)
    * **Paris, France** (48.86°N, 2.35°E - CDG)
    * **Tokyo, Japan** (35.68°N, 139.65°E - Haneda)
    * **Dubai, UAE** (25.20°N, 55.27°E - Emirates)
* **Active Indicator & Live Refresh**: Highlights the currently active airspace with a cyan glow and `[ACTIVE]` tag. Tapping any card immediately switches location, adjusts the local timezone, and re-queries live OpenSky flights, Open-Meteo weather, ISS pass elevation, and SpaceX line-of-sight!

### 10. 3D Printable Tactical Desktop Enclosure
* **Turnkey 3D Printing Files in [`/enclosure`](enclosure/)**:
  * **`AeroRadar_Front_Bezel.stl`**: Precision faceplate with recessed LCD window, 45° touch-swipe chamfer, WS2812 LED indicator aperture, and M3 counterbored screw recesses.
  * **`AeroRadar_Rear_Enclosure.stl`**: Main chassis featuring internal PCB standoffs, a dedicated upward-facing cradle for the **GY-GPS6MV2 module & ceramic antenna**, USB-C port cutout, and thermal airflow ventilation louvers.
  * **`AeroRadar_Desk_Stand_25deg.stl`**: Ergonomic 25° viewing angle desk cradle with rear USB-C cable management tunnel.
  * **`AeroRadar_CYD_C5.scad`**: Parametric OpenSCAD CAD source file for customized tolerances.
* See the complete [**3D Printing & Assembly Guide**](enclosure/3D_PRINTING_GUIDE.md) for recommended slicer settings, orientation, hardware BOM, and step-by-step wiring instructions.

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

# Flash to device (or use esptool script)
pio run -t upload --upload-port COM6
```

> **ESP32-C5 Secondary Bootloader Offset**:
> Note that the ESP32-C5 ROM bootloader expects the secondary bootloader at offset **`0x2000`** (unlike legacy ESP32 at `0x1000`).

---

## Free APIs Used

1. **[OpenSky Network](https://opensky-network.org/)**: Live ADS-B state vectors and squawk codes.
2. **[FlightAware / HexDB / ADSBdb](https://hexdb.io/)**: Aircraft flight origin and destination route lookups.
3. **[Open-Meteo](https://open-meteo.com/)**: Global weather forecasts and meteorological observations.
4. **[WhereTheISS.at](https://wheretheiss.at/)**: Real-time ISS orbital coordinates.
5. **[The Space Devs / Launch Library 2](https://thespacedevs.com/llapi)**: Upcoming rocket launches and orbital schedules.

---

## License & Credits

Distributed under the MIT License.

Special thanks to:
* [Coreymillia/Flight-CYD-ESP32-Radar](https://github.com/Coreymillia/Flight-CYD-ESP32-Radar)
* [vmalis/AirESP32ace](https://github.com/vmalis/AirESP32ace)
