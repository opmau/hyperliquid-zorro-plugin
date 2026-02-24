---
title: sign
url: https://zorro-project.com/manual/en/sign.htm
category: Functions
subcategory: None
related_pages:
- [sqrt](avar-sqrt.htm)
- [abs](abs.htm)
- [cdf, erf, dnorm, qnorm](cdf.htm)
---

# sign

## sign(int x): int

## sign(var x): var

The sign of **x** : **-1** , **0** , or **1**. 

### Parameters:

**x** \- any **var** or **int**.  

### Returns:

**x > 0** | **1**  
---|---  
**x == 0** | **0**  
**x < 0** | **-1**  
  
### Example:

```c
x = sign(2); // x is now 1
x = sign(-599); // x is now -1 
x = sign(0); // x is now 0
```

### See also:

[sqrt](avar-sqrt.htm), [abs](abs.htm), [cdf](cdf.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
