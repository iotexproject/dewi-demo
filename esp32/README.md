# ESP32 NAT Router modified to support W3bstream

Our firmware for ESP32 boards is built upon the original firmware by [martin-ger](https://github.com/martin-ger/esp32_nat_router) (specifically, [on this commit](https://github.com/martin-ger/esp32_nat_router/tree/f44c8794cb7ef59bb2602937131a8177ee6f22e5)), focusing on incorporating IoTeX's device identity protocol (ioID) and implementing a messaging protocol to communicate the online status and WiFi clients count to the DePIN Dapp that will distribute the incentives. 

The core additions are made in the `dewi.c` file, within the project's main directory. The original `esp32_nat_router.c` is modified to import `dewi.h` and to store the info on connceted WiFi clients inside the `wifi_event_handler`, specifically within the `WIFI_EVENT_AP_STACONNECTED` and `WIFI_EVENT_AP_STADISCONNECTED` events.  

## Overview of the protocol
![image](https://github.com/user-attachments/assets/99b12b36-020c-4416-a3b8-d6fcc7910488)


We have designed a simple protocol to facilitate communication between the ESP32 access point and an API service. The API service has the role of validating and packing together data messages into *W3bstream Tasks* and submitting them to W3bstream. We deployed a specific ZK logic to W3bstream to process data and compute a "work score" for each device. 

So, each ESP32 utilizes ioConnect and ioID to generate and self-assign unique *DID*, which is used by the device owner to register the device on the IoTeX blockchain. ESP32s send a heartbeat message in slots of a few seconds. This message confirms the device's operation (*proof of liveness*)  and it includes the count of connected clients as a measure of the router's activity in that slot and signs the message with it's DID private key. Here's an example of a message format:

```json
{ 
  "projectID": 123,
  "projectVersion": "0.1",
  "data": {
    "client_id": "1",
    "connections": 3,
  },
  "Signature": "0xb635...03a26d4d900"
}
```

## Key Points

- **ProjectID** is the ID of our W3bstream project (registered on the IoTeX chain using ioID)
- **projectVersion** is the specific version of the W3bstream code the message was intended for.  
- **Data**: Contains the actual data sent by the device: the number of unique WiFi clients connected in the slot (for deviceID, see below).
  
## Features

Not all the features have been implemeted yet. BElow is an overview:

- [x] Compute the number of uniquely connected WiFi clients
- [x] Store the info and connection time for each WiFi client
- [x] Send the message on every slot to the data sequencer
- [ ] Integrate ioConnect for DID generation
- [ ] Implement ioID device registration (*)
- [ ] Implement message signature
- [ ] Improve the message protocol with timestamp

(*) At this moment, we ioID is not integrated yet. We simply included a unique `client_id` in the `data` field; on-chain, that id represents an NFT and whoever owns that NFT is considered the **owner** of the device and receive the rewards. Nevertheless, ioID must be implemented to register a certain project's devices on chain, and authenticate them when a message is received (e.g. by means of a message signature or using ioConnect's DID authentication features).

## Setting up your environment

### Configure the example

Open the `main/dewi.c` replace the values of the following constants with your own ones:

- `char* did_token`: the token used for DID authentication
- `char* url`: the URL of the node
- `const int project_id`: the project id
- `const char* token_contract`: the ERC20 token contract address

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
Once configured, you should be able to connect a client device to the board's WiFi. The router will then provide internet access via your home network while sending "proofs of connectivity" to the W3bstream sequences.

The firmware will send a message periodically with the number of connected devices. The logs should display the message payload.  
The logs should display the message id if the messe is sent successfully, or an error message.
