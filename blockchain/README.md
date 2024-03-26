# DePIN dApp contracts

The project relies on the [Alpha implementation of the W3bstream Layer-1 contracts](https://github.com/machinefi/sprout/tree/develop/smartcontracts/sandbox).

## Design

### Contracts

1. **Rewards token**
   Any DePIN dApp is possibly going to include at least one token. There is currently no interface / standard reinforced in IoTeX for a DePIN token. In our demo we assume it's a standard ERC20 mintable token, we implemented it in the [DeviceReward](./contracts/DeviceReward.sol) contract. However, a real project should provide a sustainable token economy rather than an inifinite-mintable token.

2. **Receiver contract**
   To integrate with the Alpha implementation of W3bstream, a DePIN dApp is required to provide at least one "W3bstream Receiver Contract" by implementing the [IWSReceiver Interface](./contracts/interfaces/IWSReceiver.sol). Such contract is supposed to receive, for each data block processed by W3bstream, the actual validity proof and the respective block metadata (i.e., the zk-proof, Merkle root of the block, Merkle root of device contributions - the latter is currently not implemented). This can be just the token contract itself, or a dedicated smart contract that implements some complex logic. In this themo we have a dedicated contract implemented in [WSReceiver.sol](./contracts/WSReceiver.sol).

3. **Device Registry**

   The device registry is a fundamental component for any DePIN application. It maps unique device identifiers with their owner and optionally includes some device attributes. The most basic device registry maps a unique device id with the wallet address of the device owner. Optionally, a public key for device authentication or a reference to a DID is also provided. Since currently W3bstream does not reinforce any interface for a device registry, for this demo we provided a [basic implemantation](./contracts/DeviceRegistry.sol). 

