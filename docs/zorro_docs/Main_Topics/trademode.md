---
title: TradeMode
url: https://zorro-project.com/manual/en/trademode.htm
category: Main Topics
subcategory: None
related_pages:
- [Trade Statistics](winloss.htm)
- [Virtual Hedging](hedge.htm)
- [phantom](phantom.htm)
- [Included Scripts](scripts.htm)
- [setf, resf, isf](setf.htm)
- [brokerCommand](brokercommand.htm)
- [Margin, Risk, Lots, ...](lots.htm)
- [tradeUpdate](tradeupdate.htm)
- [trade management](trade.htm)
- [set, is](mode.htm)
- [Fill modes](fill.htm)
- [Broker Arbitrage](brokerarb.htm)
---

# TradeMode

# Trade Mode, Phantom Trades, Equity Curve Trading 

**Phantom trades** are "what-if" trades that are not sent to the broker. Instead the win or loss of a phantom trade is calculated from the price curve and the current slippage and trading costs. It is recorded in the [Short](winloss.htm)/[Long](winloss.htm) statistics, but not in the [Total](winloss.htm) statistics. This way the performances of phantom and real trades can be evaluated separately. Phantom trades are used for 'equity curve trading' and for [Virtual Hedging](hedge.htm).

Equity curve trading is a method of limiting losses by skipping unfavorable market periods. The strategy monitors the equity curves of any component of a portfolio strategy. When unusual losses are recognized, real trading with that component is suspended. For detecting whether it's worth to resume trading, the subsequent trades are then entered in **phantom mode** until their returns indicate that the market is getting profitable again. In this way the strategy switches in and out of phantom mode dependent on the market situation. The predefined [phantom](phantom.htm) function can be used for detecting unusual losses.

Several methods can be used for determining if the market it profitable or not. In the example below, the equity curve is permanently compared with its own long-term average by lowpass filtering. It the equity is below average and still falling, trading switches to phantom mode; it goes back to normal as soon as the equity curve is rising again. Other methods may be based on the balance instead of the equity curve, or on the ratio of winning to losing trades. 

Equity curve trading does not improve the performance when there is no clear distinction between profitable and unprofitable market periods or when the strategy filters unprofitable periods anyway. But it can often improve the perception of a live trading system, and can prevent losses by expiration of market inefficiencies. An example of a system that suddenly became unprofitable is the 'Luxor' strategy in the [script examples](scripts.htm); here equity curve trading would have saved the day by stopping trading in December 2011.

Phantom trades and the overall trade behavior can be controlled with the following variable:

## TradeMode

Determines with the flags listed below how trades are placed. Use [setf](setf.htm) and [resf](setf.htm) for setting and resetting flags. 

### Range:

**TR_PHANTOM** | Enter trades in phantom mode. The trades are simulated and not sent to the broker API.****  
---|---  
**TR_GTC** | Enter trades in GTC (good-till-cancelled) mode, if supported by the broker API. Keeps the order active until filled or cancelled by the broker.  
**TR_AON** | Enter trades in AON (all-or-nothing) mode, if supported by the broker API.   
**TR_POS** | Check the position before exiting trades, and cancel the trade when the position was zero. [GET_POSITION](brokercommand.htm) must be supported.  
**TR_FILLED** | Treat the trade as completely filled when trade status and filled orders are unavailable by the API.  
**TR_FRC** | Do not round stops, limits, and backtest fill prices to the point value of the asset. Some brokers reject unrounded stops and limits, others allow them.  
**TR_EXTEND** | Renew the life time and wait time of open or pending trades when entering trades with **0** lots, or when exceeding a [MaxLong/MaxShort](lots.htm) or margin or risk limit.  
**TR_ANYSTOP** | Move the entry stop and stop loss level of open or pending trade in any direction by [tradeUpdate](tradeupdate.htm), by entering trades with **0** lots, or by exceeding a [MaxLong/MaxShort](lots.htm) or margin or risk limit. Otherwise it is only moved closer to the current price.  
**TR_ANYLIMIT** | Move the entry limit or profit target level of an open or pending trade by [tradeUpdate](tradeupdate.htm), by entering trades with **0** lots, or by exceeding a [MaxLong/MaxShort](lots.htm) or margin or risk limit. Otherwise it is only moved closer to the current price.  
  

### Type:

**int**

### Remarks:

  * **TR_GTC** prevents Zorro from cancelling the order when it is not immediately executed. Use this for orders that take a long time for execution, for instance option combos. The broker plugin must support the [SET_ORDERTYPE](brokercommand.htm) command with order type 2. 
  * If a GTC order is not immediately filled, the partial fill amount is contained in the [TradeLots](trade.htm) variable. If the trade status is unavailable via broker API, either use **TR_GTC|TR_FILLED** for treating all accepted GTC trades as completely filled. Or retrieve the current position size via **GET_POSITION** and calculate the fill amount from it. You can find an example under [TMF](trade.htm).

### Example (equity curve trading):

```c
// don't trade when the equity curve goes down
// and is below its own lowpass filtered value
function checkEquity()
{
// generate equity curve including phantom trades
  vars EquityCurve = series(EquityLong+EquityShort);
  vars EquityLP = series(LowPass(EquityCurve,10));
  if(EquityLP[0] < LowPass(EquityLP,100) && falling(EquityLP))
    setf(TradeMode,TR_PHANTOM); // drawdown -> phantom trades
  else
    resf(TradeMode,TR_PHANTOM); // profitable -> normal trades
}
```

### See also:

[NFA](mode.htm), [Hedge](hedge.htm), [Fill](fill.htm), [broker arbitrage](brokerarb.htm), [setf](setf.htm), [resf](setf.htm)  
[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
