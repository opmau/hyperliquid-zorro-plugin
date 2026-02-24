---
title: Mode
url: https://zorro-project.com/manual/en/setf.htm
category: Functions
subcategory: None
related_pages:
- [Equity Curve Trading](trademode.htm)
- [TrainMode](opt.htm)
- [set, is](mode.htm)
- [PlotMode](plotmode.htm)
---

# Mode

# Helper macros

## setf (int Mode, int Flag)

Identical to **Mode |= (Flag)**. Sets a **Flag** or flag combination in the variable **Mode** without affecting the other flags. For instance, **setf(TradeMode,TR_PHANTOM)** sets the **TR_PHANTOM** flag for the [TradeMode](trademode.htm) variable. 

## resf (int Mode, int Flag)

Identical to **Mode &= ~(Flag)**. Resets a **Flag** or flag combination in the variable **Mode**. 

## isf (int Mode, int Flag): int

Identical to **(Mode &(Flag))**. Evaluates to nonzero when the a **Flag** in the **Mode** variable is set. For instance, **if(isf(TrainMode,PEAK))** ... checks if the **PEAK** flag is set in the [ TrainMode](opt.htm) variable. 

## swap (var a, var b)

Macro that sets **a** to **b** and **b** to **a**.

### Remarks

  * **setf(a,b)** is equivalent to **a |= b** ; and **resf(a,b)** is equivalent to **a &= ~b**.
  * The difference of **setf(Mode,Flag)** and **Mode = Flag** is that the latter resets all other flags in **Mode**.
  * After **if** , use winged brackets for **swap** , f.i. **if(some_condition) { swap(A,B); }**.
  * The macros are defined in **variables.h**.

### Example:

```c
function run()
{
  setf(TrainMode,PEAK);
  setf(Detrend,SHUFFLE); 
  ...
}
```

### See also:

[set](mode.htm), [PlotMode](plotmode.htm), [TrainMode](opt.htm), [TradeMode](trademode.htm) [► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
