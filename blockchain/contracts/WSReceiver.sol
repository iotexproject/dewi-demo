// SPDX-License-Identifier: MIT
pragma solidity ^0.8.19;

import {IERC721} from "@openzeppelin/contracts/token/ERC721/IERC721.sol";
import {IERC20} from "@openzeppelin/contracts/token/ERC20/IERC20.sol";

import {IRisc0Receiver} from "./interfaces/IRisc0Receiver.sol";
import {IDeviceReward} from "./interfaces/IDeviceReward.sol";
import {IDapp} from "./interfaces/IDapp.sol";
import {IioID} from "./interfaces/IioID.sol";
import {IioIDRegistry} from "./interfaces/IioIDRegistry.sol";

import "./lib/JournalParser.sol";

contract WSReceiver is IDapp {
    IDeviceReward public deviceRewards;
    IRisc0Receiver public risc0Verifier;
    IioID public ioID;
    IioIDRegistry public ioIDRegistry;

    mapping(uint256 => bytes32) private projectIdToImageId;

    constructor(address _verifierAddress, address _tokenAddress, address _ioIDAddress, address _ioIDRegistryAddress) {
        risc0Verifier = IRisc0Receiver(_verifierAddress);
        deviceRewards = IDeviceReward(_tokenAddress);
        ioID = IioID(_ioIDAddress);
        ioIDRegistry = IioIDRegistry(_ioIDRegistryAddress);
    }

    function process(uint256 _projectId, uint256, string memory, bytes calldata _data) external {
        if (address(risc0Verifier) == address(0)) {
            revert VerifierNotSet();
        }

        bytes32 imageId = projectIdToImageId[_projectId];
        if (imageId == bytes32(0)) {
            revert ImageIdNotFound(_projectId);
        }

        (bytes memory seal, bytes memory journal) = _decodeData(_data);
        _verify(seal, imageId, journal);

        // (JournalParser.Device[] memory devices, uint256 devicesLen) = _extractDevices(journal);
        // _distributeRewards(devices, devicesLen);
    }

    function _decodeData(bytes memory _data) internal pure returns (bytes memory, bytes memory) {
        (bytes memory proof_snark_seal, bytes memory proof_snark_journal) = abi.decode(_data, (bytes, bytes));
        return (proof_snark_seal, proof_snark_journal);
    }

    function _extractDevices(bytes memory _journal) internal pure returns (JournalParser.Device[] memory, uint256) {
        bytes memory devicesJsonString = JournalParser.byteStringToBytes(_journal);

        (JournalParser.Device[] memory devices, uint256 devicesLen) = JournalParser.parseDeviceJson(devicesJsonString);

        return (devices, devicesLen);
    }

    function _distributeRewards(JournalParser.Device[] memory devices, uint256 devicesLen) internal {
        // for (uint256 i; i < devicesLen; i++) {
        //     uint256 deviceId = devices[i].id;
        //     uint256 ioIDTokenId = ioIDRegistry.deviceTokenId(deviceId);
        //     address owner = IERC721(address(ioID)).ownerOf(ioIDTokenId);
        //     require(deviceRewards.mint(owner, devices[i].reward), "Token transfer failed");
        //     deviceRewards.mint(msg.sender, 25);
        //     // Emit the RewardsDistributed event after successful transfer.
        //     emit RewardsDistributed(owner, devices[i].reward);
        // }
    }

    function setReceiver(address _receiver) public {
        risc0Verifier = IRisc0Receiver(_receiver);
    }

    function setProjectIdToImageId(uint256 _projectId, bytes32 _imageId) public {
        projectIdToImageId[_projectId] = _imageId;
    }

    function getImageIdByProjectId(uint256 _projectId) public view returns (bytes32) {
        return projectIdToImageId[_projectId];
    }

    function _verify(bytes memory _seal, bytes32 _imageId, bytes memory _journal) internal {
        bytes32 proof_journal_hash = sha256(_journal);
        risc0Verifier.verify(_seal, _imageId, proof_journal_hash);
        emit ProofVerified(msg.sender, _imageId, proof_journal_hash);
    }
}
