---
title: Comparisons
url: https://zorro-project.com/manual/en/comparisions.htm
category: R Lectures, 1-7
subcategory: Functions
related_pages:
- [Expressions](expressions.htm)
- [between](between.htm)
- [String handling](str_.htm)
- [Functions](function.htm)
- [Variables](aarray.htm)
- [Pointers](apointer.htm)
---

# Comparisons

## Comparisons

A comparison is a special type of [expression](expressions.htm) that delivers either **true** (nonzero) or **false** (zero) as a result. There are special comparison operators for comparing variables or expressions:  
  
**==** | True if the expressions left and right of the operator are equal.  
---|---  
**!=** | True if the expressions left and right of the operator are not equal.  
**>** | True if the expression __ left of the operator is greater than the expression right of the operator.  
**> =** | True if the expression __ left of the operator is greater than or equal to the expression right of the operator.  
**<** | True if the expression __ right of the operator is greater than the expression left of the operator.  
**< =** | True if the expression __ right of the operator is greater than or equal to the expression left of the operator.  
**and, &&** | True if the expressions left and right of the operator are both true.  
**or, ||** | True if any one of the expressions left and right of the operator is true.  
**not, !** |  True if the expression right of the operator is not true.  
**()** | Brackets, for defining the priority of comparisions. Always use brackets when priority matters!   
  
###  Remarks:

  * The "equals" comparison is done with '==', to differentiate it from the assignment instruction with '**=** '. Wrongly using '=' instead of '==' causes no error message from the compiler because it's a valid assignment, but is a frequent bug in scripts. 
  * Comparing floating point variables (**var** , **double** , **float** , **DATE**) with '==' returns normally **false** because they are almost never absolutely identical. Only exception are simple cases such as comparison with **0**. Otherwise, use only **>** , **> =**, **<** , **< =**, or the [between](between.htm) function for comparing floating point variables with each other. 
  * For comparing the content of structs or arrays, compare their elements. Strings can be compared with each other with the [strstr](str_.htm) or [strcmp](str_.htm) functions. For case insensitive comparing strings with string constants, (f.i. **if(Asset == "EUR/USD) ...** ) the operators **'=='** and '**!=** ' can be also used. 
  * The precedence of comparison and expression operators follows the C/C++ standard. Use parentheses in case of doubt. For instance, the expressions **(x & y == z)** and **((x & y) == z)** give different results because the **&** operator has lower precedence than the **==** operator.
  * Unlike C/C++, lite-C evaluates all parts in a **& &** or **||** comparison, even if one of it evaluates to false. Therefore, avoid constructs like **if (p && p->data == 1)..;** use **if (p) if (p- >data == 1)..** instead. 

### Examples:

```c
10 < x // true if x is greater than 10
(10 <= x) and (15 => x) // true if x is between 10 and 15
!((10 <= x) and (15 => x)) // true if x is less than 10 or greater than 15 (lite-C only)
```

### See also:

[Functions](function.htm), [Variables](aarray.htm), [Pointers](apointer.htm), [Expressions](expressions.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
