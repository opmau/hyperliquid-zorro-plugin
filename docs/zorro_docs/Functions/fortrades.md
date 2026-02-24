---
title: for(current_trades)
url: https://zorro-project.com/manual/en/fortrades.htm
category: Functions
subcategory: None
related_pages:
- [for](for.htm)
- [asset, assetList, ...](asset.htm)
- [algo](algo.htm)
- [Asset and Account Lists](account.htm)
- [break](acrt-break.htm)
- [#define, #undef](define.htm)
- [System strings](script.htm)
- [trade management](trade.htm)
- [Spread, Commission, ...](spread.htm)
- [AlgoVar, AssetVar, AssetStr](algovar.htm)
- [Trade Statistics](winloss.htm)
- [Virtual Hedging](hedge.htm)
- [exitLong, exitShort, exitTrade](selllong.htm)
- [Bars and Candles](bars.htm)
- [enterLong, enterShort](buylong.htm)
- [results](results.htm)
- [loop](loop.htm)
---

# for(current_trades)

# Enumeration loops

## for(current_trades) { ... }

A [for](for.htm) loop that cycles through all open and pending trades with the current [asset](asset.htm) and [algo](algo.htm), starting with the earliest trade; useful f.i. for closing trades in a FIFO compliant way.

## for(last_trades) { ... }

A [for](for.htm) loop that cycles backwards through the internal list of open, pending, closed, and cancelled trades with the current [asset](asset.htm) and [algo](algo.htm), starting with the last trade. 

## for(open_trades) { ... }

A [for](for.htm) loop that cycles through all open and pending trades of all assets and algos, starting with the earliest trade.

## for(closed_trades) { ... }

A [for](for.htm) loop that cycles backwards through all closed trades of all assets and algos, starting with the latest trade. 

## for(all_trades) { ... }

A [for](for.htm) loop that cycles through the internal list of all trades; useful for a detailed trade statistic after the simulation. 

## for(past_trades) { ... }

A [for](for.htm) loop that cycles backwards through the internal list of all trades, starting with the latest trade; useful for equity curve trading. 

## for(listed_assets) { ... }

A [for](for.htm) loop that cycles through all assets from the [asset list](account.htm), in the same order as in the list, and selects one after the other without loading itss history. 

## for(used_assets) { ... }

A [for](for.htm) loop that cycles through all assets used in the script in alphabetical order, and selects one after the other. 

## for(all_algos) { ... }

A [for](for.htm) loop that cycles through all asset/algo combinations used in the script, and selects one component after the other, 

## break_trades

## break_assets

## break_algos

Special [break](acrt-break.htm) macros for aborting a trade, asset, or algo enumeration loop. Use **return_trades** for returning from inside a trade loop. All macros are defined in **variables.h**. 

### Type:

**Macro**

### Remarks:

  * The above enumeration loops are [macros](define.htm) defined in **include\variables.h**. Unlike a normal [for](for.htm) loop, enumeration loops cannot be nested, and they must not be aborted by **break** or **return** statements. The **break_trades** , **break_assets** , **break_algos** macros can be used to abort these loops. The asset list can alternatively be enumerated, with no restrictions, with a normal loop like this: **for(i=0; Assets[i]; i++) { asset(Assets[i]); ... }** ).
  * The predefined **Itor** variable starts at **0** and is incremented at any loop. It can be used for counting cycles or indexing arrays. 
  * The asset that was selected at loop begin is stored in [AssetPrev](script.htm) and automatically re-selected after the end of the loop. 
  * Trades are sorted in the order of entering, either forwards or backwards dependent on loop type.
  * The code inside a **for(...trades)** loop sets the [ThisTrade](trade.htm) pointer to the loop trade and has access to all its [trade variables](trade.htm). When called inside a [TMF](trade.htm), all [trade variables](trade.htm) inside the enumeration loop refer to the current loop trade, while outside the loop they refer to the trade of the TMF. All asset specific variables in the trade loop - [Asset](script.htm), [Spread](spread.htm), etc. - are set to the trade asset. Algo specific variables (f.i. [AlgoVar](algovar.htm)) and component specific variables (f.i. [trade statistics](winloss.htm)) are not set up in the loop; if this is desired, call [algo(TradeAlgo)](algo.htm) inside a trade loop. 
  * Asset loops select the asset name and parameters, but don't initialize the asset or load its history. Call [asset(**Asset**)](asset.htm) and/or [algo(...)](algo.htm) for evaluating component specific [trade statistics](winloss.htm) in the loop. 
  * Use an **if(TradeIsPending)** , **if(TradeIsOpen)** , or **if(TradeIsClosed)** condition, or a combination of them, to filter out the desired trades in a trade enumeration loop. When [virtual hedging](hedge.htm) is used, **open_trades** includes phantom as well as pool trades. Make sure to distinguish them with **if([TradeIsPool](trade.htm))**. You don't want to modify pool trades. 
  * Enumeration loops must normally not modify themselves. For instance, a loop over all open trades must not change the number of open trades. As an exception, a new trade can be opened inside a trade enumeration loop. But since this will add a new cycle to that loop, be careful not to create an endless loop this way. It is also allowed to close the current trade in the loop by **[exitTrade](selllong.htm)(ThisTrade)**.

### Example:

```c
// find all pending trades with the current asset and all algos
int numPending(bool isShort)
{
  int num = 0;
  for(open_trades)
    if(0 == strcmp(Asset,AssetPrev) 
      && TradeIsPending && TradeIsShort == isShort)
        num++;
  return num;
}
 
// sum up the profit/loss of all open trades with the current asset
var val = 0;
for(open_trades)
  if(0 == strcmp(Asset,AssetPrev) && TradeIsOpen && !TradeIsPhantom)
    val += TradeProfit;

// increase the stop of all winning trades slowly over time
for(open_trades) {  if(TradeIsOpen && TradeProfit > 0 && !TradeIsPool)    TradeStopLimit -= 0.02 * TradeStopDiff;}

// lock 80% profit of all winning trades
for(open_trades) {
  if(TradeIsOpen && !TradeIsPool && TradeProfit > 0) {
    TradeTrailLock = 0.80;
    if(TradeIsShort)
      TradeTrailLimit = max(TradeTrailLimit,TradePriceClose);
    else
      TradeTrailLimit = min(TradeTrailLimit,TradePriceClose);
  }
}

// sum up open profits of all assets and the current algo
var ProfitSum = 0;
for(used_assets) {
  asset(Asset); // select the current component
  ProfitSum += ProfitOpen;
}
```

### See also:

[bar](bars.htm), [enterLong](buylong.htm)/[Short](buylong.htm), [trade variables](trade.htm), [results](results.htm), [loop](loop.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
