---
title: between
url: https://zorro-project.com/manual/en/between.htm
category: Functions
subcategory: None
related_pages:
- [sqrt](avar-sqrt.htm)
- [abs](abs.htm)
- [sign](sign.htm)
- [clamp](clamp.htm)
- [min, max](min.htm)
- [ifelse, valuewhen, ...](ifelse.htm)
---

# between

## between(int x, int lower, int upper): bool 

## between(var x, var lower, var upper): bool 

Returns **true** when the variable **x** lies at or within a **lower** and **upper** border. When **lower** is above **upper** , returns **true** when either **x >= upper** or **x <= lower**, this way allowing the comparison of cyclic ranges such as hours of the day or months of the year. 

### Parameters:

**x** , **lower, upper** \- any **var** or **int**.  

### Algorithm:

```c
if(lower <= upper) 
  return (x >= lower) and (x <= upper); 
else<
  return (x >= lower) or (x <= upper);
```

### Example:

```c
if(between(x,0.,1.))
  ...  // executed when x is between 0..1
if(between(hour(0),22,4))
  ...  // executed when hour(0) is at or above 22 or at or below 4
```

### See also:

[sqrt](avar-sqrt.htm), [abs](abs.htm), [sign](sign.htm), [clamp](clamp.htm), [min](min.htm), [max](min.htm), [ifelse](ifelse.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
