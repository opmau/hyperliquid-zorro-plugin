---
title: for
url: https://zorro-project.com/manual/en/for.htm
category: R Lectures, 1-7
subcategory: Functions
related_pages:
- [Comparisons](comparisions.htm)
- [Expressions](expressions.htm)
- [break](acrt-break.htm)
- [continue](acrt-continue.htm)
- [if .. else](if.htm)
- [while, do](while.htm)
- [for(open_trades), ...](fortrades.htm)
---

# for

## for (initialization; comparison; continuation) { ... }

Performs the initialization, then evaluates the [comparison](comparisions.htm) and repeats all instructions between the winged brackets as long as the comparison is true or non-zero. The continuation statement will be executed after the instructions and before the next repetition. This repetition of instructions is called a loop. Initialization and continuation can be any [expression](expressions.htm) or function call. A **for** loop is often used to increment a counter for a fixed number of repetitions. 

### Remarks:

  * Loops can be prematurely terminated with [break](acrt-break.htm), and prematurely continued with [continue](acrt-continue.htm).
  * The winged brackets can be omitted when the loop contains only one instruction. 
  * The notorious infinite **for** loop - **for(;;)** \- is not supported in lite-C. For infinite loops, use **while(1)**.

### Example:

```c
int i;
for(i=0; i<5; i++) // repeat 5 times
  x += i;
```

### See also:

[if](if.htm), [while](while.htm), [ break](acrt-break.htm), [continue](acrt-continue.htm), [comparisons](comparisions.htm), [for(open_trades)](fortrades.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
