import { expect } from 'chai';
import { ethers } from 'hardhat';
import { loadFixture } from '@nomicfoundation/hardhat-toolbox/network-helpers';

import { DeviceReward, RiscZeroGroth16Verifier, WSReceiver } from '../typechain-types';
import { DIGEST_STRING, IMAGE_ID, IMAGE_ID_2, JOURNAL_STRING, RAW_DATA, SEAL_RAW } from './testData';

const PROJECT_1_ID = 1;
const PROVER_1_ID = 1;
const CLIENT_1_ID = '1';

const DEVICE_OWNER_1 = '0xf39fd6e51aad88f6f4ce6ab8827279cfffb92266';
const RISC0_VERIFIER = '0xefBdae0483E7CA7AFD967791549ff8221e330DAC';
const RISC0_VERIFIER_2 = '0x6325D51b6F8bC78b00c55e6233e8824231C31DE2';
const DAPP = '0xf34aca92f2309794e41c3e1f645257878ccfb7bd';
const ROUTER = '0x3841a746f811c244292194825c5e528e61f890f8';
const IOID_ADDR = '0x45Ce3E6f526e597628c73B731a3e9Af7Fc32f5b7';
const IOID_REGISTRY = '0x0A7e595C7889dF3652A19aF52C18377bF17e027D';

describe('Receiver', function () {
  let receiver: WSReceiver;
  let rewards: DeviceReward;
  let verifier: RiscZeroGroth16Verifier;
  let proofData: string;

  before(function () {
    proofData = generateProofData();
  });

  beforeEach(async function () {
    receiver = await loadFixture(deployWSReceiver);

    const rewardsAddr = await receiver.deviceRewards();
    rewards = await ethers.getContractAt('DeviceReward', rewardsAddr);

    const MINTER_ROLE = await rewards.MINTER_ROLE();
    await rewards.grantRole(MINTER_ROLE, receiver);

    verifier = await deployVerifier();
  });

  describe('Initialization, setters and getters', () => {
    it('should include verifier address', async () => {
      expect(await receiver.risc0Verifier()).to.eq(RISC0_VERIFIER);
    });
    it('should include rewards address', async () => {
      expect(await receiver.deviceRewards()).not.eq(ethers.ZeroAddress);
    });
    it('should include ioIDAddress', async () => {
      expect(await receiver.ioID()).to.eq(IOID_ADDR);
    });
    it('should include ioIDRegistryAddress', async () => {
      expect(await receiver.ioIDRegistry()).to.eq(IOID_REGISTRY);
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

      await receiver.setProjectIdToImageId(PROJECT_1_ID, IMAGE_ID_2);
      await receiver.setReceiver(verifier.target);
      await expect(receiver.process(PROJECT_1_ID, PROVER_1_ID, CLIENT_1_ID, RAW_DATA))
        .to.emit(receiver, 'ProofVerified')
        .withArgs(
          sender.address,
          IMAGE_ID_2.toLowerCase(),
          '0xefc291907b53eb9abc5806b9904cdd0898e9c400d3049e5964d0eb9bd3a6b7e1',
        );
    });
    it('should revert if imageId is invalid', async () => {
      await receiver.setProjectIdToImageId(PROJECT_1_ID, IMAGE_ID);
      await receiver.setReceiver(verifier.target);
      await expect(receiver.process(PROJECT_1_ID, PROVER_1_ID, CLIENT_1_ID, RAW_DATA)).to.be.revertedWithCustomError(
        receiver,
        'VerificationFailed',
      );
    });
    it('should not process if verifier not set yet', async () => {
      await receiver.setReceiver(ethers.ZeroAddress);

      await expect(receiver.process(PROJECT_1_ID, PROVER_1_ID, CLIENT_1_ID, proofData)).to.be.revertedWithCustomError(
        receiver,
        'VerifierNotSet',
      );
    });
    it('should revert if project image id not found', async () => {
      await expect(receiver.process(PROJECT_1_ID, PROVER_1_ID, CLIENT_1_ID, proofData)).to.be.revertedWithCustomError(
        receiver,
        'ImageIdNotFound',
      );
    });
  });
  describe.skip('Rewards distribution', () => {
    it('should mint tokens to the device owner', async () => {
      await receiver.setProjectIdToImageId(PROJECT_1_ID, IMAGE_ID_2);
      await receiver.setReceiver(verifier.target);

      await receiver.process(PROJECT_1_ID, PROVER_1_ID, CLIENT_1_ID, RAW_DATA);

      expect(await rewards.balanceOf(DEVICE_OWNER_1)).to.eq(25);
    });
  });
});

const deployERC20 = async () => {
  const factory = await ethers.getContractFactory('DeviceReward');
  return factory.deploy();
};

const deployWSReceiver = async () => {
  const deviceRewards = await deployERC20();

  const WSReceiver = await ethers.getContractFactory('WSReceiver');
  return WSReceiver.deploy(RISC0_VERIFIER, deviceRewards, IOID_ADDR, IOID_REGISTRY);
};

const deployVerifier = async () => {
  const Verifier = await ethers.getContractFactory('RiscZeroGroth16Verifier');
  return Verifier.deploy('0x0eb6febcf06c5df079111be116f79bd8c7e85dc9448776ef9a59aaf2624ab551');
};

function generateProofData(): string {
  const seal = '0x' + Buffer.from(JSON.stringify(SEAL_RAW)).toString('hex');
  const digest = '0x' + Buffer.from(DIGEST_STRING).toString('hex');
  const journal = '0x' + Buffer.from(JOURNAL_STRING).toString('hex');
  const coder = new ethers.AbiCoder();

  return coder.encode(['bytes', 'bytes', 'bytes'], [seal, digest, journal]);
}
