---
title: evaluate
url: https://zorro-project.com/manual/en/evaluate.htm
category: Functions
subcategory: None
related_pages:
- [malloc, free, ...](sys_malloc.htm)
- [Trade Statistics](winloss.htm)
- [Strategy Statistics](statistics.htm)
- [printf, print, msg](printf.htm)
- [optimize](optimize.htm)
- [set, is](mode.htm)
- [Performance Report](performance.htm)
- [Functions](funclist.htm)
---

# evaluate

## evaluate ()

User-supplied function that is called at the end of a [Test] run, or when [Result] is clicked, or in [Trade] mode when the status page is updated. Can be used for analyzing the performance or storing it for further evaluation in a user specified way. 

### Parameters:

**perf** \- optional pointer to a **PERFORMANCE** struct containing the result of the test; defined in **include\trading.h**.

##  cleanup ()

User-supplied function that is called at the end of a simulation or session. Can be used to copy files, [free memory](sys_malloc.htm), or release other allocated resources**.  
**

### Remarks:

  * All [trade statistics](winloss.htm) and [strategy statistics](statistics.htm) are available in the **evaluate** function. The **...Long/...Short** statistics parameters are for the last selected asset and algo. 
  * The [print](printf.htm)**(TO_REPORT, ...)** function can be used for printing user-specific data to the performance report.
  * In [Train] mode the [objective](optimize.htm) function is called at the end of every simulation. The **evaluate** function is called only in the test run after training when the [TESTNOW](mode.htm) flag is set. 

### Examples:

```c
function evaluate()
{
  printf("\nR2 = %.2f Mean = %.3f",ReturnR2,ReturnMean);
}
```

```c
// plot a parameter histogram in [Test] mode - profit vs. BarZone
function run()
{
  ...
  NumTotalCycles = 12;
  BarZone = 2*TotalCycle;
  ...
}

void evaluate()
{
  PlotScale = 10;
  plotBar("Objective",TotalCycle,BarZone,Balance,BARS,RED);	}
```

### See also:

[Performance](performance.htm), [trade statistics](winloss.htm), [user supplied functions](funclist.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
