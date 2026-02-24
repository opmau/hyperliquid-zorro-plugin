---
title: PlotMode
url: https://zorro-project.com/manual/en/plotmode.htm
category: Functions
subcategory: None
related_pages:
- [setf, resf, isf](setf.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [Margin, Risk, Lots, ...](lots.htm)
- [Chart Viewer / Debugger](chart.htm)
- [Multiple cycles](numtotalcycles.htm)
- [plot, plotBar, ...](plot.htm)
- [PlotBars, PlotWidth, ...](plotbars.htm)
- [Colors](colors.htm)
- [set, is](mode.htm)
---

# PlotMode

## PlotMode

Determines what's plotted at the end of a test or when clicking [Result]. Use [setf](setf.htm) and [resf](setf.htm) for setting and resetting the flags. 

### Flags:

**PL_LONG** \- begin the chart already with the [LookBack](lookback.htm) period. Otherwise the chart begins a few bars before the test or trade period.  
**PL_ALL** \- plot all asset specific curves, regardless of the selected asset (except for price curve and trades).   
**PL_ALLTRADES** \- plot all trades, regardless of the selected asset.  
**PL_FINE** \- plot equity and price curve in higher resolution.  
**PL_LINE** \- enforce a line rather than candles for the price curve.  
**PL_DIFF** \- plot the equity/balance difference to the initial value, rather than the total value. For better visibility of small equity changes when [Capital](lots.htm) is invested.  
**PL_FILE** \- export the chart to a **.png** image, rather than opening the [chart viewer](chart.htm). The file name is composed from the script name, an optional asset name, and the [LogNumber](numtotalcycles.htm).  
**PL_TITLE** \- print the chart title in the upper right area of the chart.  
**PL_BENCHMARK** \- plot the equity as a line, instead of a bar graph, for comparing equity curves with benchmarks.   
**PL_DARK** \- use a dark theme with a black background for the chart.  

### Type:

**int**

### Example:

```c
function run()
{
  ...
  setf(PlotMode,PL_ALL|PL_FINE);
  ...
}
```

### See also:

[plot](plot.htm), [PlotScale](plotbars.htm), [Colors](colors.htm), [setf](setf.htm), [resf](setf.htm), [mode](mode.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
