---
title: TrainMode
url: https://zorro-project.com/manual/en/opt.htm
category: Functions
subcategory: None
related_pages:
- [Training](training.htm)
- [Licenses](restrictions.htm)
- [Margin, Risk, Lots, ...](lots.htm)
- [Equity Curve Trading](trademode.htm)
- [Money Management](optimalf.htm)
- [optimize](optimize.htm)
- [loop](loop.htm)
- [set, is](mode.htm)
- [Walk-Forward optimization](numwfocycles.htm)
- [Status flags](is.htm)
- [objective, parameters](objective.htm)
- [adviseLong, adviseShort](advisor.htm)
- [Export](export.htm)
- [Multiple cores](numcores.htm)
- [setf, resf, isf](setf.htm)
---

# TrainMode

# Training variables

## TrainMode

Training method; affects the way how parameters, factors, rules, or machine learning models are generated in the training process. The following flags can be set: 

### Flags:

**ASCENT** | Ascent parameter optimization and [parameter chart](training.htm) export. Evaluates the effect of any parameter on the strategy separately. Starts with the first parameter and applies the results of already optimized parameters and the defaults of the rest. Seeks for 'plateaus' in the parameter space, while ignoring single peaks. This is normally the best algorithm for a robust strategy, except in special cases with highly irregular parameter spaces or with interdependent parameters.   
---|---  
**BRUTE** | Brute force parameter optimization ([Zorro S](restrictions.htm) required). Evaluates all parameter combinations, exports them to a ***par.csv** file in the **Log** folder, and selects the most profitable combination that is not a single peak. Can take a long time when many parameters are optimized or when parameter ranges have many steps. Useful when parameters affect each other in complex ways, or when it is necessary to evaluate results from any parameter combination. Brute force optimization tends to overfit the strategy, so out-of-sample testing or walk-forward optimization is mandatory.  
**GENETIC** | Genetic parameter optimization ([Zorro S](restrictions.htm) required). A population of parameter combinations is evolved toward the best solution in an iterative process. In each iteration, the best combinations are stochastically selected, and their parameters are then pair-wise recombined and randomly mutated to form a new generation. This algorithm is useful when a large number of parameters - 5 or more per component - must be optimized, or when parameters affect each other in complex ways. It will likely overfit the strategy, so out-of-sample or walk-forward testing is mandatory.  
**TRADES** | Use trade sizes by [Lots](lots.htm), [Margin](lots.htm), etc. Large trades get then more weight in the optimization process. Set this flag in special cases when the trade volume matters, f.i. for optimizing systems with adaptive trade sizes, or for optimizing the money management. Otherwise trade sizes are ignored in the training process for giving all trades equal weights.  
**LIMITS** | Observe trade limits by [MaxLong/MaxShort](lots.htm) etc. Otherwise all trades are used for training, even when the number of trades exceeds the limit.  
**PHANTOM** | Exclude [phantom trades](trademode.htm). Otherwise phantom trades are used for training just as normal trades.  
**PEAK** | Optimize toward the highest single peak in the parameter space, rather than toward hills or plateaus. This can generate unstable strategies and is for special purposes only, such as comparing training results with other platforms.  
**ALLCYCLES** | Generate individual [OptimalF](optimalf.htm) factor files for all WFO cycles, instead of a single file for the whole simulation. This produces less accurate factors due to less trades, but prevents backtest bias.  
  

## NumParameters

Number of parameters to [optimize](optimize.htm) for the current asset/algo component that was selected in a [loop](loop.htm) (**int,** read/only, valid after the first [INITRUN](mode.htm)). 

## ParCycle

Current parameter or current generation, runs from **1** to **NumParameters** in Ascent mode, or from **1** to **Generations** in Genetic mode (**int,** read/only, valid after the first [INITRUN](mode.htm)). 

## StepCycle

Number of the optimize step, starting with **1** (**int,** read/only, valid after the first [INITRUN](mode.htm)). The number of step cycles depends on the number of steps in a parameter range and of the population in a genetic optimization. Counts up after any step until the required number is reached or **StepNext** is set to 0\. 

## StepNext

Set this to **0** for early aborting the optimization (**int**). 

