import { buildModule } from '@nomicfoundation/hardhat-ignition/modules';

export default buildModule('Dapp', m => {
  const rewards = m.contract('DeviceReward');

  return { rewards };
});
