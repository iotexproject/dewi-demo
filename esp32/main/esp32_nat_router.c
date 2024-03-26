/* Console example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include "esp_system.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_vfs_dev.h"
#include "driver/uart.h"
#include "esp_vfs_usb_serial_jtag.h"
#include "driver/usb_serial_jtag.h"
#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"
#include "esp_vfs_fat.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_eap_client.h"
#include "esp_tls.h"


#include "lwip/opt.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "dhcpserver/dhcpserver.h"
#include "dhcpserver/dhcpserver_options.h"

#include "cmd_decl.h"
#include <esp_http_server.h>

#if !IP_NAPT
#error "IP_NAPT must be defined"
#endif
#include "lwip/lwip_napt.h"

#include "router_globals.h"

// On board LED
#if defined(CONFIG_IDF_TARGET_ESP32S3)
#define BLINK_GPIO 44
#else
#define BLINK_GPIO 2
#endif

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about one event
 * - are we connected to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;

#define DEFAULT_AP_IP "192.168.4.1"
#define DEFAULT_DNS "8.8.8.8"

/* Global vars */
uint16_t connect_count = 0;
bool ap_connect = false;
bool has_static_ip = false;

uint32_t my_ip;
uint32_t my_ap_ip;

struct portmap_table_entry {
  u32_t daddr;
  u16_t mport;
  u16_t dport;
  u8_t proto;
  u8_t valid;
};
struct portmap_table_entry portmap_tab[IP_PORTMAP_MAX];

esp_netif_t* wifiAP;
esp_netif_t* wifiSTA;

httpd_handle_t start_webserver(void);

static const char *TAG = "ESP32 NAT router";

/* Console command history can be stored to and loaded from a file.
 * The easiest way to do this is to use FATFS filesystem on top of
 * wear_levelling library.
 */
#if CONFIG_STORE_HISTORY

#define MOUNT_PATH "/data"
#define HISTORY_PATH MOUNT_PATH "/history.txt"

