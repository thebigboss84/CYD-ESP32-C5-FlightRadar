#pragma once

#include <Arduino.h>

// ============================================================================
// HARDWARE PIN DEFINITIONS - RockBase NM-CYD-C5 (ESP32-C5)
// ============================================================================
#define TFT_SCLK    6
#define TFT_MOSI    7
#define TFT_MISO    2
#define TFT_CS     23
#define TFT_DC     24
#define TFT_RST    -1
#define TFT_BL     25

#define SDCARD_CS  10

#define TOUCH_CS    1
#define TOUCH_CLK   6
#define TOUCH_MOSI  7
#define TOUCH_MISO  2
#define TOUCH_IRQ  -1

#define LED_PIN    27
#ifndef BOARD_BOOT_PIN
#define BOARD_BOOT_PIN 28
#endif

// Hardware LP-UART (P5) dedicated for GPS on NM-CYD-C5
#define GPS_RX_PIN    4
#define GPS_TX_PIN    5
#define GPS_BAUD      9600

// Touchscreen Calibration for 320x240 landscape on NM-CYD-C5
#define TOUCH_MIN_X 185
#define TOUCH_MAX_X 3700
#define TOUCH_MIN_Y 250
#define TOUCH_MAX_Y 3800

// ============================================================================
// SCREEN & LAYOUT DIMENSIONS (320 x 240 Landscape)
// ============================================================================
#define SCREEN_W     320
#define SCREEN_H     240
#define HEADER_H      22
#define FOOTER_H      24
#define CONTENT_Y    HEADER_H
#define CONTENT_H    (SCREEN_H - HEADER_H - FOOTER_H)  // 194px
#define CONTENT_CX   (SCREEN_W / 2)                    // 160px
#define CONTENT_CY   (CONTENT_Y + CONTENT_H / 2)       // 119px

#define RADAR_RADIUS 90                                // Scope radius in pixels

// ============================================================================
// COLOR PALETTE (RGB565)
// ============================================================================
#define COL_BLACK        0x0000
#define COL_WHITE        0xFFFF
#define COL_BG_DARK      0x0002  // Deep dark aerospace navy
#define COL_HEADER_BG    0x0861  // Slate dark header
#define COL_FOOTER_BG    0x0842  // Slate dark footer
#define COL_BTN_INACTIVE 0x18C3  // Inactive tab
#define COL_BTN_ACTIVE   0x03FF  // Bright cyan active tab
#define COL_BTN_TEXT     0xAD55  // Muted gray/cyan text

#define COL_CYAN         0x07FF  // High altitude flights / alerts
#define COL_GREEN        0x07E0  // Radar rings / OK status
#define COL_YELLOW       0xFFE0  // Low altitude flights / weather
#define COL_ORANGE       0xFD20  // SpaceX / Warnings
#define COL_RED          0xF800  // Ground / Critical
#define COL_GRAY         0x5AEB  // Secondary / borders
#define COL_DIM_GRAY     0x2945  // Faint lines / scope background
#define COL_PURPLE       0xA254  // ISS accent

// Altitude colors
#define COL_AC_HIGH      COL_CYAN    // > 10,000 ft
#define COL_AC_LOW       COL_YELLOW  // <= 10,000 ft
#define COL_AC_GND       0x7BEF      // On ground

// ============================================================================
// APP MODES
// ============================================================================
enum AppMode {
  MODE_RADAR = 0,
  MODE_FLIGHT_LIST,
  MODE_WEATHER,
  MODE_ISS,
  MODE_SPACEX,
  MODE_CITY,
  MODE_COUNT
};

enum TrafficFilter {
  FILTER_ALL = 0,
  FILTER_MILITARY,
  FILTER_HELO_POLICE,
  FILTER_COUNT
};

// ============================================================================
// DATA STRUCTURES
// ============================================================================
#define MAX_TRACKED_FLIGHTS 30
#define LIST_VISIBLE_ROWS    7
#define LIST_ROW_HEIGHT     22

