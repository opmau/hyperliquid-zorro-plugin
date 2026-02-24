---
title: NumTotalCycles, TotalCycle
url: https://zorro-project.com/manual/en/numtotalcycles.htm
category: Functions
subcategory: None
related_pages:
- [PlotMode](plotmode.htm)
- [System strings](script.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [set, is](mode.htm)
- [Oversampling](numsamplecycles.htm)
- [Walk-Forward optimization](numwfocycles.htm)
- [Log Messages](log.htm)
---

# NumTotalCycles, TotalCycle

# Multiple Test Cycles

## NumTotalCycles

Repeat the complete simulation in training or test mode in as many cycles as given by this variable (default = **0** = no repetitions). This is normally used for doing reality checks, plotting histograms, running special Montecarlo simulations or evaluating result statistics dependent on changed parameters or randomized price curves. 

## TotalCycle

The number of the current cycle from **1** to **NumTotalCycles**. Read/only, automatically set by **NumTotalCycles**. 

## LogNumber

If nonzero, any cycle will generate individual log files. When above zero, the given number is appended to the log, trade list, performance report, and chart image files (when [PL_FILE](plotmode.htm) is set). By setting **LogNumber = TotalCycle** , different log files, performance reports, and chart images can be generated for any cycle. At **0** (default), no logs, reports, or images are produced when **NumTotalCycles** is set. At **-1** , any cycle will produce log files with no appended number, so individual names can be set with the [Script](script.htm) string. Set this variable right at the begin of **run** or **main** , before calling any function that writes something into the log. 

### Type:

**int**

### Remarks:

  * If [BarOffset](barperiod.htm) was changed or the [PRELOAD](mode.htm) flag was set, prices are loaded again at the begin of every cycle. This causes a delay, but allows to run cycles with different bar period or bar mode settings, modified price curves, shuffled prices, or different assets. Otherwise prices are only loaded on the first cycle.
  * For training on any cycle, set the [TESTNOW](mode.htm) flag and run the cycles in [Train] mode.

### Example (see also Detrend.c, MRC.c): 

```c
function run()
{
  LogNumber = TotalCycle;
  BarPeriod = 1440;
  StartDate = 2015;
  NumYears = 1;
  LookBack = 0;
 
// run this simulation 1000 times  
  NumTotalCycles = 1000;
 
// some random trading strategy
  if(random() > 0)
    enterLong();
  else 
    enterShort();
 
// plot the result of every run in a bar graph  
  if(is(EXITRUN))
    plotHistogram("Profits",Balance/PIPCost,250,1,RED);   
}
```

### See also:

[NumSampleCycles](numsamplecycles.htm), [NumWFOCycles](numwfocycles.htm), [Log](log.htm) [► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
