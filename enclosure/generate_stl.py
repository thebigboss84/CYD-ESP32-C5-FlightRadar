import numpy as np
import struct
import math
import os

def write_binary_stl(filename, triangles, name="CYD_Enclosure"):
    triangles = np.asarray(triangles, dtype=np.float32)
    num_triangles = len(triangles)
    
    v0 = triangles[:, 0, :]
    v1 = triangles[:, 1, :]
    v2 = triangles[:, 2, :]
    normals = np.cross(v1 - v0, v2 - v0)
    norm = np.linalg.norm(normals, axis=1, keepdims=True)
    norm[norm == 0] = 1.0
    normals = normals / norm
    
    header = f"Antigravity 3D CAD: {name}".encode("ascii")
    header = header[:80].ljust(80, b" ")
    
    record_dtype = np.dtype([
        ("normal", "<f4", (3,)),
        ("v0", "<f4", (3,)),
        ("v1", "<f4", (3,)),
        ("v2", "<f4", (3,)),
        ("attr", "<u2")
    ])
    records = np.empty(num_triangles, dtype=record_dtype)
    records["normal"] = normals
    records["v0"] = v0
    records["v1"] = v1
    records["v2"] = v2
    records["attr"] = 0
    
    with open(filename, "wb") as f:
        f.write(header)
        f.write(struct.pack("<I", num_triangles))
        records.tofile(f)
    print(f"Generated {os.path.basename(filename)}: {num_triangles} triangles ({os.path.getsize(filename):,} bytes)")

def quad_triangles(p0, p1, p2, p3):
    return [[p0, p1, p2], [p0, p2, p3]]

def make_box(x0, x1, y0, y1, z0, z1):
    c000 = [x0, y0, z0]; c100 = [x1, y0, z0]; c110 = [x1, y1, z0]; c010 = [x0, y1, z0]
    c001 = [x0, y0, z1]; c101 = [x1, y0, z1]; c111 = [x1, y1, z1]; c011 = [x0, y1, z1]
    
    triangles = []
    triangles.extend(quad_triangles(c000, c100, c110, c010)) # Bottom (-Z)
    triangles.extend(quad_triangles(c001, c011, c111, c101)) # Top (+Z)
    triangles.extend(quad_triangles(c000, c001, c101, c100)) # Front (-Y)
    triangles.extend(quad_triangles(c110, c111, c011, c010)) # Back (+Y)
    triangles.extend(quad_triangles(c010, c011, c001, c000)) # Left (-X)
    triangles.extend(quad_triangles(c100, c101, c111, c110)) # Right (+X)
    return triangles

def make_tube(cx, cy, r_out, r_in, z0, z1, segments=32):
    triangles = []
    angles = np.linspace(0, 2*math.pi, segments, endpoint=False)
    d_ang = angles[1] - angles[0]
    
    for a in angles:
        a2 = a + d_ang
        cos1, sin1 = math.cos(a), math.sin(a)
        cos2, sin2 = math.cos(a2), math.sin(a2)
        
        o0 = [cx + r_out * cos1, cy + r_out * sin1, z0]
        o1 = [cx + r_out * cos2, cy + r_out * sin2, z0]
        o2 = [cx + r_out * cos2, cy + r_out * sin2, z1]
        o3 = [cx + r_out * cos1, cy + r_out * sin1, z1]
        
        i0 = [cx + r_in * cos1, cy + r_in * sin1, z0]
        i1 = [cx + r_in * cos2, cy + r_in * sin2, z0]
        i2 = [cx + r_in * cos2, cy + r_in * sin2, z1]
        i3 = [cx + r_in * cos1, cy + r_in * sin1, z1]
        
        triangles.extend(quad_triangles(o0, o1, o2, o3))
        triangles.extend(quad_triangles(i1, i0, i3, i2))
        triangles.extend(quad_triangles(o1, o0, i0, i1))
        triangles.extend(quad_triangles(o3, o2, i2, i3))
        
    return triangles

