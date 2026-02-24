---
title: ifdelse
url: https://zorro-project.com/manual/en/ifelse.htm
category: Functions
subcategory: None
related_pages:
- [series](series.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [if .. else](if.htm)
- [abs](abs.htm)
- [min, max](min.htm)
- [between](between.htm)
- [sign](sign.htm)
- [clamp](clamp.htm)
- [invalid](invalid.htm)
---

# ifdelse

# Helper functions

The following functions are implemented for convenience and for easier converting code from other platforms. 

## ifelse(bool c, var x, var y): var

## ifelse(bool c, int x, int y): int

## ifelse(bool c, string x, string y): string

Returns **x** when the expression or condition **c** is true or nonzero, otherwise **y**. 

### Parameters:

**c** \- any integer or boolean expression.  
**x** ,**y** \- variable, constant, or **string** ,**** both of the same type.  

### Returns:

**x** when **c** is true, otherwise **y**.   

## once(bool c): int

Helper function; returns nonzero when its **c** parameter became **true** or nonzero the first time in a session or backtest. Afterwards it returns **0** regardless of **c**. Can only be called once per bar. 

### Parameters:

**c** \- any integer or boolean expression.

### Returns:

**1** when called the first time with a nonzero **c** , otherwise **0**.   

## changed(var V): int

Helper function; returns **1** when its **V** value has increased since the last bar, **-1** when it has decreased, **0** otherwise. Can be called multiple times per bar. Creates an internal [series](series.htm) (see remarks). Source code in **indicators.c**. 

### Parameters:

**V** \- any variable.

### Returns:

**1** or **-1** when **V** has changed since the last bar, otherwise **0**.   

## barssince(bool c): int

Returns the number of bars or time frames since the expression or condition **c** was true or nonzero the last time, or **-1** when it was never true. This function internally creates [series](series.htm) (see remarks). Source code in **indicators.c**. 

### Parameters:

**c** \- any integer or boolean expression.

### Returns:

Number of bars, **0** when true on the current bar, **-1** when never true.  

## cum(var Inc): var

Accumulates **Inc** at any bar, and returns the sum. This function internally creates a [series](series.htm) (see remarks). Source code in **indicators.c**. 

### Parameters:

**Inc** \- variable to be added.

### Returns:

Sum of all **Inc** values so far.  

## valuewhen(bool c, vars Data, int n): var

Returns the **Data** value at which the expression or condition **c** was true or nonzero on the **n** -th most recent occurrence. This function internally creates [series](series.htm) (see remarks). Source code in **indicators.c**. 

### Parameters:

**c** \- any integer or boolean expression.  
**Data** \- data series of [LookBack](lookback.htm) elements.  
**n** \- number of occurrences, **1** = most recent, **2** = second most recent etc. 

### Returns:

**Data[i]** when**c** was true the **n** -th time **i** bars ago, otherwise **0**.   

### Remarks:

  * When functions are used for **x** and **y** , be aware that both are always executed when **ifelse** is called, regardless of the state of **c**. Use [if](if.htm) for executing functions depending on a condition. 
  * Some functions internally create [data series](series.htm), and thus must be called the same number of times and in the same order at any bar. They must therefore not depend on **if** conditions.

### Examples:

```c
var MaxOfXY = ifelse(X > Y,X,Y);         
var CloseAtLastCross = valuewhen(crossOver(Data1,Data2),seriesC(),1); 
if(once(!is(LOOKBACK))) printf("\nEnd of lookback reached!");
```

### See also:

[abs](abs.htm), [min](min.htm), [max](min.htm), [between](between.htm), [sign](sign.htm), [clamp](clamp.htm), [if](if.htm), [ fix0](invalid.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
