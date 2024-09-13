# WS DApp

A decentralized application integrating WS rewards and Device NFTs, designed to process ZK proofs and distribute rewards to device owners.

## Overview

The WS DApp is a blockchain-based application that processes ZK proofs sent from W3bstream. Based on the provided `projectId`, the WSRouter identifies the appropriate WS DApp and forwards the proof to it. The WS DApp then processes the proof, identifies device owners, and distributes rewards accordingly.

This system ensures secure, transparent, and efficient distribution of rewards to device owners while maintaining data privacy through zero-knowledge proofs.

## Table of Contents

- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Quickstart](#quickstart)
  - [1. Copy the Environment File](#1-copy-the-environment-file)
  - [2. Compile and Test Smart Contracts](#2-compile-and-test-smart-contracts)
- [Adjusting Smart Contracts](#adjusting-smart-contracts)
- [Testing](#testing)
- [Deployment](#deployment)
- [Sending Proof On-Chain](#sending-proof-on-chain)
- [Architecture](#architecture)
- [License](#license)

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