## NumTrainCycles

Number of training cycles (**int** , default = **1**). Use multiple cycles for special purposes, such as training a parameter set a second time, or training a combination of interdependent rules and parameters in a given order (see [Training](training.htm)). In any cycle, set either [RULES](mode.htm), or [PARAMETERS](mode.htm), or both, dependent on training method. Not to be confused with [WFO cycles](numwfocycles.htm) or **Generations**. 

## TrainCycle

The number of the current training cycle from **1** to **NumTrainCycles** , or **0** in [Test] or [Trade] mode (**int,** read/only). The training mode of the current cycle can be determined with the [PARCYCLE](is.htm), [RULCYCLE](is.htm), [FACCYLE](is.htm) flags. 

## LogTrainRun

Set this to a identifier number for logging a particular training run. The identifier is a 5-digit number in the format **WFSPO** , where **W** = WFO cycle, **F** = first loop iteration, **S** = second loop iteration, **P** = parameter number, and **O** = optimize step. At **11111** the very first training run is logged. 

## DataSlope

Applies a moving weight factor to the trade results in the training period. F.i. at **DataSlope = 2** the last trades have twice the weight than the first trades. This generates parameters that are better fitted to the most recent part of the price curve, and thus takes care of slow market changes. Default = **1** (equal weight for all trades). 

## Population

Maximum population size for the genetic algorithm (**int,** default = 100). Any parameter combination is an individual of the population. The population size reduces automatically when the algorithm converges and only the fittest individuals and the mutants remain. 

## Generations

Maximum number of generations for the genetic algorithm (**int,** default = 50). Evolution terminates when this number is reached or when the overall fitness does not increase for 10 generations. 

## MutationRate

Average number of mutants in any generation, in percent (**int,** default = 5%). More mutants can find more and better parameter combinations, but let the algorithm converge slower. 

## CrossoverRate

Average number of parameter recombinations in any generation, in percent (**int,** default = 80%). 

## BestResult

Highest [objective](objective.htm) return value so far (**var** , read/only, starting with **0**). 

## Parameters

Pointer to a lobal list of **PARAMETER** structs for the current asset/algo component. The **Min, Max,** and **Step** elements are set up in the list after the first [INITRUN](mode.htm) in [Train] mode. The **PARAMETER** struct is defined in **trading.h**. 

### Remarks:

  * **TrainMode** must be set up before the first [optimize](optimize.htm) call.
  * Training methods for machine learning or rules generating are set up with the [advise](advisor.htm) function.
  * Alternative optimization algorithms from external libraries or individual optimization targets can be set up with the [parameters](objective.htm) and [objective](objective.htm) functions.
  * [Parameter charts](training.htm) are only produced by **Ascent** optimization with [LOGFILE](mode.htm) set. It is recommended to do first an Ascent training for determining the parameter dependence of a strategy. Afterwards the final optimization can done with a genetic or brute force method if requried.
  * [Parameter spreadsheets](export.htm) are exported by **Genetic** and **Brute Force** optimization. They can be used for generating 2-d parameter heatmaps or 3-d parameter surfaces with Excel or other programs.
  * Percent steps (negative 4th parameter of the **optimize** function) are replaced by 10 equal steps for brute force and genetic optimization.
  * In genetic optimization, parameter combinations that were already evaluated in the previous generation are not evaluated again and are skipped in the log. This lets the algorithm run faster with higher generations.
  * Genetic optimization uses random mutations and thus can generate different results on any run, and also different results between optimizing with or without [multiple cores](numcores.htm).
  * When parameters are trained several times by using **NumTrainCycles** , each time the start values are taken from the last [optimization](optimize.htm) cycle in [Ascent](opt.htm) mode. This sometimes improves the result, but requires a longer time for the training process and increases the likeliness of [overfitting](training.htm). To prevent overfitting, use not more than 2 subsequent parameter training cycles. 

### Example:

```c
setf(TrainMode,TRADES+PEAK);
```

### See also:

[Training](training.htm), [optimize](optimize.htm), [advise](advisor.htm), [OptimalF](optimalf.htm), [objective](objective.htm), [setf](setf.htm), [resf](setf.htm)  
[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
