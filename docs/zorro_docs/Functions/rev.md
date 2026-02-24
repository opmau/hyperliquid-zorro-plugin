---
title: rev, conv
url: https://zorro-project.com/manual/en/rev.htm
category: Functions
subcategory: None
related_pages:
- [series](series.htm)
- [diff](diff.htm)
- [shift](shift.htm)
---

# rev, conv

## rev(vars Data,__ int Length): vars 

Generates a reverse copy of the **Data** series which starts with the oldest elements. Useful for sending data series to other software that needs the series in old-to-new order. 

## conv(float* fData,__ int Length, _int Stride_): vars 

Converts a **float** array to a temporary **var** series, for passing it to functions as a series parameter. The optional **Stride** parameter gives the number of **fData** elements to skip, for extracting columns from multidimensional arrays. 

### Parameters:

**Data** | Series or array.  
---|---  
**Length** | The number of elements; **LookBack** if 0\.   
**Stride** | Index increase, or **0** for one-dimensional arrays.   
  
### Returns

[series](series.htm).   

### Remarks

  * The **rev** function generates a dynamic [series](series.htm) and must be called in a fixed order in the script. 
  * The **conv** function generates a temporary series. It only keeps its content until the next **conv** call.

### Example:

```c
// generate reverse price series (latest prices last)
vars O = rev(seriesO(),0),
  H = rev(seriesH(),0),
  L = rev(seriesL(),0),
  C = rev(seriesC(),0));
```

### See also:

[series](series.htm), [diff](diff.htm), [shift](shift.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
