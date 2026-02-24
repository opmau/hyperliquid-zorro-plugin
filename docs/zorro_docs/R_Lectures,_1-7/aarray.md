---
title: Variables, Arrays, Strings
url: https://zorro-project.com/manual/en/aarray.htm
category: R Lectures, 1-7
subcategory: None
related_pages:
- [series](series.htm)
- [Time / Calendar](month.htm)
- [Dataset handling](data.htm)
- [round, roundto](round.htm)
- [Status flags](is.htm)
- [Error Messages](errors.htm)
- [malloc, free, ...](sys_malloc.htm)
- [Pointers](apointer.htm)
- [Structs](structs.htm)
- [Lite-C Headers](litec_h.htm)
- [String handling](str_.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [Functions](function.htm)
---

# Variables, Arrays, Strings

# Variables, Arrays, Strings 

Variables store numbers. For defining variables and giving them initial values, use a declaration like this:

```c
var Number;	// uninitialized variable of type 'var'
int Number = 123; // initialized variable of type 'int'
```

This declaration creates a variable of the given type with the given **name**. The name can contain up to 30 characters, and must begin with A..Z, a..z, or an underscore _. Variables must never have names that are already used for functions and for other previously-defined variables.

Also constant expressions can be assigned to variables. Example:

```c
int Size = 100/2; // assign the value 50
```

### Variable types 

Computers calculate with finite accuracy. Therefore all normal variable types are limited in precision and range. Lite-C supports the following variable types:

**_Type_** | **_Size (bytes)_** | **_Range_** | **_Step_** | **_Digits_** | **_Used for_**  
---|---|---|---|---|---  
**double, var** | 8  |  -1.8·10308 to 1.8·10308 |  2.2·10-308 | ~14  | high precision numbers  
**float** | 4  |  -3.4·1038 to 3.4·1038 |  1.2·10-38 | ~7  | script constants, compact numbers  
**fixed** | 4  |  -1048577.999 to 1048576.999 | 0.001 | ~9  | special purposes (HFT)  
**int, long** | 4  | -2147483648 to 2147483647 | 1 | ~10  | script constants, counting, flags   
**short** | 2  | 0 to 65536 | 1 | ~5  | compact integers   
**char, byte** | 1  | 0 to 256 | 1 | ~2  | text characters   
**bool, BOOL** | 4  | true, false or 1,0 | - | - | yes/no decisions  
**size_t, intptr_t** | 4 or 8 | 0 to 4294967295 / 18446744073709551615 | - | - | sizes, addresses, long integers  
**char*, string** | characters+1  | "..." | - | - | text  
**var*,[vars](series.htm)** | elements*8 | -1.8·10308 to 1.8·10308 |  2.2·10-308 | ~14  | arrays, simple time series  
**[DATE](month.htm)** | 8 | 12-30-1899 00:00 to doomsday | ~0.1 ms | - | time, date   
**[mat](mat.htm)** | rows*cols*8 | -1.8·10308 to 1.8·10308 |  2.2·10-308 | ~14  | vectors, matrices  
**[dataset](data.htm)** | records*(fields+1)*4 | -3.4·1038 to 3.4·1038 | 1.2·10-38 | ~7 | candles, complex time series   
  
!!  The limited accuracy of variable types affects numerical expressions. If an expression contains only integers (numbers without decimals), the result is also an integer. For instance, the expression **(99/2)** is **49** , but **(99.0/2.0)** is **49.5**. The expression **1.0/4** is **0.25** , but **1/4** is **0.** Integer numbers in the script - such as character constants (**'A'**), numeric constants (**12345**) or hexadecimal constants (**0xabcd**) are treated as **int** with 10 significant digits; numbers or constant expressions containing a decimal point (**123.456**) are treated as **float** with 7 significant digits. Due to the 7 digit limit, **var X = 123456789.0;** would in fact set **X** to **123456792.0** ; but **var X = 123456789;** sets it to **123456789.0** since **int** has 10 significant digits.

```c
var Half = 1/2;  // wrong!
var Half = 1./2; // ok!
```

Since constants or constant expressions are **float** , avoid arithmetic with constants when higher precision is needed. Use **var** or **double** variables instead: 

```c
var Days = Seconds/(24.*60.*60.);  // low precision
var SecondsPerday = 24*60*60; 
var Days = Seconds/SecondsPerday; // high precision
```

### Typecast operators

Variables are normally automatically converted to another type. So you can assign **var** to **int** variables or vice versa without having to care about type conversion. However, sometimes you'll need to convert expressions or variables when the conversion is not obvious - for instance when calling an overloaded function that accepts several variable types. In this case you can enforce a type conversion with a **typecast operator**. A typecast (or cast) operator is the target variable type in parentheses - f.i. **(int)** or **(var)** \- placed before the expression or variable to be converted. Example: 

```c
var dx = 123.456;float fy = 0.12345;
printf("The integer value of dx is %i, fy is %f",(int)dx,(var)fy); // %i expects the "int" variable type, %f expects "var"
```

Be aware of the above mentioned limited accuracy when using floating point arithmetic and then typecasting the result to an integer. Operations with the theoretical result **1** can in practice return **1.000001** or **0.999999** , and cause unexpected results. If in doubt: [round](round.htm). 

### Global, local, and static variables 

Variables can be defined _outside_ functions or _inside_ functions. When defined outside, they are called **global variables** and are accessible by all functions. They keep their values until the script is compiled the next time. Variable definitions _inside_ functions produce **local variables**. Local variables only exist within the scope of the function. They are unknown outside the function, and lose their values when the function returns. If a global variable with the same name as a local variable exists, the local variable has priority inside it's function. If you declare a variable multliple times, the most recent declared copy is used. But for your own sake you should better avoid using the same name for different variables. 

A local variable can be declared as **static**. It is then treated as a global variable even if it is declared within a function, and keeps its content when the function terminates or when another instance of the function is called. This means that it is initialized only the first time the function is called. Example: 

```c
function foo() 
{
  static bool Initialized = false;
  if(!Initialized) { initialize(); } // initialize only once
  Initialized = true;
  ...
}
```

!!  Static and global variables keep their values until the script is compiled again. So make sure to initialize them at script start - [if(Init)](is.htm) \- to their initial values if required.

The value of another variable, or the return value of a function can be assigned to a local variable at declaration, but not to a global or static variable. Use only constants for a declaration of a global or static variable. 

```c
int GlobalCounter = count(); // wrong!

function foo() 
{
  int FooCounter = count(); // ok!
  static int FooCounter2 = FooCounter; // wrong!
  ...
}
```

### Arrays 

If you group several variables of the same type together, you have an **array** : 

```c
var name[N]; // uninitialized arrayvar name[N] = { 1, 2, ... N };  // initialized global array
```

This creates a variable that contains **N** numbers, and optionally gives them default values as in the second line. Note the winged brackets { } that must surround the set of default values. Such a multi-number variable is called an **array**. The number **N** is called the **length** of the array. It must be really a number; arithmetic expressions or formulas cannot be used for defining a fixed-length array, Example: 

```c
int my_array[5] = { 0, 10, 20, 30, 40 };
```

This creates an array of 5 numbers that can be accessed in expressions through **my_array[0]...my_array[4]**. In such an expression, the number in the [ ] brackets \- the **index** \- tells which one of the 5 numbers of the array is meant. Note that there is no **my_array[5]** , as the index starts with **0**. The advantage of using an array, compared to defining single variables, is that the index can be any numeric expression. Example: 

```c
int i;
for (i=0; i<5; i++) 
  my_array[i] = i; // sets the array to 0,1,2,3,... etc.
```

Care must be taken that the index never exceeds its maximum value, **4** in this example. Otherwise the script will crash. 

!!  Initializing in the definition works only for simple arrays; multidimensional arrays, arrays that contain strings, structs, or anything else than numbers, or several arrays defined together in a single logical line must be initialized by explicitly setting their elements. Initializing local arrays makes them **static** (see below), meaning that they keep their previous values when the function is called the next time. For initializing them every time the function is called, explicitly set them to their initial values, like this:

```c
int Size = 100/2; // assign the value 50
```0

!!  All local variables are stored on a special memory area called the **stack**. The stack has a limited size that is sufficient for many variables, but not for huge arrays. Exceeding the stack size causes a [Error 111](errors.htm) message at runtime, so when you need large arrays of more than 100 variables, better declare them global, i.e. outside the function. When you want to determine the array size dynamically, use the **[malloc](sys_malloc.htm)** / **[free](sys_free.htm)** method. When you need a data structure whose size can grow or change at runtime, use no array, but a [dataset](data.htm).

In C, arrays are in fact [pointers](apointer.htm) to the start of a memory area containing the array elements. So, **my_vector[3]** is the same as ***(my_vector+3)**.

Arrays can contain not only variables, but also complex data types such as [structs](structs.htm). For sorting or searching arrays, the standard C functions **qsort** and **bsearch** can be used. They are documented in any C book and available in the [stdio.h](litec_h.htm) include file. 

Multidimensional arrays can be defined by using several indices: 

```c
int Size = 100/2; // assign the value 50
```1

Computer memory has only one dimension, so the above defined array is in fact still a pointer to a one-dimensional memory area of the size 200 (10*20). Pointer offsets or the **&** operator can be used to address array elements or pointers to array elements. In the above example, **Map[j][i]** is the same as ***(Map+j*20+i)** , and **&(Map[j][i])** is the same as **(Map+j*20+i)**. 

### **typedef**

You can define your own variable types with **typedef** statements. Example; 

```c
int Size = 100/2; // assign the value 50
```2

**typedef** can be used to redefine variables and [structs](structs.htm); it can not be used for arrays.  
  

### Strings

Strings are arrays of **char** variables that contain alphanumerical characters - letters, numbers or symbols - which can be used for messages or names. They are defined this way: 

```c
int Size = 100/2; // assign the value 50
```3

This defines a string with the given name and initializes it to the content characters between the quotation marks. Special characters within a string must be preceded by a '**\** ': \n = Line feed; \\\ = Backslash; **\"** = Quotation mark. The **string** type is simply a **char** pointer (**char***) that points to the first character. The last character is followed by a 0 byte to indicate the end of the string.

Any static **char** array can serve as a string with a fixed maximum length and variable content: 

```c
int Size = 100/2; // assign the value 50
```4

After filling this array with characters, it can be used as a **string**. Care must be taken not to exceed 39 characters, as the last one is the end mark **0**. [String functions](str_.htm) can be used for setting and manipulating string content. 

The content of a **string** can be compared with a string constant using the **'=='** operator:

```c
int Size = 100/2; // assign the value 50
```5

!!  Comparing strings with string constants is not the same as comparing two strings (f.i. **if(MyAsset1 == MyAsset2) ...**). The latter expression is only true when the two string pointers are really identical. For comparing the contents of two non-constant strings, use standard C library functions such as [strstr](str_.htm) or [strcmp](str_.htm).   
  
!!  For convenience reasons, some string functions return **temporary strings** that are allocated on the CPU stack. Temporary strings can not be used for permanently storing content, as they are eventually overwritten by further string functions calls (see [string function remarks](str_.htm)). 

### Series

[Series](series.htm) are a special array of **var** variables, normally used for time series of price data, indicators, or data curves. If not given otherwise, the length of a series is determined by the global variable [LookBack](lookback.htm). Series data is stored in reverse order, so the most recent data is at the begin of the array. 

### See also:

[pointers](apointer.htm), [structs](structs.htm), [functions](function.htm), [series](series.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
