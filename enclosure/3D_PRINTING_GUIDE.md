# 3D Printable Enclosure Guide: CYD-C5 Aerospace Radar

Custom-engineered 3D printable desktop enclosure designed specifically for the **RockBase NM-CYD-C5** (ESP32-C5 2.8" Touch Display) and the **GY-GPS6MV2 (u-blox NEO-6M)** GPS module with its ceramic patch antenna.

---

## 1. Two Enclosure Styles Available

To suit different desk setups and printing preferences, two enclosure designs are provided:

### Style A: Compact Face with Rear GPS Backpack (Recommended)
* **Dimensions**: $93.0\text{ mm} \times 57.0\text{ mm}$ front face, total depth $33.0\text{ mm}$ (main shell $19.0\text{ mm}$ + rear backpack $14.0\text{ mm}$).
* **GPS Compartment**: Dedicated rear "backpack" chamber ($64.0\text{ mm} \times 44.0\text{ mm} \times 14.0\text{ mm}$) on the back wall behind the main PCB:
  * **Bay 1 (Ceramic Patch Antenna Tray)**: Snug $26.5\text{ mm} \times 26.5\text{ mm} \times 9.0\text{ mm}$ pocket holding the $25\times 25\times 8\text{ mm}$ ceramic patch antenna facing upward toward the sky, away from the display copper shield. Includes coax cable strain relief.
  * **Bay 2 (Receiver Board Cradle)**: Snug $27.0\text{ mm} \times 37.0\text{ mm}$ tray with raised support rails holding the $25.5\times 35.5\text{ mm}$ GY-GPS6MV2 receiver PCB with full clearance for the 4-pin header connector.
  * **Internal Passthrough Window**: Direct internal wiring duct directly behind the CYD **LP-UART P5 header** (`GPIO 4, 5, 3.3V, GND`). Zero exterior wires required.
* **Parts to print**:
  1. `AeroRadar_Front_Bezel.stl`
  2. `AeroRadar_Rear_Enclosure.stl`
  3. `AeroRadar_Desk_Stand_25deg.stl`

---

### Style B: Tactical Cockpit with Top Radome Brow
* **Dimensions**: $93.0\text{ mm} \times 82.0\text{ mm}$ ($57.0\text{ mm}$ display section + $25.0\text{ mm}$ upper avionics canopy).
* **GPS Compartment**: Monolithic top canopy directly above the screen:
  * **Apex Antenna Tray**: Sits horizontally at the very top of the enclosure, pointing straight up at $90^\circ$ to the zenith for maximum satellite locking speed.
  * **Receiver Board Slot**: Vertical slide-in bay holding the GY-GPS6MV2 board.
  * **Integrated Sun Visor**: 45° shade hood over the LCD screen giving it an authentic AWACS / fighter cockpit radar console aesthetic.
* **Parts to print**:
  1. `AeroRadar_Front_Bezel_TopPod.stl`
  2. `AeroRadar_Rear_Enclosure_TopPod.stl`
  3. `AeroRadar_Desk_Stand_25deg.stl`

---

## 2. Included Files in `/enclosure`

| File | Description | Print Time (Approx.) |
| :--- | :--- | :---: |
| **`AeroRadar_Front_Bezel.stl`** | Standard front faceplate ($93\times 57\text{ mm}$) with LCD window, touch chamfer, WS2812 light aperture, and M3 counterbores | ~45 mins |
| **`AeroRadar_Rear_Enclosure.stl`** | Rear chassis with PCB standoffs, integrated GPS backpack chamber (antenna tray + receiver cradle), USB-C cutout, and vents | ~1 hr 55 mins |
| **`AeroRadar_Desk_Stand_25deg.stl`** | Universal 25° viewing angle desk cradle with central backpack clearance recess and USB-C cable tunnel | ~1 hr 15 mins |
| **`AeroRadar_Front_Bezel_TopPod.stl`** | Tactical cockpit front bezel ($93\times 82\text{ mm}$) with LCD window and upper sun-visor brow | ~55 mins |
| **`AeroRadar_Rear_Enclosure_TopPod.stl`** | Tactical cockpit rear enclosure with apex skyward antenna radome and vertical receiver slot | ~2 hr 10 mins |
| **`AeroRadar_CYD_C5.scad`** | Complete parametric OpenSCAD source file with selectors for all parts and tolerances | N/A (CAD Source) |
| **`generate_stl.py`** | Standalone Python script to regenerate all binary STL files with mathematical precision | N/A (Build Script) |

---

## 3. Hardware Bill of Materials (BOM)

| Item | Quantity | Purpose |
| :--- | :---: | :--- |
| **RockBase NM-CYD-C5** | 1 | ESP32-C5 2.8" ST7789 TFT & XPT2046 touch board |
| **GY-GPS6MV2 (u-blox NEO-6M)** | 1 | Receiver board ($25\times 35\text{ mm}$) + Ceramic Patch Antenna ($25\times 25\times 8\text{ mm}$) |
| **M3 x 12mm Socket Head Screws** | 4 | Fastens Front Bezel through PCB into Rear Enclosure |
| **M3 x 4mm Heat-Set Inserts** *(Optional)* | 4 | Melted into rear standoff bosses (or use self-tapping M3 directly into plastic) |
| **4-Pin Jumper Ribbon Cable** | 1 | Connects GPS to NM-CYD-C5 LP-UART P5 header (~6 cm length) |
| **8mm Adhesive Rubber Bumper Feet** *(Optional)* | 4 | Inserted into the underside sockets of the Desk Stand |

---

## 4. Recommended Slicer Settings (FDM)

### Material
* **PETG** or **PLA+** (Matte Black, Gunmetal Grey, or Tactical Olive Drab recommended).
* **Clear PLA/PETG** (optional 3mm snippet to act as an optical light pipe for the WS2812 LED).

### Print Parameters
* **Layer Height**: `0.20 mm` (or `0.16 mm` for ultra-fine chamfer contours).
* **Wall Loops / Perimeters**: `3` or `4` (essential for strong M3 screw boss threads).
* **Top / Bottom Shell Layers**: `4` or `5`.
* **Infill**: `20% - 25%` (Gyroid or Grid).
* **Bed Adhesion**: Textured PEI sheet recommended (no brim required).

### Print Orientation & Supports
1. **Front Bezel (`AeroRadar_Front_Bezel.stl` or `_TopPod.stl`)**:
   * **Orientation**: Place front face **flat down on the build plate**.
   * **Supports**: **NONE required**.
2. **Rear Enclosure (`AeroRadar_Rear_Enclosure.stl` or `_TopPod.stl`)**:
   * **Orientation**: Place rear back plate **flat down on the build plate**.
   * **Supports**: Tree / Organic supports enabled **only** for the USB-C side cutout bridge.
3. **Desk Stand (`AeroRadar_Desk_Stand_25deg.stl`)**:
   * **Orientation**: Place bottom base **flat down on the build plate**.
   * **Supports**: **NONE required**.

---

## 5. Wiring & Assembly Instructions

```
             GY-GPS6MV2 GPS MODULE                 NM-CYD-C5 P5 LP-UART HEADER
       +-------------------------------+          +---------------------------+
       | [VCC] ----------------------- | -------> | Pin 4: 3.3V (or 5V)       |
       | [RX]  <---------------------- | -------- | Pin 2: GPIO 5 (TX)        |
       | [TX]  ----------------------> | -------> | Pin 1: GPIO 4 (RX)        |
       | [GND] ----------------------- | -------> | Pin 3: GND                |
       +-------------------------------+          +---------------------------+
               |
         (U.FL Coax Cable)
               |
       +-------------------------------+
       | CERAMIC PATCH ANTENNA         |  <-- Face pointing upward to sky!
       | (25mm x 25mm x 8mm)           |
       +-------------------------------+
```

### Step-by-Step Assembly
1. **Prepare Wiring**:
   * Cut 4 female-to-female DuPont jumper wires to approximately $6\text{ cm}$ ($2.5\text{ in}$).
   * Plug into the GY-GPS6MV2 module header pins (`VCC, RX, TX, GND`).
2. **Install GPS & Antenna into Rear Shell**:
   * **For Style A (Backpack)**:
     * Press the $25\times 25\times 8\text{ mm}$ ceramic patch antenna into the dedicated antenna tray (right bay). Ensure the flat metallic ceramic face faces toward the top/sky.
     * Route the thin coaxial lead into the receiver board's U.FL connector.
     * Slide the GY-GPS6MV2 receiver PCB into the left cradle rails.
     * Feed the 4-pin jumper wires through the internal floor port into the main CYD chamber.
   * **For Style B (Top Radome)**:
     * Place the ceramic patch antenna horizontally in the apex tray pointing straight up.
     * Slide the receiver PCB vertically into the adjacent retention slot.
     * Route the 4 jumper wires down through the partition notch into the main CYD chamber.
3. **Connect to NM-CYD-C5 Board**:
   * Plug the 4 jumper wires into the **P5 LP-UART header** (Pin 1=RX, Pin 2=TX, Pin 3=GND, Pin 4=3.3V).
4. **Mount NM-CYD-C5 Board**:
   * Seat the CYD board over the 4 corner standoffs. Align the USB-C port with the left side wall opening and the MicroSD slot with the right opening.
5. **Secure Front Bezel**:
   * Place the Front Bezel over the display. The glass panel seats flush in the rear recess, and the conical light aperture aligns with the WS2812 RGB LED (top-left).
   * Fasten with 4x **M3 x 12mm socket head screws** through the front counterbore holes into the rear standoffs. Do not over-tighten.
6. **Desktop Display**:
   * Slide the assembled radar unit into the **AeroRadar Desk Stand**.
   * Route your USB-C power cable through the rear archway tunnel.
   * Power on! Satellite lock will acquire in 30–45 seconds with 8–11 satellites locked.
