---
title: LookBack, UnstablePeriod
url: https://zorro-project.com/manual/en/lookback.htm
category: Functions
subcategory: None
related_pages:
- [Indicators](ta.htm)
- [Live Trading](trading.htm)
- [assetHistory](loadhistory.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [tick, tock](tick.htm)
- [series](series.htm)
- [Error Messages](errors.htm)
- [adviseLong, adviseShort](advisor.htm)
- [bar](bar.htm)
- [asset, assetList, ...](asset.htm)
- [optimize](optimize.htm)
- [set, is](mode.htm)
- [Bar, NumBars, ...](numbars.htm)
- [Bars and Candles](bars.htm)
- [Oversampling](numsamplecycles.htm)
- [Dates, holidays, market](date.htm)
---

# LookBack, UnstablePeriod

## LookBack

Number of bars that are executed before Zorro can begin to trade (default = **80**). Required for most [indicators](ta.htm) or other functions that need a certain amount of previous price history for their calculations. If left at the default value, it is automatically adapted to the needed lookback period by subsequent indicators. Otherwise set it to a value that is sure to cover the longest period of all used indicators, assets, and time frames. Set it to **0** when the script needs no lookback period. The maximum lookback period for live trading is one year. Backtests can have any lookback period as long as it does not exceed the total number of bars. 

##  LookBackNeeded 

Initially **0** ; automatically set by any indicator or function to the maximum lookback period needed so far (see example). If **LookBack** is at its default and only a single asset is used, **LookBack** is automatically set to **LookBackNeeded**. If **LookBack** is set to a value less than **LookBackNeeded,** an error message is printed and the script terminates if not restarted (see example). 

## UnstablePeriod 

Runs **recursive TA-Lib indicators** over the given period (default = **40**) at any call. Background: Some indicators are influenced by an infinite number of past bars. They are recursive or cumulative - indicators "with memory". Examples are the [EMA](ta.htm) (Exponential Moving Average) and the [ATR](ta.htm) (Average True Range). They use their previous bar's value in their algorithm, which in turn use the value of their previous bars, and so forth. This way a given value will have influence on all the subsequent values. In contrast, a simple moving average ([SMA](ta.htm)) only reflects the average of its time period, without any influence from bars further in the past.   
Because a price series is always finite and starts at a certain point, the effect of missing past bars is the more significant, the closer to the start a recursive indicator is calculated. Thus a trade strategy using the EMA or derived indicators (such as the [MACD](ta.htm)) could behave different dependent on its bar number. The **UnstablePeriod** value allows to strip off such initial unstable period and remove the influence of all bars prior to this period. This way indicators are guaranteed to behave the same way regardless of the amount of past data. For coming as close to the real cumulative formula as possible, strip off as much data as you can afford.  Using 40 bars (default) is reasonable for most indicators. 

##  LookBackResolution 

Candle resolution of the historical data that is loaded at [ trading start](trading.htm) for filling the lookback period, in minutes. Also used for [assetHistory](loadhistory.htm). At **0** (default), the resolution is equivalent to [ BarPeriod](barperiod.htm). If **BarOffset** is active or **.t1** data is used, the lookback resolution is accordingly increased. Set **LookBackResolution** to **1** , **5** , **15** , **30** , **60** , **240** , or **1440** for enforcing a higher or lower resolution of the lookback data. A higher lookback resolution can replicate the backtest better when [tick functions](tick.htm) are used, a lower resolution can help with long lookback periods when the broker history is limited.  

## TradesPerBar

Maximum number of trades and [series](series.htm) per bar, or **0** (default) for automatically estimating the maxima. Determines the allowed total number of trades when multiplied with the assets and bars of the simulation. Opening more trades causes an [Error 049](errors.htm) message, .more series cause [Error 041](errors.htm) Set this to a higher value when the strategy needs to enter an unusual number of trades or create an unusual number of series, as for special systems such as grid trading, or for training an [advise](advisor.htm) function. Set it to a low value, such as **1** , for saving memory when the system does not trade more often than once per bar on average.

## MinutesPerDay

Minimum daily trading time of the least traded asset in minutes (default = **360** , i.e. 60 minutes * 6 hours). Internally used for determining the maximum lookback time. Set this to a lower value when [Error 047](errors.htm) indicates that the lookback period gets not enough historical bars at trading start. This can be the case when an asset is only traded a few hours per day, or when you use a [bar](bar.htm) function that produces only a few bars per day. 

### Type:

**int**

### Remarks:

  * All variables on this page must be set before loading an [asset](asset.htm).
  * Lookback resolutions of less than one minute require tick data in **.t1** or **.t2** format in accordingly high resolution - not only in the backtest, but also for filling the lookback period in live trading. Set History = "*.t1
  * The number of the first bar at which a trade can be entered is greater or equal to **Lookback**. All trades are skipped before that bar. 
  * If **LookBack** is not set by script, it is automatically set to the maximum time period of all used indicators, and a message "**Lookback set to nnnn bars** " is printed to the window and log file. Automatic setting is not always possible, for instance when different [time frames](basrperiod.htm) are used, when indicator time periods change in the training run, or when different assets require different lookback periods. In that case set **LookBack** by script to the maximum needed value. 
  * If a particular **StartDate** is used, the lookback period is placed before the start date. The simulation must then begin at least **N** days before **StartDate** , where **N** can be estimated from **LookBack * BarPeriod/MinutesPerDay * 365/252**. 
  * If indicators get their data from other indicators - for instance, taking the a SMA of another SMA, or normalizing the value of an indicator - **LookBack** should be set to at least the sum of the maximum time periods of the nested indicators. 
  * !!  Setting **Lookback** to **0** overrides all internal lookback checks. You must be sure that a lookback period is really not needed. Otherwise the script can try to access nonexiting history, causing an error or a crash.
  * When using series with an offset (f.i. **myseries+n**), make sure that **LookBack** is always higher than the required lookback period of the series plus **UnstablePeriod** plus the highest offset. Otherwise the series length can be exceeded, resulting in a script error at runtime.
  * When [optimizing](optimize.htm) the time period of an indicator, make sure that **LookBack** is big enough to cover the maximum time period of the optimize range, plus the **UnstablePeriod** when the indicator is recursive. Do not optimize the **LookBack** period itself, since it must not change anymore after loading the first asset.
  * **UnstablePeriod** is only used for recursive TA-Lib indicators. For other recursive indicators such as **LowPass** , **DominantPeriod** , **UO** , etc. **LookBack** serves as unstable period. 
  * In [Trade] mode, price history covering the **LookBack** duration is downloaded from the server immediately before trading starts. Due to weekends and market hours, the lookback duration is normally longer than **LookBack * BarPeriod**. The downloaded history usually exceeds the lookback duration by a safety margin, but does not exceed 1 year. A longer lookback duration requires the [PRELOAD](mode.htm) flag and available price history. A too long lookback period will be indicated with a warning message. 
  * When trading multiple assets, the lookback period can be truncated for assets that have no price data at lookback begin. Use the [ COMMONSTART](mode.htm) flag to enforce full lookback periods for all assets. Otherwise, use [AssetFirstBar](numbars.htm) to check when the lookback period is over for a particular asset.

### Example:

```c
// restart script when lookback period is exceeded
void run()
{
  LookBack = max(30,LookBackNeeded);
  ...
  SMA(seriesC(),50); // need more than 30 bars!
  if(LookBack < LookBackNeeded)
    return quit("+Restart with new LookBack");
  ...
}
```

### See also:

[Bars](bars.htm), [BarPeriod](barperiod.htm), [NumBars](numbars.htm), [TimeFrame](barperiod.htm), [BarOffset](numsamplecycles.htm), [Trading Start](trading.htm), [Date](date.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
