---
title: asset, assetType
url: https://zorro-project.com/manual/en/asset.htm
category: Functions
subcategory: None
related_pages:
- [Asset and Account Lists](account.htm)
- [set, is](mode.htm)
- [price, ...](price.htm)
- [Spread, Commission, ...](spread.htm)
- [System strings](script.htm)
- [AlgoVar, AssetVar, AssetStr](algovar.htm)
- [Status flags](is.htm)
- [ContractType, Multiplier, ...](contracts.htm)
- [Asset Symbols](symbol.htm)
- [NumAssetsListed](numassets.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [Time zones](assetzone.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [Detrend, shuffling](detrend.htm)
- [Dates, holidays, market](date.htm)
- [BarMode](barmode.htm)
- [Error Messages](errors.htm)
- [optimize](optimize.htm)
- [adviseLong, adviseShort](advisor.htm)
- [Training](training.htm)
- [Trade Statistics](winloss.htm)
- [Money Management](optimalf.htm)
- [loop](loop.htm)
- [for(open_trades), ...](fortrades.htm)
- [Tips & Tricks](tips.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [memory](memory.htm)
- [plot, plotBar, ...](plot.htm)
- [Visual Studio](dlls.htm)
- [enterLong, enterShort](buylong.htm)
- [algo](algo.htm)
- [assetHistory](loadhistory.htm)
---

# asset, assetType

## asset (string Name) : int

Selects an asset from the [asset list](account.htm), and loads its price history in the initial run from the broker or historical data. On subsequent script starts, price data is not loaded again unless [PRELOAD](mode.htm) was set or the [Edit] button was clicked. [Price](price.htm) and trade functions, and all asset related variables ([Spread](spread.htm), [Symbol](script.htm), [AssetVar](algovar.htm) etc.) are automatically switched to the new asset. Sets [AssetPrev](script.htm) to the previous asset name. Must be called in the first run (**[INITRUN](is.htm)**) for any asset used in the script.  

### Parameters:

**Name** | The name of the asset, as in the [asset list](account.htm) or the [Asset] selector. An empty string **""** or a name beginning with **'#'** creates a dummy asset with flat price history. Up to 15 characters, uppercase, with no blanks and no special characters except for slash '/' and underline '_'.  
---|---  
  
### Returns:

**0** when the **Name** string is **NULL** or empty, or when the asset or its prices are not available; otherwise nonzero. 

### Usage:

**asset("EUR/USD");** selects the EUR/USD pair.   

## assetAdd (string Name)

## assetAdd (string Name, string Symbol)

## assetAdd (string Name, var Price, var Spread, var RollLong, var RollShort, var PipVal, var PipCost, var MarginCost, var Leverage, var LotAmount, var Commission, string Symbol)

Selects an asset and optionally updates its parameters in the [INITRUN](status.htm). If the asset was not yet in the [asset list](account.htm), it is added and also appears in the [Asset] scrollbox. Unlike **asset()** , the asset history is not yet loaded and the asset is not yet subscribed. For creating a dummy asset for test purposes, let **Name** begin with a **'#'** hash - this will generate artificial bars with a flat price history. Selecting an asset before loading its price history can be useful when asset specific parameters like [Centage](contracts.htm) affect the subsequent history download or its [Symbol](symbol.htm), price source, or assigned account has to be set up by script.   

### Parameters:

**Name** | Name of the asset. A name beginning with **'#'** creates a dummy asset that will also appear in the scrollbox.  
---|---  
**Symbol** | Symbol of the asset, with optional source, in the format described under [asset list](account.htm).   
**Price, ...** | Optional asset parameters as described under [asset list](account.htm). When at **0** , the parameter is not changed.  
  
### Usage:

**assetAdd("AAPL",150,0.01,0,0,0.01,0.01,0,2,1,0.02,"STOOQ:AAPL.US");**  

## assetList (string Filename,_string Select_): int 

Loads an alternative [asset list](account.htm), adds its assets to the [Asset] scrollbox, and selects an asset from the new list. Any asset used in the script must be either in that list, or added by script with **assetAdd**. The asset list must be loaded in the first run (**[INITRUN](is.htm)**) of the script.before its assets can be selected. If this function is not called, the default list of the currently selected [account](account.htm) is used; if no such list exists, it's the **AssetsFix.csv** list with some Forex pairs and CFDs. If **Select** is omitted or **0** , a default asset - usually the first asset in the list - is selected.   

### Parameters:

**FileName** | File name of the asset list, f.i. **"AssetsIB"**. The **.csv** extension and the path can be omitted for asset lists in the **History** folder.   
---|---  
**Select** | Name of the asset to be selected in the scrollbox at first run, f.i. **"EUR/USD"**. **0** for selecting a default asset.   
  
### Returns:

Number of loaded assets, or **0** when no assets were loaded. The number of assets is also available through [NumAssetsListed](numassets.htm). 

### Usage:

**assetList("Strategy[\\\MyNewAssets.csv",0](file://MyNewAssets.csv%22,0));**   

## assetSelect () 

Sets the [Asset] scrollbox to the current asset.  

## assetType (string Name) : int

Attempts to determine the type of an asset from its name. If the name begins and ends with the 3-letter abbreviation of a currency, it is identified as Forex; if it ends with a number, it is identified as an index.   

### Parameters:

**Name** | Name of the asset   
---|---  
  
### Returns:

**0** when the type can not be identified; otherwise **FOREX** (1) or **INDEX** (2).  

### Remarks:

  * The place of an **asset** call (if any) in the script matters. All variables and flags that affect the creation of bars, such as **[BarPeriod](barperiod.htm)** , [BarZone](assetzone.htm), [LookBack](lookback.htm), [Detrend](detrend.htm), **[StartDate](date.htm)** , **[EndDate](date.htm)** ,  [TICKS](mode.htm), [BarMode](barmode.htm), [UpdateDays](date.htm), [AssetList](script.htm), [History](script.htm) etc. must be set before calling **asset()**. Otherwise the script will produce an [Error 030](errors.htm) message. All parameters specific to an asset, such as [Spread](spread.htm), [ Commission](spread.htm), etc., as well as all functions that use asset parameters or prices, such as [price()](price.htm), [ optimize()](optimize.htm), [advise()](advisor.htm), etc. must be used after calling **asset()**.
  * If the script contains no **asset** call, the scroll box asset is selected _after_ the [INITRUN](is.htm), and its name is appended to the [training](training.htm) files for being able to train different assets separately. Call **asset(Asset)** for loading the asset selected by the Scrollbox already in the [ INITRUN](is.htm).
  * In multi-asset strategies the order of asset() calls matters. When [BR_FLAT](barmode.htm) is not set, bars are created from the historical ticks of the first asset. This means tha gaps in the price history of the first asset are therefore reflected in the price data of all further assets. So select first the asset with the most complete price history; for instance, a currency pair or a cryptocurrency that is traded 24 hours. When a subsequent asset has a gap where the first asset has none, the gap is filled from the price data of previous bar. 
  * Every **asset** call switches the [asset parameters](spread.htm), [asset variables](algovar.htm), [trade statistics](winloss.htm) and [OptimalF](optimalf.htm) factors to the values of the selected asset. At begin of the simulation, [asset parameters](spread.htm) are loaded from the [asset list](account.htm). If the asset is not found in the list, an error message will be displayed and defaults are substituted for the asset parameters.
  * If an **asset** call fails, the failed asset is not selected and **0** is returned. Check the return value to make sure that only valid and available assets are traded.
  * Any asset can have up to 3 different broker symbols and 3 different sources for trading, for retrieving live prices, and for downloading historical prices. The symbols can be given in the **Symbol** field in the [asset list](account.htm), or by script in the [SymbolTrade](script.htm), [SymbolLive](script.htm), [SymbolHist](script.htm) parameters. The current asset name is stored in the **[Asset](script.htm)** string.
  * The [Assets](script.htm) array contains the names of all available assets. For selecting all assets of the [asset list](account.htm) in a [loop](loop.htm), use **while(asset(loop(Assets)))**. For enumerating assets without **loop** call, use [for(used_assets)](fortrades.htm) or [for(listed_assets)](fortrades.htm). 
  * The trading time zone of an asset can be set up with [ AssetMarketZone](assetzone.htm) and [AssetFrameZone](assetzone.htm). Trading can be restricted to market times with the [BR_LEISURE](barmode.htm) flag.
  * Artificial assets can be created by combining the prices from a 'basket' of several real assets (see example)[](tips.htm). 
  * When the asset name is an empty string or begins with a hash - like **asset("")** or **asset("#USD")** \- a dummy asset is created with default parameters and flat price history (usually with all prices at 50). This is useful when a real asset is not needed, like for testing filters or indicators with artificial price curves. The price history can be modified with [priceSet](price.htm) or [priceQuote](price.htm). For viewing the price curve of a dummy asset, use **assetAdd()** for adding it to the scrollbox.
  * When loading price data, the prices are checked for plausibility dependent on the [Outlier](outlier.htm) parameter. Invalid prices, such as extreme outliers, are automatically corrected or removed. Setting **[Detrend](detrend.htm) = NOPRICE;** before calling **asset()** prevents that asset and price data is checked and outliers are removed. 
  * If only a single asset is selected in the script, the [Asset] scrollbox is automatically set to that asset. If multiple assets are selected, the [Asset] scrollbox is unchanged and determines the price in the [Status] window and the price curve in the resulting chart. 
  * For adding new assets to the available asset set, see the description under [Asset List](account.htm#step). 
  * Assets must be subscribed before their prices are available. The **asset** function subscribes the asset automatically, but some brokers have a limit to the number of subscribed assets. Some platforms, for instance [MT4](mt4plugin.htm), need a long time after subscribing an asset before prices are available.
  * Any asset allocates computer memory (see also [memory](memory.htm)). This is normally uncritical in training or live trading, which is restricted to single assets and short lookback periods. But it can become critical in high resolution backtests with large portfolios. The memory requirement per asset in bytes can be estimated with the formula **Years / BarPeriod * 15 MB** , where **Years** is the number of backtest years (use **1** for live trading). The [LEAN](mode.htm) and [LEANER](mode.htm) flags reduce the memory requirement by about 50%, the **TICKS** flag increases it by 32 bytes per historical price quote. [ plot ](plot.htm)commands allocate 8..24 bytes per bar and asset. When the total memory requirement for backtesting large time periods exceeds ~3 GB, use **Zorro64** with a [C++ script](dlls.htm) for the backtest. Alternatively, split the portfolio in separate smaller sub-portfolios, or split the time period in separate shorter tests. 

### Examples:

```c
// trade multiple strategies and assets in a single script
function run()
{
  BarPeriod = 240;
  StartDate = 2010;
  set(TICKS); // set relevant variables and flags before calling asset()
  
// call different strategy functions with different assets
  asset("EUR/USD");
  tradeLowpass();
  tradeFisher();
  
  asset("GBP/USD");
  tradeAverage();
  
  asset("SPX500");
  tradeBollinger();  
}
```

```c
// set all asset symbols to a new source
if(Init) {
  assetList("MyAssetList.csv");
  for(listed_assets)
    assetAdd(Asset,strf("MyNewSource:%s",SymbolLive));
}
```

```c
// Basket trading - generate a snythetic asset "USD" 
// combined from the USD value of EUR, GBP, and AUD
var priceUSD()
{
  var p = 0;
  asset("GBP/USD"); p += price();
  asset("AUD/USD"); p += price();
  asset("EUR/USD"); p += price();
  return p;
}

// basket trade function with stop limit
int tradeUSD(var StopUSD)
{
  if((TradeIsLong && priceUSD() <= StopUSD) 
    or (TradeIsShort && priceUSD() >= StopUSD)) 
      return 1;   // exit the trade
  else return 0;  // continue the trade
}

// open a trade with the synthetic asset and a stop loss  
void enterLongUSD(var StopDistUSD)
{
  var StopUSD = priceUSD()-StopDistUSD;
  asset("GBP/USD"); enterLong(tradeUSD,StopUSD);
  asset("AUD/USD"); enterLong(tradeUSD,StopUSD);
  asset("EUR/USD"); enterLong(tradeUSD,StopUSD);
}

void enterShortUSD(var StopDistUSD)
{
  var StopUSD = priceUSD()+StopDistUSD;
  asset("GBP/USD"); enterShort(tradeUSD,StopUSD);
  asset("AUD/USD"); enterShort(tradeUSD,StopUSD);
  asset("EUR/USD"); enterShort(tradeUSD,StopUSD);
}
 
// plot a price curve of the synthetic asset
// (the plot command is linked to the last used asset -
// so "EUR/USD" must be selected in the scrollbox)
function run() 
{
  set(PLOTNOW);
  plot("USD",priceUSD(),0,RED);
}
```

### See also:

[enterLong/](buylong.htm)[Short](buylong.htm), [loop](loop.htm), [algo](algo.htm), [Asset](script.htm), [AssetZone](assetzone.htm), [AssetVar](algovar.htm), [Detrend](detrend.htm), [assetHistory](loadhistory.htm), [price](price.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
