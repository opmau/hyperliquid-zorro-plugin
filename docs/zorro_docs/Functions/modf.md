---
title: modf
url: https://zorro-project.com/manual/en/modf.htm
category: Functions
subcategory: None
related_pages:
- [abs](abs.htm)
- [floor, ceil](floor.htm)
- [fmod](fmod.htm)
---

# modf

## modf(var x, var* p): var

Splits **x** in a fractional and integer part, each of which has the same sign as **x**. 

### Parameters:

**x** \- any number.  
**p** \- **var** pointer for the integer part.  

### Returns:

Fractional part of **x**. Integer part is stored in *p. 

### Speed:

Fast 

### Example:

```c
var y;
var x = modf(123.456,&y); // x = 0.456 and y = 123.0
```

### See also:

[](abs.htm)[floor](floor.htm), [ceil](floor.htm), [fmod](fmod.htm), [%](avar-Ref.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
