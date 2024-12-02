import { buildModule } from '@nomicfoundation/hardhat-ignition/modules';

import RewardsModule from './DeviceRewards';
import { IOID, IOID_STORE, VERIFIER } from '../../constants';

const IMAGE_ID = process.env.IMAGE_ID ?? '';

export default buildModule('Dapp', m => {
  const { rewards } = m.useModule(RewardsModule);

  const dapp = m.contract('Dapp', [VERIFIER, rewards, IOID, IOID_STORE], {
    after: [rewards],
  });

  m.call(dapp, 'setImageId', [IMAGE_ID]);

  const MINTER_ROLE = m.staticCall(rewards, 'MINTER_ROLE');
  m.call(rewards, 'grantRole', [MINTER_ROLE, dapp]);

  return { dapp };
});
