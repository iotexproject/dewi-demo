# IoTeX DeWi Demo - Smart Contracts

## Table of Contents

- [Overview of Contracts](#overview-of-contracts)
- [Token Incentive Economy](#token-incentive-economy)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Quickstart](#quickstart)
  - [1. Copy the Environment File](#1-copy-the-environment-file)
  - [2. Compile and Test Smart Contracts](#2-compile-and-test-smart-contracts)
- [Adjusting Smart Contracts](#adjusting-smart-contracts)
- [Testing](#testing)
- [Deployment](#deployment)
- [Sending Proof On-Chain](#sending-proof-on-chain)

## Overview of Contracts

- **Dapp**: The core DeWi application that receives and verifies device work proofs from W3bstream. It interacts with ioID contracts to facilitate infrastructure rewards distribution.
- **DeviceNFT**: A customizable NFT contract that tokenizes devices on-chain. In this implementation, each device's serial number is minted as the token ID.
- **DeviceReward**: An ERC20 token contract used to incentivize device owners for building and maintaining the network infrastructure.

## Token Incentive Economy

The token economy is straightforward: each access point sends a message every 5 seconds to prove it's "online." The message will also include the number of unique clients connected during that interval. W3bstream processes these messages to compute an "Index of Physical Work" for each device: 1 point for being online and 1 point for each unique client served. The Index of Work, along with a ZK proof of the computation, is sent to the Dapp, which verifies the proof and distributes token rewards to device owners at a 1:1 ratio (tokens:work).

## Prerequisites

Before you begin, ensure you have the following installed:

- **Node.js** (v18.x or later)
- **npm** (Node Package Manager)
- **Hardhat**: Ethereum development environment

## Installation

Clone the repository and install the necessary dependencies:

```bash
git clone https://github.com/iotexproject/dewi-demo.git
cd blockchain
npm install
```

## Quickstart

Follow these steps to set up, test, and deploy the WS DApp.

### 1. Copy the Environment File

Copy the example environment file and update its values according to your configuration:

```bash
cp .env{.template,}
```

Edit the `.env` file to include your environment-specific variables.

### 2. Compile and Test Smart Contracts

Compile the smart contracts and run the test suite:

```bash
npm run test
```

## Adjusting Smart Contracts

Customize the smart contracts to fit your requirements. The main contracts are:

- **`contracts/WSDapp.sol`**: Processes ZK proofs, interacts with the DeviceNFT and DeviceRewards contracts to distribute rewards.
- **`contracts/DeviceNFT.sol`**: Manages Device NFT creation and ownership, representing ownership of devices.
- **`contracts/DeviceRewards.sol`**: Handles the reward distribution mechanism to device owners.

Make the necessary changes in these files to implement your desired functionality.

## Testing

After adjusting the smart contracts, re-run the tests to ensure everything works correctly:

```bash
npm run test
```

All tests should pass before proceeding to deployment.

## Deployment

Deploy the smart contracts to the testnet using Hardhat Ignition with the following commands:

```bash
npx hardhat ignition deploy ignition/modules/DeviceNFT.ts --network testnet
npx hardhat ignition deploy ignition/modules/DeviceRewards.ts --network testnet
npx hardhat ignition deploy ignition/modules/WSDapp.ts --network testnet
```

Replace `testnet` with your target network if different.

## Sending Proof On-Chain

After deploying the contracts, send the ZK proof on-chain to initiate the reward distribution process. The W3bstream sends a ZK proof to the `WSRouter`, which, based on the `projectId`, forwards it to the appropriate `WSDapp`. The `WSDapp` processes the proof, identifies the device owners via their Device NFTs, and distributes the rewards through the `DeviceRewards` contract.
