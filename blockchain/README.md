# IoTeX DeWi Demo - Smart Contracts

## Overview of Contracts

- **Dapp**: The core DeWi application that receives and verifies device work proofs from W3bstream. It interacts with ioID contracts to facilitate infrastructure rewards distribution.
- **DeviceNFT**: A customizable NFT contract that tokenizes devices on-chain. In this implementation, each device's serial number is minted as the token ID.
- **DeviceReward**: An ERC20 token contract used to incentivize device owners for building and maintaining the network infrastructure.

## Token Incentive Economy

The token economy is straightforward: each access point sends a message every 5 seconds to prove it's "online." The message will also include the number of unique clients connected during that interval. W3bstream processes these messages to compute an "Index of Physical Work" for each device: 1 point for being online and 1 point for each unique client served. The Index of Work, along with a ZK proof of the computation, is sent to the Dapp, which verifies the proof and distributes token rewards to device owners at a 1:1 ratio (tokens:work).

## Deploy Instructions

// Work in progress
