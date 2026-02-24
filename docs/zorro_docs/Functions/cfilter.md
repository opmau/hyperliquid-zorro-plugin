---
title: filter
url: https://zorro-project.com/manual/en/cfilter.htm
category: Functions
subcategory: None
related_pages:
- [series](series.htm)
- [distribute, knapsack](renorm.htm)
- [predict](predict.htm)
- [adviseLong, adviseShort](advisor.htm)
- [polyfit, polynom](polyfit.htm)
---

# filter

## filter (var* Data, int Length, var Kernel[6]) : var* 

Applies a 5-element convolution filter with the given **Kernel** to the **Data** array. Each **Data** element is added to its local neighbors, weighted by the **Kernel** , using this formula: **_Data[i] = Kernel[0]*Data[i-2] + Kernel[1]*Data[i-1] + Kernel[2]*Data[i] + Kernel[3]*Data[i+1] + Kernel[4]*Data[i+2] + Kernel[5]_**. This function can be used for data smoothing, summing up, subtraction, sharpening, edge enhancement, or similar data array manipulation operations. 

### Parameters:

**Data** | Array or [series](series.htm) to be filtered.  
---|---  
**Length** | Number of elements to be filtered.   
**Kernel** | Vector of 5 weights plus constant to be added.  
  
### Returns

Modified **Data  
**

## renorm (var* Data, int Length, var Sum) : var* 

Modifies the **Data** array by multiplying all elements with a factor so that they sum up to **Sum**. This function can be used to normalize a list of weights to a certain total. 

### Parameters:

**Data** | Array or [series](series.htm) to be normalized.  
---|---  
**Length** | Number of elements to be normalized.   
**Sum** | Resulting sum of elements.  
  
### Returns

Modified **Data  
**

### Remarks:

  * Use the [renorm](renorm.htm) function on the kernel when the weight sum must be 1. 
  * For boundary handling, **Data[0]** and **Data[Length-1]** are extended.
  * Some examples of filter kernels:

Identity | **{ 0, 0, 1, 0, 0, 0 }**  
---|---  
Sharpen | **{ -1, -1, 4, -1, -1, 0 }**  
Smooth | **{ 0.2, 0.2, 0.2, 0.2, 0.2, 0 }**  
Gaussian | **{ 0.1, 0.2, 0.4, 0.2, 0.1, 0 }**  
Shift to the right | **{ 0, 1, 0, 0, 0, 0 }**  
Mean subtraction | **{ 0, 0, 1, 0, 0, -mean }**  
Fill with constant | **{ 0, 0, 0, 0, 0, constant }**  
  

### Example:

```c
var* Filter = { 1,2,3,2,1,0 };
filter(Data,Length,renorm(Filter,5,1));
```

### See also:

[predict](predict.htm), [advise](advisor.htm), [polyfit](polyfit.htm), [distribute](renorm.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