# ============================================================================
# 1. FRONT BEZEL (Standard 93x57mm)
# ============================================================================
def generate_front_bezel(out_dir):
    outer_w = 93.0
    outer_h = 57.0
    bezel_t = 4.0
    win_w   = 61.0
    win_h   = 44.0
    glass_w = 69.5
    glass_h = 49.5
    glass_d = 1.8
    
    triangles = []
    x_out0, x_out1 = -outer_w/2, outer_w/2
    y_out0, y_out1 = -outer_h/2, outer_h/2
    x_win0, x_win1 = -win_w/2, win_w/2
    y_win0, y_win1 = -win_h/2, win_h/2
    x_gl0, x_gl1   = -glass_w/2, glass_w/2
    y_gl0, y_gl1   = -glass_h/2, glass_h/2
    
    # Outer side walls
    triangles.extend(quad_triangles([x_out0, y_out1, 0], [x_out0, y_out1, bezel_t], [x_out0, y_out0, bezel_t], [x_out0, y_out0, 0]))
    triangles.extend(quad_triangles([x_out1, y_out0, 0], [x_out1, y_out0, bezel_t], [x_out1, y_out1, bezel_t], [x_out1, y_out1, 0]))
    triangles.extend(quad_triangles([x_out0, y_out0, 0], [x_out0, y_out0, bezel_t], [x_out1, y_out0, bezel_t], [x_out1, y_out0, 0]))
    triangles.extend(quad_triangles([x_out1, y_out1, 0], [x_out1, y_out1, bezel_t], [x_out0, y_out1, bezel_t], [x_out0, y_out1, 0]))
    
    # Front Face (Z = bezel_t) composed of 4 segments framing the LCD window
    triangles.extend(make_box(x_out0, x_win0, y_out0, y_out1, glass_d, bezel_t))
    triangles.extend(make_box(x_win1, x_out1, y_out0, y_out1, glass_d, bezel_t))
    triangles.extend(make_box(x_win0, x_win1, y_win1, y_out1, glass_d, bezel_t))
    triangles.extend(make_box(x_win0, x_win1, y_out0, y_win0, glass_d, bezel_t))
    
    # Rear glass seating shelf: from z = 0 to glass_d
    triangles.extend(make_box(x_out0, x_gl0, y_out0, y_out1, 0, glass_d))
    triangles.extend(make_box(x_gl1, x_out1, y_out0, y_out1, 0, glass_d))
    triangles.extend(make_box(x_gl0, x_gl1, y_gl1, y_out1, 0, glass_d))
    triangles.extend(make_box(x_gl0, x_gl1, y_out0, y_gl0, 0, glass_d))
    
    # 4 Corner M3 Screw Bosses & Counterbore Holes (81mm x 45mm spacing)
    screw_x = 40.5
    screw_y = 22.5
    for sx in [-screw_x, screw_x]:
        for sy in [-screw_y, screw_y]:
            triangles.extend(make_tube(sx, sy, 4.5, 1.7, 0, bezel_t, segments=16))
            
    # WS2812 Beacon Light Guide (X=-28.0, Y=22.5)
    triangles.extend(make_tube(-28.0, 22.5, 3.5, 1.6, 0, bezel_t, segments=16))
    
    path = os.path.join(out_dir, "AeroRadar_Front_Bezel.stl")
    write_binary_stl(path, triangles, "Front_Bezel")

