// SPDX-License-Identifier: MIT
pragma solidity ^0.8.19;

interface IRisc0Receiver {
    function verify(bytes calldata seal, bytes32 imageId, bytes32 journalDigest) external view;
}
