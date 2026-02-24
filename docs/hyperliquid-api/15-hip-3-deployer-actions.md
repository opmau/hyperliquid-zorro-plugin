<!-- Source: https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/hip-3-deployer-actions -->

The API for deploying and operating builder-deployed perpetual dexs involves the following L1 action:

Copy

```
// IMPORTANT: All lists of tuples should be lexographically sorted before signing
type PerpDeployAction =
  | {
      type: "perpDeploy",
      registerAsset2: RegisterAsset2,
    }
  | {
      type: "perpDeploy",
      registerAsset: RegisterAsset,
    }
  | {
      type: "perpDeploy",
      setOracle: SetOracle,
    }
  | {
      type: "perpDeploy",
      setFundingMultipliers: SetFundingMultipliers,
    }
  | {
      type: "perpDeploy",
      setFundingInterestRates: SetFundingInterestRates,
    }
  | {
      type: "perpDeploy",
      haltTrading: { coin: string, isHalted: boolean },
    }
  | {
      type: "perpDeploy",
      setMarginTableIds: SetMarginTableIds,
    }
  | {
      type: "perpDeploy",
      setFeeRecipient: { dex: string, feeRecipient: address },
    }
  | {
      type: "perpDeploy",
      setOpenInterestCaps: SetOpenInterestCaps
    }
  | {
      type: "perpDeploy",
      setSubDeployers: { dex: string, subDeployers: Array<SubDeployerInput> }
    }
  | {
      type: "perpDeploy",
      setMarginModes: SetMarginModes
    }
  | {
      type: "perpDeploy",
      setFeeScale: SetFeeScale
    }
  | {
      type: "perpDeploy",
      setGrowthModes: SetGrowthModes
    };
```

See [https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/info-endpoint/perpetuals#retrieve-information-about-the-perp-deploy-auctionarrow-up-right](https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/info-endpoint/perpetuals#retrieve-information-about-the-perp-deploy-auction) for how to query for the perp deploy auction status.

### [hashtag](#open-interest-caps) Open interest caps

Builder-deployed perp markets are subject to two types of open interest caps: notional (sum of absolute position size times mark price) and size (sum of absolute position sizes).

Notional open interest caps are enforced on the total open interest summed over all assets within the DEX, as well as per-asset. Perp deployers can set a custom open interest cap per asset, which is documented in [https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/hip-3-deployer-actionsarrow-up-right](https://hyperliquid.gitbook.io/hyperliquid-docs/for-developers/api/hip-3-deployer-actions).

Size-denominated open interest caps are only enforced per-asset. Size-denominated open interest caps are currently a constant 1B per asset, so a reasonable default would be to set `szDecimals` such that the minimal size increment is $1-10 at the initial mark price.

[PreviousDeploying HIP-1 and HIP-2 assetschevron-left](/hyperliquid-docs/for-developers/api/deploying-hip-1-and-hip-2-assets)[NextHyperEVMchevron-right](/hyperliquid-docs/for-developers/hyperevm)

Last updated 20 days ago