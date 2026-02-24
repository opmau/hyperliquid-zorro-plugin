---
title: BarPeriod, TimeFrame
url: https://zorro-project.com/manual/en/barperiod.htm
category: Functions
subcategory: None
related_pages:
- [Fill modes](fill.htm)
- [run](run.htm)
- [bar](bar.htm)
- [Time zones](assetzone.htm)
- [Oversampling](numsamplecycles.htm)
- [price, ...](price.htm)
- [Time / Calendar](month.htm)
- [series](series.htm)
- [adviseLong, adviseShort](advisor.htm)
- [enterLong, enterShort](buylong.htm)
- [frame, frameSync](frame.htm)
- [TickTime, MaxRequests, ...](ticktime.htm)
- [set, is](mode.htm)
- [System strings](script.htm)
- [asset, assetList, ...](asset.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [LifeTime, EntryTime, ...](timewait.htm)
- [Bars and Candles](bars.htm)
- [Bar, NumBars, ...](numbars.htm)
- [dayHigh, dayLow, ...](day.htm)
- [Dates, holidays, market](date.htm)
- [BarMode](barmode.htm)
---

# BarPeriod, TimeFrame

## BarPeriod

The duration of one bar in minutes (default = **60**). The theoretical minimum bar period is one second (**1./60**) for normal scripts, and 1 ms (**0.001/60**) for [HFT simulation](fill.htm). Typical price curve inefficiencies disappear on low time frames, so don't go below **60** for indicators, filters, or analysis functions. Use **1440** for daily and **10080** for weekly bar periods. The bar period is the basic time frame in the script. It determines the width of a candle in the chart, and the frequency of the [run](run.htm) function that is called after every bar. If not explicitly set up in the script, it can be determined with the [Period] slider. User-defined bars (see [bar](bar.htm) function) have no fixed bar period; in that case **BarPeriod** should be set to the average duration of a bar.   

### Type:

**var**

## BarOffset

Bar start/end time offset in minutes; **0** or a positive number smaller than **BarPeriod** (default = **940** with daily bars, otherwise **0**). Bars and frames normally start at a date/time boundary; f.i. 60-minute bars start at every full hour, and daily bars normally start at UTC midnight when **BarOffset** is **0**. For daily bars, use **BarOffset** to shift the bar begin to the desired trading time of the day; the **940** default sets it to 15:40 in the bar [time zone](assetzone.htm). **BarOffset** is also automatically set by **[NumSampleCycles](numsamplecycles.htm)** for dividing a bar into equal time intervals and decreasing the offset by one interval per cycle; on the last cycle, **BarOffset** is 0. 

## TimeFrame

Time frame in bars (default = **1**) used for all subsequent [price](price.htm), [time](month.htm), [series](series.htm), [advise](advisor.htm), and [trade](buylong.htm) calls. This variable can be set within a strategy for using multiple time frames, for skipping bars, for synchronizing time frames to external events, or for for emulating special bars such as Range or Renko Bars. For instance, with a bar period of 5 minutes (**BarPeriod = 5**) and a time frame of 12 bars (**TimeFrame = 12**), series and trade signals use a one hour time frame (5*12 = 60 minutes).   
For synchronizing time frames to a certain event, time, or date, skip the bars inbetween by setting **TimeFrame** to **0**. Count the skipped bars, and set **TimeFrame** to the negative number of skipped bars when the event happens. The time frame synchronizing mechanism only affects the shifting of [series](series.htm); price and other functions are unaffected. See remarks and example; see also [frameSync](frame.htm) and [AssetFrame](assetzone.htm). 

## FrameOffset

Time frame offset in bars (default = **0** = no offset) used for all subsequent [price](price.htm), [time](month.htm), [series](series.htm), and [trade](buylong.htm) calls; must be smaller than **TimeFrame**. When fixed time frames are used, this variable determines the bar number within a time frame when series are shifted and trades are executed. This variable allows to generate trade signals at different times dependent on the asset. 

### Type:

**int**

### Remarks:

  * Use the [bar](bar.htm) function for special bars that are not bound to a certain bar period. 
  * **BarPeriod** can be set by script from one second (**1./60**) to one week (**7*24*60 = 10080**). Bar periods above **1** are rounded to a multiple of minutes. The [Period] slider moves in fixed steps corresponding to the standard bar periods of trading platforms; directly setting **BarPeriod** places the slider at the position closest to the **BarPeriod** value. If **BarPeriod** is set directly in the script, the slider is locked. 
  * In [Test] mode, bar generation is based on the UTC time stamps of ticks in the historical price data. All ticks that fall in the bar period contribute to the bar. For instance, a 10-minute bar ending at 12:00 contains all ticks with timestamps from 11:50:00.000 until 11:59:59.999. If the tick covers several prices, its time stamp should be from its close price. If historical time stamps are from the open or the middle price instead, you can use the [TickFix](ticktime.htm) variable for adjusting them. In [Trade] mode, bar generation is based on UTC time from the PC clock, not on the broker's server time for avoiding sync problems by connection interruptions and Internet lag.
  * On short bar periods it is recommended to set the [TICKS](mode.htm) flag and use high resolution [.t1 price data](testing.htm#m1t1). It's mandatory on bar periods of less than a minute \- not only for the backtest, but also for filling the lookback period in live trading. For this, set [History](script.htm) to **".t1"** and **BarPeriod** to the number of seconds divided by 60, f.i. **BarPeriod = 5./60.** for 5 seconds. Mind the decimal; **5/60** would be considered an **int** and thus evaluate to **0**. For extremely short bar periods, make sure that [TickTime](ticktime.htm) is always less than a bar period. 
  * For bar periods longer than a week, set **BarPeriod = 1440** and use **TimeFrame** for setting up monthly or quarterly time periods. 
  * **BarPeriod** and **BarOffset** affect the sampling of price data. Therefore they must be set in the first run before selecting the [asset](asset.htm), must not change afterwards, and must not be optimized. **TimeFrame** and **FrameOffset** can be set anytime and can be optimized without restrictions.
  * Historical price data should have sufficient resolution so that any **BarPeriod** contains at least one historical tick during market hours. If **BarOffset** is nonzero, historical and lookback data need accordingly higher resolution. For instance, with a daily bar period price data should have at least 10-minutes resolution when **BarOffset** is set so that bars end at 15:40. 
  * If **BarOffset** is **0** , bars begin and end at day/hour boundaries. For preventing that daily bars would begin and end at 00:00 midnight (and thus reflect the prices from the previous, not from the current day), **BarOffset** has a default value of **940** (equivalent to 15:40) on daily and weekly bars. Alternatively, set **BarOffset** to the time when you want the script to trade; for example, **BarOffset = 60*9+30;** for trading at market open at 9:30. Witb the default bar offset, weekly bars end on Friday 15:40.
  * All time zone settings assume UTC time for price history. If price history is in local time and the time of day is relevant for the system, either convert the timestamps to UTC or use [BarZone](assetzone.htm).  

### TimeFrame vs. BarPeriod:

  * **TimeFrame** is not the same  as a longer **BarPeriod**. The differences are listed below. If multiple time frames or time offsets are not required, set the time frame with **BarPeriod** only. This uses the lowest possible number of bars and results in the highest speed for testing and training. 
  * **Barperiod** determines the bar chart and the frequency of the [run](run.htm) function, while **TimeFrame** affects the [price](price.htm) functions, the shifting of data [series](series.htm), and the bars at which [advise](advisor.htm) and [trade](buylong.htm) calls are executed. 
  * Commands to enter or exit trades in the [run](run.htm) function are only executed when the **TimeFrame** has ended at the current bar. But trade entry, stop, or profit targets are observed at any bar period. The [run](run.htm) function itself is executed at any bar period, but the [frame](frame.htm) function can be used for restricting code execution to the end of the current time frame.
  * **TimeFrame** is normally not synchronized to day/hour boundaries, while **BarPeriod** is. For synchronizing time frames to days or hours, use the [frameSync()](frame.htm) function. For aligning daily time frames to a certain market hour in a certain time zone, use the [AssetFrame](assetzone.htm) variable. 
  * **TimeFrame** has no effect on variables in bar period units, f.i. [LookBack](lookback.htm), [UnstablePeriod](lookback.htm), [LifeTime](timewait.htm), or [WaitTime](timewait.htm). For extending those time periods, multiply the variable value with the number of bars per time frame. 
  * **TimeFrame** does not affect indicators that do not use a **Data** series, but directly the current asset price series. This is mentioned in the description of the indicator.
  * Recursive indicators from the TA-Lib such as **EMA** or **ATR** require [UnstablePeriod](lookback.htm) to be adapted to their maximum time frame. 

### Examples:

```c
#define H24 (1440/BarPeriod)
#define H4 (240/BarPeriod)
#define H1 (60/BarPeriod)
... 
BarPeriod = 60; // 1 hour (60 minutes) bars
...
// create a 4-hour price series ///////////////////////////////////
TimeFrame = H4;
vars PriceH4 = series(price());
 

// create a 24-hour price series /////////////////////////////////
TimeFrame = H24;
vars PriceH24 = series(price());

// skip bars outside market hours
static int SkippedBars = 0;if(!market(ET,0)) {   TimeFrame = 0;   SkippedBars--; // count negative number of bars outside market hours} else if(TimeFrame == 0) {   TimeFrame = SkippedBars;   SkippedBars = 0;} else  TimeFrame = 1;vars PriceInMarketHours = series(price());

// create a daily price series (equivalent to AssetZone) ////////
static int BarsPerDay = 0;
if(hour(0) < hour(1)) { // day change  
  TimeFrame = BarsPerDay; // end the frame
  BarsPerDay = 0;
} else {
  TimeFrame = 0; // inside the frame
  BarsPerDay--;  // count negative number of bars per day
}
vars PriceD1 = series(price());

// alternative: a midnight-aligned price series using frameSync()
StartWeek = 10000;
TimeFrame = frameSync(H24);
vars PriceD1 = series(price());

// back to 1-hour time frames
TimeFrame = 1;					
...
```

### See also:

[Bars and Candles](bars.htm), [NumBars](numbars.htm), [SampleOffset](numsamplecycles.htm), [LookBack](lookback.htm), [dayHigh](day.htm), [Date](date.htm), [BarMode](barmode.htm), [AssetFrame](assetzone.htm), [frameSync](frame.htm), [bar](bar.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
