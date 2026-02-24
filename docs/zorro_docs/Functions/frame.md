---
title: frame, frameSync
url: https://zorro-project.com/manual/en/frame.htm
category: Functions
subcategory: None
related_pages:
- [BarPeriod, TimeFrame](barperiod.htm)
- [Bars and Candles](bars.htm)
- [series](series.htm)
- [Tips & Tricks](tips.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [Time zones](assetzone.htm)
- [BarMode](barmode.htm)
---

# frame, frameSync

## frame(int Offset): bool

Returns **true** when the current [TimeFrame](barperiod.htm) ends at the given bar, and **false** when inside a time frame. 

## frameSync(int Period): int

Returns a synchronized [TimeFrame](barperiod.htm) that is aligned to the begin of an hour, a 4 hours period, a day, or a week. Returns **0** inside the time frame, and the negative number of skipped bars at the end of the time frame. Periods that are higher than one week or are no divisor of a full hour are not synchronized. [FrameOffset](barperiod.htm) can be used to shift the synchronized time frame to a particular time or workday. 

### Parameters:

**Offset** | [Bar](bars.htm) distance to the current bar. Must be **0** when **TimeFrame** is synchronized to a fixed time period or to an external event.   
---|---  
**Period** | Time period to be synchronized, in bars; an integer fraction of a hour, a 4 hours period, a day, or 7 days for a week.   
  
## sync(vars Data, int Type, int Period) : var

Selects a value of the **Data** series using a synchronized time frame given by **Period**. Can be used to generate higher time frames of price series or indicators. Dependent on **Type** , the first, maximum, minimum, or last element within the time frame, or the previous value is selected. The function internally creates [series](series.htm) (see remarks). Source code in **indicators.c**. 

### Parameters:

**Data** | Data series, for instance prices or indicator values.  
---|---  
**Type** | **1** first, **2** highest, **3** lowest, **4** last element, **+8** for the previous value.   
**Period** | Synchronized time period, in bars; an integer fraction of a hour, a 4 hours period, a day, or 7 days.   
  
### Remarks:

  * **TimeFrame = 24** is different to **TimeFrame = frameSync(24)**. In the former case the time frame changes every 24 bars, in the latter case every day on a 1-hour bar period, even if there are less than 24 bars per day.
  * The **Period** parameter to be synchronized on must not change from bar to bar.
  * Not all functions support synchronized time frames, but [series](series.htm) do. For synchonizing **TimeFrame** to arbitrary events, look up the **frameAlign** helper function under [Tips&Tricks](tips.htm).
  * **TimeFrame = frameSync(...)** will not automatically set [LookBack](lookback.htm), so if required, set it manually to the needed maximum value.

### Examples:

```c
// let a time frame change daily at 3:00
BarPeriod = 60;
FrameOffset = 3;
TimeFrame = frameSync(24);
...

// shift a series every 24 hours
BarPeriod = 60;
if(0 == frameSync(24)) set(NOSHIFT);
vars MySeries = series(MyValue);
set(NOSHIFT|OFF);
...

// return OHLC prices from synchronized higher time frames
var priceSyncOpen(int Period) { return sync(seriesO(),1,Period); }
var priceSyncClose(int Period) { return sync(seriesC(),4,Period); }
var priceSyncHigh(int Period) { return sync(seriesH(),2,Period); }
var priceSyncLow(int Period) { return sync(seriesL(),3,Period); }
```

### See also:

[Bar](bars.htm), [TimeFrame](barperiod.htm), [BarZone](assetzone.htm), [BarMode](barmode.htm), [suspended](suspended.htm), [series](series.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
