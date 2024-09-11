// SPDX-License-Identifier: MIT
// Compatible with OpenZeppelin Contracts ^5.0.0
pragma solidity ^0.8.19;

import "@openzeppelin/contracts/token/ERC721/ERC721.sol";
import "@openzeppelin/contracts/access/Ownable.sol";

contract DeviceNFT is ERC721, Ownable {
    constructor() ERC721("DeviceNFT", "D") Ownable() {}

    function _baseURI() internal pure override returns (string memory) {
        return "ipfs://QmQWRyjrW9uSCGSUd2wGuZfGF4c7jzSKSaEZxHfvi3Prpm";
    }

    function mint(address to, uint256 tokenId) public onlyOwner {
        _mint(to, tokenId);
    }

    function tokenURI(uint256 tokenId) public view virtual override returns (string memory) {
        _requireMinted(tokenId);

        return _baseURI();
    }
}