struct FlightRecord {
  char  icao[8];
  char  callsign[10];
  char  country[20];
  float lat;
  float lon;
  float alt_m;
  float vel_ms;
  float track;
  bool  on_ground;
  float dist_km;
  float bearing;
  char  origin_code[6];   // e.g. "FAT"
  char  origin_city[20];  // e.g. "Fresno"
  char  dest_code[6];     // e.g. "GDL"
  char  dest_city[20];    // e.g. "Guadalajara"
  char  origin_name[36];  // e.g. "Fresno Yosemite"
  char  dest_name[36];    // e.g. "Guadalajara Int'l"
  char  aircraft[8];      // e.g. "A320"
  char  airline[24];      // e.g. "Volaris"
  char  squawk[6];        // e.g. "7700", "1200"
  bool  is_emergency;     // true if 7700, 7600, 7500
  int   emergency_code;   // 7700, 7600, 7500
  bool  is_military;      // true if USAF, USN, C17, etc.
  bool  is_helo_police;   // true if police, fire, medical, rotorcraft
  float vertical_rate_ms; // m/s (+ climb, - descent)
  bool  route_fetched;
};

struct WeatherRecord {
  bool  valid;
  float temp_c;
  float feels_like_c;
  int   humidity;
  float wind_speed_kmh;
  float wind_dir_deg;
  int   weather_code;
  float daily_max[4];
  float daily_min[4];
  int   daily_code[4];
  char  daily_day[4][4];
};

struct IssRecord {
  bool  valid;
  float lat;
  float lon;
  float alt_km;
  float velocity_kmh;
  bool  daylight;
  float dist_km;
  float bearing;
  unsigned long timestamp;
};

struct SpaceXRecord {
  bool    valid;
  char    mission_name[48];
  char    rocket_name[24];
  char    pad_name[36];
  char    location_name[36];  // e.g. "Vandenberg SFB, CA"
  float   pad_lat;
  float   pad_lon;
  float   dist_km;            // Distance from user location
  float   bearing;            // Bearing from user location
  bool    visible_in_sky;     // Line-of-sight visible (< 500 km)
  char    status_name[24];
  char    net_iso[28];
  int64_t launch_epoch_utc;
  char    details[96];
};

struct GpsData {
  bool  has_fix;
  float lat;
  float lon;
  float alt_m;
  int   satellites;
  float speed_kmh;
  float hdop;
  unsigned long last_fix_ms;
};

struct CityPreset {
  const char *name;
  float lat;
  float lon;
  const char *tz;
  const char *subtitle;
};

// Predefined major aerospace & global cities with POSIX timezones
static const CityPreset CITY_PRESETS[] = {
  { "Los Angeles",   34.0522f, -118.2437f, "PST8PDT,M3.2.0,M11.1.0", "USA / SoCal" },
  { "Cape Canaveral",28.3922f,  -80.6077f, "EST5EDT,M3.2.0,M11.1.0", "SpaceX Hub" },
  { "San Francisco", 37.7749f, -122.4194f, "PST8PDT,M3.2.0,M11.1.0", "USA / NorCal" },
  { "New York",      40.7128f,  -74.0060f, "EST5EDT,M3.2.0,M11.1.0", "JFK / LGA" },
  { "London",        51.5074f,   -0.1278f, "GMT0BST,M3.5.0/1,M10.5.0", "UK / Heathrow" },
  { "Paris",         48.8566f,    2.3522f, "CET-1CEST,M3.5.0,M10.5.0/3", "France / CDG" },
  { "Tokyo",         35.6762f,  139.6503f, "JST-9",                   "Japan / Haneda" },
  { "Dubai",         25.2048f,   55.2708f, "GST-4",                   "UAE / Emirates" }
};

#define CITY_PRESETS_COUNT (sizeof(CITY_PRESETS) / sizeof(CITY_PRESETS[0]))

// ============================================================================
// TIMING CONSTANTS (Milliseconds)
// ============================================================================
#define OPENSKY_REFRESH_MS   (4UL * 60UL * 1000UL)     // 4 minutes
#define WEATHER_REFRESH_MS   (15UL * 60UL * 1000UL)    // 15 minutes
#define ISS_REFRESH_MS       (10UL * 1000UL)           // 10 seconds
#define SPACEX_REFRESH_MS    (30UL * 60UL * 1000UL)    // 30 minutes
#define CLOCK_REFRESH_MS     (1000UL)                  // 1 second
#define SWEEP_ANIM_MS        (80UL)                    // Paced radar sweep animation frame
#define EXTRAPOLATE_MS       (2000UL)                  // Aircraft dead-reckoning movement interval
