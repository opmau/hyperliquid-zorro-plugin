---
title: sortData, sortIdx
url: https://zorro-project.com/manual/en/sortdata.htm
category: Functions
subcategory: None
related_pages:
- [series](series.htm)
- [Lite-C Headers](litec_h.htm)
- [randomize](randomize.htm)
- [frechet](detect.htm)
- [Dataset handling](data.htm)
- [vector, matrix](matrix.htm)
---

# sortData, sortIdx

## sortData (var* Data, int Length)

Sorts the **Data** array in ascending (**Length > 0**) or descending (**Length < 0**) order. 

## sortIdx (var* Data, int Length): int* 

Does not affect the **Data** array, but returns a sorted **int** array of the indices from the smallest to the greatest **Data** value.

## sortRank (var* Ranks, var* Data, int Length): int* 

Stores the rank of any **Data** element in 0..1 range in the **Ranks** array, from 0 for the smallest to 1 for the greatest element. If the elements are OHLC prices, candle patterns can be encoded this way for machine learning algorithms. 

## findIdx (var* Data, int Length, var Value): int 

Returns the index of the highest **Data** element below or equal to **Value** , or**-1** when no such element was found. 

### Parameters:

**Data** | **var** / **double** array or [series](series.htm) to be sorted  
---|---  
**Rank** | **var** / **double** array to receive the **Data** ranks.  
**Length** | Length of the **Data** array; negative for sorting in descending order.  
**Value** | Value to be found in the **Data** array.,  
  
### Returns

**sortIdx, sortRank:** Pointer to a temporary **int** array of size **Length** containing the numbers of the **Data** values after sorting. The array will be overwritten by the next **sortIdx** or **sortRank** call. Returns 0 when a parameter is invalid.  

### Remarks

  * For sorting or searching arrays, the standard C functions **qsort** and **bsearch** can alternatively be used. They are available in the [stdio.h](litec_h.htm) include file. 

### Example:

```c
// Spearman trend indicatorvar Spearman(var* Data,int Period){  Period = clamp(Period,2,g->nLookBack);  int* Idx = sortIdx(Data,Period);  var sum = 0;
  int i,n;  for(i=0,n=Period-1; i<Period; i++,n--)    sum += (n-Idx[i])*(n-Idx[i]);  return 1. - 6.*sum/(Period*(Period*Period-1.));}
```

### See also:

[randomize](randomize.htm), [frechet](detect.htm), [series](series.htm), [dataSort](data.htm), [matSort](matrix.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
