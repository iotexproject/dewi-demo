import { buildModule } from '@nomicfoundation/hardhat-ignition/modules';

export default buildModule('DeviceNFT', m => {
  const nft = m.contract('DeviceNFT');
  return { nft };
});
