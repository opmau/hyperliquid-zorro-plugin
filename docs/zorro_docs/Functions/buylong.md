---
title: enterLong, enterShort
url: https://zorro-project.com/manual/en/buylong.htm
category: Functions
subcategory: None
related_pages:
- [trade management](trade.htm)
- [algo](algo.htm)
- [Included Scripts](scripts.htm)
- [Stop, Profit, Trail, Entry, ...](stop.htm)
- [Margin, Risk, Lots, ...](lots.htm)
- [Fill modes](fill.htm)
- [LifeTime, EntryTime, ...](timewait.htm)
- [brokerCommand](brokercommand.htm)
- [exitLong, exitShort, exitTrade](selllong.htm)
- [Spread, Commission, ...](spread.htm)
- [contract, ...](contract.htm)
- [ContractType, Multiplier, ...](contracts.htm)
- [BarMode](barmode.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [set, is](mode.htm)
- [DataSplit, DataSkip, ...](dataslope.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [Trade Statistics](winloss.htm)
- [Virtual Hedging](hedge.htm)
- [Variables](aarray.htm)
- [for(open_trades), ...](fortrades.htm)
- [optimize](optimize.htm)
- [Money Management](optimalf.htm)
- [Performance Report](performance.htm)
- [price, ...](price.htm)
- [Equity Curve Trading](trademode.htm)
- [order](order.htm)
- [tradeUpdate](tradeupdate.htm)
---

# enterLong, enterShort

## enterLong (_int Lots, var Entry, var Stop, var TakeProfit, var Trail, var TrailSlope, var TrailLock, var TrailStep_): TRADE* 

## enterShort (_int Lots, var Entry, var Stop, var TakeProfit, var Trail, var TrailSlope, var TrailLock, var TrailStep_): TRADE*

Enters a long or short trade with optional parameters. Entry and exit conditions are handled by Zorro. 

## enterLong (_function, var V0, ... var V7_): TRADE* 

## enterShort (_function, var V0, ... var V7_): TRADE*

Enters a long or short script-managed trade with optional user-defined parameters. Entry and exit conditions are handled by a user-provided algorithm in the given trade management function ([TMF](trade.htm)) dependent on the variables **v0** .. **v7**. 

## enterTrade (TRADE*): TRADE*

Enters an open, pending, or closed trade from a **TRADE** struct. Inserts the trade only in the internal trade list; does not send any order to the broker. The asset name can be given through [TradeStr[0]](trade.htm) resp. the **Skill** element of the **TRADE** struct; if it is not set, the current asset is used. The position size (**nLots**) and the **TR_SHORT** and **TR_OPEN** flags must be set in the **TRADE** struct; other elements are optional. The algo identifier is taken from the **TRADE** struct; if it has none, it is set to the current [algo](algo.htm). This function can be used for running backtests from a list of trades (see [Simulate](scripts.htm) script) or for reading positions from the broker API and converting them to trades.   

### Parameters:

**Lots** | Optional number of lots when nonzero; overrides the global [](stop.htm)[Lots](lots.htm), [Amount](lots.htm), [Margin](lots.htm), and [Risk](lots.htm) variables. A negative number reverses the trade direction: **enterLong** then opens a short trade and **enterShort** opens a long trade.  
---|---  
**Entry** | Optional entry stop when > 0, entry limit when < 0 (overrides the global [Entry](stop.htm)).   
**Stop** | Optional stop loss when nonzero (overrides the global [](stop.htm)[Stop](stop.htm)).   
**TakeProfit** | Optional profit target when nonzero (overrides the global [](stop.htm)[TakeProfit](stop.htm)).   
**Trail** | Optional trail limit when nonzero (overrides the global [](stop.htm)[Trail](stop.htm)).   
**TrailSlope** | Optional trailing speed when nonzero (overrides the global [](stop.htm)[TrailSlope](stop.htm)).   
**TrailLock** | Optional profit lock percentage when nonzero (overrides the global [](stop.htm)[TrailLock](stop.htm)).   
**TrailStep** | Optional autotrailing step width when nonzero (overrides the global [](stop.htm)[TrailStep](stop.htm)).   
**function** | Optional pointer of an **int** function for micro-managing the trade (see [TMF](trade.htm)).   
**V0 ... V7** | Up to 8 optional numerical variables as further arguments to the TMF.   
  
### Returns:

**TRADE*** \- a pointer to the created trade struct (see **include\trading.h** for the definition of the **TRADE** struct), or **0** when no trade could be entered because the trade volume was zero, trading was disabled (f.i. weekend, time frame, or lookback period), or the trade was rejected by the broker. For pending trades a nonzero pointer is always returned. 

### Remarks:

  * In the backtest, the result of entering a trade can be either a pending trade, an opened trade, or a skipped trade. If no [Entry](stop.htm) limit is given, trades are opened at the current price or at the next open price, dependent on [Fill](fill.htm) mode. If a trade is pending, Zorro continues to attempt opening the trade within the time period given by [EntryTime](timewait.htm). 
  * In live trading, order filling depends on the broker API and the market situation. Normally the order is cancelled when not filled within a wait time given by [SET_WAIT](brokercommand.htm) (default: about 30-60 seconds dependent on broker API). This behavior can be changed with the [ SET_ORDERTYPE](brokercommand.htm) command. Partial fills are allowed for IOC ("immediate-or-cancel") or GTC ("good-till-cancelled") orders. A GTC order stays active in the background until it is either complete, or is cancelled by expiration or by an [exit](selllong.htm) command.
  * Long trades open at the best ask price, short trades at the best bid price from the order book or the historical data. But the real fill price \- the price at which the trade is entered - normally differs to the current ask or bid price. The difference is the **slippage**. It can be simulated in the backtest with the [Slippage](spread.htm) variable. To prevent an unfavorable fill price in live trading, you can use limit orders instead of market orders. Some broker plugins, f.i. the IB plugin, support limit orders. They are normally guaranteed to be filled at the price set up through the [OrderLimit](stop.htm) variable.
  * When function parameters are **0** or omitted in the parameter list, [global trade parameters](stop.htm) are used. When no trade management function and only global parameters are used, the parentheses can be empty.
  * If a [contract](contract.htm) is selected and the [ Multiplier](contracts.htm) is set, the function opens an option or future position instead of a position of the underlying. 
  * Trades are automatically skipped during [weekends](barmode.htm) or outside market hours, during the [LookBack](lookback.htm) period, in the inactive period of [**SKIP**](mode.htm) or **[DataSplit](dataslope.htm)** , or when the current bar is not the end of a [TimeFrame](barperiod.htm). For trading inside a time frame, set **TimeFrame = 1** , enter the trade, then set **TimeFrame** back to its previous value.
  * Trades are also not opened when [Lots](lots.htm) is **0** , or **[Margin](lots.htm)** or **[Risk](lots.htm)** are too low, or the [MaxLong](lots.htm) / [MaxShort](lots.htm) limits are reached in [Test] and [Trade] mode, or when the [Algo](algo.htm) name ends with **":L"** for short trades or with **":S"** for long trades. However reverse positions are then still closed and the stop limits, profit targets, and life times of open trades with the same asset and algo are updated to the current values. This way, already open trades behave as if they were just opened by the recent signal. If this mechanism is not desired, limit the number of trades with negative [MaxLong](lots.htm) / [MaxShort](lots.htm) limits.
  * If a trade is rejected by the broker, the reason by the broker API - such as**"Outside market hours"** \- is normally printed to the log. When trading with the [MT4/5 bridge](mt4plugin.htm), the reason can be seen in the MT4/5 Experts Log. The number of rejected orders is stored in the [NumRejected](winloss.htm) variable. There can be many reasons for the broker API to reject a trade, for instance not enough capital on the account, not enough free margin, a wrong asset name, a closed or illiquid market, an asset that cannot be shorted, or a stop loss that is too tight, too far, or not an integer multiple of a pip. Zorro will not attempt to open the trade again unless either enforced by script, or when it's a [pool trade](hedge.htm). Pool trades will be rebalanced at any bar until they match the virtual trades. 
  * A returned nonzero **TRADE*** pointer means that the trade is processed, but is no guarantee that the position is really opened - it could still be pending or is rejected later by the broker. This is indicated by flags in the **TRADE** struct. The returned **TRADE*** pointer can be assigned to [ThisTrade](trade.htm) (f.i. **ThisTrade = enterLong();**). If **ThisTrade** is nonzero, all its trade variables - for instance, [TradeIsOpen](trade.htm) \- are available for checking the state of the trade.
  * A **[trade management function](trade.htm)** (**TMF**) is automatically called every tick - i.e. whenever the price changes - with the the optional variables (**V0** .. **V7**) as arguments. It can evaluate the current price and other [trade variables](trade.htm) for managing the position or adjusting stop and profit limits. Note that **V0** .. **V7** are for numbers only; they internally stored as [float](aarray.htm) variables with 32-bit precision.
  * Open, pending, and closed trades can be enumerated with the [for(open_trades)](fortrades.htm) and [for(all_trades)](fortrades.htm) macros. 
  * Every trade is linked to an algorithm identifier that can be set up through the **[Algo](algo.htm)** string. Identifiers are useful when the script trades with several different algorithms; they are then used for selecting [strategy parameters](optimize.htm) or [capital allocation factors](optimalf.htm) belonging to a certain trade algorithm, or to exit or identifiy particular trades. The [performance report](performance.htm) lists strategy results separated by their algorithm identifiers. 
  * Entry and exit conditions can be either set individually per trade, or set up globally through the [Entry](stop.htm), **[Stop](stop.htm)** , **[TakeProfit](stop.htm)** , and **[Trail](stop.htm)** variables for all subsequent trades. If the stop loss is too close, too far, to or on the wrong side of the price, the trade can be rejected by the broker. Make sure that the stop loss has sufficient distance to the current price ([priceClose()](price.htm)). A timed exit can be set up with [LifeTime](timewait.htm).
  * If [TICKS](mode.htm) is not set and the price hits the entry limit, the profit target, and/or the stop loss in the same bar, the simulation assumes a pessimistic outcome. The limits are evaluated in the order entry - stop - profit target. If **TICKS** is set, the price curve inside the bar is evaluated for determining which limit is met first. 
  * Opening a trade automatically closes opposite trades when [Hedge](hedge.htm) is **0** or **1** (i.e. **enterLong** can automatically close short positions and **enterShort** can automatically close long positions). If the trade was not opened because the [Entry](stop.htm) limit was not met within [EntryTime](timewait.htm), opposite trades are not closed; if the trade was not opened because [Lots](lots.htm) is **0** or **[Margin](lots.htm)** or **[Risk](lots.htm)** are too low, opposite trades are still closed. 
  * The trade size is determined by the [Margin](lots.htm), [Risk](lots.htm), [Amount](lots.htm), or **[Lots](lots.htm)** variables. 
  * The [TR_PHANTOM](trademode.htm) flag causes the trade to be executed in "phantom mode". Phantom trades are not sent to the broker and do not contribute to the [Total](winloss.htm) statistics, but their projected wins and losses contribute to the [Short](winloss.htm) and [Long](winloss.htm) statistics. This can be used to filter trades based on the current win/loss situation ("equity curve trading"). 
  * If an [order](order.htm) function is defined in the script, it is called with **1** for the first argument and the **TRADE*** pointer for the second argument at the moment when the buy order is sent to the broker. This function can f.i. open a message box for manually entering the trade, or control another broker's user interface. 
  * When converting scripts from other trading software, keep in mind that other trading programs use sometimes different names for trade functions. For instance, TradeStation® uses "Sell Short" for entering a short position and "Buy To Cover" for exiting a short position.
  * In [Trade] mode, all open trades are stored in a **.trd** file in the **Data** folder. The stored trades are automatically continued when the strategy is started again, for instance after a computer crash. 

### Example 1: Simple SMA crossing

```c
function run()
{
  vars Price = series(priceClose());
  vars SMA100 = series(SMA(Price,100));
  vars SMA30 = series(SMA(Price,30));
 
  if(crossOver(SMA30,SMA100))
    enterLong();
  else if(crossUnder(SMA30,SMA100))
    enterShort();
}
```

### Example 2: Grid trading script 

```c
// helper function for finding trades at a grid line
bool noTradeAt(var Price,var Grid,bool IsShort) 
{
  for(open_trades)
    if((TradeIsShort == IsShort)
      and between(TradeEntryLimit,Price-Grid/2,Price+Grid/2))
        return false; // trade found
  return true; // no open/pending trade at that price
}
  
function run() 
{
  BarPeriod = 1440;
  Hedge = 2; 
  EntryTime = LifeTime = 500;  
  var Price;
  var Grid = 100*PIP; // grid spacing
  var Current = priceClose();
// place pending trades at 5 grid lines above and below the current price
  for(Price = 0; Price < Current+5*Grid; Price += Grid) {
    if(Price < Current-5*Grid)
      continue;
    if(Price < Current and noTradeAt(Price,Grid,true))
      enterShort(0,Price,0,Grid);      
    else if(Price > Current and noTradeAt(Price,Grid,false))
      enterLong(0,Price,0,Grid);
  }
}
```

### Example 3: Using a trade management function

```c
// TMF that adjusts the stop in a special way 
int TrailingStop(){// adjust the stop only when the trade is in profit  if(TradeProfit > 0)// place the stop at the lowest bottom of the previous 3 candles
    TradeStopLimit = max(TradeStopLimit,LL(3));// plot a line to make the stop limit visible in the chart  plot("Stop",TradeStopLimit,MINV,BLACK);
// return 0 for checking the limits  return 0;}

// using the trade function
function run()
{
  ...
  Lots = 1;
  Stop = 10*PIP;
  Algo = "SIG";
  if(crossOver(MySignal,MyThreshold))
    enterLong(TrailingStop);
  ...
}
```

### See also:

[exitLong](selllong.htm)/[Short](selllong.htm), [tradeUpdate](tradeupdate.htm), [Lots](lots.htm), [Risk](lots.htm), [Entry](stop.htm), **[Stop](stop.htm)** , [EntryTime](timewait.htm), [ExitTime](timewait.htm), [BarMode](barmode.htm), [TMF](trade.htm), [Fill](fill.htm), [Hedge](hedge.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
