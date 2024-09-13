import { buildModule } from '@nomicfoundation/hardhat-ignition/modules';

export default buildModule('DeviceReward', m => {
  const rewards = m.contract('DeviceReward');

  return { rewards };
});
