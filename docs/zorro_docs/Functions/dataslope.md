---
title: DataSlope, DataSkip, DataSplit
url: https://zorro-project.com/manual/en/dataslope.htm
category: Functions
subcategory: None
related_pages:
- [Walk-Forward optimization](numwfocycles.htm)
- [set, is](mode.htm)
- [TrainMode](opt.htm)
---

# DataSlope, DataSkip, DataSplit

## DataSplit

Splits the [WFO](numwfocycles.htm) simulation in a training period (given in percent) and a following test period. F.i. when set at **60** , the training period has a length of **60%** and the test period has a length of **40%**. This works with WFO ([NumWFOCycles](numwfocycles.htm) > 1) or without WFO ([NumWFOCycles](numwfocycles.htm) == 1) and ensures that the test uses out-of-sample data. 

### Typical range:

**50..90** (default = **0** = no separate training / test period). 

### Type:

**int**

## DataSkip

Gives the number of bars to skip with the [SKIP1...SKIP3](mode.htm) flags (default: number of bars corresponding to one week). 

### Type:

**int**

## DataHorizon

Prevents trading for the given number of bars at the begin of a WFO test phase (default: **0**). This avoids test bias by future-peeking training f.i. with a machine learning algorithm. To avoid artificial triggers by parameter changes, set **DataHorizon = 2** ; to avoid peeking bias when training uses prices from 5 bars in the future, set **DataHorizon = 5**. See also [ RECALCULATE](mode.htm). 

### Type:

**int**

## DataSlope

Applies a moving weight factor to the trade results in the training period. F.i. at **DataSlope = 2** the last trades have twice the weight than the first trades. This generates parameters that are better fitted to the most recent part of the price curve, and thus takes care of slow market changes. 

### Typical range:

**1..3** (default = **1** = equal weight for all trades) 

### Type:

**var**  

### Remarks:

  * On long training periods, f.i. with [anchored WFO](numwfocycles.htm), It is recommended to set **DataSlope** at **1.5** .. **2.0** for giving the last part of the price curve more weight. 

### Example:

```c
function run()
{
  DataSlope = 2;
  DataSplit = 80;
  NumWFOCycles = -10; // anchored WFO
  ...
}
```

### See also:

[Mode](mode.htm), [TrainMode](opt.htm), [WFO](numwfocycles.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
