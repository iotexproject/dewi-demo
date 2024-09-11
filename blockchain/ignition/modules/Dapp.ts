import { buildModule } from '@nomicfoundation/hardhat-ignition/modules';

const IOID_ADDR = '0x45Ce3E6f526e597628c73B731a3e9Af7Fc32f5b7';
const IOID_REGISTRY = '0x0A7e595C7889dF3652A19aF52C18377bF17e027D';
const VERIFIER = '0xefBdae0483E7CA7AFD967791549ff8221e330DAC';
// const MOCK_REWARD = '0x7d2c3d43e41ea36fc4e5294c47ef12c6a736589b';
const IOID_STORE = '0x60cac5CE11cb2F98bF179BE5fd3D801C3D5DBfF2';
const PROJECT_1_ID = '0x036c';
const IMAGE_ID = '0xc4b0c497db7be05c23b0aacfbd0d4beebe109f94c64599c3b1265fa1d7a083ce';

export default buildModule('Dapp', m => {
  const rewards = m.contract('DeviceReward');
  const MINTER_ROLE = m.staticCall(rewards, 'MINTER_ROLE');

  const dapp = m.contract('Dapp', [VERIFIER, rewards, IOID_ADDR, IOID_STORE], {
    after: [rewards],
  });

  m.call(dapp, 'setProjectIdToImageId', [PROJECT_1_ID, IMAGE_ID]);

  m.call(rewards, 'grantRole', [MINTER_ROLE, dapp]);

  return { dapp };
});
