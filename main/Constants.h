// FIS identifiers
#define FIS_DUMMY    0
#define FIS_MLX90640 1
#define FIS_MLX90621 2

// Distance sensor identifiers
#define DIST_NONE    0
#define DIST_VL53L0X 1

// Boards
#define BOARD_NRF52 0
#define BOARD_ESP32 1
#define BOARD_LOLIND32 2

// Column aggregation algo identifiers
#define COLUMN_AGGREGATE_MAX 0
#define COLUMN_AGGREGATE_AVG 1

// Voltage Measurement related items

#ifdef _DEBUG
  #define debug(fmt, ...) { Serial.printf(fmt,## __VA_ARGS__); }
#else
  #define debug(fmt, ...) {} // Silence is golden
#endif
