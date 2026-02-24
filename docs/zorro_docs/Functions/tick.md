---
title: tick, tock
url: https://zorro-project.com/manual/en/tick.htm
category: Functions
subcategory: None
related_pages:
- [TickTime, MaxRequests, ...](ticktime.htm)
- [Dates, holidays, market](date.htm)
- [Broker API](brokerplugin.htm)
- [call](call.htm)
- [run](run.htm)
- [System strings](script.htm)
- [price, ...](price.htm)
- [Spread, Commission, ...](spread.htm)
- [Time / Calendar](month.htm)
- [Trade Statistics](winloss.htm)
- [asset, assetList, ...](asset.htm)
- [algo](algo.htm)
- [trade management](trade.htm)
- [set, is](mode.htm)
- [series](series.htm)
- [shift](shift.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [Window handle](hwnd.htm)
- [Bars and Candles](bars.htm)
- [Bar, NumBars, ...](numbars.htm)
- [Functions](funclist.htm)
---

# tick, tock

## tick()

User-supplied function that is called on arrival of a new price quote of any asset used in the strategy. This function can be used when an immediate reaction on every new price quote is required. 

## tock()

User-supplied function that is called once per minute (adjustable via [TockTime](ticktime.htm); unaffected by [DayOffset](date.htm)), regardless of price quotes. This function can be used for auxiliary purposes, such as periodic input/output tasks when live trading. 

## callback(void* Address)

User-supplied function that is called when either the [BrokerProgress](brokerplugin.htm) function is triggered with a pointer as argument, or when Zorro receives a **WM_APP+2** window message. This function is not in sync with other functions or I/O operations, so use the [call](call.htm) scheduler when modifying global variables or accessing non-reentrant parts of the broker API. 

### Remarks:

  * The **tick** function runs in irregular intervals dependent on market activity. The more assets are traded, the more quotes will trigger a **tick** run. The **tick** function normally executes much more frequently than the [run](run.htm) function, but can also execute less often when few quotes are received, f.i. after market hours. The minimum time between two **tick** calls can be set up with [TickTime](ticktime.htm). If a new quote arrives earlier, it is delayed until the **TickTime** is over. 
  * Within a **tick** function the asset name, the current ask price, and the current ask-bid spread can be evaluated with the **[Asset](script.htm)** string, with [priceClose(0)](price.htm), and with the [Spread](spread.htm) variable. The time stamp of the last quote is returned by [seconds(0)](month.htm) or [wdate(0)](month.htm). Date/time functions inside **tick** or **tock** return the current time, not the time of the last bar. For evaluating component specific parameters such [trade statistics](winloss.htm), call **[asset(Asset)](asset.htm)** and/or [algo(...)](algo.htm) at the begin of the **tick** function. 
  * The same price quote can trigger a **tick** function as well as a [TMF](trade.htm). The **tick** functions of all assets that had quotes in the last [TickTime](ticktime.htm) are executed first, afterwards all triggered TMFs are executed. 
  * In [Test] or [Train] mode, the **tick** and **tock** functions normally run only once per bar, right before the **run** function at the end of the bar. When the [TICKS](mode.htm) flag is set, the **tick** function runs at any new price quote of any asset in the historical data, and the **tock** function can run multiple times per bar. When a bar period is smaller than [TockTime](ticktime.htm), the **tock** function can run less often than once per bar. 
  * The [series](series.htm) function can not be called in any of the above functions; this includes indicators that internally create data series. However, series in global variables can be evaluated, and static series or arrays can be shifted in a tick or tock function by calling [shift](shift.htm). In this way tick based indicators or indicators on time frames shorter than [ BarPeriod](barperiod.htm) can be used.
  * As long as the current bar is not finished, the current candle is incomplete in any of the above functions. Its range and height differs to the preceding complete candles and should therefore normally not be used for indicators and trade signals. The [price](price.htm) functions reflect the current tick and return different values than in a **run** function. **priceC()** is the current price at the time of the tick. 
  * Trading with different assets in the same **tick** function can cause backtest snooping bias. When historical price ticks for assets A and B have the same time stamps, the **tick** function will first run with asset A, then with asset B. In the first run, asset A has the current price, but asset B still the previous price. This can be used to snoop the next B price especially when it strongly depends on the A price. This is a rare and mostly theoretical issue, but to prevent it, you can use the **tock** function instead of **tick** for trading multiple assets. The **tock** function is asynchronous to price quotes. Set [TockTime](ticktime.htm) to the tick resolution of the used historical data, or to [TickTime](ticktime.htm) in live trading.
  * When printing inside a **error** function, make sure that the printed text does not contain any of the trigger words. Otherwise you'll get an endless loop.
  * Tick, tock, and callback functions can be disabled or re-assigned at runtime with the [event](call.htm) function.

### Examples (see also [HWnd](hwnd.htm)):

```c
// print every price quote in a file
function tick(){  file_append("Log\\ticks.txt",    strf("\n%i.%i %2i:%2i:%2.3f => %s %4.5f",      day(),month(),hour(),minute(),second(),Asset,priceC())); }
```

```c
// store the last 100 ticks and timestamps
vars MyTicks,MyStamps; // use global series

function tick(){
  shift(MyTicks,priceC(0),100); // store the prices 
  shift(MyStamps,wdate(0),100); // store the timestamps
}

function run()
{
  MyTicks = series(0,-100); // generate static series
  MyStamps = series(0,-100); 
  ...
```

### See also:

[Bars and Candles](bars.htm), [Bar](numbars.htm), [BarPeriod](barperiod.htm), [TickTime](ticktime.htm), [manage](trade.htm), [run](run.htm), [call](call.htm), [user supplied functions](funclist.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
