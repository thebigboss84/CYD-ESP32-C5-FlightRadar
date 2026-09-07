// ============================================================================
// CYD ESP32-C5 FlightRadar & Aerospace Instrument - 3D Printable Enclosure
// Compatible with: RockBase NM-CYD-C5 (ESP32-C5 2.8" TFT Touch) + GY-GPS6MV2
// Designed for 3D Printing: FDM (PLA/PETG/ABS) or Resin (SLA)
// ============================================================================

/* [Model View Selection] */
// What to display
part_to_show = "assembly"; // [assembly:Full Assembly, exploded:Exploded View, bezel:Front Bezel Only, rear:Rear Enclosure Only, stand:Desk Stand Only]

/* [Print & Fit Tolerances] */
nozzle_dia      = 0.4;  // Nozzle diameter (mm)
clearance       = 0.5;  // General clearance for slide/drop-in fit (mm)
wall_t          = 2.4;  // Enclosure wall thickness (mm)
floor_t         = 2.0;  // Rear enclosure floor thickness (mm)
bezel_t         = 4.0;  // Front bezel thickness (mm)

/* [Hardware Dimensions] */
pcb_w           = 86.0; // NM-CYD-C5 PCB width (mm)
pcb_h           = 50.0; // NM-CYD-C5 PCB height (mm)
pcb_t           = 1.6;  // PCB thickness (mm)
standoff_h      = 6.0;  // Standoff height off rear wall (mm)

// Corner Mounting Holes (M3 / 81mm x 45mm spacing)
screw_dist_x    = 81.0; 
screw_dist_y    = 45.0;
screw_hole_d    = 3.2;  // M3 clearance hole
screw_head_d    = 6.0;  // M3 socket head diameter
screw_head_h    = 2.2;  // M3 counterbore depth
boss_d          = 7.0;  // Standoff boss outer diameter
boss_pilot_d    = 2.8;  // Standoff pilot hole for self-tapping M3

/* [Display Cutout] */
disp_view_w     = 61.0; // Viewable LCD window width (mm)
disp_view_h     = 44.0; // Viewable LCD window height (mm)
disp_glass_w    = 69.5; // Glass panel outer width (mm)
disp_glass_h    = 50.0; // Glass panel outer height (mm)
disp_recess_d   = 1.8;  // Glass panel recess depth (mm)

/* [WS2812 Indicator Beacon] */
led_x           = -28.0;// LED X offset from center (mm)
led_y           = 22.5; // LED Y offset from center (mm)
led_aperture_d  = 3.2;  // Outer light port diameter (mm)
led_cone_d      = 5.0;  // Inner light funnel diameter (mm)

/* [GY-GPS6MV2 Module Chamber] */
gps_w           = 26.5; // Module bay width (mm)
gps_h           = 36.5; // Module bay length (mm)
gps_d           = 9.0;  // Depth for module + ceramic patch antenna (mm)

/* [Desk Stand Angle] */
stand_tilt_deg  = 25.0; // Desktop viewing tilt angle (degrees)

// Calculated bounding parameters
outer_w         = pcb_w + (wall_t * 2) + (clearance * 2); // ~91.8 mm
outer_h         = pcb_h + (wall_t * 2) + (clearance * 2); // ~55.8 mm
rear_depth      = 18.0; // Enclosure depth (mm)

$fn = 40; // Circle facet resolution

// ============================================================================
// TOP-LEVEL SELECTION
// ============================================================================
if (part_to_show == "assembly") {
    assembly_view(exploded = false);
} else if (part_to_show == "exploded") {
    assembly_view(exploded = true);
} else if (part_to_show == "bezel") {
    front_bezel();
} else if (part_to_show == "rear") {
    rear_enclosure();
} else if (part_to_show == "stand") {
    desk_stand_25deg();
}