static void initialize_filesystem(void)
{
    static wl_handle_t wl_handle;
    const esp_vfs_fat_mount_config_t mount_config = {
            .max_files = 4,
            .format_if_mount_failed = true
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(MOUNT_PATH, "storage", &mount_config, &wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return;
    }
}
#endif // CONFIG_STORE_HISTORY




// ---------------------- START IoTeX code ----------------------

#include "esp_http_client.h"
#include "esp_log.h"
#include <sys/time.h> // Include for gettimeofday()
#include "hd44780_I2C.h"
//#define DISABLE_DISPLAY

const char* did_token = "eyJhbGciOiJFZERTQSIsImtpZCI6ImRpZDprZXk6ejZNa2pQMlBhMXBrVWd6MnJQNnlUWHBBVGU0cWQ3YWh3c0dBUXVVNjk3SnBjQ0xmI3o2TWtqUDJQYTFwa1VnejJyUDZ5VFhwQVRlNHFkN2Fod3NHQVF1VTY5N0pwY0NMZiJ9.eyJpc3MiOiJkaWQ6a2V5Ono2TWtqUDJQYTFwa1VnejJyUDZ5VFhwQVRlNHFkN2Fod3NHQVF1VTY5N0pwY0NMZiIsIm5iZiI6MTU5Nzg3MzMxMCwianRpIjoiaHR0cDovL2V4YW1wbGUub3JnL2NyZWRlbnRpYWxzLzM3MzEiLCJzdWIiOiJkaWQ6a2V5Ono2TWtlZUNoclVzMUVvS2tOTnpveTlGd0pKYjlnTlE5MlVUOGtjWFpITWJ3ajY3QiIsInZjIjp7IkBjb250ZXh0IjoiaHR0cHM6Ly93d3cudzMub3JnLzIwMTgvY3JlZGVudGlhbHMvdjEiLCJpZCI6Imh0dHA6Ly9leGFtcGxlLm9yZy9jcmVkZW50aWFscy8zNzMxIiwidHlwZSI6WyJWZXJpZmlhYmxlQ3JlZGVudGlhbCJdLCJjcmVkZW50aWFsU3ViamVjdCI6eyJpZCI6ImRpZDprZXk6ejZNa2VlQ2hyVXMxRW9La05Oem95OUZ3SkpiOWdOUTkyVVQ4a2NYWkhNYndqNjdCIn0sImlzc3VlciI6ImRpZDprZXk6ejZNa2pQMlBhMXBrVWd6MnJQNnlUWHBBVGU0cWQ3YWh3c0dBUXVVNjk3SnBjQ0xmIiwiaXNzdWFuY2VEYXRlIjoiMjAyMC0wOC0xOVQyMTo0MTo1MFoifX0.4dzsQ89P6A8NUy5qpdAohCbNRCRMnplrtiYaEpPvTeU9nzyrKGAPG-bQukI2Jzv2f289lWDx0fdWxWJjSgieDA";
const char* url = "http://37.27.64.79:9000/message";
//const char* url = "http://sprout-staging.w3bstream.com:9000/message";
const int project_id = 33;
const char* token_contract = "0xff94dea0be4fc5289cb60f63d55eaff71b3e9666";
//const char* token_contract = "0x6fbCdc1169B5130C59E72E51Ed68A84841C98cd1" // USDT;
//const char* owner = "0x3eF12589F004477d056Cd13222631A469969bf90";
const uint32_t update_interval_ms = 7000;

// Device info 0
const char* owner = "0x1435fc1a9170f15d708fb837d0f8b8f06e8f16e6";
const char* owner_short = "0x1435..f16e6";
const char* device_id = "0";
const char* ESP32_SSID = "DeWi-Device-0";

/* // Device info 1
const char* owner = "0xc7c415f50829c1f696fb7c16df3635262bf99193";
const char* owner_short = "0xc7c41..99193";
const char* device_id = "1";
const char* ESP32_SSID = "DeWi-Device-1"; */

// Device info 2
/* const char* owner = "0x09bb7706adaf412f17da5ab61036df966d96413c";
const char* owner_short = "0x09bb..6413c";
const char* device_id = "2";
const char* ESP32_SSID = "DeWi-Device-2"; */

// Device info 3
/* const char* owner = "0xbcafe1986bb8130bea04de6c7482ba37dad77fbd";
const char* owner_short = "0xbcaf..77fbd";
const char* device_id = "3";
const char* ESP32_SSID = "DeWi-Device-3"; */

// LCD hd44780 I2C
#define LCD_ADDR 0x27
#define SDA_PIN  22
#define SCL_PIN  21
#define LCD_COLS 20
#define LCD_ROWS 4
// -----
#define MAX_HTTP_OUTPUT_BUFFER 2048

// Define a structure to hold MAC address and connection time
typedef struct {
    uint8_t mac[6]; // MAC address
    uint64_t connect_time; // Connection time (in microseconds)
} connected_device_t;

// Define a maximum number of connected devices
#define MAX_CONNECTED_DEVICES 10

// Array to store connected devices
connected_device_t connected_devices[MAX_CONNECTED_DEVICES];
int num_connected_devices = 0;

// Function to get current time in microseconds
uint64_t get_current_time_us() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

uint64_t parse_balance_response(const char* response) {
    const char* result_start = strstr(response, "\"result\":\"");
    if (result_start == NULL) {
        ESP_LOGE("PARSE_RESPONSE", "Result field not found in response");
        ESP_LOGE("PARSE_RESPONSE", "Response: %s", response);
        return 0;
    }
    result_start += strlen("\"result\":\"");
    // Note, only geting the last 8 bytes. We need to make sure the balance does not overflow 8 bytes, or else print it as a hex string
    char hex_balance[17]; // 16 hex characters for 64-bit integer
    strncpy(hex_balance, result_start + strlen(result_start) - 16, 16);
    hex_balance[16] = '\0'; // Null-terminate the string
    uint64_t balance = strtoull(hex_balance, NULL, 16);
    return balance;
}

uint64_t get_erc20_balance() {
    esp_http_client_config_t config = {
        .url = "https://babel-api.testnet.iotex.io",
        .method = HTTP_METHOD_POST,
        .buffer_size = 1024,
        .buffer_size_tx = 2048,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE("HTTP_CLIENT", "Failed to initialize HTTP client");
        return 0;
    }

    char payload[1024] = {0};
    //snprintf(payload, sizeof(payload), "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"0xff94dea0be4fc5289cb60f63d55eaff71b3e9666\",\"data\":\"0x70a082310000000000000000000000001435fc1a9170f15d708fb837d0f8b8f06e8f16e6\"},\"latest\"],\"id\":1}");
    snprintf(payload, sizeof(payload), "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"%s\",\"data\":\"0x70a08231000000000000000000000000%s\"},\"latest\"],\"id\":1}", token_contract, &owner[2]);
    ESP_LOGI("HTTP_CLIENT", "Payload: %s", payload);

    esp_err_t err = esp_http_client_open(client, strlen(payload));
    if (err != ESP_OK) {
        ESP_LOGE("HTTP_CLIENT", "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return 0;
    }

    int wlen = esp_http_client_write(client, payload, strlen(payload));
    if (wlen < 0) {
        ESP_LOGE("HTTP_CLIENT", "Failed to write request body");
        esp_http_client_cleanup(client);
        return 0;
    }

    int content_length = esp_http_client_fetch_headers(client);
    if (content_length < 0) {
        ESP_LOGE("HTTP_CLIENT", "HTTP client fetch headers failed");
        esp_http_client_cleanup(client);
        return 0;
    }

    char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
    if (data_read >= 0) {
        // ESP_LOGI("HTTP_CLIENT", "HTTP GET Status = %d, content_length = %d, content = %s", 
        //     esp_http_client_get_status_code(client), 
        //     (int)esp_http_client_get_content_length(client),
        //     output_buffer);

        uint64_t balance = parse_balance_response(output_buffer);
        // ESP_LOGI("HTTP_CLIENT", "Parsed balance: %" PRIu64, balance);
        esp_http_client_cleanup(client);
        return balance;
    } else {
        ESP_LOGE("HTTP_CLIENT", "Failed to read response");
        esp_http_client_cleanup(client);
        return 0;
    }
}


void init_display()
{
    #ifdef DISABLE_DISPLAY
         printf("\n");
         printf("LCD: Disabled\n");
        return;
    #endif
    printf("\n");
    printf("LCD: Initializing display...\n");
    printf("\nI2C SDA pin: %d", SDA_PIN);
    printf("\nI2C SCL pin: %d", SCL_PIN);
    printf("\nLCD address: 0x%02X", LCD_ADDR);
    printf("\nLCD columns: %d", LCD_COLS);
    printf("\nLCD rows: %d", LCD_ROWS);
    LCD_init(LCD_ADDR, SDA_PIN, SCL_PIN, LCD_COLS, LCD_ROWS);
    LCD_home();
    LCD_clearScreen();
    LCD_setCursor(0, 0);
    char line[20];
    sprintf(line, "WiFi: %s", ESP32_SSID);
    LCD_writeStr(line);
    LCD_setCursor(0, 2);
    sprintf(line, "Owner: %s", owner_short);
    LCD_writeStr(line);
    printf("LCD Display initialized\n");
}


void updateClients(int count) {
    LCD_setCursor(0,1);
    char line[20];
    sprintf(line, "Clients: %d", count);
    LCD_writeStr(line);
}

void updateBalance(uint64_t balance) {
  LCD_setCursor(0,3);
  // Print the balance
  char balance_str[128];
  snprintf(balance_str, sizeof(balance_str), "Balance: %" PRIu64 " DRW", balance);
  LCD_writeStr(balance_str);
}

void update_display() 
{
    static uint64_t previous_balance = 0; 
    uint64_t balance = get_erc20_balance();

    // Print the new balance if changed
    if (balance != previous_balance) {
        ESP_LOGI("UPDATE_DISPLAY>", "Balance: %" PRIu64, balance);
        ESP_LOGI("UPDATE_DISPLAY>", "Clients: %d", num_connected_devices);

        previous_balance = balance;
        #ifdef DISABLE_DISPLAY
            return;
        #endif
        updateBalance(balance);
    }
    updateClients(num_connected_devices);
}

void updateNetworkStatus(bool connected) {
    LCD_setCursor(16, 1);
    if (connected){
        LCD_writeStr("Send"); 
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    else LCD_writeStr("Idle");
}

// Function to send a message to the web service
void send_message() {
    updateNetworkStatus(true);
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .buffer_size = 1024,  // Increased buffer size for response headers
        .buffer_size_tx = 2048,  // Increased buffer size for request headers
    };

    // Create HTTP client
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE("HTTP_CLIENT", "Failed to initialize HTTP client");
        updateNetworkStatus(false);
        return;
    }

    // Construct JSON payload
    char payload[1024] = {0};  // Adjust size as needed
    // snprintf(payload, sizeof(payload), "{\"projectID\":15,\"projectVersion\":\"0.1\",\"data\":\"{\\\"devicesConnected\\\":%d}\"}", connect_count);
    snprintf(payload, sizeof(payload), "{\"projectID\":%d,\"projectVersion\":\"0.1\",\"data\":\"{\\\"device_id\\\":%s,\\\"connections\\\":%d, \\\"receipt_type\\\":\\\"Snark\\\"}\"}", project_id, device_id, connect_count);
    ESP_LOGI(TAG, "Payload: %s", payload); 
    char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};   // Buffer to store response of http request 
    int content_length = 0; 

    // Print the payload
    // ESP_LOGI("HTTP_CLIENT", "Payload: %s", payload);

    // Set headers
    // esp_http_client_set_header(client, "Content-Type", "application/json");
    //esp_http_client_set_header(client, "Authorization", did_token);

    esp_err_t err = esp_http_client_open(client, strlen(payload)); 
    if (err != ESP_OK) { 
        ESP_LOGE("HTTP_CLIENT", "Failed to open HTTP connection: %s", esp_err_to_name(err)); 
    } else { 
        int wlen = esp_http_client_write(client, payload, strlen(payload));
        if (wlen < 0) { 
            ESP_LOGE("HTTP_CLIENT", "Failed to write request body"); 
        }
        content_length = esp_http_client_fetch_headers(client); 
        if (content_length < 0) { 
            ESP_LOGE("HTTP_CLIENT", "HTTP client fetch headers failed"); 
        } else { 
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER); 
            if (data_read >= 0) { 
                ESP_LOGI("HTTP_CLIENT", "HTTP GET Status = %d, content_length = %d, content = %s", 
                     esp_http_client_get_status_code(client), 
                     (int)esp_http_client_get_content_length(client),
                     output_buffer); 
            } else { 
                ESP_LOGE("HTTP_CLIENT", "Failed to read response"); 
            } 
        } 
    } 
    esp_http_client_close(client); 

    // Set POST data
    esp_http_client_set_post_field(client, payload, strlen(payload));

    // Clean up
    esp_http_client_cleanup(client);
    updateNetworkStatus(false);
}

