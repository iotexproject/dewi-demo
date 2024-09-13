// SPDX-License-Identifier: MIT
pragma solidity ^0.8.19;

import {IERC721} from "@openzeppelin/contracts/token/ERC721/IERC721.sol";
import {IERC20} from "@openzeppelin/contracts/token/ERC20/IERC20.sol";

import {IRisc0Receiver} from "./interfaces/IRisc0Receiver.sol";
import {IDeviceReward} from "./interfaces/IDeviceReward.sol";
import {IDapp} from "./interfaces/IDapp.sol";
import {IioIDStore} from "./interfaces/IioIDStore.sol";
import {IERC6551Account} from "./interfaces/IERC6551Account.sol";

import "./lib/JournalParser.sol";

contract Dapp is IDapp {
    IDeviceReward public deviceRewards;
    IRisc0Receiver public risc0Verifier;
    IioIDStore public ioIDStore;
    address public ioID;

    mapping(uint256 => bytes32) private projectIdToImageId;

    constructor(address _verifierAddress, address _tokenAddress, address _ioIDAddress, address _ioIDStore) {
        risc0Verifier = IRisc0Receiver(_verifierAddress);
        deviceRewards = IDeviceReward(_tokenAddress);
        ioID = _ioIDAddress;
        ioIDStore = IioIDStore(_ioIDStore);
    }

    function process(uint256 _projectId, uint256, string memory, bytes calldata _data) external {
        require(address(risc0Verifier) != address(0), "Verifier not set");

        bytes32 imageId = projectIdToImageId[_projectId];
        require(imageId != bytes32(0), "Image not found");

        (bytes memory seal, bytes memory journal) = _decodeData(_data);
        _verify(seal, imageId, journal);

        (JournalParser.Device[] memory devices, uint256 devicesLen) = _extractDevices(journal);
        _distributeRewards(_projectId, devices, devicesLen);
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

    function _decodeData(bytes memory _data) internal pure returns (bytes memory, bytes memory) {
        (bytes memory proof_snark_seal, bytes memory proof_snark_journal) = abi.decode(_data, (bytes, bytes));
        return (proof_snark_seal, proof_snark_journal);
    }

    function _verify(bytes memory _seal, bytes32 _imageId, bytes memory _journal) internal {
        bytes32 proof_journal_hash = sha256(_journal);

        risc0Verifier.verify(_seal, _imageId, proof_journal_hash);
        emit ProofVerified(msg.sender, _imageId, proof_journal_hash);
    }

    function _extractDevices(bytes memory _journal) internal pure returns (JournalParser.Device[] memory, uint256) {
        (JournalParser.Device[] memory devices, uint256 devicesLen) = JournalParser.parseDeviceJson(_journal);
        return (devices, devicesLen);
    }

    function _distributeRewards(
        uint256 _projectId,
        JournalParser.Device[] memory devices,
        uint256 devicesLen
    ) internal {
        for (uint256 i; i < devicesLen; i++) {
            uint256 deviceId = devices[i].id;
            address owner = _getDeviceOwner(_projectId, deviceId);

            deviceRewards.mint(owner, devices[i].reward);
            emit RewardsDistributed(owner, devices[i].reward);
        }
    }

    function _getDeviceOwner(uint256 _projectId, uint256 _deviceId) internal view returns (address) {
        address projectDeviceContract = ioIDStore.projectDeviceContract(_projectId);
        require(projectDeviceContract != address(0), "Device Contract not set");

        address ownerOfDeviceToken = IERC721(projectDeviceContract).ownerOf(_deviceId);
        IERC6551Account wallet = IERC6551Account(payable(ownerOfDeviceToken));
        (, , uint256 tokenId) = wallet.token();

        address owner = IERC721(ioID).ownerOf(tokenId);

        return owner;
    }
}