# ============================================================================
# 2. REAR ENCLOSURE (With Integrated GPS Module & Antenna Backpack)
# ============================================================================
def generate_rear_enclosure(out_dir):
    outer_w = 93.0
    outer_h = 57.0
    total_d = 19.0
    floor_t = 2.0
    wall_t  = 2.4
    
    inner_w = outer_w - 2 * wall_t
    inner_h = outer_h - 2 * wall_t
    
    triangles = []
    
    x0, x1 = -outer_w/2, outer_w/2
    y0, y1 = -outer_h/2, outer_h/2
    ix0, ix1 = -inner_w/2, inner_w/2
    iy0, iy1 = -inner_h/2, inner_h/2
    
    # Backpack dimensions (extends backward from Z=0 to Z=-14.0mm)
    bp_w = 64.0
    bp_h = 44.0
    bp_d = 14.0
    bp_t = 2.0
    
    bpx0, bpx1 = -bp_w/2, bp_w/2
    bpy0, bpy1 = -16.0, -16.0 + bp_h # -16 to +28
    bpz0, bpz1 = -bp_d, 0.0
    
    # --- A. Main Enclosure Floor (with central opening into GPS backpack) ---
    # Surround floor segments around the backpack opening
    triangles.extend(make_box(x0, bpx0, y0, y1, 0, floor_t))       # Left floor
    triangles.extend(make_box(bpx1, x1, y0, y1, 0, floor_t))       # Right floor
    triangles.extend(make_box(bpx0, bpx1, y0, bpy0, 0, floor_t))   # Bottom floor
    triangles.extend(make_box(bpx0, bpx1, bpy1, y1, 0, floor_t))   # Top floor
    
    # Partial bridge floor segment to support CYD board and provide wire route
    triangles.extend(make_box(bpx0, bpx1, 10.0, 14.0, 0, floor_t))
    
    # --- B. Four Outer Perimeter Walls (Z = floor_t to total_d) ---
    # Left wall (-X) with USB-C Cutout
    usb_y0, usb_y1 = -6.5, 6.5
    usb_z0, usb_z1 = floor_t + 4.0, floor_t + 11.5
    triangles.extend(make_box(x0, ix0, y0, y1, floor_t, usb_z0))
    triangles.extend(make_box(x0, ix0, y0, y1, usb_z1, total_d))
    triangles.extend(make_box(x0, ix0, y0, usb_y0, usb_z0, usb_z1))
    triangles.extend(make_box(x0, ix0, usb_y1, y1, usb_z0, usb_z1))
    
    # Right wall (+X) with MicroSD Slot
    sd_y0, sd_y1 = -7.0, 7.0
    sd_z0, sd_z1 = floor_t + 6.0, floor_t + 9.5
    triangles.extend(make_box(ix1, x1, y0, y1, floor_t, sd_z0))
    triangles.extend(make_box(ix1, x1, y0, y1, sd_z1, total_d))
    triangles.extend(make_box(ix1, x1, y0, sd_y0, sd_z0, sd_z1))
    triangles.extend(make_box(ix1, x1, sd_y1, y1, sd_z0, sd_z1))
    
    # Front wall (-Y) and Back wall (+Y)
    triangles.extend(make_box(ix0, ix1, y0, iy0, floor_t, total_d))
    triangles.extend(make_box(ix0, ix1, iy1, y1, floor_t, total_d))
    
    # --- C. Four Internal PCB Mounting Standoffs (M3 pilot holes) ---
    standoff_h = 6.0
    screw_x = 40.5
    screw_y = 22.5
    for sx in [-screw_x, screw_x]:
        for sy in [-screw_y, screw_y]:
            triangles.extend(make_tube(sx, sy, 3.5, 1.4, floor_t, floor_t + standoff_h, segments=16))
            
    # --- D. GPS Backpack Chamber (Z = -bp_d to 0) ---
    # Backpack Floor (Z = -bp_d to -bp_d + bp_t)
    triangles.extend(make_box(bpx0, bpx1, bpy0, bpy1, bpz0, bpz0 + bp_t))
    
    # Backpack Outer Walls
    # Left (-X) & Right (+X)
    triangles.extend(make_box(bpx0, bpx0 + bp_t, bpy0, bpy1, bpz0 + bp_t, bpz1))
    triangles.extend(make_box(bpx1 - bp_t, bpx1, bpy0, bpy1, bpz0 + bp_t, bpz1))
    # Bottom (-Y) & Top (+Y)
    triangles.extend(make_box(bpx0 + bp_t, bpx1 - bp_t, bpy0, bpy0 + bp_t, bpz0 + bp_t, bpz1))
    triangles.extend(make_box(bpx0 + bp_t, bpx1 - bp_t, bpy1 - bp_t, bpy1, bpz0 + bp_t, bpz1))
    
    # Internal Partition Wall between Receiver Board and Ceramic Antenna
    part_x = 1.0
    triangles.extend(make_box(part_x - 1.0, part_x + 1.0, bpy0 + bp_t, bpy1 - bp_t, bpz0 + bp_t, bpz1 - 3.0))
    
    # --- E. Right Bay: Ceramic Patch Antenna Tray (26mm x 26mm x 9mm) ---
    # Antenna sits at X = [2.0, 28.0], Y = [0.0, 26.0]
    ant_x0, ant_x1 = 2.0, 28.0
    ant_y0, ant_y1 = 0.0, 26.0
    ant_floor_z = bpz0 + bp_t
    # Cable channel at bottom of antenna tray for coax pigtail
    triangles.extend(make_box(ant_x0 + 4, ant_x1 - 4, ant_y0, ant_y0 + 3, ant_floor_z, ant_floor_z + 3.0))
    # Corner retention tabs for snug antenna friction fit
    triangles.extend(make_box(ant_x0, ant_x0 + 2.5, ant_y1 - 2.5, ant_y1, bpz1 - 2.0, bpz1))
    triangles.extend(make_box(ant_x1 - 2.5, ant_x1, ant_y1 - 2.5, ant_y1, bpz1 - 2.0, bpz1))
    
    # --- F. Left Bay: GY-GPS6MV2 Receiver Board Cradle (26.5mm x 36.5mm) ---
    # Board sits at X = [-29.5, -2.5], Y = [-13.5, 23.5]
    rx_x0, rx_x1 = -29.5, -2.5
    rx_y0, rx_y1 = -13.5, 23.5
    # Standoff support rails lifting the module PCB 2mm off floor
    rail_h = ant_floor_z + 2.5
    triangles.extend(make_box(rx_x0, rx_x0 + 2.0, rx_y0, rx_y1, ant_floor_z, rail_h))
    triangles.extend(make_box(rx_x1 - 2.0, rx_x1, rx_y0, rx_y1, ant_floor_z, rail_h))
    # Retention stop tabs
    triangles.extend(make_box(rx_x0, rx_x1, rx_y1 - 2.0, rx_y1, rail_h, rail_h + 3.0))
    
    # --- G. Wiring Passthrough Aperture to CYD P5 Header ---
    # Direct port into CYD chamber at X = [-22, -6], Y = [15, 25] (already open through floor)
    
    # --- H. Bottom Alignment Rails for Desk Stand ---
    triangles.extend(make_box(x0 + 6, bpx0 - 2, y0 + 4, y1 - 4, 0, 2.5))
    triangles.extend(make_box(bpx1 + 2, x1 - 6, y0 + 4, y1 - 4, 0, 2.5))
    
    path = os.path.join(out_dir, "AeroRadar_Rear_Enclosure.stl")
    write_binary_stl(path, triangles, "Rear_Enclosure")

