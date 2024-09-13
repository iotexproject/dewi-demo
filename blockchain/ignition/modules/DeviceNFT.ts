import { buildModule } from '@nomicfoundation/hardhat-ignition/modules';

const DEVICE_OWNER = process.env.DEVICE_OWNER ?? '';

export default buildModule('DeviceNFT', m => {
  const nft = m.contract('DeviceNFT');

  // You can adjust the code below to mint necessary amount of DeviceNFTs
  m.call(nft, 'mint', [DEVICE_OWNER, 0], { id: 'token0' });
  m.call(nft, 'mint', [DEVICE_OWNER, 1], { id: 'token1' });
  m.call(nft, 'mint', [DEVICE_OWNER, 2], { id: 'token2' });
  m.call(nft, 'mint', [DEVICE_OWNER, 3], { id: 'token3' });

  return { nft };
});
