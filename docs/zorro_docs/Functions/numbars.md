---
title: Bar, NumBars
url: https://zorro-project.com/manual/en/numbars.htm
category: Functions
subcategory: None
related_pages:
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [Dates, holidays, market](date.htm)
- [asset, assetList, ...](asset.htm)
- [run](run.htm)
- [Walk-Forward optimization](numwfocycles.htm)
- [set, is](mode.htm)
- [Strategy Statistics](statistics.htm)
- [Bars and Candles](bars.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [Oversampling](numsamplecycles.htm)
---

# Bar, NumBars

## NumBars

Total number of bars in the simulation, determined from [Lookback](lookback.htm), [StartDate](date.htm) and [EndDate](date.htm), [MaxBars](date.htm), and from the available data in the price history files. Is **0** in the initial run before the first [asset](asset.htm) call. 

## NumTicks

Total number of ticks processed in the simulation, determined from the price history resolution. Is **0** in the initial run before the first [asset](asset.htm) call. 

## Bar

Current bar number including the lookback period and preceding history, usually from **0** to **NumBars-1**. The number is not necessarily counted up straight since bars can be skipped (see remarks). 

## StartBar

Number of the first bar after the [lookback](lookback.htm) period. **(Bar-StartBar)** is the current duration of the test or trade period in bars. **(StartBar-LookBack)** is the number of the first bar after the **INITRUN**. 

## EndBar

Number of the last bar. 

## AssetFirstBar

Number of the first valid bar of the selected asset. Can be different for asset whose histories begin later than the initial bar. 

## Day

Current trading day number of the simulation, starting with the end of the lookback period. Periods with no bars, such as weekends and holidays, are skipped. 

### Type:

**int** , read/only 

### Remarks:

  * **NumBars** and **StartBar** are only valid after the [asset](asset.htm) history was loaded. 
  * The **Bar** number is normally increased by **1** on every [run](run.htm), but there are exceptions. When the simulation starts at a certain [StartDate](date.htm) or when [WFO](numwfocycles.htm) cycles are trained, bars from the price history preceding the [LookBack](lookback.htm) period are skipped and **Bar** can start with a high number after the initial run. **Bar** can even 'jump back' at the begin of any backtest WFO cyle when the [RECALCULATE](mode.htm) flag is used.
  * The number of bars spent in drawdown, and other bar statistics at the end of the simulation can be found under [Strategy statistics](statistics.htm). 

### Example:

```c
printf("\nBar %i of %i",Bar,NumBars);
```

### See also:

[Bars](bars.htm), [BarPeriod](barperiod.htm), [BarOffset](numsamplecycles.htm), [LookBack](lookback.htm), [StartDate](date.htm), [run](run.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
