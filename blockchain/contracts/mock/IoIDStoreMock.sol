// SPDX-License-Identifier: MIT
pragma solidity ^0.8.19;

import {IERC721} from "@openzeppelin/contracts/token/ERC721/IERC721.sol";
import {OwnableUpgradeable} from "@openzeppelin/contracts-upgradeable/access/OwnableUpgradeable.sol";

import "../interfaces/IioIDStore.sol";

contract IoIDStoreMock is IioIDStore, OwnableUpgradeable {
    address public project;
    address public override ioIDRegistry;
    uint256 public override price;
    mapping(uint256 => address) public override projectDeviceContract;
    mapping(address => uint256) public override deviceContractProject;
    mapping(uint256 => uint256) public override projectAppliedAmount;
    mapping(uint256 => uint256) public override projectActivedAmount;
    address public override feeReceiver;

    function applyIoIDs(uint256 _projectId, uint256 _amount) external payable {}

    function setDeviceContract(uint256 _projectId, address _contract) external {
        projectDeviceContract[_projectId] = _contract;
    }

    function changeDeviceContract(uint256 _projectId, address _contract) external {}

    function activeIoID(uint256 _projectId) external payable {}

    function changePrice(uint256 _price) external {}
}
