---
title: EntryTime, LifeTime, EntryDelay
url: https://zorro-project.com/manual/en/timewait.htm
category: Functions
subcategory: None
related_pages:
- [Stop, Profit, Trail, Entry, ...](stop.htm)
- [enterLong, enterShort](buylong.htm)
- [Margin, Risk, Lots, ...](lots.htm)
- [Fill modes](fill.htm)
- [Equity Curve Trading](trademode.htm)
- [brokerCommand](brokercommand.htm)
- [trade management](trade.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [set, is](mode.htm)
- [Bars and Candles](bars.htm)
- [TickTime, MaxRequests, ...](ticktime.htm)
---

# EntryTime, LifeTime, EntryDelay

# Trade parameters 2 - time limits 

## LifeTime

Trade time limit in bars. After the given number of bars (default: **0** for no time limit), the trade closes at the open of the next bar. Trades are only closed when the market is open. If the entry happens intrabar due to an [Entry](stop.htm) or **OrderDelay** setting, **LifeTime** = **1** causes a trade duration of 1 bar plus the remainder of the opening bar. The life time of open trades is extended by [enter](buylong.htm) commands that do not open a trade due to [ MaxLong/MaxShort](lots.htm) or due to a margin or risk limit. Open trades of the same component get then automatically the life time that the new trade would have. 

## EntryTime

Pending order lifetime in bars. When an [enter](buylong.htm) command is given, wait the given number of bars (default: **1**) until the [entry limit](stop.htm) is met. If that does not happen during that time, the trade is removed and a "Missed Entry" message is printed to the log file. The entry time of pending trades is extended by [enter](buylong.htm) commands that do not open a trade due to [ MaxLong/MaxShort](lots.htm)[.](lots.htm)

### Range:

Number of bars. 

### Type:

**int  
**

## OrderDelay

Order entry delay in seconds. Open the trade either after the given delay time (default: **0** \- no delay), or when the [entry limit](stop.htm) is met, whatever happens first. The delay can be given in high resolution (0.000001 = 1 microsecond). Useful for splitting a large order into smaller portions ("iceberg trades"), for an adaptive entry/exit algorithm or for simulating latency in [HFT fill mode](fill.htm). 

## OrderDuration

[GTC order](trademode.htm) duration in seconds. The order is automatically cancelled after the given time even when the trade is not completely filled. The broker API must support the [SET_ORDERTYPE](brokercommand.htm) and [DO_CANCEL](brokercommand.htm) commands. This variable affects all open GTC trades. 

### Range:

Second units, f.i. **1.23 == 1230 ms**. 

### Type:

**var**

### Remarks:

  * All time limits must be set before calling [enterLong](buylong.htm)/[enterShort](buylong.htm), except for **OrderDuration** , which can be changed anytime. The life time of a trade can be modified afterwards either directly by setting [TradeExitTime](trade.htm) in a TMF, or indirectly by updating the life time of open trades with the [MaxLong/Short](lots.htm) mechanism. 
  * **LifeTime** and **EntryTime** units are bar periods and not affected by [TimeFrame](barperiod.htm).
  * **OrderDelay** can prevent that orders are entered on minute/hour boundaries when many automated systems open their positions and cause high slippage. This can achieve a better entry price, especially when combined with an [entry limit](stop.htm). **OrderDelay** can also be used for inserting artificial delays between trades that are opened at the same bar. A delay of about 30 seconds between trades is often required by trade copy services such as ZuluTrade™, or useful for opening large positions in several steps ('iceberg trades'). 
  * **OrderDelay** is not recommended in combination with [Virtual Hedging](hedge.htm#). As it opens and closes trades not at bar boundaries, it can cause trades to be opened and closed shortly afterwards by another trade in opposite direction after the given delay. This causes loss of spread and commission. 
  * The effect of **OrderDelay** on the performance is simulated in [TICKS](mode.htm) mode only. For simulating **OrderDuration** , use **TICKS** mode and pending trades in the backtest.

### Examples (see also Grid.c and Hacks&Tricks):

```c
// Use an entry limit and an entry delay for not entering trades at the same time
// Call this function before entering a trade
void setDelay(var Seconds)
{
	static int PrevBar = 0;
	static var Delay = 0;
	if(Bar != PrevBar) { // reset delay at any new bar
		Delay = 0;
		PrevBar = Bar;
	}
	Delay += Seconds; // increase delay within the bar
	OrderDelay = Delay;
	Entry = -0.2*PIP * sqrt(Delay); // entry limit for additional profit
}
```

### See also:

[bar](bars.htm), [enterLong/](buylong.htm)[Short](buylong.htm), [Stop](stop.htm), [Entry](stop.htm), [TickTime](ticktime.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
