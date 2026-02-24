---
title: NumSampleCycles, SampleOffset, SampleCycle
url: https://zorro-project.com/manual/en/numsamplecycles.htm
category: Functions
subcategory: None
related_pages:
- [BarPeriod, TimeFrame](barperiod.htm)
- [asset, assetList, ...](asset.htm)
- [Multiple cycles](numtotalcycles.htm)
- [set, is](mode.htm)
- [Performance Report](performance.htm)
- [Trade Statistics](winloss.htm)
- [Chart Viewer / Debugger](chart.htm)
- [Bars and Candles](bars.htm)
- [Walk-Forward optimization](numwfocycles.htm)
- [TrainMode](opt.htm)
- [W5 - Optimizing, WFO](tutorial_fisher.htm)
---

# NumSampleCycles, SampleOffset, SampleCycle

# Oversampling

Backtests become more accurate when more data is available and more trades can be opened. The problem: price data is normally in short supply. Oversampling the bars is a method to run multiple cycles on slightly different price curves that are derived from the original curve and contain the same inefficiencies. This produces more trades, generates better trained parameters and more realistic backtests, and allows to evaluate the effect of random price curve fluctuations on the system performance. 

Oversampling can be applied to training or backtest, either **per-bar** or **per-cycle**. Per-bar oversampling requires price data with higher resolution than the bar period, f.i. M1 data with 1-hour bars. Oversampling shifts the bar begin by a fraction of the bar period on any cycle. This results in different bars and - dependent on the strategy - more or less different trades with the same price curve. 

For per-cycle oversampling, a time offset is added to the start of any cycle. It is normally used to detect start date/time dependences of the trading system in backtests.

A description of per-bar oversampling with an example can be found on [ http://www.financial-hacker.com/better-tests-with-oversampling](http://www.financial-hacker.com/better-tests-with-oversampling/). The following variables activate and control oversampling:

## NumSampleCycles

Number of oversampling cycles (default = **0** = no oversampling). When set to a number **n** > 1, the simulation is repeated **n** times. For per-bar oversampling, the bars are resampled in any cycle with different [BarOffset](barperiod.htm) values. This generates a slightly different price curve for every cycle, while maintaining the trend, spectrum, and most other characteristics of the curve. For per-cycle oversampling, the **SampleOffset** is added to the start time of any test, training, or WFO run. The performance result is calculated from the average of all cycles. This way more data for test and training is generated and a more accurate result can be achieved. 

## SampleOffset

Time offset in bars for per-cycle oversampling. The begin of a test or training cycle is automatically shifted by that time at any cycle. If at **0** (default), per-bar oversampling is used instead.

##  SampleCycle

The number of the current cycle from **1** to **NumSampleCycles**. Automatically set at the begin of any cycle. 

### Type:

**int**

### Remarks:

  * On bar periods of one or several hours, oversampling is often useful to get enough trades for properly optimizing and testing a strategy. Good values for **NumSampleCycles** are **2..6** for per-bar oversampling. Even higher oversampling factors won't increase the accuracy much further. 
  * Oversampling starts with the highest offset and ends with offset 0. For instance, on a 60 minutes bar period and **NumSampleCycles = 4** , the 4 cycles get bar offset 45, 30, 15, and 0.
  * Oversampling must be activated before the first [ asset](asset.htm) call. It cannot be used when [BarOffset](barperiod.htm) is otherwise set or when the strategy is based on certain times or dates, f.i. strategies that always trade on the first of any month or on market opening or closing time. 
  * For a histogram of the performance dependent on start time, use **SampleOffset** in combination with **[NumTotalCycles](numtotalcycles.htm)**.
  * Oversampling increases the memory footprint, similar to the [TICKS](mode.htm) flag. 
  * The performance of the separate cycles is displayed in the [performance report](performance.htm) under **Cycle performance** , the sample statistics under **Sample Cycles**. High performance differences between cycles normally indicates an unstable strategy. The global [statistics values](winloss.htm) are the average over all sample cycles.
  * When the [ALLCYCLES](mode.htm) flag is set, the component specific [statistics values](winloss.htm) and the [portfolio analysis](performance.htm) are the sum over all bar cycles; they keep their values from the last cycle when a new cycle is started. Otherwise they are reset at the begin of every cycle and thus reflect the last cycle at the end of the simulation. The log contains all cycles when [ALLCYCLES](mode.htm) is set; otherwise it contains only the last cycle.
  * In the [price chart](chart.htm), the trade symbols are taken from the last cycle, and the equity or balance curve is the average over all cycles. When plotting individual equity curves or accumulative parameters in **ALLCYCLES** mode, take them only from the first cycle to avoid their accumulation over all cycles (**if(SampleCycle == 1) plot(...);**). 

### Examples:

```c
function run() {
  NumSampleCycles = 4; // 4 cycles per-bar oversampling
  ...

function run() {
  NumSampleCycles = 3; // 3 cycles per-cycle oversampling
  SampleOffset = 10;   // with 10 bars start difference 
  ...
```

### See also:

[bar](bars.htm), [BarOffset](barperiod.htm), [NumWFOCycles](numwfocycles.htm), **[NumOptCycles](opt.htm)** , **[NumTotalCycles](numtotalcycles.htm)** , [](tutorial_fisher.htm)[](numparameters) [ALLCYCLES](mode.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
