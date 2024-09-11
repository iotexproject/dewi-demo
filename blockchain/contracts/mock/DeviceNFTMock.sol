// SPDX-License-Identifier: MIT
pragma solidity ^0.8.19;

import {OwnableUpgradeable} from "@openzeppelin/contracts-upgradeable/access/OwnableUpgradeable.sol";
import {ERC721Upgradeable} from "@openzeppelin/contracts-upgradeable/token/ERC721/ERC721Upgradeable.sol";

contract DeviceNFTMock is ERC721Upgradeable, OwnableUpgradeable {
    function mint(address _to) external returns (uint256) {
        _mint(_to, 0);
        return 0;
    }
}
