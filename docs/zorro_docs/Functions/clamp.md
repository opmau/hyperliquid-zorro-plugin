---
title: clamp
url: https://zorro-project.com/manual/en/clamp.htm
category: Functions
subcategory: None
related_pages:
- [sqrt](avar-sqrt.htm)
- [abs](abs.htm)
- [sign](sign.htm)
- [between](between.htm)
---

# clamp

## clamp(int x, int lower, int upper): int

## clamp(var x, var lower, var upper): var

Returns a variable limited to a lower and upper border. 

### Parameters:

**x** , **lower, upper** \- any **var** or **int**.  

### Returns:

**x > upper** | **upper**  
---|---  
**lower <= x <= upper** | **x**  
**x < lower** | **lower**  
  
### Example:

```c
x = clamp(x,0.,1.); // limit x to 0..1
```

### See also:

[sqrt](avar-sqrt.htm), [abs](abs.htm), [sign](sign.htm), [between](between.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