void *push_connection_status_thread(void *p) {
    int count = 0;
    while (true) {
        send_message(); // Call send_message function
        update_display();
        vTaskDelay(update_interval_ms / portTICK_PERIOD_MS); // Delay for some seconds
        count++;
        ESP_LOGI(TAG, "Message: %d", count); 
    }
    return NULL;
}


// ---------------------- END IoTeX code ----------------------


static void initialize_nvs(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);
}

esp_err_t apply_portmap_tab() {
    for (int i = 0; i<IP_PORTMAP_MAX; i++) {
        if (portmap_tab[i].valid) {
            ip_portmap_add(portmap_tab[i].proto, my_ip, portmap_tab[i].mport, portmap_tab[i].daddr, portmap_tab[i].dport);
        }
    }
    return ESP_OK;
}

esp_err_t delete_portmap_tab() {
    for (int i = 0; i<IP_PORTMAP_MAX; i++) {
        if (portmap_tab[i].valid) {
            ip_portmap_remove(portmap_tab[i].proto, portmap_tab[i].mport);
        }
    }
    return ESP_OK;
}

void print_portmap_tab() {
    for (int i = 0; i<IP_PORTMAP_MAX; i++) {
        if (portmap_tab[i].valid) {
            printf ("%s", portmap_tab[i].proto == PROTO_TCP?"TCP ":"UDP ");
            ip4_addr_t addr;
            addr.addr = my_ip;
            printf (IPSTR":%d -> ", IP2STR(&addr), portmap_tab[i].mport);
            addr.addr = portmap_tab[i].daddr;
            printf (IPSTR":%d\n", IP2STR(&addr), portmap_tab[i].dport);
        }
    }
}

