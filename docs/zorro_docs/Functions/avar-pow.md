---
title:  pow
url: https://zorro-project.com/manual/en/avar-pow.htm
category: Functions
subcategory: None
related_pages:
- [exp](avar-exp.htm)
- [log](avar-log.htm)
- [sqrt](avar-sqrt.htm)
---

#  pow

## pow(var x, var y): var 

Computes x raised to the power y. An error occurs if x is zero and y is less than or equal to 0, or if x is negative and y has a fractional part. Note that x*x is the same, but is computed faster than **pow(x,2)** .

### Parameters:

**x** \- any **var**.  
**y** \- any **var**. 

### Returns:

**x y**

### Example:

```c
x = pow(100,1); // x is now 100 
x = pow(2,8); // x is now 256 
x = pow(1.414,2); // x is now 1.999
```

### See also:

[](avar-exp.htm)[exp](avar-exp.htm), [log](avar-log.htm), [sqrt](avar-sqrt.htm) [► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
