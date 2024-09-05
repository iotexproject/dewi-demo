import { buildModule } from '@nomicfoundation/hardhat-ignition/modules';

const IOID_ADDR = '0x45Ce3E6f526e597628c73B731a3e9Af7Fc32f5b7';
const IOID_REGISTRY = '0x0A7e595C7889dF3652A19aF52C18377bF17e027D';

export default buildModule('WSReceiver', m => {
  const verifier = m.contract('RiscZeroGroth16Verifier', [
    '0x0eb6febcf06c5df079111be116f79bd8c7e85dc9448776ef9a59aaf2624ab551',
  ]);
  const rewards = m.contract('DeviceReward');
  const receiver = m.contract('WSReceiver', [verifier, rewards, IOID_ADDR, IOID_REGISTRY], {
    after: [verifier, rewards],
  });

  return { receiver };
});
