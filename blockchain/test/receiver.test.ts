import { expect } from 'chai';
import { ethers } from 'hardhat';
import { loadFixture } from '@nomicfoundation/hardhat-toolbox/network-helpers';

import { DeviceNFTMock, DeviceReward, IoIDStoreMock, RiscZeroGroth16Verifier, WSReceiver } from '../typechain-types';
import { IMAGE_ID, IMAGE_ID_2, RAW_DATA } from './testData';
import { IoIDMock } from '../typechain-types/contracts/mock/IoIDMock.sol';

const PROJECT_1_ID = '0x036c';
const PROVER_1_ID = '0x01';
const CLIENT_1_ID = '';
const ZERO_BYTES = '0x0000000000000000000000000000000000000000000000000000000000000000';
const CHAIN_ID = 4690;
const IOID_TOKEN_ID = 3;
const REWARDS_AMOUNT = 4; // update if journal changes

const DEVICE_OWNER_1 = '0xf39fd6e51aad88f6f4ce6ab8827279cfffb92266';
const RISC0_VERIFIER = '0xefBdae0483E7CA7AFD967791549ff8221e330DAC';
const RISC0_VERIFIER_2 = '0x6325D51b6F8bC78b00c55e6233e8824231C31DE2';
const DAPP = '0xf34aca92f2309794e41c3e1f645257878ccfb7bd';
const ROUTER = '0x3841a746f811c244292194825c5e528e61f890f8';
const IOID_ADDR = '0x45Ce3E6f526e597628c73B731a3e9Af7Fc32f5b7';
const IOID_REGISTRY = '0x0A7e595C7889dF3652A19aF52C18377bF17e027D';
const IOID_STORE = '0x60cac5CE11cb2F98bF179BE5fd3D801C3D5DBfF2';
const DEVICE_ADD = '0x07b7265219329a3b30ab60b8d532b9e8990cc6e0';

describe('Receiver', function () {
  let receiver: WSReceiver;
  let rewards: DeviceReward;
  let ioidStore: IoIDStoreMock;
  let ioid: IoIDMock;
  let verifier: RiscZeroGroth16Verifier;

  beforeEach(async function () {
    receiver = await loadFixture(deployWSReceiver);
    verifier = await loadFixture(deployVerifier);

    const { store, token } = await setupIoID(receiver);
    ioid = token;
    ioidStore = store;

    // ALLOW RECEIVER TO MINT REWARDS
    const rewardsAddr = await receiver.deviceRewards();
    rewards = await ethers.getContractAt('DeviceReward', rewardsAddr);
    const MINTER_ROLE = await rewards.MINTER_ROLE();
    await rewards.grantRole(MINTER_ROLE, receiver);
  });

  describe('Initialization, setters and getters', () => {
    it('should include verifier address', async () => {
      expect(await receiver.risc0Verifier()).to.eq(RISC0_VERIFIER);
    });
    it('should include rewards address', async () => {
      expect(await receiver.deviceRewards()).not.eq(ethers.ZeroAddress);
    });
    it('should include ioIDAddress', async () => {
      expect(await receiver.ioID()).to.eq(ioid.target);
    });
    it('should include ioidStore address', async () => {
      expect(await receiver.ioIDStore()).to.eq(ioidStore.target);
    });
    it('should set and get verifier', async () => {
      await receiver.setReceiver(RISC0_VERIFIER_2);

      expect(await receiver.risc0Verifier()).to.eq(RISC0_VERIFIER_2);
    });
    it('should set and get projectImageId', async () => {
      await receiver.setProjectIdToImageId(PROJECT_1_ID, IMAGE_ID);

      expect(await receiver.getImageIdByProjectId(PROJECT_1_ID)).to.eq(IMAGE_ID.toLowerCase());
    });
  });
  describe('Verification', () => {
    it('should emit verified event', async () => {
      const [sender] = await ethers.getSigners();

      await receiver.setProjectIdToImageId(PROJECT_1_ID, IMAGE_ID);
      await receiver.setReceiver(verifier.target);
      await expect(receiver.process(PROJECT_1_ID, PROVER_1_ID, CLIENT_1_ID, RAW_DATA))
        .to.emit(receiver, 'ProofVerified')
        .withArgs(
          sender.address,
          IMAGE_ID.toLowerCase(),
          '0xa4e07acb9ef3913e6639045169a005790cb1cb725dbbcdc9c69d474f1bde19e3',
        );
    });
    it('should revert if imageId is invalid', async () => {
      await receiver.setProjectIdToImageId(PROJECT_1_ID, IMAGE_ID_2);
      await receiver.setReceiver(verifier.target);
      await expect(receiver.process(PROJECT_1_ID, PROVER_1_ID, CLIENT_1_ID, RAW_DATA)).to.be.revertedWithCustomError(
        receiver,
        'VerificationFailed',
      );
    });
    it('should not process if verifier not set yet', async () => {
      await receiver.setReceiver(ethers.ZeroAddress);

      await expect(receiver.process(PROJECT_1_ID, PROVER_1_ID, CLIENT_1_ID, RAW_DATA)).to.be.revertedWithCustomError(
        receiver,
        'VerifierNotSet',
      );
    });
    it('should revert if project image id not found', async () => {
      await expect(receiver.process(PROJECT_1_ID, PROVER_1_ID, CLIENT_1_ID, RAW_DATA)).to.be.revertedWithCustomError(
        receiver,
        'ImageIdNotFound',
      );
    });
  });
  describe('Rewards distribution', () => {
    beforeEach(async () => {
      await receiver.setProjectIdToImageId(PROJECT_1_ID, IMAGE_ID);
      await receiver.setReceiver(verifier.target);
    });
    it('should mint tokens to the device owner', async () => {
      const [, deviceOwner] = await ethers.getSigners();

      await receiver.process(PROJECT_1_ID, PROVER_1_ID, CLIENT_1_ID, RAW_DATA);
      expect(await rewards.balanceOf(deviceOwner)).to.eq(REWARDS_AMOUNT);
    });
  });
});