esp_err_t get_portmap_tab() {
    esp_err_t err;
    nvs_handle_t nvs;
    size_t len;

    err = nvs_open(PARAM_NAMESPACE, NVS_READWRITE, &nvs);
    if (err != ESP_OK) {
        return err;
    }
    err = nvs_get_blob(nvs, "portmap_tab", NULL, &len);
    if (err == ESP_OK) {
        if (len != sizeof(portmap_tab)) {
            err = ESP_ERR_NVS_INVALID_LENGTH;
        } else {
            err = nvs_get_blob(nvs, "portmap_tab", portmap_tab, &len);
            if (err != ESP_OK) {
                memset(portmap_tab, 0, sizeof(portmap_tab));
            }
        }
    }
    nvs_close(nvs);

    return err;
}

esp_err_t add_portmap(u8_t proto, u16_t mport, u32_t daddr, u16_t dport) {
    esp_err_t err;
    nvs_handle_t nvs;

    for (int i = 0; i<IP_PORTMAP_MAX; i++) {
        if (!portmap_tab[i].valid) {
            portmap_tab[i].proto = proto;
            portmap_tab[i].mport = mport;
            portmap_tab[i].daddr = daddr;
            portmap_tab[i].dport = dport;
            portmap_tab[i].valid = 1;

            err = nvs_open(PARAM_NAMESPACE, NVS_READWRITE, &nvs);
            if (err != ESP_OK) {
                return err;
            }
            err = nvs_set_blob(nvs, "portmap_tab", portmap_tab, sizeof(portmap_tab));
            if (err == ESP_OK) {
                err = nvs_commit(nvs);
                if (err == ESP_OK) {
                    ESP_LOGI(TAG, "New portmap table stored.");
                }
            }
            nvs_close(nvs);

            ip_portmap_add(proto, my_ip, mport, daddr, dport);

            return ESP_OK;
        }
    }
    return ESP_ERR_NO_MEM;
}

