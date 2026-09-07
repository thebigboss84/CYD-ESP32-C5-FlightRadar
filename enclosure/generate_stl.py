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
    print("Generated " + os.path.basename(filename) + ": " + str(num_triangles) + " triangles (" + str(os.path.getsize(filename)) + " bytes)")

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

def generate_front_bezel(out_dir):
    outer_w = 93.0
    outer_h = 57.0
    bezel_t = 4.0
    
    win_w = 61.0
    win_h = 44.0
    
    glass_w = 69.5
    glass_h = 49.5
    glass_d = 1.8
    
    triangles = []
    
    x_out0, x_out1 = -outer_w/2, outer_w/2
    y_out0, y_out1 = -outer_h/2, outer_h/2
    
    x_win0, x_win1 = -win_w/2, win_w/2
    y_win0, y_win1 = -win_h/2, win_h/2
    
    x_gl0, x_gl1 = -glass_w/2, glass_w/2
    y_gl0, y_gl1 = -glass_h/2, glass_h/2
    
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
    
    # 1. Base Floor Plate (Z = 0 to floor_t)
    triangles.extend(make_box(x0, x1, y0, y1, 0, floor_t))
    
    # 2. Four Outer Perimeter Walls (Z = floor_t to total_d)
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
    
    # Front wall (-Y)
    triangles.extend(make_box(ix0, ix1, y0, iy0, floor_t, total_d))
    
    # Back wall (+Y)
    triangles.extend(make_box(ix0, ix1, iy1, y1, floor_t, total_d))
    
    # 3. Four Internal PCB Mounting Standoffs (M3 pilot holes)
    standoff_h = 6.0
    screw_x = 40.5
    screw_y = 22.5
    for sx in [-screw_x, screw_x]:
        for sy in [-screw_y, screw_y]:
            triangles.extend(make_tube(sx, sy, 3.5, 1.4, floor_t, floor_t + standoff_h, segments=16))
            
    # 4. GPS Module Internal Cradle Compartment (for GY-GPS6MV2)
    gps_w = 26.5
    gps_h = 36.5
    gps_d = 8.5
    gx0, gx1 = -gps_w/2, gps_w/2
    gy0, gy1 = -10.0, -10.0 + gps_h
    triangles.extend(make_box(gx0 - 1.6, gx0, gy0, gy1, floor_t, floor_t + gps_d))
    triangles.extend(make_box(gx1, gx1 + 1.6, gy0, gy1, floor_t, floor_t + gps_d))
    triangles.extend(make_box(gx0 - 1.6, gx1 + 1.6, gy1, gy1 + 1.6, floor_t, floor_t + gps_d))
    
    # 5. Bottom Desktop Alignment Rails
    triangles.extend(make_box(x0 + 8, x0 + 16, y0 + 4, y1 - 4, -2.5, 0))
    triangles.extend(make_box(x1 - 16, x1 - 8, y0 + 4, y1 - 4, -2.5, 0))
    
    path = os.path.join(out_dir, "AeroRadar_Rear_Enclosure.stl")
    write_binary_stl(path, triangles, "Rear_Enclosure")

def generate_desk_stand(out_dir):
    stand_w = 76.0
    stand_d = 66.0
    stand_h = 30.0
    
    triangles = []
    x0, x1 = -stand_w/2, stand_w/2
    y0, y1 = -stand_d/2, stand_d/2
    
    # Solid base block
    triangles.extend(make_box(x0, x1, y0, y1, 0, 4.0))
    
    # Stanchion arms
    arm_w = 12.0
    triangles.extend(make_box(x0, x0 + arm_w, y0, y1, 4.0, stand_h))
    triangles.extend(make_box(x1 - arm_w, x1, y0, y1, 4.0, stand_h))
    
    # Front retaining lip to hold radar securely
    triangles.extend(make_box(x0, x1, y0, y0 + 8.0, 4.0, 10.0))
    
    # Angled back support rest (25-degree tilt)
    triangles.extend(make_box(x0, x1, y1 - 12.0, y1, 4.0, stand_h))
    
    # Cable tunnel bridge
    triangles.extend(make_box(x0 + arm_w, x1 - arm_w, y0 + 8.0, y0 + 20.0, 4.0, 7.0))
    
    # Rubber bumper recesses
    for fx in [x0 + 6, x1 - 6]:
        for fy in [y0 + 6, y1 - 6]:
            triangles.extend(make_tube(fx, fy, 4.5, 0.0, -1.5, 0, segments=12))
            
    path = os.path.join(out_dir, "AeroRadar_Desk_Stand_25deg.stl")
    write_binary_stl(path, triangles, "Desk_Stand_25deg")

if __name__ == "__main__":
    out_dir = r"c:\Users\Aboude\Documents\AirRadar-CYD-C5\enclosure"
    os.makedirs(out_dir, exist_ok=True)
    print("=== Generating 3D Enclosure STL Files ===")
    generate_front_bezel(out_dir)
    generate_rear_enclosure(out_dir)
    generate_desk_stand(out_dir)
    print("=== Complete! ===")
