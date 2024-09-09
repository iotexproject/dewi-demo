# ESP32 NAT Router modified to support W3bstream

Our firmware for ESP32 boards is built upon the original firmware by [martin-ger](https://github.com/martin-ger/esp32_nat_router), focusing on incorporating W3bstream's messaging protocol. The core additions are made in the esp32_nat_router.c file within the project's main directory.

## W3bstream integration overview
We have designed a simple protocol to facilitate communication between the router and W3bstream. Each router is assigned a unique device ID and sends a heartbeat message every 7 seconds. This message confirms the device's operation and includes the count of connected clients as a measure of the router's activity. Here's an example of a message format:

```json
{ 
  "projectID": 33,
  "projectVersion": "0.1",
  "data": {
    "client_id": "1",
    "connections": 3,
    "receipt_type": "Snark"
  }
}
```

## Key Points
- **ProjectID** and **projectVersion** align with W3bstream's messaging protocol.
  
- **Data**: Contains the actual message, including device ID and connections count.
  
- **Note**:  Currently, W3bstream does not support the concept of device IDs within its protocol, meaning that no device authentication or data integrity checks are performed by W3bstream nodes. Although we include the Device ID within the message payload for later processing in the prover, practical device authentication by ZK is not feasible, and thus, this feature has been omitted. Leveraging the flexibility of the Risc Zero proving framework, we extract device IDs in the prover's result and use them on-chain to find the device owner. However, it's important to note that no digital signatures on the data messages are utilized at any point.


## Setting up your environment

### ESP-IDF installation
First, install the ESP-IDF (Espressif IoT Development Framework) using the guidelines provided by Espressif. Follow the steps outlined here for the installation process: https://docs.espressif.com/projects/esp-idf/en/v5.1.2/esp32/get-started/linux-macos-setup.html

### Install the VS Code extension
Next, install the ESP-IDF extension for Visual Studio Code. This extension simplifies development and debugging. Detailed installation instructions are available at the VS Code ESP-IDF Extension Installation Guide. During installation, select the "Express configuration" for automatic path completion: https://github.com/espressif/vscode-esp-idf-extension/blob/master/docs/tutorial/install.md

These are example settings with a default installation:
![img_v3_028a_41e6f65f-ecd6-4be2-babb-6ac6aa02625h](https://github.com/machinefi/iotex-dewi-demo/assets/11096047/e309b676-cada-4db4-bbe1-c37916521b00)

### Project setup
After setting up your environment, it's time to import the project into VS Code:

- Open VS Code and press F1.
- Type and select `ESP-IDF: Import Project`, then choose the project's folder.
- Again, press F1, search for `ESP-IDF: Device configuration -> Device target`, select your board (e.g., esp32) when prompted.

### Build and flash the firmware
With the project imported, you're ready to build and flash the firmware:

- In the ESP-IDF extension panel, click `Build` to compile the firmware.

![img_v3_028a_788d7cee-be79-4f37-896c-a6fbcd63067h](https://github.com/machinefi/iotex-dewi-demo/assets/11096047/616ad6df-0fad-426b-89c4-282051f555d2)

- Connect your board via USB, then click `Flash` to install the firmware.
- Select `Monitor` to view real-time logs.

Remember, any modifications to the firmware require a rebuild and reflash.

### Configure the board WiFi
Initially, configure the board to connect to your WiFi network. This can be done graphically by connecting to the board's WiFi (e.g., "DeWi-Device-0") and navigating to` 192.168.4.1`, or by executing the following command in the VS Code log window:

```sh
set_sta WIFI_SSID WIFI_PASSWROD
```
Just copy past the command in the log window after the board booted the firmware, then press enter.
Replace YOUR_WIFI_SSID and YOUR_WIFI_PASSWORD with your actual WiFi details.

### Testing
Once configured, you should be able to connect a client device to the board's WiFi. The router will then provide internet access via your home network while sending "proofs of connectivity" to the W3bstream sequences. You can configure more into the firmware:

<img width="1107" alt="image" src="https://github.com/machinefi/iotex-dewi-demo/assets/11096047/5f9cf0cb-07a5-4aba-b291-a7e647c31ab1">


