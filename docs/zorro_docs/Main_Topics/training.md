---
title: Train mode
url: https://zorro-project.com/manual/en/training.htm
category: Main Topics
subcategory: None
related_pages:
- [set, is](mode.htm)
- [optimize](optimize.htm)
- [objective, parameters](objective.htm)
- [Margin, Risk, Lots, ...](lots.htm)
- [TrainMode](opt.htm)
- [Oversampling](numsamplecycles.htm)
- [Virtual Hedging](hedge.htm)
- [loop](loop.htm)
- [Money Management](optimalf.htm)
- [adviseLong, adviseShort](advisor.htm)
- [DataSplit, DataSkip, ...](dataslope.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [Walk-Forward optimization](numwfocycles.htm)
- [Performance Report](performance.htm)
- [Multiple cores](numcores.htm)
- [W7 - Machine Learning](tutorial_pre.htm)
- [Time zones](assetzone.htm)
- [asset, assetList, ...](asset.htm)
- [algo](algo.htm)
- [Export](export.htm)
- [Dates, holidays, market](date.htm)
- [Testing](testing.htm)
- [Live Trading](trading.htm)
- [W5 - Optimizing, WFO](tutorial_fisher.htm)
---

# Train mode

# Training an algo trading system 

Training can modify strategy parameters, generate trading rules, and assign optimal capital allocation factors for profit reinvestment or portfolio distribution. Here's a short [introduction to algo system training](https://zorro-project.com/training.php).

For preparing a parameter based strategy to be trained, set the [PARAMETERS](mode.htm) flag and assign [optimize](optimize.htm) calls to all optimizable parameters. The code for training the SMA time period of a simple moving average crossover system would look like this:

```c
// n-bar Moving Average system (just for example - don't trade this!)
function run() 
{
  set(PARAMETERS);
  var TimePeriod = optimize(100,20,200,10);
  vars Prices = series(price());
  vars Averages = series(SMA(Prices,TimePeriod));
  if(crossOver(Prices,Averages)) 
    enterLong();
  else if(crossUnder(Prices,Averages)) 
    exitLong();
}
```

Now click [Train]. Zorro will now run through several optimization cycles for finding the most robust parameter combination. Zorro optimizes very fast; still, dependent on the number of parameters, assets, and trade algorithms, optimization can take from a few seconds up to several hours for a complex portfolio strategy. The progress bar indicates the time you have to wait until it's finished: 

![](../images/training.png)

The displayed values per line are the loop and parameter number, the optimization step, the parameter value, the result of the **[objective](objective.htm)** function, and the number of winning and losing trades. 

In [Train] mode, trades usually open 1 lot, regardless of [Lots](lots.htm) or [Margin](lots.htm) settings. If the strategy depends on different trade sizes, set the [TRADES](opt.htm) flag. The results are summed up over all [sample cycles](numsamplecycles.htm). [Pool trades](hedge.htm) are not used, and [phantom trades](lots.htm) are treated like normal trades. This behavior can be modified for special cases with [TrainMode](opt.htm) flags. 

The optimal parameters are stored in a ***.par** file in the **Data** folder, and are automatically used by Zorro when testing or trading the system. Parameters are normally optimized and stored separately per asset/algorithm component in a portfolio system; therefore all **optimize** calls must be located inside the asset/algo [loop](loop.htm) if there is any. When the [FACTORS](mode.htm) flag is set, [capital allocation factors](optimalf.htm) are also generated and stored in a ***.fac** file in the **Data** folder. When the strategy contains [advise calls](advisor.htm) and the [RULES](mode.htm) flag is set, decision tree, pattern detection, or perceptron rules are generated and and stored in ***.c** files in the **Data** folder. If external machine learning functions are used, the trained models are stored in ***.ml** files in the **Data** Folder. In [Train] mode, trades usually open 1 lot, and pool and phantom trades are converted to normal trades (unless special [TrainMode](opt.htm) flags are set). Parameters, factors, and rules can be generated either together or independent of each other.

Training shows if the strategy is robust, i.e. insensitive to small parameter changes. Therefore it goes hand in hand with strategy development. When training a strategy in Ascent mode with the [ LOGFILE](mode.htm) flag set, Zorro shows the performance variance over all parameter ranges in parameter charts on a HTML page, like this: 

![](../images/optimize.png)

The charts are stored in the **Log** folder and end with the number of the parameter, f.i. **_p1** for the first, **_p2** for the second parameter and so on. The red bars are the result of the [objective](objective.htm) function, which is by default the profit factor of the training period, corrected by a factor for preferring a higher number of trades. The grey bars show the number of losing trades and the black bars show the number of winning trades. In walk forward optimization, the parameter charts display the average results over all WFO cycles. Genetic or brute force optimization does not generate parameter charts, but contour plots from the first two parameters as shown below.

![](../images/contour.png)

Training is critical for almost all strategies, and should not be omitted even when a strategy appears to be already profitable with its default parameters. Most strategies only work within certain parameter ranges. Those ranges are usually different for every asset and also vary with the market situation - a bullish or bearish market requires different parameters than a market in a sidewards phase. Due to the asymmetry of many markets, long and short trades often also require different parameters. Naturally, parameters can only be optimized to a specific historical price curve, therefore they perform best with that particular price curve and will perform not as good with any other data - such as real time prices in live trading. This effect is called overfitting. It varies with the strategy, and is discussed below. 

Select carefully the parameters that you train. Parameters that depend on external events - for instance, the market opening hour when the strategy exploits the overnight gap - must never be trained, even if training improves the test result. Likewise, parameters that should have no effect on strategy performance, such as price patterns from long ago or the start date of a backtest, must never be trained. Otherwise you're in danger to replace a real market inefficiency by a random training effect. 

For avoiding overfitting bias when testing a strategy, parameter values are typically optimized on a training data set (the "in-sample" data), and then tested on a different data set (the "out-of-sample" or "unseen" data) to simulate the process of trading the system. If the optimized parameters do not perform well on out-of-sample data, the strategy is not suited for trading. Zorro offers several methods for optimizing and testing a strategy with unseen data:

### Horizontal split optimization 

This method divides the price data into one-week segments (a different segment length can be set up through [DataSkip](dataslope.htm)). Usually two of every three weeks are then used for the optimization, and every third week is skipped by the optimization and used for the strategy test. The [LookBack](lookback.htm) period is the part of the price curve required for functions that use past price data as input, such as moving averages or frequency filters. Thus a preceding lookback period is always cut off in front of the data for both training and testing. 

**Simulation period**  
---  
**LookBack** |  **Training** |  |  |  |  |  |  |  |  |   
**Test** |  |  |  |  |  |  |  |  |   
  
  
For setting up this method, set different [SKIP1](mode.htm), [SKIP2](mode.htm), [SKIP3](mode.htm) flags for training and testing. Usually, [SKIP3](mode.htm) is set in train mode and and [SKIP1](mode.htm)+[SKIP2](mode.htm) in test mode; any other combination will do as well as long as two weeks are used for training and one week for testing. A script setup for horizontal split optimization looks like this:

```c
function run() // horizontal split optimization
{
  if(Train) set(PARAMETERS+SKIP3);
  if(Test) set(PARAMETERS+SKIP1+SKIP2);
  ...
}
```

Horizontal split optimization is good for quick tests while developing the strategy, or for testing when you don't have enough price data or enough trades for walk-forward optimization. It is the fastest method, uses the largest possible data set for optimization and test, and gives usually quite realistic test results. For checking the consistency you can test and train also with different split combinations, f.i. **SKIP1** for training and **SKIP2+SKIP3** for testing. The disadvantage is that the test, although it uses different price data, runs through the same market situation as the training, which can make the result too optimistic. 

### Vertical split optimization 

Alternatively, you can select only a part of the data period - such as 75% - for training. The rest of the data is then used exclusively for testing. This separates the test completely from the training period. Again, a lookback period is split off for functions that need past price data. 

**Simulation period**  
---  
**LookBack** |  **Training** |  **Test**  
|  |  |  |  |   
  
For setting up this method, use **[DataSplit](dataslope.htm)** to set up the length of the training period. A script setup for vertical split optimization looks like this:

```c
function run() // vertical split optimization
{
  set(PARAMETERS);
  DataSplit = 75; // 75% training, 25% test period
  ...
}
```

Vertical split optimization has the advantage that the test happens after the training period, just as in live trading. The disadvantage is that this restricts testing to a small time window, which makes the test quite inaccurate and subject to fluctuations. It also does not use the full simulation period for optimization, which reduces the quality of the parameters and rules. 

### Walk Forward Optimization

For combining the advantages of the two previous methods, an analysis and optimization algorithm named **Walk Forward Optimization** (**WFO** in short) can be used. This method is a variant of cross-validation and was first suggested for trading strategies by Robert Pardo. It trains and tests the strategy in several cycles using a data frame that "walks" over the simulation period: 

**WFOCycle** |  **Simulation period**  
---|---  
**1** |  **LookBack** |  **Training** |  **Test1** |  |  |   
**2** |  |  **LookBack** |  **Training** |  **Test2** |  |   
**3** |  |  |  **LookBack** |  **Training** |  **Test3** |   
**4** |  |  |  |  **LookBack** |  **Training** |  **Test4**  
**5** |  |  |  |  |  **LookBack** |  **Training**  
**OOS Test** |  **LookBack** |  **Test5** |  |  |   
**WFO Test** |  |  |  |  **Look** |  **Back** |  **Test1** |  **Test2** |  **Test3** |  **Test4**  
  
Rolling Walk Forward Optimization   

**WFOCycle** |  **Simulation period**  
---|---  
**1** |  **LookBack** |  **Training** |  **Test1** |  |  |   
**2** |  **LookBack** |  **Training** |  **Test2** |  |   
**3** |  **LookBack** |  **Training** |  **Test3** |   
**4** |  **LookBack** |  **Training** |  **Test4**  
**5** |  **LookBack** |  **Training**  
**Test** |  |  |  |  |  |  **Test1** |  **Test2** |  **Test3** |  **Test4**  
  
Anchored Walk Forward Optimization   

The parameters and rules are stored separately for every cycle in files ending with **"_n.par"** or **"_n.c"** , where **n** is the number of the cycle. This way the strategy can be tested repeatedly without having to generate all the values again. The test cycle is divided into **(n-1)** test periods that use the parameters and factors generated in the preceding cycle. The last training cycle has no test period, but can be used to generate parameters and rules for real trading. 

For setting up this method, use **[DataSplit](dataslope.htm)** to set up the length of the training period in percent, and set either [NumWFOCycles](numwfocycles.htm) to the number of cycles, or [WFOPeriod](numwfocycles.htm) to the length of one cycle. A script setup for Walk Forward Optimization looks like this:

```c
function run() // walk forward optimization
{
  set(PARAMETERS);
  DataSplit = 90; // 90% training, 10% test
  NumWFOCycles = 10; // 10 cylces, 9 test periods
  ...
}
```

The WFO test splits the test period into several segments according to the WFO cycles, and tests every segment separately. The test result in the [performance report](performance.htm) is the sum of all segments. The parameters from the last cycle are not included in the WFO test as they are only used for real trading. A special OOS test can be used for out-of-sample testing those parameters with price data from before the training period. 

WFO can be used to generate a 'parameter-free' strategy - a strategy that does not rely on an externally optimized parameter set, but optimizes itself in regular intervals while trading. This makes optimization a part of the strategy. WFO simulates this method and thus tests not only the strategy, but also the quality of the optimization process. Overfitted strategies - see below - will show bad WFO performance, profitable and robust strategies will show good WFO performance. The disadvantage of WFO is that it's slow because it normally runs over many cycles. Like vertical split optimization, it uses only a part of the data for optimizing parameters and factors. Depending on the strategy, this causes sometimes worse, sometimes better results than using the full data range. Still, WFO is the best method for generating robust and market independent strategies.

Another advantage of WFO is that training is a lot faster than the other methods when using [several CPU cores](numcores.htm). 

### Combining rules and parameters

If a machine learning strategy also requires training of parameters, several [training cycles](opt.htm) in a certain order are required. There are several possible scenarios: 

  * Parameters depend on rules. This is the usual case; an example would be the [Workshop 7](tutorial_pre.htm) system with additionally optimizing the **Stop** distance after rules have been generated. This requires two training runs. In the first run, set only the [RULES](mode.htm) flag for generating the rules with the default values for the parameters. In the second training run, set only the [PARAMETERS](mode.htm) flag. The parameters are then optimized using the rules from the first run. In [Test] and [Trade] mode set both [RULES](mode.htm) and [PARAMETERS](mode.htm) so that the generated rules and the trained parameters are used. Example:  

```c
NumTrainCycles = 2;
if(TrainCycle == 0) set(RULES,PARAMETERS); // Test or Trade mode
else if(TrainCycle == 2) set(PARAMETERS);
else if(TrainCycle == 1) set(RULES);
```

  * Rules depend on parameters. An example is a machine learning system with a neural network, where the parameter is the number of neurons in the central layer. In [Train] mode set both [ RULES](mode.htm) and [PARAMETERS](mode.htm) flags. [NumTrainCycles](opt.htm) needs not be set. The training run will now automatically repeat through many cycles. For every parameter step a new set of rules will be generated. At the end of the process, the rules with the most robust performance are stored in the **Data** folder. In [Test] and [Trade] mode set the **RULES** flag only, not **PARAMETERS** , as the parameter only affects rule generation and has no further relevance for the script. Example:

```c
if(Train) set(RULES,PARAMETERS);
else set(RULES);
```

  * Rules and parameters affect each other. An example would be a candle pattern trading system similar to [Workshop 7](tutorial_pre.htm), but with additionally optimizing the [time zone](assetzone.htm) for finding the best bar start time for the selected asset. Set both [ RULES](mode.htm) and [PARAMETERS](mode.htm) flags in test and training. The training run will now repeat through many cycles. For every parameter step a new set of rules will be generated. The rules with the most robust performance are then selected. Using those rules, the parameters will be optimized again so that the training run ends up with the best combination of parameters and rules. Example:

```c
set(RULES,PARAMETERS);
```

Training rules and parameters at the same time only works with single assets, not with a portfolio system that contains [loops](loop.htm) with [asset](asset.htm) or [algo](algo.htm) calls. If required, train all portfolio components separately and combine the resulting **.c** files. The **Combine.c** script can be used as a template for automatically combining parameters and rules from different assets or algos. Machine learning models (**.ml** files) can be combined with an appropriate R function. With Zorro S, the process can be automatized with a batch file. 

### Portfolio training

When training a portfolio of assets and algorithms, it is normally desired to train any asset/algo component separately, since their parameters and rules can be very different. Only in special cases it could make sense to train parameters that are common to all components, or use a combination of commonly and separately trained parameters. Examples for all 3 cases can be found under [loop](loop.htm).

### Training results 

Before training a system the first time, make sure to run a [Test] with the raw, unoptimized script. Examine the log and check it for error messages, such as skipped trades or wrong parameters. You can this way identify and fix script problems before training the script. Training won't produce error messages - trades are just not executed when trade parameters are wrong.

Money management or profit reinvesting should not be active for training strategy parameters, because it would introduce artifacts from the different weights of trades. For this reason training uses fixed-size trades by default. If you want to train a money management algorithm or use different lot sizes, do that in a separate step using the [TRADES](opt.htm) flag after all other parameters have been trained.

After training, files are generated which contain the training results and the generated rules, parameters, and factors (see [Export](export.htm)). A Zorro strategy can thus consist of up to 4 files that determine its trading behavior:

  * The strategy script. It contains the basic trade rules, is written by the user, and stored in the **Strategy** folder. It either has the file name***.c** (the * stands for the strategy name) or ***.x** , dependent on if it's in lite-C or compiled to executable machine language. Compiled scripts start faster, and can be used to hide the underlying souce code (the included Z strategies are compiled). The strategy script is the only file that is really required for a trade strategy, and 90% of this documentation is for learning how to write it. The 3 other files are optional.
  * The strategy parameters. They are generated by [optimize](optimize.htm) calls when training a strategy. They are stored in **Data\\*.par**. This file contains the parameters in the order they appear in the script. If the strategy does set up the [asset](asset.htm) itself, training generates different parameter files dependent on the asset selected in the [Asset] scrollbox.
  * The [OptimalF](optimalf.htm) capital allocation factors, either one file per WFO cycle or a single file for the whole simulation. Capital allocation factors tell Zorro how to distribute the capital among the trades, dependent on the assets and trade rules. They are automatically generated in a training run, but can also be manually edited by the user. They are stored in **Data\\*.fac**. 
  * The trade rules and models, generated by the [advise](advisor.htm) function. Rules are small scripts that contain lite-C functions, stored in **Data\\*.c**. Models generated by external machine learning algorithms are stored in **Data\\*.ml**.

All 4 files, except for the executable ***.x** and the ***.ml** machine learning models, are normal text files and can be edited with any plain text editor. 

### Overfitting 

The optimizing process naturally selects parameters that fare best with the used price data. Therefore, an optimized strategy is always more or less 'curve fitted' to the used price curve, even with Zorro's range-based optimizing. Neural networks, genetic algorithms, or brute force optimization methods tend to greatly overfit strategies. They produce **panda bears** that are perfectly adapted to a specific data set, but won't survive anywhere else. Some suggestions to reduce overfitting:

  * Keep strategies simple and don't use too many parameters, filters, and extra conditions. A strategy should not use more than 3 optimized parameters for entering trades and another 3 for exiting them. Too many parameters increase the likeliness of overfitting, and will cause worse results in WFO and in real trading.  

  * While developing the strategy, check out which parameters are best candidates for optimization. Optimize only those that largely afffect the strategy performance. Check out the parameter charts. They give valuable information about the effect of a parameters. Don't use a parameter that does not generate a 'broad hill', but instead narrow peaks, single lines, or a chaotic parameter performance chart.   

  * Keep the [optimize](optimize.htm) ranges small, as large ranges tend to produce 'local maxima' and generate an overfitted strategy. For the same reason, do not use too small [optimize](optimize.htm) steps. Dependent on the range and importance of the parameter, don't select steps smaller than 5% to 10% of the parameter range.  

  * Optimize entry parameters first and exit parameters afterwards. When your exit system is complex, optimize entry parameters with a simplified exit such as a simple stop. For using a simple exit while the entry parameters are optimized, evaluate the current parameter number (**[ParCycle](opt.htm)**), f.i. **if(Train && ParCycle <= NumParameters-3) setExitSimple();**.  

  * Do not optimize parameters more than twice, i.e. **[NumTrainCycles](opt.htm)** should not exceed **2** when the same paramerers are optimized in both cycles. Higher values increase the danger of overfitting.  

  * Get enough trades for optimizing. The more trades are simulated, the better is the quality of the parameter and rule set. Set [NumYears](date.htm) accordingly, and don't use too many WFO cycles with too small optimization periods. As a rule of thumb, you should have at least 20 trades per parameter and asset. Use [oversampling](numsamplecycles.htm) for generating more trades from different price curves with resampled bars.  

  * Always use different price data for [Train] and [Test] mode by using WFO or setting [DataSplit](dataslope.htm) or [SKIP1](mode.htm), [SKIP2](mode.htm), [SKIP3](mode.htm) accordingly.

### See also:

[Testing](testing.htm), [Trading](trading.htm), [Result](performance.htm), [WFO](numwfocycles.htm), [DataSplit](dataslope.htm), [optimize](optimize.htm), [Workshop5](tutorial_fisher.htm), [Workshop 7](tutorial_pre.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
