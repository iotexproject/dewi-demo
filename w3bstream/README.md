# DeWi demo prover for W3bstream

This is a minimal DeWi example for W3bstream. In this demo we assume a device message like below:

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

We implemented a simple Risc Zero prover (see the implementation in [main.rs](./methods/guest/src/main.rs)) that iterates over all the messages into a block and computes a mapping `[device_id => score]` where the `score` represents the amount of "work" performed by a certain device. It's computed las the sum of two components: a `BASE_REWARDS` for just maintaining the device running (a "proof of liveness", basically provided by the simple presence of a data message) plus an extra reward depending on the clients load in the specific time slot:

```rust
score = BASE_REWARDS + REWARDS_PER_CLIENT * message.connections
```

## Quick Start

### Test locally
```
cargo run --release
```

On a Macbok M1 you can leverage GPU acceleration with

```sh
cargo run --release -F metal
```

### Build the prover

```sh
cargo build --release
```

Locate the prover file `method.rs` with:

```sh
find ./target/release -name 'methods.rs' -print0 | xargs -0 stat -f "%Sm: %N" -t "%Y-%m-%d %H:%M:%S"
```