esp_err_t del_portmap(u8_t proto, u16_t mport) {
    esp_err_t err;
    nvs_handle_t nvs;

    for (int i = 0; i<IP_PORTMAP_MAX; i++) {
        if (portmap_tab[i].valid && portmap_tab[i].mport == mport && portmap_tab[i].proto == proto) {
            portmap_tab[i].valid = 0;

            err = nvs_open(PARAM_NAMESPACE, NVS_READWRITE, &nvs);
            if (err != ESP_OK) {
                return err;
            }
            err = nvs_set_blob(nvs, "portmap_tab", portmap_tab, sizeof(portmap_tab));
            if (err == ESP_OK) {
                err = nvs_commit(nvs);
                if (err == ESP_OK) {
                    ESP_LOGI(TAG, "New portmap table stored.");
                }
            }
            nvs_close(nvs);

            ip_portmap_remove(proto, mport);
            return ESP_OK;
        }
    }
    return ESP_OK;
}

static void initialize_console(void)
{
    /* Disable buffering on stdin */
    setvbuf(stdin, NULL, _IONBF, 0);

#if CONFIG_ESP_CONSOLE_UART_DEFAULT || CONFIG_ESP_CONSOLE_UART_CUSTOM
    /* Drain stdout before reconfiguring it */
    fflush(stdout);
    fsync(fileno(stdout));
    
    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_uart_port_set_rx_line_endings(0, ESP_LINE_ENDINGS_CR);
    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_uart_port_set_tx_line_endings(0, ESP_LINE_ENDINGS_CRLF);

    /* Configure UART. Note that REF_TICK is used so that the baud rate remains
     * correct while APB frequency is changing in light sleep mode.
     */
    const uart_config_t uart_config = {
            .baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            #if defined(CONFIG_IDF_TARGET_ESP32) || defined(CONFIG_IDF_TARGET_ESP32S2)
                .source_clk = UART_SCLK_REF_TICK,
            #else
                .source_clk = UART_SCLK_XTAL,
            #endif
    };
    /* Install UART driver for interrupt-driven reads and writes */
    ESP_ERROR_CHECK( uart_driver_install(CONFIG_ESP_CONSOLE_UART_NUM,
            256, 0, 0, NULL, 0) );
    ESP_ERROR_CHECK( uart_param_config(CONFIG_ESP_CONSOLE_UART_NUM, &uart_config) );

    /* Tell VFS to use UART driver */
    esp_vfs_dev_uart_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);
#endif

#if CONFIG_ESP_CONSOLE_USB_SERIAL_JTAG
    /* Enable non-blocking mode on stdin and stdout */
    fcntl(fileno(stdout), F_SETFL, O_NONBLOCK);
    fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);

    /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
    esp_vfs_dev_usb_serial_jtag_set_rx_line_endings(ESP_LINE_ENDINGS_CR);

    /* Move the caret to the beginning of the next line on '\n' */
    esp_vfs_dev_usb_serial_jtag_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);
    usb_serial_jtag_driver_config_t usb_serial_jtag_config = {
        .tx_buffer_size = 256,
        .rx_buffer_size = 256,
    };

    /* Install USB-SERIAL-JTAG driver for interrupt-driven reads and writes */
    usb_serial_jtag_driver_install(&usb_serial_jtag_config);

    /* Tell vfs to use usb-serial-jtag driver */
    esp_vfs_usb_serial_jtag_use_driver();
#endif

    /* Initialize the console */
    esp_console_config_t console_config = {
            .max_cmdline_args = 8,
            .max_cmdline_length = 256,
#if CONFIG_LOG_COLORS
            .hint_color = atoi(LOG_COLOR_CYAN)
#endif
    };
    ESP_ERROR_CHECK( esp_console_init(&console_config) );

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(100);

#if CONFIG_STORE_HISTORY
    /* Load command history from filesystem */
    linenoiseHistoryLoad(HISTORY_PATH);
#endif
}