describe('Distribution reverts', () => {
  let receiver: WSReceiver;
  let rewards: DeviceReward;
  let verifier: RiscZeroGroth16Verifier;

  beforeEach(async function () {
    receiver = await loadFixture(deployWSReceiver);
    verifier = await loadFixture(deployVerifier);

    // ALLOW RECEIVER TO MINT REWARDS
    const rewardsAddr = await receiver.deviceRewards();
    rewards = await ethers.getContractAt('DeviceReward', rewardsAddr);
    const MINTER_ROLE = await rewards.MINTER_ROLE();
    await rewards.grantRole(MINTER_ROLE, receiver);

    await receiver.setProjectIdToImageId(PROJECT_1_ID, IMAGE_ID);
    await receiver.setReceiver(verifier.target);
  });

  it('should revert if project device contract not set', async () => {
    await expect(receiver.process(PROJECT_1_ID, PROVER_1_ID, CLIENT_1_ID, RAW_DATA))
      .to.be.revertedWithCustomError(receiver, 'DeviceContractNotSet')
      .withArgs(PROJECT_1_ID);
  });
});

const deployERC20 = async () => {
  const factory = await ethers.getContractFactory('DeviceReward');
  return factory.deploy();
};

const deployWSReceiver = async () => {
  const deviceRewards = await deployERC20();
  const ioidStore = await deployIoIDStore();
  const ioid = await deployIoID();

  const WSReceiver = await ethers.getContractFactory('WSReceiver');
  return WSReceiver.deploy(RISC0_VERIFIER, deviceRewards, ioid, ioidStore);
};

const deployVerifier = async () => {
  const Verifier = await ethers.getContractFactory('RiscZeroGroth16Verifier');
  return Verifier.deploy('0x0eb6febcf06c5df079111be116f79bd8c7e85dc9448776ef9a59aaf2624ab551');
};

const deployIoIDStore = async () => {
  const factory = await ethers.getContractFactory('IoIDStoreMock');
  return factory.deploy();
};

const deployDeviceNFT = async () => {
  const factory = await ethers.getContractFactory('DeviceNFTMock');
  return factory.deploy();
};

const deployERC6551Implementation = async () => {
  const factory = await ethers.getContractFactory('ERC6551Account');
  return factory.deploy();
};

const deployERC6551Registry = async () => {
  const factory = await ethers.getContractFactory('ERC6551Registry');
  return factory.deploy();
};

const deployIoID = async () => {
  const factory = await ethers.getContractFactory('IoIDMock');
  return factory.deploy();
};

const setupIoID = async (receiver: WSReceiver) => {
  const deviceNft = await loadFixture(deployDeviceNFT);

  const ioidStore = await setDeviceContract(receiver, deviceNft);
  const wallet = await deployDeviceWallet();
  // MINT DEVICE NFT 0 TO THE DEVICE WALLET
  await deviceNft.mint(wallet);
  const ioid = await mintIoIDToOwner(receiver);

  return { store: ioidStore, token: ioid };
};

const setDeviceContract = async (receiver: WSReceiver, deviceNft: DeviceNFTMock) => {
  const ioidStoreAddr = await receiver.ioIDStore();
  const ioidStore = await ethers.getContractAt('IoIDStoreMock', ioidStoreAddr);
  await ioidStore.setDeviceContract(PROJECT_1_ID, deviceNft);

  return ioidStore;
};

const deployDeviceWallet = async () => {
  const erc6551Implementation = await loadFixture(deployERC6551Implementation);
  const erc6551Registry = await loadFixture(deployERC6551Registry);

  await erc6551Registry.createAccount(erc6551Implementation, ZERO_BYTES, CHAIN_ID, IOID_ADDR, IOID_TOKEN_ID);
  const wallet = await erc6551Registry.account(erc6551Implementation, ZERO_BYTES, CHAIN_ID, IOID_ADDR, IOID_TOKEN_ID);

  return wallet;
};

const mintIoIDToOwner = async (receiver: WSReceiver) => {
  const [, deviceOwner] = await ethers.getSigners();

  const ioidAddr = await receiver.ioID();
  const ioid = await ethers.getContractAt('IoIDMock', ioidAddr);
  await ioid.mint(PROJECT_1_ID, DEVICE_ADD, deviceOwner);

  return ioid;
};
