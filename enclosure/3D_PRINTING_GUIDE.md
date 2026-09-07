# 3D Printable Enclosure Guide: CYD-C5 Aerospace Radar

Custom-engineered 3D printable desktop enclosure designed specifically for the **RockBase NM-CYD-C5** (ESP32-C5 2.8" Touch Display) and the **GY-GPS6MV2 (u-blox NEO-6M)** GPS module.

---

## Included Files in `/enclosure`

| File | Description | Print Time (Approx.) |
| :--- | :--- | :--- |
| **`AeroRadar_Front_Bezel.stl`** | Front faceplate with recessed LCD window, 45-deg touch chamfer, WS2812 LED light pipe, and M3 counterbore holes | ~45 mins |
| **`AeroRadar_Rear_Enclosure.stl`** | Main chassis with PCB standoffs, dedicated GPS module & antenna bay, USB-C cutout, and cooling vents | ~1 hr 45 mins |
| **`AeroRadar_Desk_Stand_25deg.stl`** | Ergonomic 25-degree viewing angle desk cradle with rear USB-C cable management tunnel | ~1 hr 15 mins |
| **`AeroRadar_CYD_C5.scad`** | Complete parametric OpenSCAD source file (fully customizable clearances, wall thickness, tilt angles) | N/A (CAD Source) |
| **`generate_stl.py`** | Standalone Python script to regenerate all binary STL files with mathematical precision | N/A (Build Script) |

---

## Hardware Bill of Materials (BOM)

| Item | Quantity | Purpose |
| :--- | :---: | :--- |
| **RockBase NM-CYD-C5** | 1 | Main ESP32-C5 board with 2.8" ST7789 TFT & touch |
| **GY-GPS6MV2 GPS Module** | 1 | u-blox NEO-6M with ceramic patch antenna |
| **M3 x 12mm Screws** | 4 | Fastens Front Bezel through PCB into Rear Enclosure |
| **M3 x 4mm Heat-Set Inserts** *(Optional)* | 4 | Melted into rear standoff bosses (or use self-tapping M3) |
| **4-Pin Jumper Ribbon Cable** | 1 | Connects GPS to NM-CYD-C5 LP-UART P5 header |
| **8mm Adhesive Rubber Feet** *(Optional)* | 4 | Inserted into the bottom of the Desk Stand |

---

## Recommended Slicer Settings (FDM / SLA)

### Material
* **PETG** or **PLA+** (Matte Black, Gunmetal Grey, or Military Olive Drab recommended for tactical avionics look).
* **Clear / Transparent PLA/PETG** (optional for a 3mm snippet to act as an optical light pipe for the WS2812 LED).

### Print Parameters
* **Layer Height**: `0.20 mm` (or `0.16 mm` for ultra-fine chamfer contours)
* **Wall Loops / Perimeters**: `3 - 4` (vital for strong screw boss threads)
* **Top / Bottom Shell Layers**: `4`
* **Infill**: `20% - 25%` (Gyroid or Grid)
* **Bed Adhesion**: Textured PEI sheet recommended (no brim required)

### Print Orientation & Supports
1. **Front Bezel (`AeroRadar_Front_Bezel.stl`)**:
   * **Orientation**: Place front face **flat against the build plate**.
   * **Supports**: **NONE required**.
2. **Rear Enclosure (`AeroRadar_Rear_Enclosure.stl`)**:
   * **Orientation**: Place rear back floor **flat against the build plate**.
   * **Supports**: Tree / Organic supports enabled **only** for the USB-C side cutout overhang.
3. **Desk Stand (`AeroRadar_Desk_Stand_25deg.stl`)**:
   * **Orientation**: Place bottom base **flat against the build plate**.
   * **Supports**: **NONE required**.

---

## Assembly Instructions

### Step 1: Wire the GPS Module
Connect 4 female-to-female DuPont jumper wires (cut to ~6cm length) between the **GY-GPS6MV2** and the NM-CYD-C5 **P5 Header**:
* **GY-GPS6MV2 VCC** -> **P5 Pin 4 (3.3V or 5V)**
* **GY-GPS6MV2 GND** -> **P5 Pin 3 (GND)**
* **GY-GPS6MV2 TX**  -> **P5 Pin 1 (GPIO 4 / RX)**
* **GY-GPS6MV2 RX**  -> **P5 Pin 2 (GPIO 5 / TX)**

### Step 2: Install Electronics into Rear Enclosure
1. Slide the **GY-GPS6MV2 module and its ceramic antenna** into the internal upper GPS cradle. The metallic ceramic patch must **face upwards towards the ceiling/sky** for optimal satellite reception.
2. Route the 4 jumper wires through the internal pass-through channel.
3. Seat the **RockBase NM-CYD-C5 board** over the 4 corner standoff bosses. Align the USB-C port with the left side wall opening.

### Step 3: Fasten the Front Bezel
1. Place the **Front Bezel** over the front of the display. The LCD glass panel fits snugly into the rear recess.
2. Ensure the conical WS2812 LED light aperture aligns with the onboard RGB LED (top-left).
3. Insert 4x **M3 x 12mm socket head screws** through the front counterbore holes and tighten gently into the rear standoffs until snug.

### Step 4: Seat into Desktop Cradle
1. Place the assembled radar unit into the **25-deg Desk Stand**.
2. Route your USB-C power cable through the rear archway channel.
3. Power on and enjoy your tactical desktop aerospace radar!