void * led_status_thread(void * p)
{
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    while (true)
    {
        gpio_set_level(BLINK_GPIO, ap_connect);

        for (int i = 0; i < connect_count; i++)
        {
            gpio_set_level(BLINK_GPIO, 1 - ap_connect);
            vTaskDelay(50 / portTICK_PERIOD_MS);
            gpio_set_level(BLINK_GPIO, ap_connect);
            vTaskDelay(50 / portTICK_PERIOD_MS);
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    esp_netif_dns_info_t dns;

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG,"disconnected - retry to connect to the AP");
        ap_connect = false;
        esp_wifi_connect();
        ESP_LOGI(TAG, "retry to connect to the AP");
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        ap_connect = true;
        my_ip = event->ip_info.ip.addr;
        delete_portmap_tab();
        apply_portmap_tab();
        if (esp_netif_get_dns_info(wifiSTA, ESP_NETIF_DNS_MAIN, &dns) == ESP_OK)
        {
            esp_netif_set_dns_info(wifiAP, ESP_NETIF_DNS_MAIN, &dns);
            ESP_LOGI(TAG, "set dns to:" IPSTR, IP2STR(&(dns.ip.u_addr.ip4)));
        }
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        connect_count++;
        ESP_LOGI(TAG,"New client connected with MAC: %02x:%02x:%02x:%02x:%02x:%02x", 
            event->mac[0], event->mac[1], event->mac[2],
            event->mac[3], event->mac[4], event->mac[5]);

        // Store the MAC address and connection time
        if (num_connected_devices < MAX_CONNECTED_DEVICES) {
            memcpy(connected_devices[num_connected_devices].mac, event->mac, sizeof(event->mac));
            connected_devices[num_connected_devices].connect_time = get_current_time_us();
            num_connected_devices++;
        }
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        connect_count--;
        ESP_LOGI(TAG,"client disconnected - %d remain", connect_count);

        // Get the current time
        uint64_t current_time = get_current_time_us();

        // Remove disconnected device from the list
        for (int i = 0; i < num_connected_devices; i++) {
            if (memcmp(connected_devices[i].mac, event->mac, sizeof(event->mac)) == 0) {
                // Found the disconnected device
                // Optionally, calculate the duration of the connection
                uint64_t duration = (current_time - connected_devices[i].connect_time) / 1000000;
                ESP_LOGI(TAG, "Client disconnected with MAC: %02x:%02x:%02x:%02x:%02x:%02x. Connection duration: %llu seconds",
                        connected_devices[i].mac[0], connected_devices[i].mac[1], connected_devices[i].mac[2],
                        connected_devices[i].mac[3], connected_devices[i].mac[4], connected_devices[i].mac[5], duration);

                // Shift elements in the array to remove the disconnected device
                for (int j = i; j < num_connected_devices - 1; j++) {
                    connected_devices[j] = connected_devices[j + 1];
                }
                num_connected_devices--;
                break;
            }
        }
    }
}

const int CONNECTED_BIT = BIT0;
#define JOIN_TIMEOUT_MS (2000)


