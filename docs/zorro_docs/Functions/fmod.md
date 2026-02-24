---
title: fmod
url: https://zorro-project.com/manual/en/fmod.htm
category: Functions
subcategory: None
related_pages:
- [abs](abs.htm)
- [floor, ceil](floor.htm)
- [modf](modf.htm)
---

# fmod

## fmod(var x, var y): var

Returns the fractional remainder of **x** divided by **y** , similar to the integer **%** operator. 

### Parameters:

**x** \- dividend **  
y** \- divisor.  

### Returns:

Remainder of **x/y**.

### Speed:

Fast 

### Example:

```c
var x = fmod(1.255,0.250); // x = 0.005
```

### See also:

[](abs.htm)[floor](floor.htm), [ceil](floor.htm), [modf](modf.htm), [%](avar-Ref.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
