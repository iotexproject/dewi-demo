# ESP32 NAT Router modified to support W3bstream

This firmware is based on https://github.com/martin-ger/esp32_nat_router

We modified the original firmware to implement a W3bstream client. Most of the code we added is located in [esp32_nat_router.c](./main/esp32_nat_router.c).

We implemented a simple message protocol where each router has a unique device ID and would send a data message every "slot" of 7 seconds. The message itself represents a "hearthbeat" for the device meaing that it's actually live and providing connectivity. Additionally, the message includes the number of connected clients in that slot to account for some sort of "work" done by the device. For the demo we had 4 devices, with device IDs from 0 to 3. The typical device message would look like the following:

```json
{ 
  "projectID": 33,
  "projectVersion": "0.1",
  "data": {
    "device_id": "1",
    "connections": 3,
    "receipt_type": "Snark"
  }
}
```

`ProjectID`, `projectVersion`and `data` belong to the W3bstream message protocol, where `data` is the actual payload that will be bundled in a data block and processed by W3bstream provers.

> **Note**: Since W3bstream does not implement a concept of device IDs in the messaging protocol, these IDs are included in the data payload. This means that we should perform device authorization and data integriti checks into the W3bstream prover when processing the payload. Since the prover is a ZK circuit, this would strongly limit the performance (performing an digital signature verification in a ZK prover is very texpensive) as well as the flexibility (the ZK prover cannot access the device registry on-chain). Fer these reasons we have implemented a "lightweight" device identity with just a device id passed in the payload but without any device authentication or data integrity check. Waiting for support in W3bstream.

