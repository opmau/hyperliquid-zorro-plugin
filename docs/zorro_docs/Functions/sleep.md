---
title: wait
url: https://zorro-project.com/manual/en/sleep.htm
category: Functions
subcategory: None
related_pages:
- [quit](quit.htm)
- [progress](progress.htm)
- [call](call.htm)
- [timer](timer.htm)
- [run](run.htm)
- [R Bridge](rbridge.htm)
- [lock, unlock](lock.htm)
- [Status flags](is.htm)
- [require, version](version.htm)
---

# wait

## wait (int n): int 

Suspends execution for the time given by **n** in milliseconds. The user interface remains responsive during the wait time. The function returns **0** when the [Stop] button was hit or [quit](quit.htm) was called, otherwise nonzero. If **n** is negative, the function checks and updates the user interface only every **n** th time, for not delaying fast loops. Use **wait** in a loop for avoiding unresponsiveness during long computations or external function calls. For displaying progress during very long processes, use [ progress](progress.htm) instead. 

### Parameters:

**n** \- wait time in milliseconds when positive, user interface update frequency when negative, **0** for immediate user interface update. 

### Returns

**0** when the script is to be terminated, **1** otherwise. 

### Remarks:

  * Trades and prices are not updated during **wait** time, so do not use this function to wait for a trade to enter or a price limit to be reached. For triggering a function at a certain event, use [call](call.htm).
  * Since **wait()** triggers the Windows message system, even **wait(0)** can take several hundred microseconds on slow systems. Use **wait(-1000)** in time critical loops.

### Examples:

```c
Rx("TrainNeuralNet()",1); // start a long computation
while(Rrun() == 2)
  if(!wait(100)) return;  // wait until computation is finished or [Stop] was hit
```

```c
while(some_long_loop) {
  ...
  if(!wait(-1000)) break; // prevent unresponsiveness in long loops
}
```

### See also:

[timer](timer.htm), [run](run.htm), [R bridge](rbridge.htm), [lock](lock.htm), [BUSY](is.htm), [quit](quit.htm), [version](version.htm), [progress](progress.htm), [suspended](suspended.htm), [call](call.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
