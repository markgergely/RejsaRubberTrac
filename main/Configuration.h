#include "Constants.h"
#define WHEELPOS 0        // DEFAULT is 7
                          // 7 = "RejsaRubber" + four last bytes from the bluetooth MAC address

                          // 0 = "RejsaRubberFL" + three last bytes from the bluetooth MAC address
                          // 1 = "RejsaRubberFR" + three last bytes from the bluetooth MAC address
                          // 2 = "RejsaRubberRL" + three last bytes from the bluetooth MAC address
                          // 3 = "RejsaRubberRR" + three last bytes from the bluetooth MAC address
                          // 5 = "RejsaRubberF" + one space + three last bytes from the bluetooth MAC address
                          // 6 = "RejsaRubberR" + one space + three last bytes from the bluetooth MAC address
                        
                          // NOTE!!! THIS CAN BE OVERRIDDEN WITH HARDWARE CODING WITH GPIO PINS

                    
#define MIRRORTIRE 0      // 0 = default
                          // 1 = Mirror the tire, making the outside edge temps the inside edge temps

                          // NOTE!!! THIS CAN BE OVERRIDDEN WITH HARDWARE CODING WITH A GPIO PIN
                          


#define DISTANCEOFFSET 0         // Write distance to tire in mm here to get logged distance data value centered around zero
                                 // If you leave this value here at 0 the distance value in the logs will always be positive numbers

// -- Far Infrared Sensor related settings

#define TEMPSCALING        1.00  // Default = 1.00
#define TEMPOFFSET         0     // Default = 0      NOTE: in TENTHS of degrees Celsius --> TEMPOFFSET 10 --> 1 degree

#define FIS_SENSOR         FIS_MLX90640  // Device to use, see Constants.h                        
#define FIS_REFRESHRATE    4     // Sets the FIS refresh rate in Hz, MLX90640 should be 4 with nRF52, MLX90621 works at 16Hz

#define IGNORE_TOP_ROWS    1     // Ignore this many rows from the top of the sensor
#define IGNORE_BOTTOM_ROWS 1     // Ignore this many rows from the bottom of the sensor

#define COLUMN_AGGREGATE   COLUMN_AGGREGATE_MAX // Set column aggregation algorhytm, see Constants.h

#define SERIAL_UPDATERATE  1
#define DEBUG                    // Set debug mode, results in more verbose output on serial port

// -- Dynamically calculated configuration values

#if FIS_SENSOR == FIS_MLX90621
  #define FIS_X           16  // Far Infrared Sensor columns
  #define FIS_Y            4  // Far Infrared Sensor rows
#elif FIS_SENSOR == FIS_MLX90640
  #define FIS_X           32
  #define FIS_Y           24
#elif FIS_SENSOR == FIS_DUMMY
  #define FIS_X           16
  #define FIS_X            4
#endif

#define EFFECTIVE_ROWS ( FIS_Y - IGNORE_TOP_ROWS - IGNORE_BOTTOM_ROWS )