// ============================================================================
// MODULE: FRONT BEZEL
// ============================================================================
module front_bezel() {
    difference() {
        // Outer Faceplate with Rounded Corners
        hull() {
            for (x = [-outer_w/2 + 4, outer_w/2 - 4]) {
                for (y = [-outer_h/2 + 4, outer_h/2 - 4]) {
                    translate([x, y, 0])
                        cylinder(r = 4, h = bezel_t);
                }
            }
        }

        // 1. LCD Active Display Aperture (with 45° touch bevel)
        translate([0, 0, -0.1]) {
            // Straight through window
            cube([disp_view_w, disp_view_h, bezel_t + 0.2], center = true);
        }
        // Chamfered bezel edge for smooth finger touch swipe
        translate([0, 0, bezel_t]) {
            hull() {
                cube([disp_view_w + 3.0, disp_view_h + 3.0, 0.01], center = true);
                translate([0, 0, -1.5])
                    cube([disp_view_w, disp_view_h, 0.01], center = true);
            }
        }

        // 2. Rear LCD Glass Panel Seating Recess
        translate([0, 0, -0.1]) {
            cube([disp_glass_w, disp_glass_h, disp_recess_d + 0.1], center = true);
        }

        // 3. WS2812 Beacon Light Guide (Conical Light Aperture)
        translate([led_x, led_y, -0.1]) {
            cylinder(d1 = led_cone_d, d2 = led_aperture_d, h = bezel_t + 0.2);
        }

        // 4. Corner M3 Counterbored Screw Holes
        for (x = [-screw_dist_x/2, screw_dist_x/2]) {
            for (y = [-screw_dist_y/2, screw_dist_y/2]) {
                translate([x, y, -0.1]) {
                    // Screw shank through hole
                    cylinder(d = screw_hole_d, h = bezel_t + 0.2);
                    // Screw head counterbore recess
                    translate([0, 0, bezel_t - screw_head_h + 0.1])
                        cylinder(d = screw_head_d, h = screw_head_h + 0.2);
                }
            }
        }
    }
}

// ============================================================================
// MODULE: REAR ENCLOSURE
// ============================================================================
module rear_enclosure() {
    difference() {
        union() {
            // Main Outer Rounded Body
            hull() {
                for (x = [-outer_w/2 + 4, outer_w/2 - 4]) {
                    for (y = [-outer_h/2 + 4, outer_h/2 - 4]) {
                        translate([x, y, 0])
                            cylinder(r = 4, h = rear_depth);
                    }
                }
            }

            // Standoff Bosses for PCB Mounting
            for (x = [-screw_dist_x/2, screw_dist_x/2]) {
                for (y = [-screw_dist_y/2, screw_dist_y/2]) {
                    translate([x, y, floor_t])
                        cylinder(d = boss_d, h = standoff_h);
                }
            }

            // GPS Module Internal Retention Brackets
            translate([0, 5, floor_t]) {
                // Bracket walls holding GY-GPS6MV2
                difference() {
                    cube([gps_w + 3.0, gps_h + 3.0, gps_d], center = true);
                    cube([gps_w, gps_h, gps_d + 0.2], center = true);
                }
            }
        }

        // 1. Hollow Inner PCB Cavity
        translate([0, 0, floor_t]) {
            hull() {
                for (x = [-(pcb_w + clearance*2)/2 + 2, (pcb_w + clearance*2)/2 - 2]) {
                    for (y = [-(pcb_h + clearance*2)/2 + 2, (pcb_h + clearance*2)/2 - 2]) {
                        translate([x, y, 0])
                            cylinder(r = 2, h = rear_depth + 1);
                    }
                }
            }
        }

        // 2. M3 Screw Pilot Holes in Standoff Bosses
        for (x = [-screw_dist_x/2, screw_dist_x/2]) {
            for (y = [-screw_dist_y/2, screw_dist_y/2]) {
                translate([x, y, floor_t - 0.5])
                    cylinder(d = boss_pilot_d, h = standoff_h + 1.0);
            }
        }

        // 3. USB-C Power Connector Cutout (Left Side Wall)
        translate([-outer_w/2, 0, floor_t + standoff_h + pcb_t/2 + 3]) {
            hull() {
                translate([0, -5, 0]) rotate([0, 90, 0]) cylinder(d = 6.5, h = wall_t * 2, center = true);
                translate([0,  5, 0]) rotate([0, 90, 0]) cylinder(d = 6.5, h = wall_t * 2, center = true);
            }
        }

        // 4. MicroSD Card Access Slot (Right Side Wall)
        translate([outer_w/2, -4, floor_t + standoff_h + pcb_t + 1]) {
            cube([wall_t * 2, 13.0, 3.2], center = true);
        }

        // 5. Hardware BOOT / RESET Pinhole Access
        translate([0, -18, -0.1])
            cylinder(d = 2.5, h = floor_t + 0.2); // BOOT Button
        translate([-20, -18, -0.1])
            cylinder(d = 2.5, h = floor_t + 0.2); // RESET Button

        // 6. Tactical Airflow Cooling Louvers (Rear Backplate)
        for (i = [-2 : 2]) {
            translate([i * 12, -8, -0.1]) {
                cube([2.2, 22.0, floor_t + 0.2], center = true);
            }
        }

        // 7. GPS Wiring Passthrough Channel
        translate([0, -15, floor_t + 1]) {
            cube([10.0, 6.0, 4.0], center = true);
        }
    }
}

