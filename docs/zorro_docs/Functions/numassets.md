---
title: NumAssetsListed
url: https://zorro-project.com/manual/en/numassets.htm
category: Functions
subcategory: None
related_pages:
- [Asset and Account Lists](account.htm)
- [asset, assetList, ...](asset.htm)
---

# NumAssetsListed

## NumAssetsListed

Number of assets in the selected [asset list](account.htm). 

## NumAssetsUsed

Number of assets selected by [asset](asset.htm) calls. 

### Type:

**int** , read/only 

### Example:

```c
for(i=0; i<NumAssetsListed; i++) {
  string AssetName = Assets[i];
  ...
}
```

### See also:

[asset](asset.htm), [asset list](account.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
