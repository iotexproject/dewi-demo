// SPDX-License-Identifier: MIT
pragma solidity ^0.8.19;

interface IDapp {
    error VerifierNotSet();
    error VerificationFailed();
    error ImageIdNotFound(uint256 projectId);
    error DeviceIsNotEnabled(uint256);

    event ProofVerified(address indexed sender, bytes32 imageId, bytes32 proof_journal_hash);
    event RewardsDistributed(address indexed owner, uint256 reward);

    function process(uint256 _projectId, uint256 _proverId, string memory _clientId, bytes calldata _data) external;
}
