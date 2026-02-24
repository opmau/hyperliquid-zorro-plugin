---
title: run
url: https://zorro-project.com/manual/en/run.htm
category: Functions
subcategory: None
related_pages:
- [slider](slider.htm)
- [Dates, holidays, market](date.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [tick, tock](tick.htm)
- [asset, assetList, ...](asset.htm)
- [Variables](aarray.htm)
- [Zorro.ini Setup](ini.htm)
- [Status flags](is.htm)
- [set, is](mode.htm)
- [watch](watch.htm)
- [W4 - Trend Trading](tutorial_trade.htm)
- [Bars and Candles](bars.htm)
- [Bar, NumBars, ...](numbars.htm)
- [Functions](funclist.htm)
---

# run

## run()

Main strategy function, written by the user. It is automatically started after clicking on the [Train], [Test], and [Trade] buttons. After the first run, it runs again at the end of every bar. This is continued until the end of the simulation or until [Stop] is hit. The **run** function initializes [sliders](slider.htm) and sets up all needed indicators. 

### Remarks:

  * If [StartDate](date.htm) is set to **NOW** , an extra **run** is executed in live trading mode immediately after the lookback period, regardless whether the current bar has ended. If [LookBack](lookback.htm) is 0, this extra run is the first run. After that extra run, the **run** function is furtherhin executed at the end of every bar. 
  * For executing the **run** function several times within a candle, make the [BarPeriod](barperiod.htm) shorter and set up the candle width with [TimeFrame](barperiod.htm) so that several bars lie within a candle. Alternatively, use a [tick](tick.htm) function.
  * At the first call of the **run** function, **is(INITRUN)** and **is(FIRSTINITRUN)** are true. Since price and bar data are not yet set up at that initial run before the first [asset](asset.htm) call, all price functions and indicators return **0**. 
  * If the script has both a **main** and a **run** function, the **main** function is executed before the first **run** , and can be used for initializing variables. 
  * Local variables 'forget' their values after every call of the **run** function, but [static](aarray.htm) and [global](aarray.htm) variables keep their values.  !!  When **[AutoCompile](ini.htm)** is set, static and global variables really keep their values until the script is compiled again, which happens only when it was modified or when the [Edit] button was clicked. Therefore make sure to initialize them at every cycle start ([is(INITRUN)](is.htm)) to their initial values when this is required.
  * For debugging the **run** function, use the [STEPWISE](mode.htm) flag and [watch](watch.htm) commands. For starting debugging after the lookback period, use **if(!is(LOOKBACK)) set(STEPWISE);**. 
  * For immediate script reaction on an incoming price quote, use the [tick](tick.htm) function. For repeated actions independent on bars or price quotes, use the [tock](tick.htm) function. 

### Example:

```c
function run()
{
  vars SmoothPrice = series(LowPass(series(price()),100));

  if(valley(SmoothPrice)) enterLong();
  if(peak(SmoothPrice)) exitLong();
}
```

### See also:

[Workshop 4](tutorial_trade.htm), [Bars and Candles](bars.htm), [Bar](numbars.htm), [BarPeriod](barperiod.htm), [tick](tick.htm), [tock](tick.htm), [user supplied functions](funclist.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
