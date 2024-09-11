// File: dewi.h
// Description: Fucntions and constants for used for the DeWi demo

// Uncomment below to disable the display
// #define DISABLE_DISPLAY

// LCD hd44780 I2C pins and settings
#define LCD_ADDR 0x27
#define SDA_PIN  22
#define SCL_PIN  21
#define LCD_COLS 20
#define LCD_ROWS 4

// The buffer size for storing the HTTP response
#define MAX_HTTP_OUTPUT_BUFFER 2048

// A struct to hold the information of a connected client
typedef struct
{
    uint8_t mac[6]; // MAC address
    uint64_t connect_time; // Connection time (in microseconds)
} connected_device_t;

// The maximum number of clients than can be connected at once
#define MAX_CONNECTED_DEVICES 10

extern const char* ESP32_SSID;
extern connected_device_t connected_devices[MAX_CONNECTED_DEVICES];
extern int num_connected_devices;

// ------------------- Functions -------------------

/**
 * @brief Start the DeWi demo
 */
void start_dewi_monitoring();

/**
 * @brief Get the current time in microseconds
 */
uint64_t get_current_time_us();
