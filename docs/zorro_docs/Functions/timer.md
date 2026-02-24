---
title: timer
url: https://zorro-project.com/manual/en/timer.htm
category: Functions
subcategory: None
related_pages:
- [Time / Calendar](month.htm)
- [wait](sleep.htm)
---

# timer

## timer(): var 

High precision timer function. Can be used to measures the time between calls for testing function execution speed and time optimizing the script for faster training. 

### Returns:

Time elapsed since Zorro start in milliseconds (1/1000 sec). 

### Remarks:

  * This function uses the Pentium performance counter register. The returned value has a precision of a few nanoseconds, depending on the processor clock rate.

### Example:

```c
...var Time = timer();myfunction();printf("\nmyfunction call time = %.3f ms",timer()-Time); // execution time in microsecond accuracy for the function call...
```

### See Also:

[time functions](month.htm), [wait](sleep.htm)

[► latest version online](javascript:window.location.href = 'https://manual.conitec.net' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
