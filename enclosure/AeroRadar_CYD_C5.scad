// ============================================================================
// CYD ESP32-C5 FlightRadar & Aerospace Instrument - 3D Printable Enclosure
// Compatible with: RockBase NM-CYD-C5 (ESP32-C5 2.8" TFT Touch) + GY-GPS6MV2
// Features Dedicated Internal Compartments for BOTH GPS Receiver & Patch Antenna!
// Designed for 3D Printing: FDM (PLA/PETG/ABS) or Resin (SLA)
// ============================================================================

/* [Model View Selection] */
// Select which component to preview or render
part_to_show = "rear_backpack"; // [assembly:Full Assembly, bezel:Standard Front Bezel, rear_backpack:Rear Enclosure with GPS Backpack, stand:Desk Stand 25deg, bezel_toppod:Top-Pod Bezel, rear_toppod:Top-Pod Rear Enclosure]

/* [Print & Fit Tolerances] */
nozzle_dia      = 0.4;  // Nozzle diameter (mm)
clearance       = 0.5;  // General clearance for slide/drop-in fit (mm)
wall_t          = 2.4;  // Enclosure wall thickness (mm)
floor_t         = 2.0;  // Rear enclosure floor thickness (mm)
bezel_t         = 4.0;  // Front bezel thickness (mm)

/* [Hardware Dimensions - Exact RockBase NM-CYD-C5 Specifications] */
pcb_w           = 86.01; // NM-CYD-C5 PCB width (mm, from EasyEDA 3D STEP)
pcb_h           = 60.0;  // NM-CYD-C5 PCB height (mm, 59.98mm actual)
pcb_t           = 1.6;   // PCB thickness (mm)
standoff_h      = 11.0;  // Standoff height off rear floor for 11mm basement (mm)

// Corner Mounting Holes (M3 / 78.0mm x 42.0mm spacing, symmetric in X, offset in Y)
screw_dist_x    = 78.0;  // Hole center spacing X (4.0mm to 82.0mm)
screw_dist_y    = 42.0;  // Hole center spacing Y (14.0mm to 56.0mm)
screw_hole_d    = 3.4;   // M3 clearance hole (mm)
screw_head_d    = 6.2;   // M3 socket head counterbore diameter (mm)
screw_head_h    = 2.0;   // M3 counterbore depth (mm)
boss_d          = 7.0;   // Standoff boss outer diameter (mm)
boss_pilot_d    = 2.8;   // Standoff pilot hole for self-tapping M3 (mm)

/* [Display Cutout - Centered with LCD Glass & Screen at Y = +5.0mm] */
disp_center_y   = 5.0;   // Screen center offset in Y from PCB center
disp_view_w     = 62.0;  // Viewable LCD window width (mm)
disp_view_h     = 44.0;  // Viewable LCD window height (mm)
disp_glass_w    = 69.0;  // Glass panel outer width (mm)
disp_glass_h    = 50.0;  // Glass panel outer height (mm)
disp_recess_d   = 2.0;   // Glass panel recess depth (mm)

/* [WS2812 Indicator Beacon] */
led_x           = -28.0;// LED X offset from center (mm)
led_y           = 22.5; // LED Y offset from center (mm)
led_aperture_d  = 3.2;  // Outer light port diameter (mm)
led_cone_d      = 5.0;  // Inner light funnel diameter (mm)

/* [GY-GPS6MV2 Receiver & Ceramic Antenna Chambers] */
gps_rx_w        = 26.5; // Receiver module bay width (mm)
gps_rx_h        = 36.5; // Receiver module bay length (mm)
gps_ant_w       = 26.0; // Ceramic antenna pocket width (mm)
gps_ant_h       = 26.0; // Ceramic antenna pocket length (mm)
gps_ant_d       = 9.0;  // Ceramic antenna pocket depth (mm)

/* [GPS Backpack Dimensions] */
bp_w            = 64.0; // Backpack outer width (mm)
bp_h            = 44.0; // Backpack outer height (mm)
bp_d            = 14.0; // Backpack depth extending behind shell (mm)

/* [Desk Stand Angle] */
stand_tilt_deg  = 25.0; // Desktop viewing tilt angle (degrees)

// Calculated bounding parameters
outer_w         = pcb_w + (wall_t * 2) + (clearance * 2); // 91.8 -> 93.0 mm
outer_h         = pcb_h + (wall_t * 2) + (clearance * 2); // 55.8 -> 57.0 mm
rear_depth      = 19.0; // Main enclosure depth (mm)

$fn = 40; // Circle facet resolution

