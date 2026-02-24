---
title: is
url: https://zorro-project.com/manual/en/is.htm
category: Functions
subcategory: None
related_pages:
- [set, is](mode.htm)
- [Button](button.htm)
- [Licenses](restrictions.htm)
- [require, version](version.htm)
- [printf, print, msg](printf.htm)
- [Multiple cycles](numtotalcycles.htm)
- [evaluate](evaluate.htm)
- [optimize](optimize.htm)
- [quit](quit.htm)
- [panel, ...](panel.htm)
- [tick, tock](tick.htm)
- [call](call.htm)
- [slider](slider.htm)
- [Money Management](optimalf.htm)
- [adviseLong, adviseShort](advisor.htm)
- [Command Line Options](command.htm)
- [Chart Viewer / Debugger](chart.htm)
- [loop](loop.htm)
- [asset, assetList, ...](asset.htm)
- [setf, resf, isf](setf.htm)
---

# is

# Status Flags

Zorro's current status can be checked through **status flags** that can be either on (active) or off (not active). The [is()](mode.htm) function is used for reading the state of a status flag. The following status flags are available:

## TESTMODE

## TRAINMODE

## TRADEMODE

The script is doing a [Test], [Train], or [Trade] run (see also [Button](button.htm)). 

## DEMO

A demo account is selected through the [Account] scrollbox.   

## SPONSORED

The script is running on a [Zorro S](restrictions.htm) version. See also [version()](version.htm). 

## CHANGED

The [Script] or [Asset] scrollboxes have been changed since the last script run, or the [Edit] button has been clicked. The sliders are then set back to their default positions. Use this flag when you want to initialize something only at the first run of the script. 

## AFFIRMED

The [Ok] button of a [nonmodal message box](printf.htm) has been clicked.  

## INITRUN

Initial run of the session or simulation before the price data is loaded and before the log file is opened. Can be used to initialize global and static variables. System variables that are not yet known - f.i. variables that depend on the asset and simulation period - are at **0** during the initial run. 

## FIRSTRUN

First run of the session or simulation with valid price data, parameters, factors, and system variables. Normally immediately after the **INITRUN**. 

## FIRSTINITRUN

First initial run of the script, in the case of multiple simulation runs due to training cycles or [NumTotalCycles](numtotalcycles.htm). While **INITRUN** and **FIRSTRUN** is set at the start of any simulation run, **FIRSTINITRUN** is only set at the first initial run. It is not set in any subsequent training or test cycles. 

## EXITRUN

Last run of the simulation. All trades are closed. Can be used to calculate results, aside from the [evaluate](evaluate.htm) or [objective](optimize.htm) functions. If a live session or simulation is aborted with [Stop] or [quit(0)](quit.htm), no **EXITRUN** is performed. 

## MARGINCALL

At least once in the simulation a trade was liquidated because the equity dropped below the maintenance margin.   

## RUNNING

The script is currently running in simulation or live trading. Script functions can also be executed outside a simulation run f.i. by clicking on [Result] or on a [panel](panel.htm) button. 

## BUSY

A broker API call or a **main** , **run** , **tick** , or **tock** function is currently executed. Check this flag to determine if an asychronous process, f.i. triggered by a [click](panel.htm) or [callback](tick.htm) function, is allowed to access the broker API or global variables. See also [call()](call.htm). 

## SLIDERS

One of the [sliders](slider.htm) has been moved. Reset this flag with **set(SLIDERS|OFF);** for detecting the next slider movement.   

## LOOKBACK

The script is currently in the lookback period, either at the begin of the simulation, or due to a [RECALCULATE](mode.htm) run. 

## NEWDAY

The current bar is the first bar of the current day. 

## PARCYCLE

The script is currently generating [parameters](optimize.htm) in [Train] mode. 

## FACCYCLE

The script is currently generating [capital allocation factors](optimalf.htm) in [Train] mode. 

## RULCYCLE

The script is currently generating [rules](advisor.htm) or machine learning models in [Train] mode.   

## COMMAND

The Zorro instance was started through the [command line](command.htm). 

## COMPILED

The script is an executable (*.x or *.dll). 

## STEPWISE

The script runs in single step mode. For details see [debugging](chart.htm).  

## PORTFOLIO

The script called the [loop](loop.htm) function. 

## ASSETSET

The script called the [asset](asset.htm) or [ assetAdd](asset.htm) function. 

## PLOTSTATS

The script called commands that plot a histogram rather than a price chart.   

Some macros for often-used flags have been defined for convenience:

## Train

The same as **is(TRAINMODE)**. 

## Test

The same as **is(TESTMODE)**. 

## Live

The same as **is(TRADEMODE)**. 

## ReTrain

Process for updating parameters or rules, started by clicking [Train] while live trading ([Zorro S](restrictions.htm) only). 

## ReTest

Process started by clicking [Test] while live trading, f.i. for comparing the live trading results with backtest results of the same period ([Zorro S](restrictions.htm) only). 

## Init

The same as **is(INITRUN)**.  

### Example:

```c
function run()
{
  if(is(TRAINMODE)) set(SKIP3);
  if(is(TESTMODE)) set(SKIP1,SKIP2,LOGFILE,TICKS); 
  ...
}
```

### See also:

[System flags](mode.htm), [isf](setf.htm), [Button](button.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
