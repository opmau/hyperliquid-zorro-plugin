---
title: slope, line
url: https://zorro-project.com/manual/en/slope.htm
category: Functions
subcategory: None
related_pages:
- [series](series.htm)
- [Statistics, Transformations](transform.htm)
- [Indicators](ta.htm)
- [crossOver, crossUnder](crossover.htm)
- [rising, falling](rising.htm)
- [peak, valley](peak.htm)
- [fuzzy logic](fuzzy.htm)
- [predict](predict.htm)
---

# slope, line

## slope (int Type, vars Data, int TimePeriod) : var 

Connects the two highest maxima or lowest minima of the **Data** series within the given **TimePeriod** , and returns the slope of the connecting line. This function can be used to calculate traditional divergence or convergence (triangle) patterns. 

### Parameters:

**Type** | **PEAK** for a line through the maxima, **VALLEY** for a line through the minima.  
---|---  
**Data** | Time [series](series.htm).  
**TimePeriod** | Period to be examined.  
  
### Returns:

Slope (movement per bar, positive for a rising, negative for a falling line), or **0** if the series is flat or no minima/maxima are found. 

### Modifies

**rMin, rMinIdx** \- value and bar offset of the lower connecting point.  
**rMax, rMaxIdx** \- value and bar offset of the higher connecting point.  

## line (int Offset) : var 

Returns the value of a straight line at the bar with the given **Offset** (0 = current bar). The line is defined as connecting the **rMin** , **rMinIdx** , **rMax** , **rMaxIdx** coordinates which can be either set directly, or generated with **slope** or with [Min/Max/MinIndex/MaxIndex](transform.htm). 

### Parameters:

**Offset** | Bar offset  
---|---  
  
### Returns:

Value of the line at the given **Offset**. 

### Remarks:

  * The **slope** function is used for the [ Support](ta.htm), [Resistance](ta.htm), and [ Divergence](ta.htm) indicators.
  * Source code in **indicators.c**.

### Example:

```c
var HighsDn[10] = { 1,1,1,2,1,1,1,3,1,1 }; // mind the reverse order
var LowsUp[10] = { 1,1,1,0.7,1,1,1,0.5,1,1 };

function main()
{
  printf("\nSlope dn %.3f",slope(PEAK,HighsDn,10));
  printf("\nSlope up %.3f",slope(VALLEY,LowsUp,10));
}
```

### See also:

[crossOver](crossover.htm), [crossUnder](crossover.htm), [rising](rising.htm), [falling](rising.htm), [peak](peak.htm), [valley](peak.htm), [ peakF](fuzzy.htm), [valleyF](fuzzy.htm), [predict](predict.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