# ============================================================================
# 3. UNIVERSAL DESK STAND (25-Degree Viewing Tilt)
# ============================================================================
def generate_desk_stand(out_dir):
    stand_w = 78.0
    stand_d = 72.0
    stand_h = 32.0
    
    triangles = []
    x0, x1 = -stand_w/2, stand_w/2
    y0, y1 = -stand_d/2, stand_d/2
    
    # 1. Solid base floor block (Z = 0 to 4.0mm)
    triangles.extend(make_box(x0, x1, y0, y1, 0, 4.0))
    
    # 2. Side Stanchion Arms supporting the radar perimeter
    arm_w = 12.0
    triangles.extend(make_box(x0, x0 + arm_w, y0, y1, 4.0, stand_h))
    triangles.extend(make_box(x1 - arm_w, x1, y0, y1, 4.0, stand_h))
    
    # 3. Front Retaining Lip (holds unit from sliding forward)
    triangles.extend(make_box(x0, x1, y0, y0 + 10.0, 4.0, 14.0))
    
    # 4. Rear Angled Rest Bar (supports 25° inclination)
    triangles.extend(make_box(x0, x1, y1 - 12.0, y1, 4.0, stand_h))
    
    # 5. Central Recess for GPS Backpack (width 54mm, depth 16mm)
    # The middle section between arms from y0 + 10 to y1 - 12 is at Z = 4.0,
    # giving 28mm vertical clearance for the backpack chamber!
    
    # 6. Rear USB-C Cable Route Bridge
    triangles.extend(make_box(x0 + arm_w, x1 - arm_w, y1 - 12.0, y1, stand_h - 10.0, stand_h))
    
    # 7. Underside Rubber Bumper Recesses (4 corners)
    for fx in [x0 + 6, x1 - 6]:
        for fy in [y0 + 6, y1 - 6]:
            triangles.extend(make_tube(fx, fy, 4.5, 0.0, -1.5, 0, segments=12))
            
    path = os.path.join(out_dir, "AeroRadar_Desk_Stand_25deg.stl")
    write_binary_stl(path, triangles, "Desk_Stand_25deg")

