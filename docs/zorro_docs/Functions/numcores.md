---
title: NumCores
url: https://zorro-project.com/manual/en/numcores.htm
category: Functions
subcategory: None
related_pages:
- [Walk-Forward optimization](numwfocycles.htm)
- [Licenses](restrictions.htm)
- [Integrating Zorro](engine.htm)
- [Status flags](is.htm)
- [slider](slider.htm)
- [Zorro.ini Setup](ini.htm)
- [Command](cmd.htm)
- [set, is](mode.htm)
- [R Bridge](rbridge.htm)
- [Multiple cycles](numtotalcycles.htm)
- [Training](training.htm)
---

# NumCores

## NumCores

Determines the number of logical CPU cores for [WFO](numwfocycles.htm) training ([Zorro S](restrictions.htm) required). Either a positive value for the number of cores to use, or a negative value for the number of cores not to use, f.i. **-1** for using all available cores but one. Default = **0** for no multi-core training. The WFO cycles are evenly distributed among parallel Zorro processes assigned to different CPU cores. The more cores, the faster is the training run; f.i. assigning 5 cores of a 6-core CPU will reduce the training time by 80%. 

## Core

The current core number in a WFO multicore training run from **1..NumCores** , or the current [ process number](engine.htm), or **0** when no multicore process is set up (read/only). 

### Type:

**int**

### Remarks:

  * **NumCores** must be set in the [INITRUN](is.htm). [Sliders](slider.htm) and other user interface controls should be at default settings for WFO training, since their positions are not transferred to other cores. 
  * Zorro supports up to 192 cores. How many cores are really used depends on available CPU threads (logical processors) and on the number of WFO cycles. Obviously, the number of used cores won't exceed the number of WFO cycles, but it is possible to set up more cores than CPU threads are available. Training will then consume most CPU resources and can render the PC temporarily unresponsive. The number of physical and logical threads is displayed in the Windows Task Manager. 
  * Multicore training uses the [command line](coommand.htm) for passing parameters and script name to the separate training processes. The script name must be command line compliant, i.e. no blanks and no path, and the script must be located in the **StrategyFolder** given in [Zorro.ini](ini.htm). Aside from the script, the [Define](cmd.htm) string and the 4 [Command](cmd.htm) variables are passed to the training processes via command line. In this way the training script can pass additional information to the processes.
  * The processes run as minimized Zorro instances. The message windows display only the cycles trained by the dedicated process. Parameter diagrams in multi-core mode are generated from the main process only, not from the parallel processes. The [TESTNOW](mode.htm) flag is not compatible with multicore training.
  * Training [R-based algorithms](rbridge.htm) works also in multi-core mode. Several R instances will then run in parallel. The used R libraries must support parallel processes. That's not the case when they connect to a local server (such as **H2O**), or when they use the GPU or other hardware resources (such as **Keras** , **TensorFlow** , or **MxNe** t in GPU mode).
  * Normally only the main process can write to the log file. For getting logs also from the additional cores, set **[LogNumber](numtotalcycles.htm) = Core**.
  * Aside from WFO, Zorro processes on different cores can also be started by command line and communicate with each other. For details see [processes](engine.htm).

### Example:

```c
NumCores = -2; // use all logical cores but two
```

### See also:

[NumWFOCycles](numwfocycles.htm), [Training](training.htm), [Processes](engine.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
