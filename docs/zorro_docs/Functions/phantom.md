---
title: phantom
url: https://zorro-project.com/manual/en/phantom.htm
category: Functions
subcategory: None
related_pages:
- [Equity Curve Trading](trademode.htm)
- [series](series.htm)
- [predict](predict.htm)
---

# phantom

## phantom (int Dir, int FastPeriod, int SlowPeriod) : int 

Indicator for 'equity curve trading'. Detects from the equity curve if the market is in an unprofitable state and trading with the current asset/algo combination is not advised. Can be used to switch from live trading to phantom trading by setting the [TR_PHANTOM](trademode.htm) flag. 

### Parameters:

**Dir** | **1** for long trading, **-1** for short trading, **0** for both.  
---|---  
**FastPeriod** | Lowpass period for the short-term component equity curve.  
**SlowPeriod** | Lowpass period for the long-term component equity curve.  
  
### Returns

**1** when the short-term component equity is falling and below its long-term curve, otherwise **0**. 

### Remarks:

  * This function creates [series](series.htm) and thus must be called in a fixed order in the script.
  * The source code can be found in **indicators.c**. 

### Example:

```c
if(phantom(0,5,50)) setf(TradeMode,TR_PHANTOM);
else resf(TradeMode,TR_PHANTOM);
```

### See also:

[predict](predict.htm), [Phantom Trading](trademode.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
