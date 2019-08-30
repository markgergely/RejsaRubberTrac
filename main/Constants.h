// Distance Sensor identifiers
#define FIS_MLX90621 0
#define FIS_MLX90640 1
#define FIS_DUMMY    2

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
#define VBAT_PIN A7

#define MILLIVOLTFULLSCALE 3600
#define STEPSFULLSCALE     1024
#define BATRESISTORCOMP       1.403 // Compensation for a resistor voltage divider between battery and ADC input pin
#define BATTERY_UPDATERATE   60     // Update rate for the battery

#ifdef DEBUG
  #define debug(fmt, ...) { Serial.printf(fmt,## __VA_ARGS__); }
#else
  #define debug(fmt, ...) {} // Silence is golden
#endif
