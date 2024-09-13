#include <pthread.h>
#include <sys/time.h> // Required for gettimeofday()

#include "dewi.h"
#include "hd44780_I2C.h" // Display library
#include "esp_http_client.h"
#include "esp_log.h"

static const char *TAG = "DeWi";
extern uint16_t connect_count;

// The DID token to communicate with the server
// TODO: Integrate DID token generation with ioConnect + ioID SDK
// Ref. Examples: https://github.com/iotexproject/ioConnect/tree/main/example/esp32
const char* did_token = "eyJhbGciOiJFZERTQSIsImtpZCI6ImRpZDprZXk6ejZNa2pQMlBhMXBrVWd6MnJQNnlUWHBBVGU0cWQ3YWh3c0dBUXVVNjk3SnBjQ0xmI3o2TWtqUDJQYTFwa1VnejJyUDZ5VFhwQVRlNHFkN2Fod3NHQVF1VTY5N0pwY0NMZiJ9.eyJpc3MiOiJkaWQ6a2V5Ono2TWtqUDJQYTFwa1VnejJyUDZ5VFhwQVRlNHFkN2Fod3NHQVF1VTY5N0pwY0NMZiIsIm5iZiI6MTU5Nzg3MzMxMCwianRpIjoiaHR0cDovL2V4YW1wbGUub3JnL2NyZWRlbnRpYWxzLzM3MzEiLCJzdWIiOiJkaWQ6a2V5Ono2TWtlZUNoclVzMUVvS2tOTnpveTlGd0pKYjlnTlE5MlVUOGtjWFpITWJ3ajY3QiIsInZjIjp7IkBjb250ZXh0IjoiaHR0cHM6Ly93d3cudzMub3JnLzIwMTgvY3JlZGVudGlhbHMvdjEiLCJpZCI6Imh0dHA6Ly9leGFtcGxlLm9yZy9jcmVkZW50aWFscy8zNzMxIiwidHlwZSI6WyJWZXJpZmlhYmxlQ3JlZGVudGlhbCJdLCJjcmVkZW50aWFsU3ViamVjdCI6eyJpZCI6ImRpZDprZXk6ejZNa2VlQ2hyVXMxRW9La05Oem95OUZ3SkpiOWdOUTkyVVQ4a2NYWkhNYndqNjdCIn0sImlzc3VlciI6ImRpZDprZXk6ejZNa2pQMlBhMXBrVWd6MnJQNnlUWHBBVGU0cWQ3YWh3c0dBUXVVNjk3SnBjQ0xmIiwiaXNzdWFuY2VEYXRlIjoiMjAyMC0wOC0xOVQyMTo0MTo1MFoifX0.4dzsQ89P6A8NUy5qpdAohCbNRCRMnplrtiYaEpPvTeU9nzyrKGAPG-bQukI2Jzv2f289lWDx0fdWxWJjSgieDA";

// The URL of the W3bstream sequencer
const char* url = "sprout-stress.w3bstream.com/message";

// The W3bstream Project id as created in ioID
// ioctl ioid register $PROJECT_NAME
const int project_id = 884;

// The token contract address, used for DePIN infra incentives
const char* token_contract = "0xB1f2d24F82e114bD44AF492E7ead45003DBb51d6";

// The interval for sending updates
// TODO: The contract should not reward updates more frequent than 
// a preset interval, e.g. 5 seconds. Currently this is not implemented
// in the incentives contract.
const uint32_t update_interval_ms = 5000;

// TODO: The owner address should be obtained from the ioID device registration
// client_id can be the DID itself or any custom serial number or identifier. 
// It is minted as the Device NFT for device tokenization.

// Info for Device 0
const char* owner = "0x837816467573afc94b3bdd6bc68ab37cc71325f6";
const char* owner_short = "0x8378..325f6";
const char* client_id = "0"; 
const char* ESP32_SSID = "DEPIN-AP-0"; 

// Info for Device 1
// const char* owner = "0xc7c415f50829c1f696fb7c16df3635262bf99193";
// const char* owner_short = "0xc7c41..99193";
// const char* client_id = "1";
// const char* ESP32_SSID = "DEPIN-AP-1"; 

