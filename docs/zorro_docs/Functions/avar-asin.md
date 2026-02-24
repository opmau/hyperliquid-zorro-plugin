---
title: asinv
url: https://zorro-project.com/manual/en/avar-asin.htm
category: Functions
subcategory: None
related_pages:
- [sin, cos, tan](avar-sin.htm)
---

# asinv

## asin(var x): var 

## acos(var x): var 

## atan(var x): var 

## atan2(var a, var b): var 

Arc functions - the opposite of the **sin, cos, tan** function, return an angle for a given value. **atan2** is used for higher precision when **a** is a sine and **b** a cosine. 

### Parameters:

**x** \- any **var**.   
**a, b** \- any **var** ; the tangent value is **a/b**. 

### Returns:

Angle in radians.  

### Example:

```c
x = asin(1); // x is pi/2
x = asin(0.707); // x is pi/4
```

### See also:

[ ](aTrigonometry.htm)[Trigonometry](aTrigonometry.htm), [sin](avar-sin.htm), [cos](avar-sin.htm), [tan](avar-sin.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
