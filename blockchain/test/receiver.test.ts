import { expect } from 'chai';
import { ethers } from 'hardhat';
import { loadFixture } from '@nomicfoundation/hardhat-toolbox/network-helpers';

import { DeviceNFT, DeviceReward, IoIDStoreMock, Dapp, IoIDMock } from '../typechain-types';
import { IMAGE_ID, IMAGE_ID_2, RAW_DATA } from './testData';
import { IOID } from '../constants';

const PROJECT_1_ID = '0x036c';
const TASK_1_ID = ethers.randomBytes(32);
const PROVER_1 = ethers.ZeroAddress;
const DEVICE_ID = ethers.ZeroAddress;
const REWARDS_AMOUNT = 4; // UPDATE IF THE VALUE IN JOURNAL CHANGES

describe('Dapp', function () {
  let dapp: Dapp;
  let rewards: DeviceReward;
  let ioidStore: IoIDStoreMock;
  let ioid: IoIDMock;

  let verifier: string;

  beforeEach(async function () {
    dapp = await loadFixture(deployDapp);
    verifier = await dapp.risc0Verifier();

    const { store, token } = await setupIoID(dapp);
    ioid = token;
    ioidStore = store;

    // ALLOW THE DAPP TO MINT REWARDS
    const rewardsAddr = await dapp.deviceRewards();
    rewards = await ethers.getContractAt('DeviceReward', rewardsAddr);
    const MINTER_ROLE = await rewards.MINTER_ROLE();
    await rewards.grantRole(MINTER_ROLE, dapp);
  });

  describe('Initialization, setters and getters', () => {
    it('should include verifier address', async () => {
      expect(await dapp.risc0Verifier()).to.eq(verifier);
    });
    it('should include rewards address', async () => {
      expect(await dapp.deviceRewards()).not.eq(ethers.ZeroAddress);
    });
    it('should include ioIDAddress', async () => {
      expect(await dapp.ioID()).to.eq(ioid.target);
    });
    it('should include ioidStore address', async () => {
      expect(await dapp.ioIDStore()).to.eq(ioidStore.target);
    });
    it('should set and get verifier', async () => {
      const RISC0_VERIFIER_2 = '0x6325D51b6F8bC78b00c55e6233e8824231C31DE2';

      expect(await dapp.risc0Verifier()).to.eq(verifier);
      await dapp.setReceiver(RISC0_VERIFIER_2);
      expect(await dapp.risc0Verifier()).to.eq(RISC0_VERIFIER_2);
    });
    it('should set and get projectImageId', async () => {
      await dapp.setImageId(IMAGE_ID);

      expect(await dapp.imageId()).to.eq(IMAGE_ID.toLowerCase());
    });
  });
  describe('Verification', () => {
    it('should emit verified event', async () => {
      const [sender] = await ethers.getSigners();

      await dapp.setImageId(IMAGE_ID);
      await expect(dapp.process(PROJECT_1_ID, TASK_1_ID, PROVER_1, DEVICE_ID, RAW_DATA))
        .to.emit(dapp, 'ProofVerified')
        .withArgs(
          sender.address,
          IMAGE_ID.toLowerCase(),
          '0xa4e07acb9ef3913e6639045169a005790cb1cb725dbbcdc9c69d474f1bde19e3',
        );
    });
    it('should revert if imageId is invalid', async () => {
      await dapp.setImageId(IMAGE_ID_2);
      const verifierContract = await ethers.getContractAt('RiscZeroGroth16Verifier', verifier);
      await expect(dapp.process(PROJECT_1_ID, TASK_1_ID, PROVER_1, DEVICE_ID, RAW_DATA)).to.be.revertedWithCustomError(
        verifierContract,
        'VerificationFailed',
      );
    });
    it('should not process if verifier not set yet', async () => {
      await dapp.setReceiver(ethers.ZeroAddress);

      await expect(dapp.process(PROJECT_1_ID, TASK_1_ID, PROVER_1, DEVICE_ID, RAW_DATA)).to.be.revertedWith(
        'Verifier not set',
      );
    });
    it('should revert if project image id not found', async () => {
      await expect(dapp.process(PROJECT_1_ID, TASK_1_ID, PROVER_1, DEVICE_ID, RAW_DATA)).to.be.rejectedWith(
        'Image not found',
      );
    });
  });
  describe('Rewards distribution', () => {
    beforeEach(async () => {
      await dapp.setImageId(IMAGE_ID);
    });
    it('should mint tokens to the device owner', async () => {
      const [, deviceOwner] = await ethers.getSigners();

      await dapp.process(PROJECT_1_ID, TASK_1_ID, PROVER_1, DEVICE_ID, RAW_DATA);
      expect(await rewards.balanceOf(deviceOwner)).to.eq(REWARDS_AMOUNT);
    });
  });
});