# ============================================================================
# 4. TACTICAL AVIONICS TOP-POD BEZEL (93mm x 82mm with Radome Canopy)
# ============================================================================
def generate_top_pod_bezel(out_dir):
    outer_w = 93.0
    outer_h = 82.0 # 57mm base + 25mm top pod
    bezel_t = 4.0
    win_w   = 61.0
    win_h   = 44.0
    glass_w = 69.5
    glass_h = 49.5
    glass_d = 1.8
    
    triangles = []
    x_out0, x_out1 = -outer_w/2, outer_w/2
    y_out0, y_out1 = -outer_h/2, outer_h/2
    # Screen is centered in the lower 57mm area
    screen_center_y = -12.5
    x_win0, x_win1 = -win_w/2, win_w/2
    y_win0, y_win1 = screen_center_y - win_h/2, screen_center_y + win_h/2
    x_gl0, x_gl1   = -glass_w/2, glass_w/2
    y_gl0, y_gl1   = screen_center_y - glass_h/2, screen_center_y + glass_h/2
    
    # Outer side walls
    triangles.extend(quad_triangles([x_out0, y_out1, 0], [x_out0, y_out1, bezel_t], [x_out0, y_out0, bezel_t], [x_out0, y_out0, 0]))
    triangles.extend(quad_triangles([x_out1, y_out0, 0], [x_out1, y_out0, bezel_t], [x_out1, y_out1, bezel_t], [x_out1, y_out1, 0]))
    triangles.extend(quad_triangles([x_out0, y_out0, 0], [x_out0, y_out0, bezel_t], [x_out1, y_out0, bezel_t], [x_out1, y_out0, 0]))
    triangles.extend(quad_triangles([x_out1, y_out1, 0], [x_out1, y_out1, bezel_t], [x_out0, y_out1, bezel_t], [x_out0, y_out1, 0]))
    
    # Lower Screen Frame (Z = bezel_t)
    triangles.extend(make_box(x_out0, x_win0, y_out0, y_win1, glass_d, bezel_t))
    triangles.extend(make_box(x_win1, x_out1, y_out0, y_win1, glass_d, bezel_t))
    triangles.extend(make_box(x_win0, x_win1, y_out0, y_win0, glass_d, bezel_t))
    
    # Rear glass seating shelf: from z = 0 to glass_d
    triangles.extend(make_box(x_out0, x_gl0, y_out0, y_gl1, 0, glass_d))
    triangles.extend(make_box(x_gl1, x_out1, y_out0, y_gl1, 0, glass_d))
    triangles.extend(make_box(x_gl0, x_gl1, y_out0, y_gl0, 0, glass_d))
    
    # Upper Avionics Canopy Brow (solid top canopy faceplate from y_win1 to y_out1)
    triangles.extend(make_box(x_out0, x_out1, y_win1, y_out1, 0, bezel_t))
    # Top Sun-Visor 45-degree angled brow extension (adds 3mm tactical visor)
    triangles.extend(make_box(x_out0 + 2, x_out1 - 2, y_win1 - 2.0, y_win1 + 4.0, bezel_t, bezel_t + 3.0))
    
    # 4 Corner M3 Screw Bosses (relative to lower display center)
    screw_x = 40.5
    screw_y = 22.5
    for sx in [-screw_x, screw_x]:
        for sy in [screen_center_y - screw_y, screen_center_y + screw_y]:
            triangles.extend(make_tube(sx, sy, 4.5, 1.7, 0, bezel_t, segments=16))
            
    # WS2812 Beacon Light Guide
    triangles.extend(make_tube(-28.0, screen_center_y + 22.5, 3.5, 1.6, 0, bezel_t, segments=16))
    
    path = os.path.join(out_dir, "AeroRadar_Front_Bezel_TopPod.stl")
    write_binary_stl(path, triangles, "Front_Bezel_TopPod")

