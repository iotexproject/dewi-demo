// SPDX-License-Identifier: MIT
pragma solidity ^0.8.19;

import {ERC721Upgradeable, ERC721EnumerableUpgradeable} from "@openzeppelin/contracts-upgradeable/token/ERC721/extensions/ERC721EnumerableUpgradeable.sol";

import "../interfaces/IioID.sol";

contract IoIDMock is IioID, ERC721EnumerableUpgradeable {
    mapping(address => uint256) public override deviceProject;
    mapping(uint256 => uint256) public override projectDeviceCount;

    function wallet(uint256 _id) external view override returns (address wallet_, string memory did_) {}

    function mint(uint256 _projectId, address _device, address _owner) external override returns (uint256) {
        return _mint(_projectId, _device, _owner);
    }

    function _mint(uint256 _projectId, address _device, address _owner) internal returns (uint256 id_) {
        _mint(_owner, 3);
    }

    function removeDID(address _device) external {}

    function projectIDs(
        uint256 _projectId,
        address _start,
        uint256 _pageSize
    ) external view override returns (address[] memory array, address next) {}

    function did(address _device) public view override returns (string memory) {}
}
