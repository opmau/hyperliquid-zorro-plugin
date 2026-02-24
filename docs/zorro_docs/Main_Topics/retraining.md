---
title: Retraining, retesting
url: https://zorro-project.com/manual/en/retraining.htm
category: Main Topics
subcategory: None
related_pages:
- [Licenses](restrictions.htm)
- [Dates, holidays, market](date.htm)
- [Training](training.htm)
- [Status flags](is.htm)
- [W5 - Optimizing, WFO](tutorial_fisher.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [Asset Symbols](symbol.htm)
- [Historical Data](history.htm)
- [Performance Report](performance.htm)
- [Asset and Account Lists](account.htm)
- [TickTime, MaxRequests, ...](ticktime.htm)
- [Spectral Analysis](filter.htm)
- [IB / TWS Bridge](ib.htm)
- [Oanda](oanda.htm)
- [Bars and Candles](bars.htm)
- [set, is](mode.htm)
- [Testing](testing.htm)
- [Command Line Options](command.htm)
---

# Retraining, retesting

# Retraining and Retesting (Zorro S)

A live trading strategy should be retrained in regular intervals for adapting its parameters to the current market situation. It should also be retested after a certain trading time for verifying that the script behaves identically in live trading and in the backtest. Retraining and retesting are [Zorro S](restrictions.htm) features and are executed by a separate process, this way preventing any interruption of the live trading process. The newly trained parameters, rules, or machine learning models are then automatically loaded by the trading system at the begin of the next bar. 

### Retraining

The retraining process is triggered either automatically by setting the [ReTrainDays](date.htm) variable, or manually by clicking the [Train] button while trading. A second Zorro instance will pop up and start a [training run](training.htm), while the first Zorro continues trading. In this training run the **[ReTrain](is.htm)** boolean variable is nonzero. The script can then download new price data from the server, and generate a new parameter and AI rule set (see also [workshop 5](tutorial_fisher.htm)). After finishing the training run, the second Zorro closes. The new parameters, rules and factors are then automatically loaded by the trading instance. Example of a retraining statement in the script:

```c
if(ReTrain) { 
  EndDate = 0;     // set the end date to the current date
  UpdateDays = -1; // reload new price data from the server
  SelectWFO = -1;  // select the last cycle for re-optimization
  reset(FACTORS);  // don't re-train factors
}
```

If the trading system is connected to a broker that does not provide enough price history (such as [MT4](mt4plugin.htm)), the recent price history can be downloaded from a different source with a specific [history symbol](symbol.htm) or by another Zorro instance with the [Download](history.htm) script. 

A trading system can be retrained even without a **ReTrain** statement in the script. In that case just download price history and train the system with another Zorro instance. Training produces new **.c** , **.par** , **.fac** , or **.ml** files in the **Data** folder. You can train on a different PC and then copy the files into the **Data** folder of the trading system. Zorro will detect that the files were updated, and load them automatically at the end of the current bar. 

Re-training a WFO system is required in intervals corresponding to the length of a WFO test cycle, which is displayed in the [performance report](performance.htm). The system normally needs not to be retrained more often, with one exception. If live trading results of a certain component or asset are exceptionally bad, the market could have been changed - for instance it could have got a different dominant cycle. In that case an extraordinary training run can be useful for getting the strategy back on track. On the other hand, re-training continues open trades and running functions with different parameters, which can cause a slight reduction in profit. So don't re-train too often. 

### Retesting

The retesting process is triggered by clicking the [Test] button while trading. Its purpose is verifying whether the script behaves in the simulation exactly as in live trading. For this, Zorro downloads historical data and then performs a backtest over the period since the begin of trading. The log, profits, and trades from the backtest can then be compared to the live trading session. 

For retesting the trading session, a second Zorro instance will perform the actual test while the first Zorro continues trading. The start date of the trading session and the bar offset are transferred to the second instance via command line. In the test run the **[ReTest](is.htm)** boolean variable is nonzero and can be used to set up test parameters in the script. After finishing the test, the second Zorro closes. The trade list ***_trd.csv** can then be compared to **trades.csv**. Example of a retesting statement in the script:

```c
if(ReTest) { 
  StartDate = Command[0]; // get start date and bar offset from the trading Zorro via command line
  BarOffset = Command[1];
  EndDate = 0;     // set the end date to the current date
  UpdateDays = -1; // reload new price data from the server
  NumSampleCycles = 0; // no sample cycles
  SelectWFO = -1;  // use parameters from the last WFO cycle
}
```

Retest a system when at least about 50 trades have been entered; retests with few trades make not much sense. Make sure that the test runs with exactly the same parameters - bar offset, trade volume, etc. - as live trading. For retesting accurately, the price data must come from the same source as in live trading, must have T1 resolution, and the [asset list](account.htm) must reflect the actual trading account parameters. The following effects cause to differences between test and trading results:

  * Strategy sensitiveness to small differences between historical prices and live prices or volumes. Even when from the same source, historical prices often differ from live prices due to outlier filtering, gap filling, differences between PC time and server time, and Internet latency. Likewise, live volume can be different to historical volume where delayed transactions, busted trades, combos and unreportable trades are filtered out. Price differences can cause trades to enter or exit at a different bar. Depending on the sensitivity of the strategy to small price differences, the test result in those cases can be very different to the live trading result.
  * Strategy sensitiveness to the tick frequency. In the backtest, the tick frequency is determined by the historical data (usually 1 minute with M1 data), in live trading the ticks come in irregular intervals with a minimum distance set up by [TickTime](ticktime.htm).
  * Strategy sensitiveness to small indicator value differences. Recursive indicators, such as [LowPass](filter.htm), [HighPass](filter.htm), [DominantPeriod](filter.htm) etc., are influenced by all past bars and therefore can have slightly different values between simulation and live trading at the begin of the trading session. The differences are normally negligible, but can in extreme cases with short lookback periods affect entry and exit conditions of the first trades.
  * Different start parameters, f.i. slider settings. For comparable results, sliders and other strategy-affecting parameters must have the same values in trading and test. Sliders must not have been moved during the trading session. 
  * Different asset parameters. For comparable results, all [asset data](account.htm) \- such as spread, slippage, rollover, account currency rate - must be from the same broker account. But even then, asset data such as spread, pip cost, or margin cost varies in live trading, but has normally constant values in the test. This will cause profit differences between test and live trading.
  * Retraining the strategy during the trading session. Retraining invalidates further retesting because it modifies the parameters or rules. So you can either retrain or retest a strategy, but not retest after retraining. 

### Remarks

  * Retraining and retesting use the [command line](coommand.htm) for passing parameters and script name to the training or test process. The script name must be command line compliant. 
  * The retrain and retest processes need recent price history. It can be automatically downloaded per [UpdateDays](date.htm) setting. When the broker API does not provide price history (as with [MT4](mt4plugin.htm) or [IB](ib.htm)) or when the account only supports a single connection (as with [Oanda](oanda.htm)), download recent price history from another source with the [Download](history.htm) script prior to retraining or retesting.
  * Retesting or retraining large portfolio strategies, such as the Z systems, can require a large amount of free RAM (~4 GB) and thus might not work on a low-end VPS. 

### See also:

[bar](bars.htm), [mode](mode.htm), [Testing](testing.htm), [Training](training.htm), [ReTrainDays](date.htm), [command line](command.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
