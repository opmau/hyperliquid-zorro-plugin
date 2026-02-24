---
title: invalid
url: https://zorro-project.com/manual/en/invalid.htm
category: Functions
subcategory: None
related_pages:
- [floor, ceil](floor.htm)
- [abs](abs.htm)
---

# invalid

## invalid(var V): int

Checks whether **V** is the result of an invalid expression. 

### Parameters:

**V** \- **var** to be examined.  

### Returns:

**0** \- V is a valid number.   
**1** \- V is invalid, for instance the square root of a negative number.  
**-1** \- V is infinite, or instance the result of a division by zero.   
  

## fix0(var V): var

Convenience function that allows dividing by **V** even when it is zero or invalid. 

### Parameters:

**V** \- **var** to be fixed.  

### Returns:

**0.0001** when **V** is **0** or an invalid number, otherwise **V**.   
  

### Remarks:

Invalid numbers in an expression cause the result to become also invalid. 

### Example:

```c
if(invalid(Margin)) printf("\nCheck the Margin calculation!");
```

### See also:

[floor](floor.htm), [ceil](floor.htm), [abs](abs.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
