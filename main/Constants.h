// FIS identifiers
#define FIS_MLX90621 0
#define FIS_MLX90640 1
#define FIS_DUMMY    2

// Distance sensor identifiers
#define DIST_VL53L0X 0
#define DIST_NONE    1

// Display device
#define DISP_128X32 0
#define DISP_NONE   1

// Boards
#define BOARD_NRF52 0
#define BOARD_ESP32 1

// Column aggregation algo identifiers
#define COLUMN_AGGREGATE_MAX 0
#define COLUMN_AGGREGATE_AVG 1

// PIN Assignments 
#define GPIODISTSENSORXSHUT 12  // GPIO pin number
#define GPIOCAR    28           // GPIO pin number
#define GPIOFRONT  29           // GPIO pin number
#define GPIOLEFT   13           // GPIO pin number
#define GPIOMIRR   14           // GPIO pin number

// Voltage Measurement related items

#ifdef _DEBUG
  #define debug(fmt, ...) { Serial.printf(fmt,## __VA_ARGS__); }
#else
  #define debug(fmt, ...) {} // Silence is golden
#endif