# ============================================================================
# 5. TACTICAL AVIONICS TOP-POD REAR ENCLOSURE (Zenith Antenna Radome)
# ============================================================================
def generate_top_pod_rear(out_dir):
    outer_w = 93.0
    outer_h = 82.0
    total_d = 20.0
    floor_t = 2.0
    wall_t  = 2.4
    
    inner_w = outer_w - 2 * wall_t
    inner_h = outer_h - 2 * wall_t
    
    triangles = []
    x0, x1 = -outer_w/2, outer_w/2
    y0, y1 = -outer_h/2, outer_h/2
    ix0, ix1 = -inner_w/2, inner_w/2
    iy0, iy1 = -inner_h/2, inner_h/2
    
    screen_center_y = -12.5
    
    # 1. Base Floor Plate (Z = 0 to floor_t)
    triangles.extend(make_box(x0, x1, y0, y1, 0, floor_t))
    
    # 2. Four Outer Perimeter Walls (Z = floor_t to total_d)
    # Left wall (-X) with USB-C Cutout
    usb_y0, usb_y1 = screen_center_y - 6.5, screen_center_y + 6.5
    usb_z0, usb_z1 = floor_t + 4.0, floor_t + 11.5
    triangles.extend(make_box(x0, ix0, y0, y1, floor_t, usb_z0))
    triangles.extend(make_box(x0, ix0, y0, y1, usb_z1, total_d))
    triangles.extend(make_box(x0, ix0, y0, usb_y0, usb_z0, usb_z1))
    triangles.extend(make_box(x0, ix0, usb_y1, y1, usb_z0, usb_z1))
    
    # Right wall (+X) with MicroSD Slot
    sd_y0, sd_y1 = screen_center_y - 7.0, screen_center_y + 7.0
    sd_z0, sd_z1 = floor_t + 6.0, floor_t + 9.5
    triangles.extend(make_box(ix1, x1, y0, y1, floor_t, sd_z0))
    triangles.extend(make_box(ix1, x1, y0, y1, sd_z1, total_d))
    triangles.extend(make_box(ix1, x1, y0, sd_y0, sd_z0, sd_z1))
    triangles.extend(make_box(ix1, x1, sd_y1, y1, sd_z0, sd_z1))
    
    # Front wall (-Y) and Back wall (+Y)
    triangles.extend(make_box(ix0, ix1, y0, iy0, floor_t, total_d))
    triangles.extend(make_box(ix0, ix1, iy1, y1, floor_t, total_d))
    
    # 3. Four Internal PCB Mounting Standoffs for CYD board
    standoff_h = 6.0
    screw_x = 40.5
    screw_y = 22.5
    for sx in [-screw_x, screw_x]:
        for sy in [screen_center_y - screw_y, screen_center_y + screw_y]:
            triangles.extend(make_tube(sx, sy, 3.5, 1.4, floor_t, floor_t + standoff_h, segments=16))
            
    # 4. Upper Avionics Canopy GPS Bays (Y in [16.0, y1 - wall_t])
    part_y = screen_center_y + 26.0 # ~13.5mm
    triangles.extend(make_box(ix0, ix1, part_y - 1.2, part_y + 1.2, floor_t, floor_t + 10.0))
    
    # Bay A: Upward Skyward Ceramic Antenna Tray (26.5mm x 26.5mm x 9mm)
    ant_x0, ant_x1 = 2.0, 29.0
    ant_y0, ant_y1 = part_y + 3.0, y1 - wall_t - 2.0
    triangles.extend(make_box(ant_x0, ant_x1, ant_y0, ant_y0 + 2.0, floor_t, floor_t + 9.5))
    triangles.extend(make_box(ant_x0, ant_x0 + 2.0, ant_y0, ant_y1, floor_t, floor_t + 9.5))
    triangles.extend(make_box(ant_x1 - 2.0, ant_x1, ant_y0, ant_y1, floor_t, floor_t + 9.5))
    
    # Bay B: Vertical Slot for GY-GPS6MV2 Receiver PCB (26.5mm x 36.5mm)
    rx_x0, rx_x1 = -30.0, -2.0
    rx_y0, rx_y1 = part_y + 3.0, y1 - wall_t - 2.0
    triangles.extend(make_box(rx_x0, rx_x0 + 2.0, rx_y0, rx_y1, floor_t, floor_t + 7.0))
    triangles.extend(make_box(rx_x1 - 2.0, rx_x1, rx_y0, rx_y1, floor_t, floor_t + 7.0))
    
    path = os.path.join(out_dir, "AeroRadar_Rear_Enclosure_TopPod.stl")
    write_binary_stl(path, triangles, "Rear_Enclosure_TopPod")

# ============================================================================
# MAIN ENTRYPOINT
# ============================================================================
if __name__ == "__main__":
    out_dir = r"c:\Users\Aboude\Documents\AirRadar-CYD-C5\enclosure"
    os.makedirs(out_dir, exist_ok=True)
    print("=== Generating Upgraded 3D Enclosure STL Files with GPS Compartments ===")
    generate_front_bezel(out_dir)
    generate_rear_enclosure(out_dir)
    generate_desk_stand(out_dir)
    generate_top_pod_bezel(out_dir)
    generate_top_pod_rear(out_dir)
    print("=== All 5 STL Models Successfully Built! ===")
