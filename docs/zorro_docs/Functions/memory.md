---
title: memory
url: https://zorro-project.com/manual/en/memory.htm
category: Functions
subcategory: None
related_pages:
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [set, is](mode.htm)
- [series](series.htm)
- [plot, plotBar, ...](plot.htm)
- [Visual Studio](dlls.htm)
- [timer](timer.htm)
- [malloc, free, ...](sys_malloc.htm)
---

# memory

## memory(int mode): size_t 

Returns the current memory consumption of the script. 

### Parameters:

**mode -  
0:** Current total memory allocation in bytes, including virtual memory and cache pages that have been written to.   
**1:** Sum of all memory allocations in bytes by functions, price data, and series during the script run.  
**2:** Current number of allocated memory areas by functions, price data, and series.  
**4:** Memory allocation in bytes per asset for historical price data.  
  

### Returns:

See **mode**. 

### Remarks:

  * Divide by 1024 for getting kBytes and by (1024*1024) for getting MBytes.
  * This function was reported not to return the correct memory allocation under Linux/Wine with **mode == 0**. 
  * Assets, bars, series, trades, and charts consume memory. The memory requirement per asset in bytes can be estimated with the formula **Years / BarPeriod * 15 MB** , where **Years** is the number of backtest years including the [LookBack](lookback.htm) period (use **1** for live trading). For options, add about **100 MB** per year and asset.
  * The [LEAN](mode.htm) and [LEANER](mode.htm) flags reduce the memory requirement by about 50%, the **TICKS** flag increases it by 32 bytes per historical price tick. [Series](series.htm) allocate 8 bytes per element and asset, [ plot ](plot.htm)commands allocate 8..24 bytes per bar and asset.
  * For backtests with large memory requirement, use the 64 bit version **Zorro64**. and write the scripts in [C++](dlls.htm)

### Example:

```c
function run()
{ 
  BarPeriod = 1;
  LookBack = 1;
  ...
  printf("\rMemory %i areas %i kb",memory(2),memory(0)/1024);
  plot("Memory",memory(0),NEW,RED);
}
```

### See Also:

[timer](timer.htm), [malloc](sys_malloc.htm), [zalloc](sys_malloc.htm)

[► latest version online](javascript:window.location.href = 'https://manual.conitec.net' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
