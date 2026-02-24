---
title: #define
url: https://zorro-project.com/manual/en/define.htm
category: R Lectures, 1-7
subcategory: Functions
related_pages:
- [#ifdef, #ifndef, #else, #endif](ifdef.htm)
- [AlgoVar, AssetVar, AssetStr](algovar.htm)
- [Command Line Options](command.htm)
- [Variables](aarray.htm)
- [DLLs and APIs](litec_api.htm)
---

# #define

## #define name

Defines the name as a condition for later including or excluding lines (see [#ifdef](ifdef.htm)), or for setting other special conditions during compilation. 

### Example:

```c
#define TEST
...
#ifdef TEST
printf("This is a test!");
#endif
```

## #define name value

Every time the **name** appears in the script below the **#define** , it will be replaced by the **value** , which can be another name, a number, or a simple arithmetic expression. Replacing names makes functions more 'readable', for instance by giving [general purpose variables](algovar.htm) some meaningful names. 

### Examples:

```c
#define PI 3.14159
#define HEALTH skill17
#define WINAPI __stdcall
...
x = 2.0*PI;
my.HEALTH -= 50;

long WINAPI MessageBox(HWND,char *,char *,long);
```

### Remarks

  * **#define** s are valid within all subsequent code.
  * **#define** s are only evaluated during compilation, not in already-compiled scripts. 
  * A **#define** can be given in the [command line](command.htm) with a **-d** statement (Zorro S only).
  * As a convention, defined names are normally written in uppercase. 

## #undef name 

Undefines a previously defined name. 

## #define macro(parameter,..) expression(parameter,..)

Defines a **macro** as a replacement or abbreviation for a numerical expression. Whenever the macro is encountered in the code, the expression is executed. Macros work rather like functions, but with some minor differences. Since macros are implemented as a textual substitution, there is no effect on program performance (as with functions), however they produce larger code than functions. They are normally only used for fairly small expressions. 

### Examples:

```c
#define set(obj,flag) obj.flags |= (flag)#define reset(obj,flag) obj.flags &= ~(flag)#define toggle(obj,flag) obj.flags ^= (flag)#define is(obj,flag) (obj.flags & (flag))#define zero(ptr) memset((void*)&ptr,0,sizeof(ptr))
```

## **** #define macro(parameter,..) expression(parameter##token,..)

The merging operator **##** adds the token to the parameter. Useful for redefining variable or functions names in a macro. 

### Example:

```c
#define merge3(name) merge(name##1,name##2,name##3) // merge3(test) is evaluated to merge(test1,test2,test3)
```

### Remarks

  * Check for already-defined names in **include\trading.h** and **include\variables.h** when using **#define** in the script. Re-defining or inadvertently using a system define can lead to unexpected script behavior.

# **** Some special #defines:****

## #define PRAGMA_ZERO

Initializes all [local variables](aarray.htm) to 0. 

## #define PRAGMA_API FunctionName;ModuleName!ProcName

Loads the function prototype **FunctionName** from function **ProcName** in the DLL **ModuleName** (see [Using DLLs](litec_api.htm)). Example: 

```c
#define PRAGMA_API MessageBox;user32!MessageBoxA
```

###  See also:

[#ifdef, #ifndef, #else, #endif](ifdef.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
