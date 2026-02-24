---
title: call
url: https://zorro-project.com/manual/en/call.htm
category: Functions
subcategory: None
related_pages:
- [objective, parameters](objective.htm)
- [evaluate](evaluate.htm)
- [tick, tock](tick.htm)
- [trade management](trade.htm)
- [panel, ...](panel.htm)
- [adviseLong, adviseShort](advisor.htm)
- [bar](bar.htm)
- [Error Messages](errors.htm)
- [run](run.htm)
- [lock, unlock](lock.htm)
- [quit](quit.htm)
- [require, version](version.htm)
- [wait](sleep.htm)
---

# call

## call (int Event, void* Function, int P1, var P2) 

Put a user-supplied function on a scheduler in order to to be exeuted at a certain event. Two parameters, **int** and **var** , can optionally be passed to the function. 

### Parameters:

**Event** | Run the function: **1** when the system is idle, **2** at the next incoming tick, **4** after closing a trade, **+16** repeat at any event.   
---|---  
**Function** | Pointer to a script-defined function, with no, 1, or 2 parameters.  
**P1** | **int** variable to be passed to the function as first parameter, or **0** when the function has no parameters.  
**P2** | **var** variable to be passed to the function as second parameter, or **0** when the function has no second parameter.  
  
## event (int Event, void* Function): void* 

Assign a user-supplied function to a particular event. Functions with the same name as the event are automatically assigned at script start. 

### Parameters:

**Event** | **1** [objective](objective.htm), **2** [evaluate](evaluate.htm), **3** [tick](tick.htm), **4** [tock](tick.htm), **5** [manage](trade.htm), **6** [click](panel.htm), **7** [ neural](advisor.htm), **8** [bar](bar.htm), **9** [callback](tick.htm), **10** [parameters](objective.htm), **11** [error](errors.htm), **12**  
[cleanup](evaluate.htm).  
---|---  
**Function** | Pointer to a script-defined function, or **0** for disabling a previous event function.  
  
### Returns:

Previously assigned event function. 

### Remarks:

  * **call(1, ...)** can be used to prevent that the called function interrupts other functions or I/O operations. This is useful f.i. for changing the asset, entering a trade, or writing to a file in a [click](panel.htm) or [ callback](tick.htm) function.
  * After the first run, **Function** normally removes itself from the scheduler. If **16** was added to **Event** , the function stays on the scheduler and keeps being called at the given event.
  * Up to 16 functions can be placed on the scheduler at the same time. They run in the order of their placement. 

### Example:

```c
void calltest(int a,var b)
{
  printf("\nCalled with (%i,%.2f) at bar %i",a,b,Bar);
}

void run()
{
  ...
  call(1,calltest,2,3.45);
  ...
}
```

### See also:

[run](run.htm), [lock](lock.htm), [quit](quit.htm), [version](version.htm), [wait](sleep.htm), [click](panel.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
