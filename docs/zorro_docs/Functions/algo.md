---
title: algo
url: https://zorro-project.com/manual/en/algo.htm
category: Functions
subcategory: None
related_pages:
- [optimize](optimize.htm)
- [adviseLong, adviseShort](advisor.htm)
- [Money Management](optimalf.htm)
- [asset, assetList, ...](asset.htm)
- [Virtual Hedging](hedge.htm)
- [System strings](script.htm)
- [String handling](str_.htm)
- [Trade Statistics](winloss.htm)
- [Performance Report](performance.htm)
- [AlgoVar, AssetVar, AssetStr](algovar.htm)
- [loop](loop.htm)
- [enterLong, enterShort](buylong.htm)
- [Variables](aarray.htm)
- [trade management](trade.htm)
---

# algo

## algo (string name): int 

Sets the algorithm identifier for identifying trades. Using algorithm identifiers is recommended in portfolio strategies that contain different trade algorithms; they are used to create separate [strategy parameters](optimize.htm), [rules](advisor.htm), and [capital allocation factors](optimalf.htm) per algorithm. 

### Parameters:

**name** | The algorithm identifier (max. 15 characters, no spaces). If **name** ends with **":L"** , the algo is for long trades only; if it ends with **":S"** the algo is for short trades only.   
---|---  
  
### Returns:

**0** when the **name** string is **NULL** or empty, otherwise nonzero. 

### Usage:

**algo("TREND:L");** sets up the identifier **"TREND:L"** for the current algorithm for long trades. Short trades are suppressed. 

### Remarks:

  * The [assetList](asset.htm) (if any) and an [asset](asset.htm) must be selected before calling **algo()**. 
  * For getting separate factors, parameters, or rules for long and short trades, call the same algorithm with two algo identifiers that end with **":L"** and **":S"** , like the trade names in the message window and parameter files. Long trades are then automatically suppressed on **":S"** algos and short trades on **":L"** algos, but reverse positions are still closed depending on [Hedge](hedge.htm). Don't use **":L"** or **":S"** for symmetric strategies with the same factors, parameters, or rules for long and short trades, or for strategies that trade only long or only short. 
  * The algorithm identifier is stored in the **[Algo](script.htm)** string variable, and can be evaluated in strategies with the [strstr](str_.htm) function.
  * Every **algo** call switches the [trade statistics parameters](winloss.htm) and the [OptimalF](optimalf.htm) factors to the current statistics and factors of the selected algorithm identifier. 
  * Any algo/asset combination is a **component** in a portfolio strategy. The [performance report](performance.htm) lists strategy results separated by components. The **Component** variable is the number of the current component, starting with **0** , and can be used as an array index. 
  * Algorithm specific data can be stored in the [AlgoVar](algovar.htm) variables.
  * For [training](optimize.htm) algo dependent parameters separately, switch algos in a [loop](loop.htm). 

### Example:

```c
algo("TREND:L"); // algorithm identifier for a trend trading algorithm with long trades
...
if(short_condition) 
  enterShort(ifelse(strstr(Algo,":L"),0,Lots)); // suppress short trades when Algo ends with ":L"
```

### See also:

[enterLong](buylong.htm), [enterShort](buylong.htm), [loop](loop.htm), [string](aarray.htm), [asset](asset.htm), [Algo](script.htm), [AlgoVar](algovar.htm), [TradeAlgo](trade.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
