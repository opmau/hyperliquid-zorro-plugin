---
title: series
url: https://zorro-project.com/manual/en/series.htm
category: Functions
subcategory: None
related_pages:
- [Variables](aarray.htm)
- [Bars and Candles](bars.htm)
- [set, is](mode.htm)
- [shift](shift.htm)
- [run](run.htm)
- [if .. else](if.htm)
- [Indicators](ta.htm)
- [Spectral Analysis](filter.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [Status flags](is.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [tick, tock](tick.htm)
- [rev](rev.htm)
- [Error Messages](errors.htm)
- [price, ...](price.htm)
- [sortData, sortIdx](sortdata.htm)
- [diff](diff.htm)
- [frame, frameSync](frame.htm)
---

# series

## series(_var Value_ , _int Length_): vars

Creates a time series of the given **Length** with the given **Value** , and returns the pointer to the series. A series is a [var array](aarray.htm) that contains the history of the variable. It is normally used by indicators in algorithmic trading. Series can be static or dynamic. Dynamic series are automatically shifted at every time frame, so that every element corresponds to a certain [bar](bars.htm) or time frame; the **[0]** element to the value at the current bar or time frame, the **[1]** element to the value from one bar or time frame ago, and so on. If a series is static by giving a negative **Length** , or if the [NOSHIFT](mode.htm) flag is set, the series is not shifted.  

### Parameters:

**Value** | Optional data value of the series. The series is initially filled with this value, otherwise with **0**. On dynamic series the value is copied into the first element.  
---|---  
**Length** | Optional number of elements of the series; must not change once set. When omitted or **0** , the series gets the length of **[LookBack](nhistory.htm)**. A negative number allocates a static series that can be shifted by script with the [shift](shift.htm) function.   
  
### Returns:

Pointer to the **var** array (the **vars** type is just a **var*** pointer). 

### Usage:

**vars Prices = series(price());** defines a series with the length of the **LookBack** period that contains the mean prices of the current asset  

## ref(var Value, int Index): var

Convenience macro that generates a series and returns the **Value** from **Index** time frames ago. All macros are defined in **variables.h**.  

## SeriesLength

**int** , r/o, containing the number of elements of the date series returned by the last **series()** call.

## SeriesBuffer

**var*** pointer that can be set to a memory area, and will then be returned by the next **series** call (lite-C only). After a **series** call it is set to the returned pointer. This allows to use static or pre-filled buffers for a series. For an example, see [ Financial Hacker: Truncated Indicators](https://financial-hacker.com/petra-on-programming-truncated-indicators).  

### Remarks:

  * Series correspond to serial variables in EasyLanguage™. The **n** -th element of a series is accessed by adding a **[n]** to the series name, where **n** is a positive integer number. For instance, **myseries[0]** is the most recent element, **myseries[1]** is the element of one time frame ago, and so on. **n** must be positive and smaller than the series length. 
  * !!  The series function is responsible for allocating and shifting series. Therefore it must be called at any bar, either inside the [run](run.htm) function or from a function that is called from the **run** function. 
  * !!  An internal counter determines the pointer to be returned by a **series()** call. For keeping the counter in sync, all **series()** calls must be in the same order. Therefore they must not be skipped by [if](if.htm) or other conditions that change from bar to bar (see example below). If the content of a series shall depend on **if** conditions, simply set its **[0]** element dependent on **if**. These restrictions also apply to all functions that internally create **series** , such as some [indicator](ta.htm) or [signal processing](filter.htm) functions. 
  * !!  Since the [LookBack](lookback.htm) value is normally only known after the [INITRUN](is.htm), series are allocated in the [FIRSTRUN](is.htm). During the **INITRUN** they are set to a temporary pointer and filled with the initial value. This temporary content is overwritten by the series allocation in the [FIRSTRUN](is.htm). Series are only valid during the session and released after the [EXITRUN](mode.htm).
  * Dynamic series are normally shifted by the **series** call at the end of any bar or time frame when the [NOSHIFT](mode.htm) flag is not set. The **[0]** element is then moved to the **[1]** element, the **[1]** element to the **[2]** element and so on. The current **Value** becomes the new **[0]** element. Due to this mechanism, the series contains its elements in reverse chronological order with the most recent element at the start. For synchronizing series to external events or to longer time frames, either set the [TimeFrame](barperiod.htm) variable accordingly before the **series()** call, and set it back afterwards. Or use the **NOSHIFT** flag. The element **[n]** then corresponds to the **n** th event or time frame in the past. 
  * Static series (defined with a negative **Length**) are not automatically shifted, but can be shifted by script with the [shift](shift) function. They can be used for storing arbitrary per-asset or per-algo variables, for excluding out-of-market time periods from indicators, or for shifting series by arriving price ticks in the [tick](tick.htm) function. Like any series, static series are allocated in the **FIRSTRUN** , so don't initialize them before as those values will be lost. Usage examples can be found in the **SpecialBars** script or in the SAR code in **indicators.c**.
  * For offsetting a series into the past, add an offset **+n**. This creates a pointer onto the **n** -th element of the series. For instance, **(MySeries+1)** is **MySeries** at 1 bar offset into the past, excluding the last element **MySeries[0]**. This allows to access series elements in different ways: for instance, **MySeries[3]** is the same as **(MySeries+2)[1]** and the same as **(MySeries+3)[0]**. As the offset **+n** lets the series begin at the **n** th element, it reduces the available length of the series by **n**. When calling an [indicator](ta.htm) with a series with an offset, make sure that [LookBack](lookback.htm) is always higher than the required lookback period of the function plus the unstable period plus the highest possible offset of the series. 
  * For adding or subtracting two series, create a series of the sum or difference of the recent elements, f.i. **vars Sums = series(Data1[0] + Data2[0]);**.
  * Some functions expect a single value, other functions expect a series as parameter. When a function expects a single value from a series, use the last element (**MySeries[0]**). When the function expects a whole series, use **MySeries** or **MySeries+n**. 
  * If a series changes slowly, like an EMA, fill it initially with an average **value**. This prevents initialization effects when an accumulative indicator needs many bars to 'creep' from 0 to its first value.
  * A value returned by a function can be converted to a series by using it as the first parameter to the series function. For instance, **series(price())** is a series that contains the **price** value; **series(SMA(series(priceClose()),30))** is a series containing the 30-bar Simple Moving Average of the **Close** value.
  * The [rev](rev.htm) function reverses a series so that the **[0]** element is the earliest.
  * The **length** parameter should not exceed the [LookBack](lookback.htm) period, at least not when the series affects trading. Otherwise the script would trade differently in the fist time after starting..
  * Every series requires memory and CPU resources. Therefore do not create more or longer series than needed. The longer a series, the more memory is required and the slower is script execution due to internal shifting the series on every time frame. 
  * If you need to create extremely many series and get an [Error 041](errors.htm), increase [TradesPerBar](lookback.htm). This determines not only the maximum number of trades, but also the maximum number of series.
  * For accessing the same series from several functions, declare a global **vars** , and set it with a **series** call in the **run** function.

### Examples:

```c
// create a series with the high-low price differences
vars PriceRanges = series(priceH()-priceL());
 
// compare the current range with the range from 3 bars ago
if(PriceRanges[0] > PriceRanges[3])
  ...
 
// calculate a 20-bar Simple Moving Average containing the price differences from 5 bars ago
var Average5 = SMA(PriceRange+5,20);

// wrong use of conditional series
if(priceClose() > Threshold) {
  vars X = series(priceClose()); // error message!
  vars Y = series(SMA(X,100)); // error message!
  ...
}

// correct use of conditional series
vars X = series(), Y = series();
if(priceClose() > Threshold) {
  X[0] = priceClose(); // ok!
  Y[0] = SMA(X,100);
  ...
}

// exclude out-of-market bars from a series
if(!market(EST,0)) set(NOSHIFT);
vars InMarketPrices = series(priceC());
set(NOSHIFT|OFF);

// use arrays of series
vars MySeriesArray[3]; // declare an array of series 
...
for(i=0; i<3; i++) 
  MySeriesArray[i] = series(); // fill the array with series 
...
(MySeriesArray[0])[0] = 123; // access the first element of the first series. Mind the parentheses!
```

### See also:

[price](price.htm), [sort](sortdata.htm), [rev](rev.htm), [diff](diff.htm), [shift](shift.htm), [sync](frame.htm) [► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