void wifi_init(const char* ssid, const char* ent_username, const char* ent_identity, const char* passwd, const char* static_ip, const char* subnet_mask, const char* gateway_addr, const char* ap_ssid, const char* ap_passwd, const char* ap_ip)
{
    esp_netif_dns_info_t dnsserver;
    // esp_netif_dns_info_t dnsinfo;

    wifi_event_group = xEventGroupCreate();
  
    esp_netif_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifiAP = esp_netif_create_default_wifi_ap();
    wifiSTA = esp_netif_create_default_wifi_sta();

    esp_netif_ip_info_t ipInfo_sta;
    if ((strlen(ssid) > 0) && (strlen(static_ip) > 0) && (strlen(subnet_mask) > 0) && (strlen(gateway_addr) > 0)) {
        has_static_ip = true;
        ipInfo_sta.ip.addr = esp_ip4addr_aton(static_ip);
        ipInfo_sta.gw.addr = esp_ip4addr_aton(gateway_addr);
        ipInfo_sta.netmask.addr = esp_ip4addr_aton(subnet_mask);
        esp_netif_dhcpc_stop(ESP_IF_WIFI_STA); // Don't run a DHCP client
        esp_netif_set_ip_info(ESP_IF_WIFI_STA, &ipInfo_sta);
        apply_portmap_tab();
    }

    my_ap_ip = esp_ip4addr_aton(ap_ip);

    esp_netif_ip_info_t ipInfo_ap;
    ipInfo_ap.ip.addr = my_ap_ip;
    ipInfo_ap.gw.addr = my_ap_ip;
    esp_netif_set_ip4_addr(&ipInfo_ap.netmask, 255,255,255,0);
    esp_netif_dhcps_stop(wifiAP); // stop before setting ip WifiAP
    esp_netif_set_ip_info(wifiAP, &ipInfo_ap);
    esp_netif_dhcps_start(wifiAP);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /* ESP WIFI CONFIG */
    wifi_config_t wifi_config = { 0 };
        wifi_config_t ap_config = {
        .ap = {
            .channel = 0,
            .authmode = WIFI_AUTH_WPA2_WPA3_PSK,
            .ssid_hidden = 0,
            .max_connection = 8,
            .beacon_interval = 100,
        }
    };

    strlcpy((char*)ap_config.sta.ssid, ap_ssid, sizeof(ap_config.sta.ssid));
    if (strlen(ap_passwd) < 8) {
        ap_config.ap.authmode = WIFI_AUTH_OPEN;
    } else {
	    strlcpy((char*)ap_config.sta.password, ap_passwd, sizeof(ap_config.sta.password));
    }

    if (strlen(ssid) > 0) {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA) );

        //Set SSID
        strlcpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
        //Set passwprd
        if(strlen(ent_username) == 0) {
            ESP_LOGI(TAG, "STA regular connection");
            strlcpy((char*)wifi_config.sta.password, passwd, sizeof(wifi_config.sta.password));
        }
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
        if(strlen(ent_username) != 0 && strlen(ent_identity) != 0) {
            ESP_LOGI(TAG, "STA enterprise connection");
            if(strlen(ent_username) != 0 && strlen(ent_identity) != 0) {
                esp_eap_client_set_identity((uint8_t *)ent_identity, strlen(ent_identity)); //provide identity
            } else {
                esp_eap_client_set_identity((uint8_t *)ent_username, strlen(ent_username));
            }
            esp_eap_client_set_username((uint8_t *)ent_username, strlen(ent_username)); //provide username
            esp_eap_client_set_password((uint8_t *)passwd, strlen(passwd)); //provide password
            esp_wifi_sta_enterprise_enable();
        }

        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config) );
    } else {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP) );
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_config) );        
    }



    // Enable DNS (offer) for dhcp server
    dhcps_offer_t dhcps_dns_value = OFFER_DNS;
    esp_netif_dhcps_option(wifiAP,ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &dhcps_dns_value, sizeof(dhcps_dns_value));

    // // Set custom dns server address for dhcp server
    dnsserver.ip.u_addr.ip4.addr = esp_ip4addr_aton(DEFAULT_DNS);
    dnsserver.ip.type = ESP_IPADDR_TYPE_V4;
    esp_netif_set_dns_info(wifiAP, ESP_NETIF_DNS_MAIN, &dnsserver);

    // esp_netif_get_dns_info(ESP_IF_WIFI_AP, ESP_NETIF_DNS_MAIN, &dnsinfo);
    // ESP_LOGI(TAG, "DNS IP:" IPSTR, IP2STR(&dnsinfo.ip.u_addr.ip4));

    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
        pdFALSE, pdTRUE, JOIN_TIMEOUT_MS / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(esp_wifi_start());

    if (strlen(ssid) > 0) {
        ESP_LOGI(TAG, "wifi_init_apsta finished.");
        ESP_LOGI(TAG, "connect to ap SSID: %s ", ssid);
    } else {
        ESP_LOGI(TAG, "wifi_init_ap with default finished.");      
    }
}

char* ssid = NULL;
char* ent_username = NULL;
char* ent_identity = NULL;
char* passwd = NULL;
char* static_ip = NULL;
char* subnet_mask = NULL;
char* gateway_addr = NULL;
char* ap_ssid = NULL;
char* ap_passwd = NULL;
char* ap_ip = NULL;

char* param_set_default(const char* def_val) {
    char * retval = malloc(strlen(def_val)+1);
    strcpy(retval, def_val);
    return retval;
}

