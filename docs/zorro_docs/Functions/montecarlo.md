---
title: Monte Carlo simulation
url: https://zorro-project.com/manual/en/montecarlo.htm
category: Functions
subcategory: None
related_pages:
- [Evaluation](shell.htm)
- [Performance Report](performance.htm)
- [Margin, Risk, Lots, ...](lots.htm)
- [Testing](testing.htm)
- [Trade Statistics](winloss.htm)
---

# Monte Carlo simulation

# Confidence Analysis

"Montecarlo" is a common term for an analysis based on randomized data or random samples. A Montecarlo simulation can be applied to the price curve as well as to the equity curve. The former method can be used for a [Montecarlo Reality Check](shell.htm), the latter method for generating confidence levels. For this, Zorro automatically calculates [performance metrics](performance.htm) for a distribution of many possible equity curves for any backtest. The Montecarlo method produces results that are more accurate and less subject to random fluctuations.

The equity curve from the backtest is randomly resampled many times. This generates many different equity curves that each represent a different order of trades and different price movements inside each trade. The curves are analyzed, and their results are sorted according to their performance. This way a confidence level is assigned to every result. The confidence level determines how many of the curves had the same or better performance than the given result. 

Without confidence analysis, the annual rate of return is calculated from the profits and drawdowns of the backtest equity curve. For example, it might be found that the annual return over the curve was 200%. With confidence analysis, hundreds of different equity curves are analyzed, and the annual return determined from them might be, for instance, 145% with 95% confidence. This means that of all the thousands of possible outcomes of the simulation, 95% had annual return rates better than or equal to 145%.

Confidence analysis is particularly helpful in estimating the risk and capital requirement of a strategy. The maximum historical drawdown is normally used as a measure of risk, but this means we're basing our risk calculations on a historical price curve that won't repeat in live trading. Even if the statistical distribution of trades is the same in the future, the sequence of those trades and their equity movements are largely a matter of chance. Calculating the drawdown based on one particular sequence is thus somewhat arbitrary: with a sequence of several losses in a row, you can get a very large drawdown. But the same trades arranged in a different order, such that the losses are evenly dispersed, might produce a negligible drawdown. This randomness in the result can be eliminated by Montecarlo analysis that takes many different equity curves and many different trade sequences into account. 

Example of the result distribution in the performance report: 

```c
Confidence level     AR   DDMax  Capital
 
 10%                236%  1440$  1390$
 20%                227%  1550$  1470$
 30%                218%  1680$  1570$
 40%                209%  1830$  1680$
 50%                202%  1940$  1760$
 60%                193%  2140$  1900$
 70%                186%  2340$  2040$
 80%                174%  2730$  2320$
 90%                165%  3080$  2580$
 95%                145%  4010$  3580$
100%                104%  5640$  4710$
```

The first column identifies the confidence level; in the next columns the annual return, the maximum drawdown, and the required capital at that level are listed. The most likely result is at **50%** confidence level (here, 202% annual return). This means that half the simulations had this or a better result, and half a worse result. The higher the confidence level, the more pessimistic are the results. The result at **100%** confidence level is the worst of all simulations; thus the probability to get this (or an even worse) result is **1/n** , where **n** is the number of simulations. Note that the capital can be smaller than the maximum drawdown when the test period was longer than 3 years (capital and annual return are based on a 3-years normalized drawdown). 

The following variables are used for confidence analysis: 

## MonteCarlo

Number of Montecarlo simulations (default: **200**). Every simulation generates a different equity curve. The more simulations, the more accurate is the result, but the more time is needed for the computation. If set to **0** , no confidence analysis is performed. 

## Confidence

Confidence level for the [performance analysis](performance.htm) in percent (**0..100**); determines the calculation of the main performance parameters such as annual return, drawdown, required capital, etc. The higher the confidence level, the more pessimistic are the results. At **0** (default) the performance parameters are calculated from the real equity curve and not from the Montecarlo simulation. 

### Remarks

  * If the [Capital](lots.htm) variable is set, reinvesting is assumed and no Montecarlo analysis can be performed.
  * Confidence analysis is biased when trade returns are strongly correlated (i.e. the strategy produces long winning and losing streaks) or strongly anticorrelated (winning and losing alternate). In the former case the analysis can display too optimistic results, otherwise too pessimistic results. 
  * Confidence analysis does not guarantee a minimum performance. If the strategy was expired or was [tested](testing.htm) in a wrong way (f.i. in-sample), live trading returns can be worse than the Montecarlo performance even at 100% confidence level.

### Examples

```c
function run()
{
  MonteCarlo = 1000; // 1000 simulations
  Confidence = 75; // 75% confidence level
  ...
```

### See also:

[Testing](testing.htm), [Win/Loss](winloss.htm), [Performance](performance.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
