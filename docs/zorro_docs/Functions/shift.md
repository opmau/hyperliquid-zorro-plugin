---
title: shift
url: https://zorro-project.com/manual/en/shift.htm
category: Functions
subcategory: None
related_pages:
- [tick, tock](tick.htm)
- [series](series.htm)
- [diff](diff.htm)
- [rev](rev.htm)
---

# shift

## shift (vars Data, var Value, int Length) 

Shift the first **Length** elements of **Data** by one position, then fill **Data[0]** with **Value**. 

### Parameters:

**Data** | Series or array to be shifted.   
---|---  
**Value** | New value for the first element.   
**Length** | Number of elements to be shifted. If negative, the array is shifted backwards.  
  
### Remarks

  * The functions can be used to shift static series or var arrays conditionally or in a [ tick](tick.htm) or [tock](tick.htm) function. Dynamic series are otherwise automatically shifted at any time frame.

### Example:

```c
function run()
{
  ...
  vars Data10 = series(0,-10); // create a static series
  if(SomeCondition) shift(Data10,NewValue,10);
  ...
```

### See also:

[series](series.htm), [diff](diff.htm), [rev](rev.htm), [tick](tick.htm)  
[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