void app_main(void)
{
    initialize_nvs();

#if CONFIG_STORE_HISTORY
    initialize_filesystem();
    ESP_LOGI(TAG, "Command history enabled");
#else
    ESP_LOGI(TAG, "Command history disabled");
#endif

    get_config_param_str("ssid", &ssid);
    if (ssid == NULL) {
        ssid = param_set_default("");
    }
    get_config_param_str("ent_username", &ent_username);
    if (ent_username == NULL) {
        ent_username = param_set_default("");
    }
    get_config_param_str("ent_identity", &ent_identity);
    if (ent_identity == NULL) {
        ent_identity = param_set_default("");
    }
    get_config_param_str("passwd", &passwd);
    if (passwd == NULL) {
        passwd = param_set_default("");
    }
    get_config_param_str("static_ip", &static_ip);
    if (static_ip == NULL) {
        static_ip = param_set_default("");
    }
    get_config_param_str("subnet_mask", &subnet_mask);
    if (subnet_mask == NULL) {
        subnet_mask = param_set_default("");
    }
    get_config_param_str("gateway_addr", &gateway_addr);
    if (gateway_addr == NULL) {
        gateway_addr = param_set_default("");
    }
    get_config_param_str("ap_ssid", &ap_ssid);
    if (ap_ssid == NULL) {
        ap_ssid = param_set_default(ESP32_SSID);
    }   
    get_config_param_str("ap_passwd", &ap_passwd);
    if (ap_passwd == NULL) {
        ap_passwd = param_set_default("");
    }
    get_config_param_str("ap_ip", &ap_ip);
    if (ap_ip == NULL) {
        ap_ip = param_set_default(DEFAULT_AP_IP);
    }

    get_portmap_tab();

    // Setup WIFI
    wifi_init(ssid, ent_username, ent_identity, passwd, static_ip, subnet_mask, gateway_addr, ap_ssid, ap_passwd, ap_ip);

    pthread_t t1;
    pthread_create(&t1, NULL, led_status_thread, NULL);

    ip_napt_enable(my_ap_ip, 1);
    ESP_LOGI(TAG, "NAT is enabled");

    char* lock = NULL;
    get_config_param_str("lock", &lock);
    if (lock == NULL) {
        lock = param_set_default("0");
    }
    if (strcmp(lock, "0") ==0) {
        ESP_LOGI(TAG,"Starting config web server");
        start_webserver();
    }
    free(lock);

    initialize_console();

    /* Register commands */
    esp_console_register_help_command();
    register_system();
    register_nvs();
    register_router();

    /* Init the display */
    init_display();
    /* Register connection monitor thread */
    pthread_t t2;
    pthread_attr_t attr;
    size_t stack_size = 4096 * 2; // Define the desired stack size
    pthread_attr_init(&attr); // Initialize thread attributes
    pthread_attr_setstacksize(&attr, stack_size); // Set stack size attribute
    pthread_create(&t2, &attr, push_connection_status_thread, NULL); // Create the thread
    pthread_attr_destroy(&attr);


    /* Prompt to be printed before each line.
     * This can be customized, made dynamic, etc.
     */
    const char* prompt = LOG_COLOR_I "esp32> " LOG_RESET_COLOR;

    printf("\n"
           "ESP32 NAT ROUTER\n"
           "Type 'help' to get the list of commands.\n"
           "Use UP/DOWN arrows to navigate through command history.\n"
           "Press TAB when typing command name to auto-complete.\n");

    if (strlen(ssid) == 0) {
         printf("\n"
               "Unconfigured WiFi\n"
               "Configure using 'set_sta' and 'set_ap' and restart.\n");       
    }

    /* Figure out if the terminal supports escape sequences */
    int probe_status = linenoiseProbe();
    if (probe_status) { /* zero indicates success */
        printf("\n"
               "Your terminal application does not support escape sequences.\n"
               "Line editing and history features are disabled.\n"
               "On Windows, try using Putty instead.\n");
        linenoiseSetDumbMode(1);
#if CONFIG_LOG_COLORS
        /* Since the terminal doesn't support escape sequences,
         * don't use color codes in the prompt.
         */
        prompt = "esp32> ";
#endif //CONFIG_LOG_COLORS
    }

    /* Main loop */
    while(true) {
        /* Get a line using linenoise.
         * The line is returned when ENTER is pressed.
         */
        char* line = linenoise(prompt);
        if (line == NULL) { /* Ignore empty lines */
            continue;
        }
        /* Add the command to the history */
        linenoiseHistoryAdd(line);
#if CONFIG_STORE_HISTORY
        /* Save command history to filesystem */
        linenoiseHistorySave(HISTORY_PATH);
#endif

        /* Try to run the command */
        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND) {
            printf("Unrecognized command\n");
        } else if (err == ESP_ERR_INVALID_ARG) {
            // command was empty
        } else if (err == ESP_OK && ret != ESP_OK) {
            printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(ret));
        } else if (err != ESP_OK) {
            printf("Internal error: %s\n", esp_err_to_name(err));
        }
        /* linenoise allocates line buffer on the heap, so need to free it */
        linenoiseFree(line);
    }
}