// ============================================================================
// TOP-LEVEL SELECTION
// ============================================================================
if (part_to_show == "assembly") {
    assembly_view();
} else if (part_to_show == "bezel") {
    front_bezel();
} else if (part_to_show == "rear_backpack") {
    rear_enclosure_backpack();
} else if (part_to_show == "stand") {
    desk_stand_25deg();
} else if (part_to_show == "bezel_toppod") {
    front_bezel_toppod();
} else if (part_to_show == "rear_toppod") {
    rear_enclosure_toppod();
}

// ============================================================================
// MODULE: FRONT BEZEL (Standard 93x57mm)
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

        // 3. WS2812 Beacon Light Guide
        translate([led_x, led_y, -0.1]) {
            cylinder(d1 = led_cone_d, d2 = led_aperture_d, h = bezel_t + 0.2);
        }

        // 4. Corner M3 Counterbored Screw Holes
        for (x = [-screw_dist_x/2, screw_dist_x/2]) {
            for (y = [-screw_dist_y/2, screw_dist_y/2]) {
                translate([x, y, -0.1]) {
                    cylinder(d = screw_hole_d, h = bezel_t + 0.2);
                    translate([0, 0, bezel_t - screw_head_h + 0.1])
                        cylinder(d = screw_head_d, h = screw_head_h + 0.2);
                }
            }
        }
    }
}

// ============================================================================
// MODULE: REAR ENCLOSURE (With Integrated GPS Backpack & Antenna Tray)
// ============================================================================
module rear_enclosure_backpack() {
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

