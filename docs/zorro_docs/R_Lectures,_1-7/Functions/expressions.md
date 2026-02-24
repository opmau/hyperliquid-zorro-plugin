---
title: Expressions
url: https://zorro-project.com/manual/en/expressions.htm
category: R Lectures, 1-7
subcategory: Functions
related_pages:
- [Functions](function.htm)
- [Variables](aarray.htm)
- [Pointers](apointer.htm)
- [Comparisons](comparisions.htm)
---

# Expressions

# Expressions

An expression is an arithmetical operation that delivers a result, which can then be assigned to a variable or object parameter. The arithmetic expression may be composed of any numbers, further variables or object parameters, function calls, brackets, and arithmetic operators.   
  
The following operators are available in expressions:  **=** | Assigns the result right of the '=' to the variable left of the '='.   
---|---  
**+-*/** | The usual mathematical operators. * and / have a higher priority than + and -.   
**%** | Modulo operator, the integer remainder of a division.  
**|** | Bitwise OR, can be used to set certains bits in a variable.   
**^** | Bitwise exclusive OR, can be used to toggle certain bits in a variable.   
~ | Bitwise invert, toggles all bits of a variable.  
**&** | Bitwise AND, can be used to reset certains bits in a variable.   
**> >** | Bitwise right shift, can be used to divide a positive integer value by 2.   
**< <** | Bitwise left shift, can be used to multiply a positive integer value by 2.   
**()** | Parentheses for defining the priority of mathematical operations.  
  
###  Examples:

```c
x = (a + 1) * b / c;
z = 10;
x = x >> 2; // divides x by 4 (integer only)
x = x << 3; // multiplies x by 8  (integer only)
x = fraction(x) << 10; // copies the fractional part of x (10 bits) into the integer part
```

### Assignment operators

The "**=** "-character can be combined with the basic operators:   
  
**+=** | Adds the result right of the operator to the variable left of the operator.   
---|---  
**-=** | Subtracts the result right of the operator from the variable left of the operator.   
***** **=** | Multiplies the variable left of the operator by the result right of the operator.  
**/****=** | Divides the variable left of the operator by the result right of the operator.  
**%=** |  Sets the variable left of the operator to the remainder of the division by the result right of the operator.  
|= |  Bitwise OR's the the result right of the operator and the variable left of the operator.  
&= |  Bitwise AND's the the result right of the operator and the variable left of the operator.  
^= |  Bitwise exclusive OR's the the result right of the operator and the variable left of the operator.  
>>= |  Bitwise right shift the variable left of the operator by the result right of the operator.  
<<= |  Bitwise left shift the variable left of the operator by the result right of the operator.  
  
### Increment and decrement operators 

Variables can be counted up or down by attaching '**++** ' or '**\--** ' before or after a variable.  
**x++** | Increments **x** by **1** ; the result is the previous value of **x** (before incrementing).  
---|---  
**++x** | Increments **x** by **1** ; the result is the current (incremented) value of **x**. This is slightly faster than **x++**.   
**x--** | Decrements **x** by **1** ; the result is the previous value of **x** (before decrementing).  
**\--x** | Decrements **x** by **1** ; the result is the current (decremented) value of **x**. This is slightly faster than **x--**.   
  
### Examples:

```c
x = x + 1; // add 1 to x
x += 1; // add 1 to x
++x; // add 1 to x
```

### Remarks:

  * For setting and resetting flags through the **&** or **|** operators, use **long** or **int** variables.
  * !!  The precedence of comparison and expression operators follows the C/C++ standard. Use parentheses in case of doubt. For instance, the expressions **(x & y == z)** and **((x & y) == z)** give different results because the **&** operator has lower precedence than the **==** operator.
  * Bugs in expressions - for instance, division by zero - generate results that are not a number (**NaN**). They are printed like **"1.#J"** or **"1.#IND"**. If you see such a number in the log, there's a faulty expression somewhere in your script.

See also:

[Functions](function.htm), [Variables](aarray.htm), [Pointers](apointer.htm), [Comparisons](comparisions.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
