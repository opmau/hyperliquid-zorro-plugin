---
title: break
url: https://zorro-project.com/manual/en/acrt-break.htm
category: R Lectures, 1-7
subcategory: Functions
related_pages:
- [while, do](while.htm)
- [for](for.htm)
- [switch .. case](switch.htm)
- [continue](acrt-continue.htm)
---

# break

## break

The **break** statement terminates a **while** or **for** loop or a **switch..case** statement, and continues with the first instruction after the closing bracket. 

### Example:

```c
while (x < 100)
{ 
  x+=1;
  if (x == 50) break; // loop will be ended prematurely
}
```

###  See also:

[while](while.htm), [for](for.htm), [switch](switch.htm), [continue](acrt-continue.htm) [► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
