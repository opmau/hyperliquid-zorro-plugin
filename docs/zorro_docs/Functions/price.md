---
title: price
url: https://zorro-project.com/manual/en/price.htm
category: Functions
subcategory: None
related_pages:
- [bar](bar.htm)
- [set, is](mode.htm)
- [Broker API](brokerplugin.htm)
- [Licenses](restrictions.htm)
- [series](series.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [Indicators](ta.htm)
- [asset, assetList, ...](asset.htm)
- [Fill modes](fill.htm)
- [PIP, PIPCost, Leverage, ...](pip.htm)
- [Bar, NumBars, ...](numbars.htm)
- [run](run.htm)
- [Verbose](verbose.htm)
- [TickTime, MaxRequests, ...](ticktime.htm)
- [Retraining](retraining.htm)
- [Detrend, shuffling](detrend.htm)
- [Spread, Commission, ...](spread.htm)
- [brokerCommand](brokercommand.htm)
- [assetHistory](loadhistory.htm)
- [IB / TWS Bridge](ib.htm)
- [Tips & Tricks](tips.htm)
- [orderCVD](ordercvd.htm)
- [adviseLong, adviseShort](advisor.htm)
- [enterLong, enterShort](buylong.htm)
- [tick, tock](tick.htm)
- [Historical Data](history.htm)
- [dayHigh, dayLow, ...](day.htm)
---

# price

# Price and Volume functions

## price(int offset) : var 

Returns the **TWAP** (time-weighted average price) of the selected asset. TWAP is the mean of all price ticks of the bar or time frame with the given **offset**. For indicators the mean price is normally preferable to the close price, since it is less susceptible to random fluctuations and makes systems more robust and less dependent on the data feed. 

## priceO(int offset) : var 

## priceC(int offset) : var 

## priceH(int offset) : var 

## priceL(int offset) : var 

Returns the open, close, maximum and minimum price of the current asset and the bar or time frame with the given **offset**. **priceC(0)** returns the current price.

## priceReal() : var 

Returns the current real price when OHLC prices have been artificially calculated by [special bars](bar.htm). The [TICKS](mode.htm) flag must be set. 

## marketVal(int offset) : var 

## marketVol(int offset) : var 

Returns additional market data, such as spread, quote size, trade volume, accumulated volume, tick rate, or quote frequency - see remarks and examples below. The type of data returned depends in live trading on the [Broker API](brokerplugin.htm), and in backtesting on the **fVal** and **fVol** fields in **.t6** historical data, or on the ask-bid difference and volume field in **.t1** or **.t2** data. Normally **marketVal** returns the average historical spread of the last bar period, and **marketVol** returns the average historical trade volume of the last bar period. For the total historical volume of a bar, multiply **marketVol()** with **BarPeriod** divided by the historical data resolution in minutes. The **market** functions require [Zorro S](restrictions.htm) and no [LEAN](mode.htm) flag.   

### Parameters:

**offset** | Bar or time frame number for which the prices are returned, relative to the current bar or time frame. **0** for the the current bar or time frame. Negative offsets return prices of future bars or time frames when the [PEEK](mode.htm) flag is set; otherwise they give an error message.   
---|---  
  
### Returns:

Price or raw market data.   

## seriesO(): vars 

## seriesH(): vars 

## seriesL(): vars 

## seriesC(): vars 

Returns pointers to a temporary open, high, low, and close price [series](series.htm) of the current asset. Can be used as a price series parameter to any function or indicator, or for referencing a price **N** bars ago (example: **var PreviousClose = (seriesC())[1];** \- mind the parentheses). Unlike real series, they don't consume memory and can be conditionally called. Temporary series do not support [TimeFrame](barperiod.htm) and [special bars](bar.htm), and keep their content only until the next [indicator](ta.htm) call.  

## priceSet (int offset _,_ var Open, var High, var Low, var Close) 

Modifies the open, high, low, close, and mean price of the current asset at the given bar, for [dummy assets](asset.htm), artificial price curves, displaying special candles, or using a different price source. Use **offset = -1** for modifying the prices of the next bar when trades should be entered at the modified prices. Does not affect intrabar prices in [TICKS](mode.htm) mode. 

## priceQuote (var Timestamp, var Quote) : int 

Enters a new price quote of the current [asset](asset.htm) in the system; especially for [HFT simulation](fill.htm) or when prices are not available from the broker connection. Filters outliers and updates the current best ask ([AskPrice](pip.htm)) and best bid ([AskPrice](pip.htm) \- [Spread](+)). Increases [Bar](numbars.htm) after every [BarPeriod](barperiod.htm) when no [run](run.htm) function is used. Price quotes are also printed to the log when [Verbose](verbose.htm) is at 3 or above, so be careful with **Verbose** for preventing awfully large log files. 

### Parameters:

**offset** | Optional bar number for which the prices are returned, in time frames before the current time frame. If **0,** the price of the current bar is returned. Negative offsets return future prices when the [PEEK](mode.htm) flag is set; otherwise they give an error message.   
---|---  
**Timestamp** | The exchange timestamp of the quote in DATE format. Quotes older than the previous quote are ignored.  
**Quote** | The price. **Quote > 0** indicates an ask price, **Quote < 0** a bid price.   
  
### Returns:

1 if the price quote was accepted, 0 if an [Outlier](ticktime.htm) was detected or the timestamp was outdated.  

## priceRecord ()

Appends the current OHLC candle and the current spread and volume of the current asset to the begin of the price history in **.t1** or **.t6** format; only in [Trade] mode and after the **INITRUN**. This allows recording live prices for [re-training](retraining.htm) or other purposes.  

### Remarks:

  * The **priceO** , **priceH** , **priceL** , **priceC** functions can be also used under the names **priceOpen** , **priceHigh** , **priceLow** , **priceClose**.
  * At the initial run of the strategy before an [asset](asset.htm) is selected, all price functions return **0**. After switching to a different [asset](asset.htm), the price and market functions automatically change to the prices and market data of that asset. In backtesting or training prices are affected by [Detrend](detrend.htm) flags.
  * If not otherwise mentioned, all prices are ask prices. This way stops, entry limits, or profit targets can be calculated without a distinction between long and short trades. Zorro automatically handles conversion from Ask to Bid when entering or exiting trades, calculating trade profit, or setting limit levels. For getting the bid price, subtract [Spread](spread.htm); for getting the pip value of a price, divide by **[PIP](pip.htm)**. The current bid/ask mean price is **priceClose() - 0.5*Spread**. More variants of prices - i.e. the Center Price, the Typical Price, the Haiken Ashi Price, the VWAP etc. - can be found on the [indicators](ta.htm) page. Live prices can be switched between different types (f.i. quotes and trades) with the [SET_PRICETYPE](brokercommand.htm) command when supported by the broker.
  * For changing the price range of an asset, f.i. for shifting negative prices to the positive scale, use [PriceOffset](spread.htm).
  * When loading price data, the prices are checked for plausibility dependent on the [Outlier](outlier.htm) parameter. Invalid prices, such as negative values or outliers, are automatically corrected or removed. Setting **[Detrend](detrend.htm) = NOPRICE** before calling **asset()** prevents that asset and price data is checked and invalid prices are removed. 
  * When connected to a broker or exchange, prices are updated at any incoming tick; when connected to an online [price source](loadhistory.htm), at any bar. Outliers and stock splits are detected and can be evaluated with the [PriceJump](outlier.htm) variable. Outliers are removed, splits are logged. 
  * If the [LEAN](mode.htm) flag is set and M1 historical data is used, open and close prices of a bar are not explicitely stored for preserving memory. They are derived from the mean prices of its first and last M1 tick.
  * If [LookBack](nhistory.htm) is not explicitely set, it is automatically expanded to the biggest **offset** value. If **offset** is higher than the current bar number ([Bar](numbars.htm)), the price of the first bar is returned. 
  * When using multiple time frames in the strategy ([TimeFrame](barperiod.htm) variable), the **offset** parameter gives the number of time frames rather than the number of bars. The function **priceOpen** returns the price from the begin of the time frame, **priceClose** from its end, **priceHigh, priceLow** the maximum and minimum price within the time frame, and **price** the current average of the time frame. Synchronizing time frames (**TimeFrame = 0**) to certain events or dates is not supported by price functions; use price [series](series.htm) for that when required. 
  * In live trading, **marketVal** and **marketVol** return spread and volume from the [broker API](brokerplugin.htm), when available. If volume is not available, a proxy is returned, such as the tick frequency. The type of returned volume is described on the specific broker page (f.i. [IB](ib.htm)).Some broker APIs support the [SET_VOLTYPE](brokercommand.htm) command for setting up the volume source between ask/bid or trade volumes. Historical volume is often the sum of quote sizes or trade volumes of the current minute or current day. Live volume is often the accumulated trade volume of the current day or trading session. The functions return the current value when **Offset** is 0, otherwise the average of the time frame. Depending on which sort of raw volume they return, it can be converted by script to the total volume per time frame. For instance, to convert accumulated volume to per-minute volume, subtract the volume from 1 bar ago from the current volume, and divide by **BarPeriod.** See examples below.  
In test and train modes, **marketVol** returns the **fVol** element of **.t6** historical data, or otherwise 0. The **marketVal** function returns the **fVal** element of .t6 data, and the ask-bid spread of .t1 or .t2 data, otherwise 0. 
  * Depending on the broker, live volume can be different to historical volume where delayed transactions, busted trades, combos and unreportable trades are filtered out. Likewise, outliers are often filtered out of historical price data. Trade signals should therefore not rely on comparing live volume with historical volume or live volatility with historical volatility.
  * For basket trading, artificial assets can be created by combining the prices from several real assets. An example can be found under [Tips & Tricks](tips.htm).
  * For evaluating live or historical order book data or BBO data, and for generating order flow profiles, use the [orderUpdate](ordercvd.htm) function.
  * For calculating VWAP prices from a given time period, store prices and volumes in [series](series.htm) and apply the [VWAV](ta.htm) function.
  * If the [PEEK](mode.htm) flag is set, price functions can peek into the future by using negative offsets. This is often used for training machine learning algorithms (see [advise](advisor.htm)) with historical price data. If [PEEK](mode.htm) mode is not set, negative offsets will generate an error message. 
  * In a [TMF](buylong.htm) or [tick](tick.htm) function, **priceC(0)** returns the last price quote, updated every live or historical tick when new price data becomes available. **marketVal(0)** returns the last ask-bid spread and **marketVol(0)** returns the last trade or quote volume if available.
  * If [.t1 data](history.htm) contains both ask and bid quotes, **marketVal** returns the recent ask-bid spread. In [.t6 data](history.htm) it returns the **fVal** value. The command **if(Test)** **Spread = max(0,marketVal());** can be used for backtesting with variable spread when it is available in the historical .t1, .t2, or .t6 data.
  * An example of using the **priceQuote** function for simulating a HFT system can be found on [Financial Hacker](http://www.financial-hacker.com/hacking-hft-systems/). 

### Examples:

```c
BarPeriod = 60; // set up 1 hour bars (60 minutes)

TimeFrame = 4;
asset("EUR/USD");
vars EUR4Hs = series(price(0));	// create a series of 4 hour EUR mean prices
...
TimeFrame = frameSync(24);
asset("SPX500");
vars SPXDays = series(priceC(0));	// create a series of daily S&P 500 close prices
TimeFrame = 1;
...
```

```c
// plot a daily spread histogram
void run()
{
  BarPeriod = 10;
  PlotLabels = 4;
  plotBar("Spread",
    (60*hour(0)+minute(0))/BarPeriod,tod(0),marketVal(0),AVG,RED);
}
```

```c
// convert accumulated daily trade volume to per-bar volume
var volumeDiff(var* Buffer,var Volume)
{
// Buffer[1] = previous accumulated volume
  if(Buffer[1] > Volume) // day change? Reset volume
    Buffer[1] = 0;
  Buffer[0] = Volume-Buffer[1]; // per-bar volume difference
  Buffer[1] = Volume;
  return Buffer[0]; 
}

void run()
{
  BarPeriod = 1;
  LookBack = 0;
  
  assetList("AssetsIB");
  brokerCommand(SET_PRICETYPE,2); // trade prices
  brokerCommand(SET_VOLTYPE,4); // trade volume
  
  vars VBuffer = series(0,-2);
  var Volume = marketVol(0);
  if(Live) // if marketVol is daily accumulated volume
    Volume = volumeDiff(VBuffer,Volume);
  printf("# Vol %.0f",Volume);
}
```

### See also:

[enterLong](buylong.htm)/[Short](buylong.htm), [series](series.htm), [asset](asset.htm), **[Spread](spread.htm)** , [AskPrice](pip.htm), [AskSize](pip.htm), [Detrend](detrend.htm), [dayHigh/Low](day.htm), [History](history.htm), [GET_PRICE](brokercommand.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
