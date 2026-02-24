---
title: return
url: https://zorro-project.com/manual/en/return.htm
category: R Lectures, 1-7
subcategory: Functions
related_pages:
- [Expressions](expressions.htm)
- [Functions](function.htm)
---

# return

## return

## return _expression_

Terminates the function. In the second case the given expression, variable, or constant is returned as result to the calling function, where it can be evaluated. 

### Parameters:

**expression** \- value to be returned, like a variable, [expression](expressions.htm) or constant. 

### Example:

```c
var compute_days(var age, var days)
{
  return age * days;
}
```

See also:

[function](function.htm), [expression](expressions.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