// Info for Device 2
// const char* owner = "0x199f87e13fa62c344e845139d706461f3f0cc030";
// const char* owner_short = "0x199f..cc030";
// const char* client_id = "2";
// const char* ESP32_SSID = "DEPIN-AP-2";

// Info for Device 3
// const char* owner = "0xf3c4b4d9714c88cd4c768975108eacf27f3a0138";
// const char* owner_short = "0xf3c4..a0138";
// const char* client_id = "3";
// const char* ESP32_SSID = "DEPIN-AP-3"; 

// Variables to store the connected clients
connected_device_t connected_devices[MAX_CONNECTED_DEVICES];
int num_connected_devices = 0;

void update_clients(int count)
{
    LCD_setCursor(0,1);
    char line[20];
    sprintf(line, "Clients: %d", count);
    LCD_writeStr(line);
}

void update_balance(uint64_t balance)
{
  LCD_setCursor(0,3);
  char balance_str[128];
  snprintf(balance_str, sizeof(balance_str), "Balance: %" PRIu64 " DRW", balance);
  LCD_writeStr(balance_str);
}

void update_network_status(bool connected)
{
    LCD_setCursor(16, 1);
    if (connected)
    {
        LCD_writeStr("Send"); 
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
    else LCD_writeStr("Idle");
}

uint64_t get_current_time_us()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

uint64_t parse_balance_response(const char* response)
{
    const char* result_start = strstr(response, "\"result\":\"");
    if (result_start == NULL)
    {
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

uint64_t get_erc20_balance()
{
    esp_http_client_config_t config =
    {
        .url = "https://babel-api.testnet.iotex.io",
        .method = HTTP_METHOD_POST,
        .buffer_size = 1024,
        .buffer_size_tx = 2048,
    };
    
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL)
    {
        ESP_LOGE("HTTP_CLIENT", "Failed to initialize HTTP client");
        return 0;
    }

    char payload[1024] = {0};
    snprintf(payload, sizeof(payload), "{\"jsonrpc\":\"2.0\",\"method\":\"eth_call\",\"params\":[{\"to\":\"%s\",\"data\":\"0x70a08231000000000000000000000000%s\"},\"latest\"],\"id\":1}", token_contract, &owner[2]);
    ESP_LOGI("HTTP_CLIENT", "Payload: %s", payload);

    esp_err_t err = esp_http_client_open(client, strlen(payload));
    if (err != ESP_OK) {
        ESP_LOGE("HTTP_CLIENT", "Failed to open HTTP connection: %s", esp_err_to_name(err));
        esp_http_client_cleanup(client);
        return 0;
    }

    int wlen = esp_http_client_write(client, payload, strlen(payload));
    if (wlen < 0)
    {
        ESP_LOGE("HTTP_CLIENT", "Failed to write request body");
        esp_http_client_cleanup(client);
        return 0;
    }

    int content_length = esp_http_client_fetch_headers(client);
    if (content_length < 0)
    {
        ESP_LOGE("HTTP_CLIENT", "HTTP client fetch headers failed");
        esp_http_client_cleanup(client);
        return 0;
    }

    char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
    int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
    if (data_read >= 0) 
    {
        // ESP_LOGI("HTTP_CLIENT", "HTTP GET Status = %d, content_length = %d, content = %s", 
        //     esp_http_client_get_status_code(client), 
        //     (int)esp_http_client_get_content_length(client),
        //     output_buffer);

        uint64_t balance = parse_balance_response(output_buffer);
        // ESP_LOGI("HTTP_CLIENT", "Parsed balance: %" PRIu64, balance);
        esp_http_client_cleanup(client);
        return balance;
    }
    else
    {
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
    sprintf(line, "SSID: %s", ESP32_SSID);
    LCD_writeStr(line);
    LCD_setCursor(0, 2);
    sprintf(line, "Owner: %s", owner_short);
    LCD_writeStr(line);
    printf("LCD Display initialized\n");
}

void update_display() 
{
    static uint64_t previous_balance = 0; 
    uint64_t balance = get_erc20_balance();

    // Print the new balance if changed
    if (balance != previous_balance)
    {
        ESP_LOGI("UPDATE_DISPLAY>", "Balance: %" PRIu64, balance);
        ESP_LOGI("UPDATE_DISPLAY>", "Clients: %d", num_connected_devices);

        previous_balance = balance;
        #ifdef DISABLE_DISPLAY
            return;
        #endif
        update_balance(balance);
    }
    update_clients(num_connected_devices);
}

void send_message()
{
    update_network_status(true);
    esp_http_client_config_t config = 
    {
        .url = url,
        .method = HTTP_METHOD_POST,
        .buffer_size = 1024,  // Increased buffer size for response headers
        .buffer_size_tx = 2048,  // Increased buffer size for request headers
    };

    // Print the url
    ESP_LOGI(TAG, "Server URL: %s", url);
    // Create HTTP client
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL)
    {
        ESP_LOGE("HTTP_CLIENT", "Failed to initialize HTTP client");
        update_network_status(false);
        return;
    }

    // Construct JSON payload
    char payload[1024] = {0};  // Adjust size as needed
    // snprintf(payload, sizeof(payload), "{\"projectID\":15,\"projectVersion\":\"0.1\",\"data\":\"{\\\"devicesConnected\\\":%d}\"}", connect_count);
    snprintf(payload, sizeof(payload), "{\"projectID\":%d,\"projectVersion\":\"0.1\",\"data\":\"{\\\"client_id\\\":\\\"%s\\\",\\\"connections\\\":%d, \\\"receipt_type\\\":\\\"Snark\\\"}\"}", project_id, client_id, connect_count);
    ESP_LOGI(TAG, "Payload: %s", payload); 
    char output_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};   // Buffer to store response of http request 
    int content_length = 0; 

    // Print the payload
    // ESP_LOGI("HTTP_CLIENT", "Payload: %s", payload);

    // Set headers
    // esp_http_client_set_header(client, "Content-Type", "application/json");
    //esp_http_client_set_header(client, "Authorization", did_token);

    esp_err_t err = esp_http_client_open(client, strlen(payload)); 
    if (err != ESP_OK)
    { 
        ESP_LOGE("HTTP_CLIENT", "Failed to open HTTP connection: %s", esp_err_to_name(err)); 
    } 
    else
    { 
        int wlen = esp_http_client_write(client, payload, strlen(payload));
        if (wlen < 0)
        { 
            ESP_LOGE("HTTP_CLIENT", "Failed to write request body"); 
        }
        content_length = esp_http_client_fetch_headers(client); 
        if (content_length < 0)
        { 
            ESP_LOGE("HTTP_CLIENT", "HTTP client fetch headers failed"); 
        }
        else
        {
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER); 
            if (data_read >= 0)
            { 
                ESP_LOGI("HTTP_CLIENT", "HTTP GET Status = %d, content_length = %d, content = %s", 
                     esp_http_client_get_status_code(client), 
                     (int)esp_http_client_get_content_length(client),
                     output_buffer); 
            }
            else
            { 
                ESP_LOGE("HTTP_CLIENT", "Failed to read response"); 
            } 
        } 
    } 
    esp_http_client_close(client); 

    // Set POST data
    esp_http_client_set_post_field(client, payload, strlen(payload));

    // Clean up
    esp_http_client_cleanup(client);
    update_network_status(false);
}

void *push_connection_status_thread(void *p)
{
    int count = 0;
    while (true)
    {
        send_message();
        update_display();
        vTaskDelay(update_interval_ms / portTICK_PERIOD_MS);
        count++;
        ESP_LOGI(TAG, "Message: %d", count); 
    }
    return NULL;
}

void start_dewi_monitoring()
{
    init_display();
    pthread_t t2;
    pthread_attr_t attr;
    size_t stack_size = 4096 * 2; // Define the desired stack size
    pthread_attr_init(&attr); // Initialize thread attributes
    pthread_attr_setstacksize(&attr, stack_size); // Set stack size attribute
    pthread_create(&t2, &attr, push_connection_status_thread, NULL); // Create the thread
    pthread_attr_destroy(&attr);
}