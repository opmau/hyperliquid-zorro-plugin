---
title: rising, falling
url: https://zorro-project.com/manual/en/rising.htm
category: Functions
subcategory: None
related_pages:
- [fuzzy logic](fuzzy.htm)
- [ifelse, valuewhen, ...](ifelse.htm)
- [slope, line](slope.htm)
- [crossOver, crossUnder](crossover.htm)
- [peak, valley](peak.htm)
- [series](series.htm)
---

# rising, falling

## rising (vars Data) : bool 

## falling (vars Data) : bool 

Determines if a series rises or falls, i.e. if **Data[0]** is above or below **Data[1]**. 

### Parameters:

**Data** | Data series.   
---|---  
  
### Returns:

**true** if the series rises or falls, **false** otherwise. 

### Modifies

**rMomentum** \- **Data** movement in percent per time frame; indicates the 'speed' of the rising or falling. 

### Remarks:

  * The **data** series must have a minimum length of **2**. 
  * For a fuzzy logic version, use [risingF](fuzzy.htm)/[fallingF](fuzzy.htm). 
  * For a single value, use the [changed](ifelse.htm) function.
  * For checking if the series was rising **n** bars before, use **rising(Data+n)**. 
  * For checking the amount of rising or falling over a given time period, use the [slope](slope.htm) function.

### Example:

```c
function run()
{
  ...
  vars Price = series(price());
  if(rising(Price)) 
    enterLong();
  else
    exitLong();
}
```

### See also:

[crossOver](crossover.htm)/[Under](crossover.htm), [peak](peak.htm)/[valley](peak.htm), [risingF](fuzzy.htm)/[fallingF](fuzzy.htm), [slope](slope.htm), [changed](ifelse.htm), [series](series.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