// ============================================================================
// MODULE: DESKTOP STAND (25-DEGREE ERGONOMIC CRADLE)
// ============================================================================
module desk_stand_25deg() {
    stand_w = 74.0;
    stand_d = 65.0;
    stand_h = 28.0;

    difference() {
        // Main Wedge Body
        translate([-stand_w/2, -stand_d/2, 0]) {
            cube([stand_w, stand_d, stand_h]);
        }

        // 25-Degree Tilted Enclosure Resting Cradle Pocket
        translate([0, 8, 12]) {
            rotate([stand_tilt_deg, 0, 0]) {
                // Receptive pocket for rear enclosure
                cube([outer_w + 1.2, outer_h + 1.2, 35.0], center = true);
            }
        }

        // Angled Top Slice
        translate([0, -stand_d/2, stand_h + 10]) {
            rotate([stand_tilt_deg - 5, 0, 0])
                cube([stand_w + 10, stand_d * 2, 20], center = true);
        }

        // Rear USB-C Cable Route Arch
        translate([0, -stand_d/2 + 10, -0.1]) {
            cylinder(d = 14.0, h = stand_h + 1);
            translate([-7, -15, 0])
                cube([14.0, 20.0, stand_h + 1]);
        }

        // Under-Desk Anti-Slip Rubber Bumper Pockets (4 Corners)
        for (x = [-stand_w/2 + 8, stand_w/2 - 8]) {
            for (y = [-stand_d/2 + 8, stand_d/2 - 8]) {
                translate([x, y, -0.1])
                    cylinder(d = 8.5, h = 1.6);
            }
        }
    }
}

// ============================================================================
// MODULE: ASSEMBLY VIEW
// ============================================================================
module assembly_view(exploded = false) {
    bezel_z = exploded ? 40 : rear_depth;
    pcb_z   = exploded ? 20 : floor_t + standoff_h;
    gps_z   = exploded ? 10 : floor_t + 2;
    stand_z = exploded ? -30 : -10;

    // 1. Rear Enclosure (Dark Gunmetal Tactical Shell)
    color([0.18, 0.20, 0.22, 1.0])
        rear_enclosure();

    // 2. GY-GPS6MV2 Module (Blue PCB + Golden Ceramic Antenna)
    translate([0, 5, gps_z + 4]) {
        color([0.1, 0.3, 0.8, 0.9])
            cube([25.0, 35.0, 1.6], center = true);
        translate([0, 0, 3.5])
            color([0.85, 0.70, 0.25, 1.0])
                cube([25.0, 25.0, 4.0], center = true); // Ceramic Patch Antenna
    }

    // 3. NM-CYD-C5 Board (Green PCB with 2.8" LCD Screen)
    translate([0, 0, pcb_z]) {
        // PCB Base
        color([0.1, 0.5, 0.2, 0.9])
            cube([pcb_w, pcb_h, pcb_t], center = true);
        // LCD Module Glass
        translate([0, 0, 2.5])
            color([0.05, 0.1, 0.2, 0.95])
                cube([disp_glass_w, disp_glass_h, 2.8], center = true);
        // Active Screen Illuminated Image
        translate([0, 0, 4.0])
            color([0.0, 0.7, 0.9, 0.9])
                cube([disp_view_w, disp_view_h, 0.2], center = true);
        // WS2812 RGB LED Indicator
        translate([led_x, led_y, 1.5])
            color([0.0, 1.0, 0.8, 1.0])
                cylinder(d = 2.5, h = 1.0, center = true);
    }

    // 4. Front Bezel Faceplate (Tactical Matte Carbon / Dark Slate)
    translate([0, 0, bezel_z])
        color([0.28, 0.30, 0.34, 0.95])
            front_bezel();

    // 5. Desktop Angled Cradle Stand
    translate([0, -8, stand_z])
        color([0.12, 0.14, 0.16, 0.95])
            desk_stand_25deg();
}
