---
title:  if .. else
url: https://zorro-project.com/manual/en/if.htm
category: R Lectures, 1-7
subcategory: Functions
related_pages:
- [Comparisons](comparisions.htm)
- [while, do](while.htm)
- [switch .. case](switch.htm)
- [ifelse, valuewhen, ...](ifelse.htm)
---

#  if .. else

## if (comparison) { ... }

## else { ... }

If the [comparison](comparisions.htm) or expression between the round brackets is **true** or non-zero, all commands between the first pair of winged brackets are executed. Otherwise all commands between the second pair of winged brackets following **else** will be executed. The **else** part with the second set of commands can be omitted. The winged brackets can be omitted when only one instruction is to be executed dependent on the comparison. 

### Example:

```c
if (((x+3)<9) or (y==0))   // set z to 10 if x+3 is below 9, or if y is equal to 0
  z = 10;else    
  z = 5;// set z to 5 in all other cases
```

### See also:

[comparisons](comparisions.htm), [while](while.htm), [switch](switch.htm), [ifelse ](ifelse.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
