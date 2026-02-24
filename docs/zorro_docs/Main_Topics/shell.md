---
title: Evaluation Sherll
url: https://zorro-project.com/manual/en/shell.htm
category: Main Topics
subcategory: None
related_pages:
- [Oversampling](numsamplecycles.htm)
- [Licenses](restrictions.htm)
- [Links & Books](links.htm)
- [Walk-Forward optimization](numwfocycles.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [DataSplit, DataSkip, ...](dataslope.htm)
- [TrainMode](opt.htm)
- [Performance Report](performance.htm)
- [Money Management](optimalf.htm)
- [Equity Curve Trading](trademode.htm)
- [Error Messages](errors.htm)
- [Chart Viewer / Debugger](chart.htm)
- [Structs](structs.htm)
- [optimize](optimize.htm)
- [loop](loop.htm)
- [Export](export.htm)
- [Log Messages](log.htm)
- [Getting Started](started.htm)
- [Asset and Account Lists](account.htm)
---

# Evaluation Sherll

# The Evaluation Shell (Zorro S)

The strategy evaluation shell is an addendum to the [Zorro project](https://zorro-project.com). Its purpose is determining the ideal parameters and functions for a given trading strategy, generating a portfolio of optimal asset, algo, and timeframe combinations, and predicting its live perfomance under different market situations. The Zorro evaluation shell can make the best of a strategy, especially of 'indicator soup' strategies that combine many different indicators and analysis functions.

A robust trading strategy has to meet several criteria:

  * It must exploit a real and significant market inefficiency. Random-walk markets cannot be algo traded.
  * It must work in all market situations. A trend follower must survive a mean reverting regime.
  * It must work under many different optimization settings and parameter ranges.
  * It must be unaffected by random events and price fluctuations.

The shell evaluates parameter combinations by these criteria. The robustness under different market situations is determined through the **R2 coefficient** , the parameter range robustness with a WFO profile (aka **Cluster Analysis**), the price fluctuation robustness with [oversampling](numsamplecycles.htm). A **Montecarlo analysis** finds out whether the strategy is based on a real market inefficiency. More about WFO profiles and Montecarlo analysis can be found on [ financial-hacker.com/why-90-of-backtests-fail](https://financial-hacker.com/why-90-of-backtests-fail), more about oversampling on [ financial-hacker.com/better-tests-with-oversampling](https://financial-hacker.com/better-tests-with-oversampling).

Zorro already offers functions for all these tests, but they require a large part of code in the strategy, more than for the algorithm itself. The evaluation shell skips the coding part. It can be simply attached to a strategy script. It makes all strategy variables accessible in a panel, adds optimization and money management as well as support for multiple assets and algos, runs automated analysis processes, and builds the optimal portfolio of strategy variants. It compiles unmodified under lite-C and under C++, and supports **.c** as well as **.cpp** scripts.

With almost 1500 lines, the shell is probably the largest Zorro script so far, and goes far beyond other strategy evaluation software on the market. It comes with source code, so it can be easily modified and adapted to special needs. It is restricted to **personal use** ; any commercial use or redistribution, also partially, requires explicit permission by oP group Germany. Since it creates a panel and a menu, it needs a [Zorro S](restrictions.htm) license to run. Theoretically you could remove the panel and menu functions and use the shell with the free Zorro version. This is allowed by the license, but would require clumsy workarounds, like calling functions by script and manually editing CSV files.

### Overview

Developing a successful strategy is a many-step process, described in the [Black Book](links.htm) and briefly in an article series on [ Financial Hacker](https://financial-hacker.com/build-better-strategies-part-3-the-development-process/). The evaluation shell cannot replace research and model selection. But it takes over when a first, raw version of the strategy is ready. At that stage you're usually experimenting with different functions for market detection and generating trading signals. It is difficult to find out which indicator or filter works best, since they are usually interdependent. Market detector A may work best with asset B and lowpass filter C on time frame D, but this may be the other way around with asset E. It is very time consuming to try out all combinations. 

The evaluation shell solves that task with a semi-automated process. 

**  
  
Creeate  
Jobs** | **  
  
![](../images/arrow.gif)** | **  
  
****Generate  
Summary****  
** | **  
  
![](../images/arrow.gif)  
  
** | **  
  
Cluster  
Analysis** | **  
  
![](../images/arrow.gif)  
  
** | **  
  
Correlation  
Analysis** | **  
  
![](../images/arrow.gif)  
  
** | **  
  
Montecarlo  
Analysis** | **  
  
![](../images/arrow.gif)  
  
** | **  
  
Final  
Portfolio**  
---|---|---|---|---|---|---|---|---|---|---  
  
The first step is generating sets of parameter settings, named jobs. Any job is a variant of the strategy that you want to test and possibly include in the final portfolio. You can have parameters that select betwen different market detection algorithms, and others that select between different lowpass filters. The parameters are edited in the variables panel, then saved with a mouse click as a job. Instead of entering values in the panel, you can also edit the jobs directly with a spreadsheet program or a text editor. 

The next step is an automated process that runs through all jobs, trains and tests any of them with different asset, algo, and time frame combinations, and stores their results in a summary. The summary is a CSV list with the main performance parameters of all jobs. It is sorted by performance: The best performing jobs are at the top. So you can see at a glance which parameter combinations work with which assets and time frames, and which are not worth to examine further. You can repeat this step with different global settings, such as bar period or optimization method, and generate multiple summaries in this way. 

The next step in the process is cluster analysis. Every job in a selected summary is optimized multiple times with different [ WFO](numwfocycles.htm) settings. These settings are taken from - you guessed it - a separate CSV file that may contain a regular WFO matrix, a list of irregular cycles/datasplit combinations, or both. For reducing the process time, only profitable jobs with rising equity curves - determined by a nonzero **R2 coefficent** \- get a cluster analysis. You can also further exclude jobs by removing or outcommenting them in the summary. 

After this process, you likely ended up with a couple survivors in the top of the summary. The surviving jobs have all a positive return, a steady rising equity curve, shallow drawdowns, and robust parameter ranges since they passed the cluster analysis. But not all of them are suited for the final portfolio. The purpose of a strategy portfolio is diversifiction, but this would not work with a set of variants that are all tightly correlated and have their drawdowns all at the same time. You want a balanced portfolio with uncorrelated algorithms. Automatically reducing the portfolio to the combinations with the smallest correlation is planned for a future shell version, but at the moment it involves a manual selection. Check the equity curves and keep only the best of all variants with similar curves or the same assets, algos, and time frames. They can now be combined with a mouse click to the final balanced portfolio. 

But you're not finished yet. Any selection process generates selection bias. Your perfect portfolio will likely produce a great backtest, but will it perform equally well in live trading? To find out, run a Montecarlo analysis. This is the most important test of all, since it can determine whether your strategy exploits a real market inefficiency. If the Montecarlo analysis fails with the final portfolio, it will likely also fail with any other parameter combination, so you need to run it only at the end. If your system passes Montecarlo with a p-value below 5%, you can be relatively confident that the system will return good and steady profit in live trading. Otherwise, back to the drawing board.   

The following sections cover the various operating modes of the shell. For a quick example of an evaluation process, scroll down to the  tutorial.  

### Step 1: Attaching the shell

You can attach the shell to an existing script in several ways. For the full process it must take control over all variables and replace its asset/algo loops But for adding cluster analysis, Montecarlo analysis, oversampling, various walk-forward optimization methods, and different training objectives, you only need to insert two lines. Include **eval.h** at the begin of the script:

```c
#include <eval.h>
```

... and **eval.c** at the end:

```c
#include <eval.c>
```

You can find a script example with attached shell in **Workshop6a**. When you start it, the [Result] button will change to [Start], a new menu will be available under [Action], and a panel with many parameter settings will pop up: 

![](../images/shelldefault.png)

The panel and menu are the reason why a Zorro S license is required, since panel functions are not supported by the free version. The variables in the panel are mostly self explaining:

**_Bar_Period** | The basic [bar period](barperiod.htm) from which the time frames are derived.  
---|---  
**_Lookback_Bars** | The [lookback](lookback.htm) period in bar units.  
**_WFO_Cycles** | Number of [WFO cycles](numwfocycles.htm) in the backtest, not counting the last cycle for live trading. Rule of thumb: Use 1-2 cycles per backtest year.  
**_WFO_OOS** | [Out-of-Sample period](dataslope.htm) for WFO, in percent.  
**_Oversampling** | Number of [oversampling](numsamplecycles.htm) cycles with differently sampled price curves in the backtest.  
**_Backtest_Mode** | **-1** for a 'naive' backtest with no trading costs, **1** for a realistic bar based backtest, **2** for a minute based backtest, **3** for backtest with tick-based history. Historical data in the required resolution must be available.  
**_Backtest_Start** | Start year or date of the backtest. Use at least 5, better 10 years for the test period.  
**_Backtest_End** | End year or date of the backtest.  
**_Train_Mode** | **0** Script default**, 1** Ascent (fastest), **2** Genetic, **3** Brute Force (see [TrainMode](opt.htm)). Results of genetic optimization can differ on any training due to random mutations.  
**_Objective** | Training objective, aka 'fitness function'. **0** PRR (default), **1** R2*PRR, **2** Profit/DD, **3** Profit/Investment, **4** Profit/day, **5** Profit/trade. For the metrics, see [performance](performance.htm).  
**_Criteria** | The field number by which the summary is sorted and the profiles are generated. **0** no sorting, **7** Net Profit, **13** Win rate, **14** Objective, **15** Profit factor, **16** Profit per trade, **17** Annual return (default), **18** CAGR, **19** Calmar ratio, **20** Sharpe, **21** R2 coefficient. See [performance](performance.htm) for details about the metrics.  
**_MRC_Cycles** | Number of cycles for the Montecarlo 'Reality Check' analysis (default **200**). Results differ from run to run due to randomizing the price curve, therefore more cycles produce more precise results.   
**_Max_Threads** | Number of CPU cores to use for WFO (**0** = single core), or number not to use when negative (f.i. **-2** = all cores but 2). [Zorro S](restrictions.htm) required. Using multiple cores is recommended for faster training.  
**_Investment** | Method for calculating the trade size. **0** Script default, **1** Lots slider, **2** Margin slider, **3** Margin * OptimalF, **4** Percent of account balance, **5** Percent * square root reinvestment. At **3** or above, [OptimalF](optimalf.htm) factors are generated and affect the margin. **4** and above reinvest profits and should only be used in the last stages of development.  
**_Phantom** | Activate [equity curve trading](trademode.htm) by setting the time period between 10..50. A falling equity curve is detected and trading is suspended until the market turns profitable again.  
**_Verbose** | Set to **0** for no logs and reports, **1..3** for more verbose logs, **17** for halting on any error. The more verbose, the slower the test. Critical error messages are collected in **Log\Errors.txt**. Performance summary reports are always generated, regardless of this setting.  
  
The settings have only effect when your script does not set those parameters itself. If it does, the script will override all settings in the panel, except for the **_WFO_Cycles** and **_WFO_OOS** values. They override the script, since they are used for the cluster analysis. 

The **Fixed** column can be toggled on or off. On optimizing or loading jobs - more about that below - fixed variables keep the values set in the panel. By default, the above variables are all fixed and the strategy variables are not. 

The **Min** and **Max** columns display the valid ranges of the variables. Edited values in the panel are remembered at the next start. Changes are indicated with a white background. Exceeding the valid range is allowed, but indicated with a red background.

The default values of some variables can be set up in the strategy script by **#define** statements. Example:

```c
#define _BAR_PERIOD     240
#define _LOOKBACK_BARS  4000
#define _WFO_CYCLES     16
#define _BACKTEST_START 201501011
#define _BACKTEST_END   20251231
```

  

### Action menu

After starting the script in [Train] mode, a click on [Action] opens a dropdown menu. It contains only a few items at first: 

Reset | Reset all variables to their defaults, and remove all loaded jobs and algos.  
---|---  
Run Cluster Analysis | Run a WFO cluster analysis, with WFO offsets and OOS periods defined in a CSV file. Several files with various combinations of offsets and periods are included. For any WFO cycles / OOS period combination, the resulting performance metrics are stored in a **Cluster summary**. For files containing a regular NxM cluster matrix, the returns are displayed in a heatmap as shown below, otherwise in a WFO profile.  
Run Montecarlo Analysis | Run a Montecarlo 'Reality Check' with the current parameters and shuffled price curves. A reality check can tell whether the WFO performance was just luck, or was caused by a real market inefficiency. The resulting performance metrics are stored in a **MRC summary** , and the results are displayed in a histogram together with the original result with unmodified price curve. Depending on the resulting **p-value** , the backtest performance is qualified as significant, possibly significant, or insignificant.  
Download History | Open the Zorro Download Page for getting historical data.   
  
When nothing is selected, clicking on [Start] will just start a test or training run with the current variable settings in the panel (if not overridden by the script). After a successful backtest, the [performance report](performance.htm) will pop up with the price curve, the trades, and the equity curve. 

When you select Cluster Analysis or Montecarlo analysis, a click on [Start] will run the whole process with the current settings. Dependent on the selected template, Cluster analysis will generate either a WFO profile or a heatmap like this:

![](../images/RangerMatrix.png)

The x axis is the number of WFO cycles, the y axis is the OOS period in percent. The numbers in the fields are the resulting strategy performance, selected by **_Criteria**. You want as much red in the heatmap as possible. The above example, with the Pessimistic Return Ratio (PRR) as metric, has only 13 red fields out of 25. This job would not pass the CA analysis.

Montecarlo Analysis, aka 'Reality Check', is normally only applied to the final strategy with all assets and algorithms. It generates a histogram from results with randomized price curves (red) and the original price curve (black). This one below (from the 'placebo system' in the FH article) indicates that you better don't trade that system live, even though it produced a great walk-forward analyis:

![](../images/MRC_placebo.png)

Another one from a real stragegy that generated less performance in the walk-forward analysis, but exploits a real market inefficiency:

![](../images/RealityCheck_s1.png)

Train mode will always use walk forward optimization, even when none is defined in the script. Test mode will only run a backtest; so the strategy should have been trained before. After changing any variable, train again. If a variable range was exceeded or historical data was missing for the selected backtest period, you'll get an [error message](errors.htm). Training errors will store the error message in **Log\Errors.txt** and abort.  

### Step 2: Creating jobs

The next step of evalution is letting the shell control your script variables. This allows you to experiment with many different parameter settings and optimization ranges, and testing them all in an automated process. Any such variant is a 'job'. As a side benefit, putting script variables in a panel also allows you to observe their bar-to-bar behavior with the [Visual Debugger](chart.htm).

Instead of **eval.h** , now include **evars.h** at the begin of the script:

```c
#include <evars.h>
```

Below the **#include** statement, put all relevant script variables that you want to observe, optimize, or experiment with in a list. Add comments and put an **END_OF_VARS** statement at the end of the list. Example (from **Workshop6b**):

```c
var CNTR_BPPeriod; //= 30, 20..40, 5; Bandpass period in bars
var CNTR_BPWidth; //= 0.5, 0.2..1; Bandpass width
var CNTR_FisherPeriod; //=500, 300..700; Fisher transform period
var CNTR_Threshold; //= 1,0.5..1.5,0.1; Bandpass threshold
var CNTR_Fisher; //= 500,300..500; Time frame of Fisher transform
var CNTR_TrailDist; //=4, 0..10; Distance factor for trailing
var TRND_Mode; //= 0, 0..1; 0 - Lowpass, 1 - Butterworth filter
var TRND_Detect; //= 0, 0..1; 0 - MMI, 1 - Hurst
var TRND_LPPeriod; //= 300,100..700,50; Lowpass/Butterworth period in bars
var TRND_MMIPeriod; //= 300,100..500; MMI/Hurst Filter time period
var StopDist; //= 10,5..30,5; Stop distance in ATR units
var ATRPeriod; //=100, 30..150; ATR period for stop/trail distances
END_OF_VARS
```

This encapsulates the variables in a [C struct](structs.htm) named **V**. Thus, in the script the variables are accessed with a preceding **V.** , for instance **V.ATRPeriod**. They all must be of type **var**. It you need an integer variable, as in a switch/case statement, use an **(int)** typecast. Replace all your [optimize](optimize.htm) calls with **_optimize** and pass the variable as sole parameter. Example:

```c
Stop = _optimize(V.StopDist) * ATR(V.ATRPeriod);
```

The startup panel will now look like this (**Workshop6b**):

![](../images/shellvars0.png)

The added script variables appear at the end with a blue background. For illustrating the process, we have added two new variables to workshop 6 for selecting between two market regime detectors (**Hurst** and **MMI**) and two filters (**Lowpass** and **Butterworth**). The variable names and the comments matter. Observe the following rules:

  * If the variable belongs exclusively to a certain algo, let its name begin with the algo name (f.i. **TRND_Mode**). Otherwise it is assumed that the variable is common for all algos.
  * The default value is given with **'= value'** in the comment. If omitted, the variable is set to **0**.
  * If the variable is to be limited by a range, enter it with **'min..max'** following a comma after the default value.
  * If the variable is to be optimized, enter the **step width** following a comma after the range. If the step width is **0** , the variable is not optimized, even if it appears in an **_optimize** call in the script. In this way, different jobs can optimize different variables. 
  * After the default value or range, enter a semicolon **';'** and then a short description of the variable. 

The Action menu got some new items:

Save Job to CSV | Save the current panel state with all variables to a job file in CSV format, in a folder of your choice (**Job** by default). If the system has no own algos, the file name will be later used for the algo names, so don't use a complex or long name.  
---|---  
Load Single Job | Load a set of variables from a job file to the panell for training and analyzing them. If one or more jobs are loaded, backtests will store their 'papertrails' of charts, logs, and reports in a subfolder named after the job.  
Load Multiple Jobs  | Select a folder by clicking on a job file inside. All jobs in that folder are loaded, together with all their asset, algo, and timeframe variants. A click on [Start] will train all jobs and store their results in the **Summary** and in their 'papertrail' folders.  
Load Jobs from Summary | Select a summary generated by previous training. All profitable jobs listed in that summary with a positive R2 coefficient are loaded. Jobs can be excluded by adding a **'#'** in front of the name or by deleting them from the summary.  
Browse Jobs | Select a job from a previously loaded set of jobs, and load its variables into the panel.   
Rearrange Summary | Re-sort the entries in the summary if **_Criteria** has changed.  
  
In the **Mode** colum the grey system variables can be switched with a mouse click between two modes. In the global setting they affect all jobs, in the job setting they are job specific and loaded from a job file. Normally you'll use the  global setting for them. The blueish strategy variables are always job specific.

Experiment with different variable settings and save them to the **Jobs** folder (or any other folder). Load all of them with Load Multiple Jobs. Four example jobs from workshop 6 are included, but you normally have a lot more jobs for thorougly testing a strategy, When you click [Start] after loading one or multiple jobs, they are all trained. At the end of the process, a bell will ring, and the Summary will open in the editor. It is a list of all trained jobs with their performance parameters. Any job that you train adds to the summary. If a summary entry of that job already exists, it is updated with the new results. The jobs with the best performances are at the top of the summary. The metrics are explained below. 

If you activated Cluster Analysis after loading jobs from the summary, WFO profile or matrices are generated for any profitable job, and the percentage of positive training runs with different WFO/OOS settings is stored in the summary under CA%. If an otherwise profitable job got a bad cluster analysis result, like less than 60%, remove it from the summary. 

### The summary

Every job and every asset, algo, and timeframe variant of a job is listed as a record in the summary. The top jobs are at the begin. The summary is stored in CSV format, so it can be further evaluated in a spreadsheet program. Every record contains these fields:

**Job** | Job name  
---|---  
**Asset** | Last asset used by the job.   
**Algo** | Last algo used by the job (if any).  
**TimeFrame** | Last time frame of the job, in minutes.  
**Days** | Total days in the backtest, not counting lookback and training.  
**WFO** | Number of WFO Cycles  
**OOS%** | Out-of-sample period in percent  
**Profit** | Sum of wins minus sum of losses minus trading and margin cost.  
**Margin** | Amount required for covering the maximum open margin  
**MaxDD** | Maximum equity drawdown from a preceding balance peak  
**PeakDD%** | Max equity drawdown in percent ot the preceding peak  
**TimeDD%** | Time spent in drawdown in percent of backtest time   
**Trades** | Number of trades  
**Win%** | Percent of won trades.  
**Obj** | Backtest return of the objective function.  
**P/L** | Profit factor, the sum of wins divided by sum of losses  
**P/Trade** | Average profit per trade  
**Return** | Annual net profit in percent of max margin plus max drawdown.  
**CAGR%** | Annual growth of the investment in percent.  
**Calmar%** | Annual net profit in percent of max drawdown.  
**Sharpe** | Annualized mean return divided by standard deviation  
**R2** | Equity curve straightness, 1 = perfect  
**CA%** | Percentage of runs that passed the cluster analysis.  
  

### Step 3: Creating multiple variants

For finding out which asset, algo, and time frame combinations work best with which job, you can test it with any combination. Since this interferes with your asset/algo loops (if any) in the script, it has to be modified further:

First, put all assets, algos, and timeframes you want to test in **#define** **_ASSETS** , **_ALGOS** , **_TIMEFRAMES** statements:

```c
#define _ASSETS "EUR/USD","USD/JPY"
#define _ALGOS "TRND","CNTR"
#define _TIMEFRAMES 60,240  // in minutes
```

The syntax is the same as in a [loop](loop.htm) statement, so you can also use the **Assets** keyword to include all assets from the current asset list. Next, replace your main asset and algo loops with a single **assetLoop()** call. Assume your **run** function previously looked like this:

```c
function rum()
{
  BarPeriod = 60; 
  LookBack = 2000; 

  while(asset(loop("EUR/USD","USD/JPY")))
  while(algo(loop("TRND","CNTR")))
  {
    if(strstr(Algo,"TRND")) {
      TimeFrame = 1;
      tradeTrend();
    } else if(strstr(Algo,"CNTR")) {
      TimeFrame = 4;
      tradeCounterTrend();
    }
  }
}
```

It now becomes this (see **workshop6c.c**):

```c
#define _ASSETS "EUR/USD","USD/JPY"
#define _ALGOS "TRND","CNTR"
#define _TIMEFRAMES 60,240

function run()
{
  BarPeriod = 60; 
  LookBack = 2000; 

  assetLoop();

  if(strstr(Algo,"TRND")) 
    tradeTrend();
  else if(strstr(Algo,"CNTR")) 
    tradeCounterTrend();
}
```

If your script has no asset/algo loops, replace your first **asset()** call with **assetLoop()**. Mind the **Algo** comparison with **strstr**. Using **strcmp** or **'='** for algo selection would not work anymore, since algo names now get a number appended. If you start the system now, it assigns different colors to the variables since it recognizes from their name to which algo they belong.

![](../images/shellvars1.png)

If you now run the system without loading a job, or if you select only a single job , it will use the first asset, algo, and time frame from the definitions. If multiple jobs are loaded, they will be trained and tested with all their asset, algo, and time frame combinations. The Action menu is now complete:

Create Algos from Summary | Select a summary generated by previous training. All profitable and robust jobs from that summary with a CA result of 85% or better are automatically selected for the final portfolio. Their variables, assets, algos, and time frames are stored in **Data\\*_algo.bin**. This file is automatically loaded at start. Training, testing, and trading will now use the full porfolio. To discard it, use Reset.  
---|---  
  

### Results and reports

The test results for any job are automatically stored in subfolders of the job folder. In this way any job leaves a 'papertrail', consisting of text, CSV, image, or html files from its training runs. All reports can be opened with a plain text editor such as the included Notepad++. The .csv files can also be opened with Excel, and the charts with an image viewer like the included **ZView**. 

Aside from the summary and the CA or MA charts, the following reports are produced when jobs are processed (for details see [exported files](export.htm)): 

* **.txt** \- the last [performance report](performance.htm).   
***history.csv** \- the performance history of the job. Any test or training run adds a line to the history.  
***.png** \- the chart with the equity curve, trades, and indicators.  
***.htm** \- a page with [parameter charts](optimize.htm) that visually display the effect of parameters on the performance.  
***.log** \- the [event log](log.htm) of the backtest.  
***train.log** \- the [training log](optimize.htm) with the objective results for all tested parameter combinations.   
***pnl.csv** \- the equity curve of the backtest, for further evaluation.  
***trd.csv** \- the trade list of the backtest, for further evaluation.  
***par.csv** \- the results of genetic or brute force training runs, for further evaluation.  
**Log\Errors.txt** \- the last errors encountered (if any) while training or testing.  

### Very concise tutorial

  * First, create the performance summary. For this select **Workshop6c** and click [Train].
  * From the [Action] scrollbox, select Load Multiple Jobs. 
  * 4 example jobs beginning with **"W6..."** are in the **Job** folder. Double click one of them.
  * Click [Start]. All jobs with their asset, algo, timeframe variants are now trained. 
  * At the end of the process, the Summary will pop up in the editor.  

  * Next, cluster analysis. Click [Train] and select Load Jobs from Summary.
  * Double click on **Job\Workshop6c_Summary.csv** that was generated in the previous step.
  * From the [Action] scrollbox, select Run Cluster Analysis. 
  * Now you have the choice between several predefined clusters. Select **WFO_Cluster3x3**.
  * After a lengthy process, the summary appears with the **CA%** column of the best jobs populated,  

  * Next, portfolio creation. Click [Train] and select Create Algos from Summary.
  * Double click on **Job\Workshop6c_Summary.csv.** You'll see a list of algo variants from the top of the summary. 
  * Click [Start] to train. The perfomance report will pop up at the end.  

  * Finally, the Montecarlo Reality Check. Click [Train]. The algos are automatically loaded,
  * From the [Action] scrollbox, select Run MonteCarlo Analysis. 
  * At the end of the process you'll get the **p-value** and a histogram of the system.  

  * If according to the p-value the system is not worth trading, come up with a better system. 
  * Otherwise click [Train]. It must be trained again after the montecarlo process.
  * After training, select your broker and account, click [Trade], and get rich. 

Naturally, training hundreds of jobs and variants will take its time. For highest speed, activate multiple threads, code in C++, and use the 64 bit Zorro version.   

### Remarks 

  * After changing variable values or ranges in the script, click Reset on the next start, otherwise the variables keep their last stored values and ranges.
  * If the strategy was changed after creating algos, the whole proces needs not be repeated unless the change is very substantial. Otherwise it is sufficient to create algos with the current summary and train them again.
  * Compare algo names not with **'='** , but with **strstr()** , as in the above example. First, it is needed because the algos generated by the shell get a number appended. Second, it is needed when you want to run your strategy with VC++. VC++ will give no error message when comparing strings with **'='** , but will just skip the clause.
  * In a multi-algo portfolio, your run function is called multiple times at any bar. If you want something to be only executed once, like a start message, put it in a **main** function.
  * Make sure that **_Lookback_Bars** is sufficient for running all loaded jobs with the largest time frame. When extending or reducing the backtest years, adapt **_WFO_Cycles** accordingly. 
  * For highest speed, activate multiple threads, code in C++, and use the 64 bit Zorro version. 
  * If a thread encounters an error, the optimization will stop and the error message can be found in the file **Log\Errors.txt**. 
  * When modifying **eval.c** , use a copy in the **Strategy** folder and include it with **#include "eval.c"**. Do not modify the original version in the **include** folder, since it will be overwritten on any Zorro update.
  * Training results are only kept for last training run or for the final portfolio. When you switch jobs or change parameters, always train again.
  * Do not open results in Excel while the shell is running. Excel will block any access to its open files.

### See also:

**[Get Started with Zorro](started.htm), [ Portfolio Lists](account.htm), [Walk Forward Optimization](numwfocycles.htm), [Performance Reports](performance.htm)**

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
