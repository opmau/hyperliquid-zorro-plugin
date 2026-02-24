---
title: Pointers
url: https://zorro-project.com/manual/en/apointer.htm
category: R Lectures, 1-7
subcategory: None
related_pages:
- [series](series.htm)
- [Variables](aarray.htm)
- [Structs](structs.htm)
- [Functions](function.htm)
---

# Pointers

# Pointers

Pointers store references to variables. They point to the location in memory where the variable stores it's content. Pointers allow, for instance, the same function to do the same with different global variables, depending on what variable is currently referenced through a certain pointer. Pointers can be used like synonyms or alternative names for single variables or for arrays of variables. 

A pointer is defined by adding a '***** ' to the name, like this:

**var *mypointer;_// defines a pointer of type var with the name mypointer_**

The '*' is also used for multiplication, but the compiler knows from the context if a pointer or a multiplication is meant.

You can get a pointer to any variable by adding a '**&** ' to the variable name:

**var myvar = 77;  
mypointer = &myvar; _// now mypointer points to myvar_ **

You can see the '**&** ' as the opposite of '***** '. For accessing a variable that is the target of the pointer, add a '***** ' to the pointer name, just as in the pointer definition. This way the variable can be directly read or set: 

***mypointer = 66;_// now myvar contains 66_**

Pointers can also point to variable arrays, and can access their elements just by adding the usual **[0]** , **[1]** , ... etc. to the pointer name. In fact pointers and arrays are the same internal type. When **mypointer** is a pointer to an array, **mypointer+n** is a pointer to the **n** -th element of that array. Therefore for accessing elements of the array, ***mypointer** points to the same as element as **mypointer[0]** and ***(mypointer+n)** points to the same element as **mypointer[n]**. 

### Variable pointers in functions 

There can be some situation where variable pointers might be useful. Normally if you pass a variable to a function, the function works merely with a copy of that variable. Changing the variable within the function only affects the copy. However if you pass the pointer to a variable, the function can change the original variable. For getting a pointer to a variable, just place a '**&** ' before the variable name. Example: 

```c
function change_variable(var myvar)
{
  myvar += 1;
}

function change_variable_p(var *myvar)
{
  *myvar += 1;
}
...
var x = 10;
change_variable(x);   // now x is still 10
change_variable_p(&x); // now x is 11
```

Lite-C automatically detects if a function expects a variable or a pointer to a variable, so you can usually omit the '**&** ' and just write: 

```c
change_variable_p(x); // now x is 1
```

### Arrays of pointers or series

When accessing elements in an array of pointers or [ series](series.htm), parentheses must be used: 

```c
vars MySeriesArray[3]; // declars an array of series 
...
for(i=0; i<3; i++) MySeriesArray[i] = series();
...
(MySeriesArray[0])[0] = 123; // access the first element of the first array. Mind the parentheses!
```

### Function pointers

A function pointer is defined just as a function prototype with return and parameter types. Example: 

```c
float myfunction(int a, float b); // define a function pointer named "myfunction"  

float fTest(int a, float b) { return (a*b); }
...
myfunction = fTest;
x = myfunction(y,z);
```

For storing arrays of function pointers in C,**void*** arrays can be used. Example: 

```c
float myfunction(int a, float b); // define a function pointer

void* function_array[100];        // define a pointer array

float fTest(int a, float b) { return (a*b); }
...
function_array[n] = fTest;
myfunction = function_array[n];
x = myfunction(y,z);
```

### See also:

[Variables](aarray.htm), [Structs](structs.htm), [Functions](function.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
