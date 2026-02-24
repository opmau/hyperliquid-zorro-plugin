---
title: optimize, objective
url: https://zorro-project.com/manual/en/objective.htm
category: Functions
subcategory: None
related_pages:
- [optimize](optimize.htm)
- [adviseLong, adviseShort](advisor.htm)
- [Strategy Statistics](statistics.htm)
- [Trade Statistics](winloss.htm)
- [File access](file_.htm)
- [Dataset handling](data.htm)
- [call](call.htm)
- [TrainMode](opt.htm)
- [Licenses](restrictions.htm)
- [Status flags](is.htm)
- [Training](training.htm)
- [Included Scripts](scripts.htm)
---

# optimize, objective

# User-supplied optimization targets and algorithms

This page is about user-defined methods and objectives for backtest-based [parameter optimization](optimize.htm). For other sorts of optimization, for instance training neural nets or user-specific models, use [adviseLong(NEURAL, ...)](advisor.htm) with a user-supplied **neural()** function. 

## objective(): var

A user-supplied **objective** **function** \- also known as **'fitness function'** \- can be used for calculating the optimization target by analyzing the preceding training run. It is called at the end of every optimization step and supposed to return a performance metric based on the [statistics](statistics.htm) or [trade results](winloss.htm) of that step. If required, it can also be used to store results in a [ file](file_.htm) or [dataset](data.htm) for further evaluation, such as plotting a 3D parameter chart. A default **objective** function is located in the **default.c** file. It uses the Pessimistic Return Ratio (PRR) for the optimization target, requires at least 10 trades, and treats the highest win and worst loss as outliers: 

```c
// optimizing objective based on PRR
var objective()
{
  if(NumWinTotal < 2 || NumWinTotal+NumLossTotal < MinTrades) 
    return 0.; // needs at least 10 trades, 2 of them positive
  var wFac = 1./sqrt(1.+NumWinTotal); 
  var lFac = 1./sqrt(1.+NumLossTotal);
  var Win = WinTotal, Loss = LossTotal;
// remove possible outlier wins/losses
  if(NumLossTotal > 1) {
    Loss -= (NumLossTotal-1)*LossMaxTotal/NumLossTotal;
    Win -= (NumWinTotal-1)*WinMaxTotal/NumWinTotal;
  }
// return PRR
  return (1.-wFac)/(1.+lFac)*(1.+Win)/(1.+Loss);
}
```

For some strategies, for instance binary options, the above PRR is not the best optimization target. For alternative targets, put an individual **objective** function in your script. It will override the default **objective**. Examples:

```c
// alternative objective function based on Calmar ratio
var objective()
{
  return(WinTotal-LossTotal)/fix0(DrawDownMax);
}

// alternative objective function based on Sharpe ratio
var objective()
{
  if(!NumWinTotal && !NumLossTotal) return 0.;
  return ReturnMean/fix0(ReturnStdDev);
}

// alternative objective function that just hunts for max profit
var objective()
{
  return WinTotal-LossTotal;
}

// alternative objective function for binary options
var objective()
{
  return ((var)(NumWinLong+NumWinShort))/max(1,NumLossLong+NumLossShort);
}
```

Alternatively, objective functions of different names can be assigned with the [event](call.htm) function.  

## parameters(): int 

Alternatively to Zorro's own [Ascent](opt.htm), [ Genetic](opt.htm), or [Brute Force](opt.htm) algorithms, parameters can be optimized with an external optimization algorithm in an external DLL, R package, or Python library, or with a script-based algorithm ([Zorro S](restrictions.htm) 2.32 or above). For this a **parameters** function must be supplied and the [ASCENT](opt.htm), [BRUTE](opt.htm), [ GENETIC](opt.htm) flags must be off. The **parameters** function is automatically called after the initial run ([INITRUN](is.htm)) of any optimization step. Its purpose is setting up the **Value** elements of the [Parameters](opt.htm) list for the subsequent training. The **Value** s can be either directly calculated or retrieved with a function from an external library. The **parameters** function is supposed to return **0** when the last step is reached and the optimization is finished after that step. 

Below an example of a **parameters** function for a script-based brute force optimization. The condition **StepCycle == 1** intializes the parameters, afterwards they are counted up until all have reached their maximum. 

```c
int parameters()
{
// set up parameters for brute force optimization
  int i;
  for(i=0; i<NumParameters; i++) {
    if(StepCycle == 1 || Parameters[i].Value >= Parameters[i].Max)
      Parameters[i].Value = Parameters[i].Min;
    else { // count up
      Parameters[i].Value += Parameters[i].Step;
      break;
    } 
  }
// last step reached?
  for(i=0; i<NumParameters; i++)
    if(Parameters[i].Value < Parameters[i].Max)
      return 1;  // not yet
  return 0; // yes, optimization complete
}
```

The **objective** function now got the job not only to return the performance metric, but also to store the best result in the [BestResult](opt.htm) variable and its parameter values in the **Best** elements of the **Parameters** list. Alternatively, the best parameter values can be retrieved from an external library in the **objective** call after the last optimization step.

```c
var objective()
{
// calculate performance metric from profit factor
  var Result = ifelse(LossTotal > 0,WinTotal/LossTotal,10);
// store best result and best parameters
  int i;
  if(BestResult < Result) {
    BestResult = Result;
    for(i=0; i<NumParameters; i++)
      Parameters[i].Best = Parameters[i].Value;
  }
  return Result;
}
```

### Remarks:

  * A bit of optimization theory is described under [Training](training.htm). All optimization modes, such as portfolio component optimization, walk forward optimization, or data-split optimization, can be used with internal as well as external optimization algorithms.
  * In external optimization mode, [optimize](optimize.htm) calls return the default parameter values in the **INITRUN**. Afterwards they return the values set up by the **parameters** function.
  * The [OptimizeByScript](scripts.htm) script contains an example of a script-based brute force optimization.

### Example: Using an external optimization library

```c
// assumed library functions:
// set_experiment(Parameters) // initialize the module with a list of parameters
// get_suggested(Parameters) // return suggested parameter assignment
// set_observation(Result) // deliver the result from the suggested parameters
// get_optimized(Parameters) // return the best parameter set

int MaxSteps = 1000; // optimization budget

int parameters()
{
// initialize librariy and send parameter boundaries
  if(StepCycle == 1)
    set_experiment(Parameters);
// get parameter assignment for the current step
  get_suggested(Parameters); 
// optimization complete?
  if(StepCycle == MaxSteps) 
    return 0;	// yes
  else return 1;
}

var objective()
{
// calculate performance metric from profit factor
  var Result = ifelse(LossTotal > 0,WinTotal/LossTotal,10);
  BestResult = max(Result,BestResult);
// send result to library
  set_observation(Result);
// after the last step, get best parameter set 
  if(StepCycle == MaxSteps) 
    get_optimized(Parameters);  
  return Result;
}
```

### See also:

[Training](training.htm), [TrainMode](opt.htm), [optimize](optimize.htm), [NumParameters](opt.htm), [advise](advisor.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
