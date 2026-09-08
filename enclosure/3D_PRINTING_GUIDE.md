# 3D Printable Enclosure Guide: CYD-C5 Aerospace Radar

Custom-engineered 3D printable desktop enclosure designed specifically for the **RockBase NM-CYD-C5** (ESP32-C5 2.8" Touch Display) and the **GY-GPS6MV2 (u-blox NEO-6M)** GPS module with its ceramic patch antenna.

---

## 1. Two Enclosure Styles Available

To suit different desk setups and printing preferences, two enclosure designs are provided:

### Style A: Unified Flat-Bottom Enclosure with Internal Basement (Recommended)
* **Dimensions**: $93.0\text{ mm} \times 57.0\text{ mm}$ front face, total depth $24.0\text{ mm}$ (clean, 100% flat bottom at $Z=0$).
* **Unified Chassis (No Exterior Backpack Protrusion)**:
  * In earlier prototypes, an exterior "backpack" was placed on the back wall. The current final version integrates everything into a single, sleek, flat-bottom enclosure body with zero exterior protrusions.
  * **$11.0\text{ mm}$ Internal Basement**: Beneath the CYD PCB, an $11.0\text{ mm}$ clear chamber houses both the GPS receiver board and the ceramic antenna side-by-side:
    * **Left Bay ($28.0\text{ mm} \times 36.0\text{ mm}$)**: Snug cradle rails holding the $25.5\times 35.5\text{ mm}$ GY-GPS6MV2 receiver PCB with full clearance for the 4-pin header connector.
    * **Right Bay ($26.0\text{ mm} \times 26.0\text{ mm}$)**: Dedicated pocket holding the $25\times 25\times 8\text{ mm}$ ceramic patch antenna facing skyward, with a $3.0\text{ mm}$ air gap to the CYD board.
  * **Zero Support Printing**: The bottom of the rear chassis is completely flat ($Z=0$), laying flat on the 3D printer bed without any exterior support material needed.
  * **Mating & Interlocking**: The Front Bezel features an underside stepped alignment tongue ($87.6\text{ mm} \times 51.6\text{ mm} \times 1.5\text{ mm}$) that drops directly **inside** the top opening of the rear enclosure, locking the bezel flush with zero wobble or slide.
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
| **`AeroRadar_Front_Bezel.stl`** | Standard front faceplate ($93\times 57\text{ mm}$) with stepped underside alignment tongue, LCD glass pocket, WS2812 light aperture, and M3 counterbores | ~45 mins |
| **`AeroRadar_Rear_Enclosure.stl`** | Unified flat-bottom chassis ($93\times 57\times 24\text{ mm}$) with $11\text{ mm}$ internal basement, dual GPS bays, 4x PCB corner standoffs, USB-C & MicroSD cutouts | ~1 hr 45 mins |
| **`AeroRadar_Desk_Stand_25deg.stl`** | Universal 25° viewing angle desk cradle with anti-slip front lip and rear USB-C cable tunnel | ~1 hr 15 mins |
| **`AeroRadar_Front_Bezel_TopPod.stl`** | Tactical cockpit front bezel ($93\times 82\text{ mm}$) with LCD window and upper sun-visor brow | ~55 mins |
| **`AeroRadar_Rear_Enclosure_TopPod.stl`** | Tactical cockpit rear enclosure with apex skyward antenna radome and vertical receiver slot | ~2 hr 10 mins |
| **`AeroRadar_CYD_C5.scad`** | Complete parametric OpenSCAD source file with selectors for all parts and tolerances | N/A (CAD Source) |
| **`generate_stl.py`** | Standalone cellular B-Rep generator script producing 100% 2-manifold, zero-error STLs | N/A (Build Script) |

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
    * **For Style A (Unified Flat-Bottom Chassis)**:
      * Place the $25\times 25\times 8\text{ mm}$ ceramic patch antenna into the dedicated antenna bay (right bay on the floor). Ensure the ceramic patch face points upward toward the sky.
      * Connect the thin coaxial cable to the GY-GPS6MV2 receiver board's U.FL connector.
      * Slide the GY-GPS6MV2 receiver PCB into the left bay retention rails on the floor.
      * Connect the 4 DuPont jumper wires to the receiver header (`VCC, RX, TX, GND`).
    * **For Style B (Top Radome)**:
      * Place the ceramic patch antenna horizontally in the apex tray pointing straight up.
      * Slide the receiver PCB vertically into the adjacent retention slot.
      * Route the 4 jumper wires down through the partition notch into the main CYD chamber.
3. **Connect to NM-CYD-C5 Board**:
   * Plug the 4 jumper wires into the **P5 LP-UART header** (Pin 1=RX, Pin 2=TX, Pin 3=GND, Pin 4=3.3V).
4. **Mount NM-CYD-C5 Board**:
   * Lower the CYD board onto the 4 corner standoffs ($11\text{ mm}$ above the floor). The board floats above the GPS module and antenna with ample clearance. Align the USB-C port with the left side wall cutout and the MicroSD slot with the right opening.
5. **Secure Front Bezel**:
   * Place the Front Bezel over the display. The underside stepped alignment tongue drops directly **inside** the top opening of the rear enclosure, locking it firmly against sideways motion. The glass panel seats flush in the underside recess, and the conical light aperture aligns with the WS2812 RGB LED (top-left).
   * Fasten with 4x **M3 x 14mm (or 12mm–16mm) socket head screws** through the front counterbore holes, passing through the CYD PCB corner holes and threading directly into the rear standoff bosses. Tighten gently until snug.
6. **Desktop Display**:
   * Slide the assembled radar unit into the **AeroRadar Desk Stand**.
   * Route your USB-C power cable through the rear archway tunnel.
   * Power on! Satellite lock will acquire in 30–45 seconds with 8–11 satellites locked.
