---
title: exitLong, exitShort
url: https://zorro-project.com/manual/en/selllong.htm
category: Functions
subcategory: None
related_pages:
- [algo](algo.htm)
- [asset, assetList, ...](asset.htm)
- [contract, ...](contract.htm)
- [for(open_trades), ...](fortrades.htm)
- [Stop, Profit, Trail, Entry, ...](stop.htm)
- [BarMode](barmode.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [set, is](mode.htm)
- [DataSplit, DataSkip, ...](dataslope.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [Trade Statistics](winloss.htm)
- [Virtual Hedging](hedge.htm)
- [Fill modes](fill.htm)
- [order](order.htm)
- [enterLong, enterShort](buylong.htm)
---

# exitLong, exitShort

## exitLong(_string Filter, var Price, int Lots_)

## exitShort(_string Filter, var Price_ _, int Lots_)

Exits all long or all short trades that match the given filter condition (see below), at market or at a given price limit, until the given number of lots is closed. 

## exitTrade(TRADE*_,_ _var Price, int Lots_): int 

Exits a particular trade completely or partially at market or at a given price limit. Returns 0 when the order failed, otherwise nonzero. 

## cancelTrade(int Id)

## cancelTrade(TRADE*)

Cancels an open trade with a particular identifier or **TRADE*** pointer without sending a close order to the broker and without modifying the statistics or the balance. If the trade was still open on the broker side, it will become orphaned and is not anymore under Zorro control. Use this function for removing an externally closed position that was not transmitted to Zorro, for instance on [NFA](mode.htm#nfa) compliant accounts. 

### Parameters:

**Filter** |  **0** or **""** for closing all trades with the current algo/asset component (default).  
An [algo name](algo.htm) for closing all trades with the given algo and the current asset.  
An [asset name](asset.htm) for closing all trades with the given asset and the current algo.  
**"c"** for closing all [call](contract.htm) trades with the current component.  
**"p"** for closing all [put](contract.htm) trades with the current component.  
**"cp"** for closing all [ contract](contract.htm) trades with the current component.  
**"u"** for closing all underlying trades with the current component.  
**"*"** for closing all trades with the current asset.  
**"**"** for closing all trades.   
---|---  
**TRADE*** |  A pointer to the trade to be closed, or **ThisTrade** for closing the current trade in a [trade enumeration loop](fortrades.htm).   
**Price** | Optional price or distance to current price for selling the position, or **0** (default) for selling at market. A positive price or price distance constitutes an exit stop, a negative price is an exit limit (similar to [Entry](stop.htm)). An exit stop closes the position when the asset price is at or worse than the given price, like a stop loss; an exit limit closes the position when the asset price is at or better than the given price, like a profit target.  
**Lots** | Optional number of lots to be closed, or **0** (default) for closing all open lots. Partially closing trades is not available with some brokers.   
**Id** | Identifier of the trade to be cancelled. Either the full number as assigned by the broker, or the last 4 digits as displayed in the Zorro window.   
  
### Remarks:

  * Optional parameters can be omitted. F.i. **exitLong()** exits all long positions with the current asset and algo at market. 
  * Long trades exit at the best bid price, short trades at the best ask price from the order book or the historical data. The real exit price in live trading can differ from the current ask or bid price due to slippage.
  * If an exit stop or exit limit is given, the position is closed as soon as the target price is reached, no matter how long it takes. An exit stop overrides a more distant [stop loss](stop.htm), an exit limit overrides a previous [profit target](stop.htm). The **exitLong/Short** function can thus be used to tighten the stop loss or to change the profit target of open trades (see example).
  * Before closing a trade, check the business hours of the broker. Not all assets can be traded 24 hours per day. Trading is normally disabled during [weekends](barmode.htm), during the [LookBack](lookback.htm) period, or in the inactive periods of [TimeFrame](barperiod.htm), [**SKIP**](mode.htm), or **[DataSplit](dataslope.htm)**. 
  * If the market was closed when exiting a trade in [Test] or [Train] mode, the trade will be closed when the market opens again.
  * If the close order is rejected by the broker in [Trade] mode, the reason - such as**"Outside market hours"** \- is printed to the log and the message window. With the [MT4/5 bridge](mt4plugin.htm) the reason can be seen in the MT4/5 Experts Log. The number of rejected orders is stored in the [NumRejected](winloss.htm) variable. There can be several reasons for the broker API to refuse closing a trade, for instance NFA compliance, FIFO compliance, a wrong asset name, an invalid trade size, a closed market, or no liquidity. Check the requirements of your account and set the [NFA](mode.htm) flag and [Hedge](hedge.htm) mode accordingly.   
Zorro will repeat a rejected close order in increasing time intervals until it is eventually executed (except for [options](contract.htm) or when [OrderLimit](stop.htm) is used). If the position can not be closed after 4 working days, Zorro will assume that is was already externally closed and remove the trade from its internal list. If the market is not open due to holidays or weekend, a message **"Can't exit at weekends"** is printed and the position is closed as soon as the market opens again. 
  * Some cryptocurrency brokers deduct their commission from an open position, and subsequently reject the close order due to insufficient funds. In that case make sure to have a small additional amount on the account for covering the commissions.
  * When no **price** was given, pending positions - positions either immediately entered before, or positions with an [entry](stop.htm) stop or limit - are also closed.
  * The price at which the trade is closed in the backtest can be affected with the [Fill](fill.htm) variable. 
  * If an [order](order.htm) function is defined in the script, it is called with **2** for the first argument and the **TRADE*** pointer for the second argument at the moment when the sell order is sent to the broker. This function can f.i. open a message box for manually exiting the trade, or control another broker's user interface. 

### Example:

```c
exitShort(0,1.234); // exits all short trades with the current Algo/Asset as soon as the price is at or above 1.234 (exit stop).
```

### See also:

[enterLong/](buylong.htm)[Short](buylong.htm), [Entry](stop.htm), [Hedge,](hedge.htm) [Fill](fill.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
