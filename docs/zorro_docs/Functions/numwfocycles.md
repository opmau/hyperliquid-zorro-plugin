---
title: NumWFOCycles, WFOCycle, SelectWFO
url: https://zorro-project.com/manual/en/numwfocycles.htm
category: Functions
subcategory: None
related_pages:
- [optimize](optimize.htm)
- [adviseLong, adviseShort](advisor.htm)
- [asset, assetList, ...](asset.htm)
- [Status flags](is.htm)
- [Training](training.htm)
- [Links & Books](links.htm)
- [set, is](mode.htm)
- [Performance Report](performance.htm)
- [Dates, holidays, market](date.htm)
- [DataSplit, DataSkip, ...](dataslope.htm)
- [Multiple cores](numcores.htm)
- [Included Scripts](scripts.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [series](series.htm)
- [Retraining](retraining.htm)
- [plotProfile, ...](profile.htm)
- [W5 - Optimizing, WFO](tutorial_fisher.htm)
- [Oversampling](numsamplecycles.htm)
- [Multiple cycles](numtotalcycles.htm)
- [TrainMode](opt.htm)
---

# NumWFOCycles, WFOCycle, SelectWFO

# Walk Forward Optimization

Walk Forward Optimization (WFO in short) is a variant of cross-validation for time series and was first published by Robert Pardo as a backtest method for algorithmic trading strategies. It trains and tests the strategy in several cycles using a data frame that "walks" over the simulation period. This allows an [out-of-sample backtest](https://zorro-project.com/backtest.php) that still uses most of the historical data. WFO is not restricted to backtests, but can be continued In live trading and regularly adapt the strategy parameters to the current market situation. In this way, a WFO trained strategy is essentially a 'parameter-less' system.  
  
For greatly reducing the backtest time with WFO, Zorro separates test and training and stores all trained parameters, trading rules, or machine learning models separately for any WFO cycle. This way a backtest needs not perform all optimizations again. It automatically switches between sets of parameters or rules. To activate WFO in a strategy script, only the following variable needs to be set: 

## NumWFOCycles

Number of cycles in a **Walk Forward Optimization / Analysis** (default = **0** = no Walk Forward Optimization). If **NumWFOCycles** is set to a positive number at or above**2** , rolling walk forward optimization is enabled with the given number of cycles; if it is set to a negative number at or below **-2** , anchored walk forward optimization is enabled. In Walk Forward Optimization, a data frame consisting of a training and test period is shifted over the simulation period in the given number of cycles (see image for ).  
  
**WFOCycle** | **Simulation period**  
---|---  
**1** | **LookBack** | **Training** | **Test1** |  |  |   
**2** |  | **LookBack** | **Training** | **Test2** |  |   
**3** |  |  | **LookBack** | **Training** | **Test3** |   
**4** |  |  |  | **LookBack** | **Training** | **Test4**  
**5** |  |  |  |  | **LookBack** | **Training**  
**OOS Test** | **LookBack** | **Test5** |  |  |   
**WFO Test** |  |  |  | **Look** | **Back** | **Test1** | **Test2** | **Test3** | **Test4**  
  
Rolling Walk Forward Optimization (**NumWFOCycles** **= 5**)   

**WFOCycle** | **Simulation period**  
---|---  
**1** | **LookBack** | **Training** | **Test1** |  |  |   
**2** | **LookBack** | **Training** | **Test2** |  |   
**3** | **LookBack** | **Training** | **Test3** |   
**4** | **LookBack** | **Training** | **Test4**  
**5** | **LookBack** | **Training**  
**WFO Test** |  |  |  | **Look** | **Back** | **Test1** | **Test2** | **Test3** | **Test4**  
  
Anchored Walk Forward Optimization (**NumWFOCycles** **= -5**)  

[Strategy parameters](optimize.htm) and [trade rules](advisor.htm) are generated separately for every cycle in [Train] mode, and are separately loaded for every test segment in [Test] mode. This way, the strategy test is guaranteed to be out of sample - it uses only parameters and rules that were generated in the preceding training cycle from data that does not occur in the test. This simulates the behavior of real trading where parameters and rules are generated from past price data. A Walk Forward Test gives the best prediction possible of the strategy performance over time. 

## WFOPeriod

Alternative to **NumWFOCycles** ; number of bars of one WFO cycle (training + test), f.i. **5*24** for a one-week WFO cycle with 1-hour bars. For getting a fixed test period of **N** bars, set **WFOPeriod** to **N*100/(100-DataSplit)**. Since the number of WFO cycles now depends on the number of bars in the history, [asset](asset.htm) must be called before. The WFO period is automatically adjusted so that the simulation period covers an integer number of WFO cycles. 

## WFOSelect

Set this to a certain WFO cycle number for selecting only this cycle in a training or test; otherwise the process runs over all cycles. Setting it to **-1** selects the last cycle that's normally used for live trading. 

### Type:

**int**

## WFOCycle

The number of the current WFO cycle, from **1** to **NumWFOCycles**. Automatically set during WFO test and training. 

## WFOBar

The bar number inside the current WFO cycle, starting with **0** at the begin of the cycle. Can be used to determine when the cycle starts: **if(WFOBar == 0)**. Automatically set during a WFO test. 

## ISPeriod

Number of bars of the in-sample (training) period of a WFO cycle. Automatically set in the [FIRSTRUN](is.htm) of a WFO test. 

## OOSPeriod

Number of bars of the out-of-sample (test) period of a WFO cycle. Automatically set in the [FIRSTRUN](is.htm) of a WFO test. 

### Type:

**int, read/only**

### Remarks:

  * WFO theory is explained in the [Training](training.htm) chapter and in more detail in the [Black Book](links.htm). In training, every WFO cycle is a separate simulation run including [INITRUN](mode.htm) and [EXITRUN](mode.htm). In backtesting all WFO cycles are tested in a single simulation run. In live trading, only the data trained in the last WFO cycle is used.
  * Due to the preceding training cycle, the overall test period is reduced by WFO (see diagrams above). For a longer test period, increase the number of WFO cycles and/or reduce **DataSplit**. The length of a test and training cycle can be found in the [performance report](performance.htm) under "WFO Test Cycles" and "WFO Train Cycles". The WFO test starts at [StartDate](date.htm) plus one WFO training cycle.
  * When changing the number of WFO cycles or the backtest period, the system must be trained again. Otherwise the WFO test and training cycles will be out of sync. For avoiding unintended changes of the test period when new price data becomes available, set a fixed [StartDate](date.htm) and [EndDate](date.htm). 
  * If **[DataSplit](dataslope.htm)** is not set otherwise, WFO uses a default training period of **85%** and a default test period of **15%**. 
  * [DataSlope](dataslope.htm) can improve the parameter quality with anchored WFO cycles. 
  * Use [DataHorizon](dataslope.htm) to block trades at the begin of a new WFO period. This prevents accidental signals by parameter changes, or test bias when a training target is based on future prices or future trade returns, as for [machine learning](advisor.htm) algorithms.
  * WFO training is much faster than normal training when using [multiple CPU cores](numcores.htm). 
  * For executing code at the begin of every WFO cycle, check **if(WFOBar == 0) ...**.
  * The optimal number of WFO cycles depends on the strategy, the backtest period, and the time frame. Normally a WFO cycle should cover about six to twelve months. Large time frames or infrequent trading require a small number of long WFO cycles for getting enough trades per cycle. Very market-dependent strategies with fast expiring parameters require a high number of short WFO cycles. If the strategy performance highly varies with small changes of **NumWFOCycles** , a periodic seasonal effect can be the reason. Try to determine and eliminate seasonal effects before optimizing a strategy. Use the [WFOProfile](scripts.htm) script for finding the most robust number of WFO cycles for your strategy.
  * Parameters and rules by WFO are stored in files in the **Data** folder. The number of the cycle is added to the file name, except for the last WFO cycle. F.i. a parameter WFO of a script "**Trade.c** " and 4 cycles will generate the following parameter files in the **Data** folder: **Trade_1.par** , **Trade_2.par** , **Trade_3.par** , **Trade.par** , **Trade.fac**. [Test] mode reloads the parameters and rules for every segment during the test cycle. [Trade] mode uses the parameters and rules from the last WFO cycle, without an attached number. 
  * Normally the [LookBack](lookback.htm) period precedes only the first WFO cycle in [Test] mode. Subsequent cycles just continue series and indicators. If the [RECALCULATE](mode.htm) flag is set, a dedicated lookback period precedes every WFO cycle. The content of all [series](series.htm) is discarded and recalculated from the parameters and rules of the new cycle. This increases the test time, but produces a slightly more realistic test by simulating the start of a new trade session at every WFO cycle. 
  * While live trading a walk-forward optimized strategy, it is recommended to [re-train](retraining.htm) the last cycle perodically for making the strategy independent on parameter settings. The best time period between retraining is the time of a WFO test cycle.
  * The last WFO cycle (cycle **5** in the figure above) has no test period. It can be a few bars longer than the other cycles to ensure training until the very **EndDate**. If **SelectWFO** is set to **-1** in [Test] mode, the out-of-sample period before the training is tested (**OOS Test** in the figure; rolling WFO only). The OOS test often generates low profit, but can inform about the long-term performance of the strategy when it is not re-trained.
  * Anchored WFO can be used to test the lifetime of parameters or rules. If anchored WFO generates a better result than rolling WFO, the strategy is longlived and does not need to be retrained often. Normally, rolling WFO produces better results, meaning that the market changes and that parameters / rules must be adapted from time to time. 
  * Use the [plotWFOCycle](profile.htm) and [plotWFOProfit](profile.htm) functions for analyzing the profit curve over one or several WFO cycles. 

### Examples:

```c
// anchored WFO, 10 cycles 
function run()
{
  NumWFOCycles = -10;
  DataSplit = 80; // 20% test cycle
  DataSlope = 2;  // more weight to more recent data
  set(PARAMETERS,TESTNOW); // run a test immediately after WFO
  ...
}

// set WFO period and DataSplit from a fixed test and training cycle
function setWFO1(int TestDays,int TrainDays)
{
  WFOPeriod = (TestDays+TrainDays) * 1440/BarPeriod;
  DataSplit = 100*TrainDays/(TestDays+TrainDays);
}

// set NumWFOCycles from a fixed test cycle in days
// (DataSplit and LookBack should be set before)
function setWFO2(int TestDays){  asset(Asset); // requires NumBars, so asset must be set  int TestBars = TestDays * 1440/BarPeriod;  var Split = ifelse(DataSplit > 0,DataSplit/100.,0.85);
  int TrainBars = TestBars * Split/(1.-Split);  int TotalBars = NumBars-LookBack;  NumWFOCycles = (TotalBars-TrainBars)/TestBars + 1;}

// set a fixed date for the WFO test start
// modifies StartDate, needs EndDate and TestDate in YYYYMMDD
function setWFO3(int TestDate)
{
  var Split = ifelse(DataSplit > 0,DataSplit/100.,0.85);
  var TrainDays; 
  if(WFOPeriod > 0)
    TrainDays = Split*WFOPeriod*BarPeriod/1440;
  else if(NumWFOCycles > 1)
    TrainDays = Split/(1.-Split)*(dmy(EndDate)-dmy(TestDate))/(NumWFOCycles-1);
  else return;
  StartDate = ymd(dmy(TestDate)-TrainDays);
}
```

### See also:

[Training](training.htm), [Tutorial](tutorial_fisher.htm), **[DataSplit](dataslope.htm)** , [DataHorizon](dataslope.htm), [NumSampleCycles](numsamplecycles.htm), **[NumTotalCycles](numtotalcycles.htm)** , [](tutorial_fisher.htm) [](numparameters)[NumOptCycles](opt.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
