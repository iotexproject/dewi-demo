// SPDX-License-Identifier: MIT
pragma solidity ^0.8.19;

interface IDapp {
    event ProofVerified(address indexed sender, bytes32 imageId, bytes32 proof_journal_hash);
    event RewardsDistributed(address indexed owner, uint256 reward);

    function process(
        uint256 _projectId,
        bytes32 _taskId,
        address _prover,
        address _deviceId,
        bytes calldata _data
    ) external;
}