describe('Distribution reverts', () => {
  let dapp: Dapp;
  let rewards: DeviceReward;

  beforeEach(async function () {
    dapp = await loadFixture(deployDapp);

    // ALLOW THE DAPP TO MINT REWARDS
    const rewardsAddr = await dapp.deviceRewards();
    rewards = await ethers.getContractAt('DeviceReward', rewardsAddr);
    const MINTER_ROLE = await rewards.MINTER_ROLE();
    await rewards.grantRole(MINTER_ROLE, dapp);

    await dapp.setImageId(IMAGE_ID);
  });

  it('should revert if project device contract not set', async () => {
    await expect(dapp.process(PROJECT_1_ID, TASK_1_ID, PROVER_1, DEVICE_ID, RAW_DATA)).to.be.rejectedWith(
      'Device Contract not set',
    );
  });
});

const deployERC20 = async () => {
  const factory = await ethers.getContractFactory('DeviceReward');
  return factory.deploy();
};

const deployDapp = async () => {
  const deviceRewards = await deployERC20();
  const ioidStore = await deployIoIDStore();
  const ioid = await deployIoID();
  const verifier = await deployVerifier();

  const Dapp = await ethers.getContractFactory('Dapp');
  return Dapp.deploy(verifier, deviceRewards, ioid, ioidStore);
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
  const factory = await ethers.getContractFactory('DeviceNFT');
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

const setupIoID = async (dapp: Dapp) => {
  const deviceNft = await loadFixture(deployDeviceNFT);

  const ioidStore = await setDeviceContract(dapp, deviceNft);
  const wallet = await deployDeviceWallet();
  // MINT DEVICE NFT 0 TO THE DEVICE WALLET
  await deviceNft.mint(wallet, 0);
  const ioid = await mintIoIDToOwner(dapp);

  return { store: ioidStore, token: ioid };
};

const setDeviceContract = async (dapp: Dapp, deviceNft: DeviceNFT) => {
  const ioidStoreAddr = await dapp.ioIDStore();
  const ioidStore = await ethers.getContractAt('IoIDStoreMock', ioidStoreAddr);
  await ioidStore.setDeviceContract(PROJECT_1_ID, deviceNft);

  return ioidStore;
};

const deployDeviceWallet = async () => {
  const CHAIN_ID = 4690;
  const IOID_TOKEN_ID = 3;

  const erc6551Implementation = await loadFixture(deployERC6551Implementation);
  const erc6551Registry = await loadFixture(deployERC6551Registry);

  await erc6551Registry.createAccount(erc6551Implementation, ethers.ZeroHash, CHAIN_ID, IOID, IOID_TOKEN_ID);
  const wallet = await erc6551Registry.account(erc6551Implementation, ethers.ZeroHash, CHAIN_ID, IOID, IOID_TOKEN_ID);

  return wallet;
};

const mintIoIDToOwner = async (dapp: Dapp) => {
  const DEVICE_ADDRESS = '0x07b7265219329a3b30ab60b8d532b9e8990cc6e0';
  const [, deviceOwner] = await ethers.getSigners();

  const ioidAddr = await dapp.ioID();
  const ioid = await ethers.getContractAt('IoIDMock', ioidAddr);
  await ioid.mint(PROJECT_1_ID, DEVICE_ADDRESS, deviceOwner);

  return ioid;
};