            // GPS Backpack Outer Extension (Extending backwards)
            translate([-bp_w/2, -16.0, -bp_d]) {
                hull() {
                    for (x = [3, bp_w - 3]) {
                        for (y = [3, bp_h - 3]) {
                            translate([x, y, 0])
                                cylinder(r = 3, h = bp_d + 1);
                        }
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

            // Internal Antenna Retention Lips
            translate([15.0, 13.0, -bp_d + 2.0]) {
                difference() {
                    cube([gps_ant_w + 3.0, gps_ant_h + 3.0, gps_ant_d], center = true);
                    cube([gps_ant_w, gps_ant_h, gps_ant_d + 1], center = true);
                }
            }

            // Internal GPS Receiver Board Rails
            translate([-16.0, 5.0, -bp_d + 2.0]) {
                difference() {
                    cube([gps_rx_w + 3.0, gps_rx_h + 3.0, 7.0], center = true);
                    cube([gps_rx_w, gps_rx_h, 8.0], center = true);
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

        // 2. Hollow GPS Backpack Cavity
        translate([0, 6.0, -bp_d + 2.0]) {
            cube([bp_w - 4.4, bp_h - 4.4, bp_d + 2.0], center = true);
        }

        // 3. M3 Screw Pilot Holes in Standoff Bosses
        for (x = [-screw_dist_x/2, screw_dist_x/2]) {
            for (y = [-screw_dist_y/2, screw_dist_y/2]) {
                translate([x, y, floor_t - 0.5])
                    cylinder(d = boss_pilot_d, h = standoff_h + 1.0);
            }
        }

        // 4. USB-C Power Connector Cutout (Left Side Wall)
        translate([-outer_w/2, 0, floor_t + standoff_h + pcb_t/2 + 3]) {
            hull() {
                translate([0, -5, 0]) rotate([0, 90, 0]) cylinder(d = 6.5, h = wall_t * 2, center = true);
                translate([0,  5, 0]) rotate([0, 90, 0]) cylinder(d = 6.5, h = wall_t * 2, center = true);
            }
        }

        // 5. MicroSD Card Access Slot (Right Side Wall)
        translate([outer_w/2, -4, floor_t + standoff_h + pcb_t + 1]) {
            cube([wall_t * 2, 13.0, 3.2], center = true);
        }

        // 6. Direct Wire Routing Aperture from GPS to P5 LP-UART Header
        translate([-14.0, 19.0, -2.0]) {
            cube([16.0, 10.0, floor_t + 4.0], center = true);
        }

        // 7. Tactical Airflow Cooling Louvers on Backpack Base
        for (i = [-1 : 1]) {
            translate([i * 12, -6, -bp_d - 0.1]) {
                cube([2.2, 16.0, 3.0], center = true);
            }
        }
    }
}

// ============================================================================
// MODULE: DESKTOP STAND (25-DEGREE ERGONOMIC CRADLE)
// ============================================================================
module desk_stand_25deg() {
    stand_w = 78.0;
    stand_d = 72.0;
    stand_h = 32.0;

    difference() {
        // Main Base Block
        translate([-stand_w/2, -stand_d/2, 0]) {
            cube([stand_w, stand_d, stand_h]);
        }

        // 25-Degree Tilted Enclosure Resting Cradle Pocket
        translate([0, 10, 14]) {
            rotate([stand_tilt_deg, 0, 0]) {
                cube([outer_w + 1.4, outer_h + 1.4, 40.0], center = true);
            }
        }

        // Central Clearance Recess for GPS Backpack
        translate([0, -2, stand_h/2]) {
            cube([bp_w + 2.0, stand_d, stand_h + 2], center = true);
        }

        // Rear USB-C Cable Route Arch
        translate([0, -stand_d/2 + 10, -0.1]) {
            cylinder(d = 16.0, h = stand_h + 1);
            translate([-8, -15, 0])
                cube([16.0, 20.0, stand_h + 1]);
        }

        // Underside Rubber Foot Sockets
        for (fx = [-stand_w/2 + 8, stand_w/2 - 8]) {
            for (fy = [-stand_d/2 + 8, stand_d/2 - 8]) {
                translate([fx, fy, -0.1])
                    cylinder(d = 8.5, h = 1.8);
            }
        }
    }
}

// ============================================================================
// MODULE: TACTICAL AVIONICS TOP-POD BEZEL (93x82mm with Radome Brow)
// ============================================================================
module front_bezel_toppod() {
    top_h = 82.0;
    difference() {
        hull() {
            for (x = [-outer_w/2 + 4, outer_w/2 - 4]) {
                for (y = [-top_h/2 + 4, top_h/2 - 4]) {
                    translate([x, y, 0]) cylinder(r = 4, h = bezel_t);
                }
            }
        }
        // LCD window centered in lower 57mm
        translate([0, -12.5, -0.1]) cube([disp_view_w, disp_view_h, bezel_t + 0.2], center = true);
        translate([0, -12.5, -0.1]) cube([disp_glass_w, disp_glass_h, disp_recess_d + 0.1], center = true);
        translate([led_x, -12.5 + 22.5, -0.1]) cylinder(d1 = 5.0, d2 = 3.2, h = bezel_t + 0.2);
        for (x = [-screw_dist_x/2, screw_dist_x/2]) {
            for (y = [-12.5 - screw_dist_y/2, -12.5 + screw_dist_y/2]) {
                translate([x, y, -0.1]) {
                    cylinder(d = 3.2, h = bezel_t + 0.2);
                    translate([0, 0, bezel_t - 2.0]) cylinder(d = 6.0, h = 2.2);
                }
            }
        }
    }
}

// ============================================================================
// MODULE: TACTICAL AVIONICS TOP-POD REAR ENCLOSURE
// ============================================================================
module rear_enclosure_toppod() {
    top_h = 82.0;
    difference() {
        union() {
            hull() {
                for (x = [-outer_w/2 + 4, outer_w/2 - 4]) {
                    for (y = [-top_h/2 + 4, top_h/2 - 4]) {
                        translate([x, y, 0]) cylinder(r = 4, h = 20.0);
                    }
                }
            }
            // Standoffs for lower PCB
            for (x = [-screw_dist_x/2, screw_dist_x/2]) {
                for (y = [-12.5 - screw_dist_y/2, -12.5 + screw_dist_y/2]) {
                    translate([x, y, floor_t]) cylinder(d = boss_d, h = standoff_h);
                }
            }
            // Top antenna tray
            translate([15.0, 24.0, floor_t]) {
                difference() {
                    cube([gps_ant_w + 3.0, gps_ant_h + 3.0, gps_ant_d], center = true);
                    cube([gps_ant_w, gps_ant_h, gps_ant_d + 1], center = true);
                }
            }
            // Top receiver PCB slot
            translate([-16.0, 24.0, floor_t]) {
                difference() {
                    cube([gps_rx_w + 3.0, gps_rx_h + 3.0, 8.0], center = true);
                    cube([gps_rx_w, gps_rx_h, 9.0], center = true);
                }
            }
        }
        // Hollow main cavity
        translate([0, 0, floor_t]) {
            hull() {
                for (x = [-outer_w/2 + 3, outer_w/2 - 3]) {
                    for (y = [-top_h/2 + 3, top_h/2 - 3]) {
                        translate([x, y, 0]) cylinder(r = 2, h = 22.0);
                    }
                }
            }
        }
        // Ports & screw holes
        for (x = [-screw_dist_x/2, screw_dist_x/2]) {
            for (y = [-12.5 - screw_dist_y/2, -12.5 + screw_dist_y/2]) {
                translate([x, y, floor_t - 0.5]) cylinder(d = boss_pilot_d, h = standoff_h + 1.0);
            }
        }
        // USB-C on left wall
        translate([-outer_w/2, -12.5, floor_t + standoff_h + pcb_t/2 + 3])
            cube([wall_t * 2, 13.0, 7.0], center = true);
    }
}

// ============================================================================
// ASSEMBLY VIEW HELPER
// ============================================================================
module assembly_view() {
    color([0.2, 0.2, 0.25, 0.9])
        rear_enclosure_backpack();
    color([0.3, 0.3, 0.35, 0.8])
        translate([0, 0, rear_depth + 4])
            front_bezel();
}
