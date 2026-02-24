---
title: C for programmers
url: https://zorro-project.com/manual/en/litec_c.htm
category: R Lectures, 1-7
subcategory: Functions
related_pages:
- [Included Scripts](scripts.htm)
- [ifelse, valuewhen, ...](ifelse.htm)
- [for](for.htm)
- [Structs](structs.htm)
- [#include](include.htm)
- [DLLs and APIs](litec_api.htm)
- [Pointers](apointer.htm)
- [Functions](function.htm)
---

# C for programmers

# Lite-C for C/C++ programmers

Zorro's lite-C compiler uses the familiar syntax of C based languages such as C, C++, C#, or Javascript. Although it has some differences to allow a simpler compiler structure and faster compiling, it is 'compatible enough' to access the Windows API and even use complex external C++ libraries such as DirectX or OpenGL. Therefore it can be used to write any normal computer program (see**[Mandelbrot](scripts.htm)**). 

The differences of lite-C to standard C/C++ are listed here: 

### No early if(A && B) abort 

The **& &** and **||** operators are aliased with **and** and **or** , but can of course still be used. In C/C++, comparisons are early aborted when a **& &** is encountered and the expression so far evaluated to **false** , or when a || is encountered and the expression evaluated to **true**. Lite-C always calculates the expressions up to the end; the order of comparisons plays no role. This requires a different syntax for checking the value of a struct pointer and its element in the same expression: 

```c
if((ptr != NULL) && (ptr->element == ..)) .. // C/C++
if(ptr != NULL) if(ptr->element == ..) .. // lite-C
```

### Trinary operators 

Lite-C does not support the **comparison ? expression : expression;** syntax. Use the [ifelse](ifelse.htm) statement instead. 

```c
x = (x<0 ? -1 : 1); // C/C++
x = ifelse(x<0,-1,1);  // lite-C
```

### Infinite loops 

Lite-C requires the continuation statement in a [for](for.htm) loop. So don't use **for(;;)** for infinite loops.. 

```c
for(;;) ... // C/C++
while(1) ... // lite-C
```

### Multiple operations in one command

Lite-C does not support the comma for clustering multiple operations in one command. 

```c
x = 2, y = 2; // C/C++x = y = 2;    // lite-C
x = 2; y = 2; // lite-C
```

### Struct and array initialization

In C/C++  structs can be initialized just like arrays, by giving a list of member values. This is only supported for numerical arrays in lite-C. Lite-C can initialize structs by directly setting their elements (see [structs](structs.htm)). 

### Struct copying

In C++, structs can be copied into each other with the '**=** ' operator. In lite-C, use **memcpy** for copying structs or arrays: 

```c
// C++:
D3DXVector3 vecA, vecB;
...
vecA = vecB;

// lite-C:
D3DXVector3 vecA, vecB;
...
memcpy(&vecA,&vecB,sizeof(D3DXVector3));
```

### Struct returning

In C++, functions can return structs. In lite-C, return struct pointers instead. 

### sizeof

In C/C++, **sizeof** is a built-in function. In lite-C, it's a macro with some restrictions for structs and arrays (see [structs](structs.htm)). 

### Enums

Enums are not supported and must be replaced by defines: 

```c
enum RGB { RED=1; BLUE=2; GREEN=3 }; // C/C++
#define RED 1 // lite-C
#define BLUE 2
#define GREEN 3
```

###  Unions

Union members of the same type can be substituted by a **#define** , and union members of different type can be treated as a different members of the struct. Example: 

```c
typedef struct S_UNION { 
   int data1;
   union { int data2; float data3; };
   union { int data4; int data5; };
} S_UNION; // C/C++

typedef struct S_UNION { 
   int data1;
   int data2;
   float data3;
   int data4; 
} S_UNION; // lite-C
#define data5 data4
```

If the struct size must not change, or if for some reason the program requires different variable types to occupy the same place in the struct, a special conversion function can be used to convert the type of a variable without converting the content:

```c
typedef struct S_UNION { 
   int data1;
   union { int data2; float data3; };

} S_UNION; // C/C++
...
S_UNION s_union;
s_union.data3 = 3.14;
typedef struct S_UNION { 
   int data1;
   int data2;
} S_UNION; // lite-C
#define data3 data2
...
int union_int_float(float x) { return *((int*)&x); }
...
S_UNION s_union;
s_union.data3 = union_int_float(3.14);
```

### Function pointers

In C/C++, function pointers are declared like this: **int (*foo)(int a, int b);**. In lite-C there's no difference between function prototypes and function pointers: **int foo(int a, int b);**. See details under [pointers](apointers.htm). 

### Signed, unsigned, extern, const, register...

In lite-C, variables are only defined by their type; further qualifiers are not used. **Float** , **double** , **var** , **long** , **int** are generally signed, and pointers, **char** and **short** are generally unsigned, according to the normal way they are used. The **include\litec.h** file contains definitions for all usual unsigned variables like **DWORD** or **WORD** that are used in Windows functions. So using unsigned variables normally does not cause any problems. However you need to take care when variables exceed their range. For instance, subtracting **1** from **(DWORD)0** results in **-1** under lite-C, but in **0xFFFFFFFF** in standard C/C++. 

### Relaxed variable and function type checking

Lite-C allows a lot of things where a C++ compiler would issue a warning, such as assigning an int to a pointer, or not returning a value from a function. This relaxed checking is heavily used in the predefined scripts for making the code shorter and less complicated, but it also allows the user to make more mistakes. 

### main() and run() functions

The **main()** function works like in C. The **run()** function indicates that the program is a strategy script. It is called once before the start of the strategy for initialization, and then once for each bar. 

### Using C library or Windows API functions 

Lite-C already contains the most often used functions from the standard C/C++ libraries - they are listed in **< litec.h>**, which is automatically included in all scripts without explicit [#include](include.htm) statement. So you normally won't need to include C headers such as **stdio.h** or **math.h**. If you need a function that is not contained in **< litec.h>**, add it as described under [Using the Windows API](litec_api.htm). Here's a brief instruction of how to add an external function to lite-C: 

  * Find out in which Windows DLL the function is contained. C library function are normally found in the **mscvrt.dll**. You can use a free DLL browser, for instance the **DLL Export Viewer** from <http://www.nirsoft.net>, for checking which DLL contains which function. 
  * Declare a function prototype in your source code. In most cases you can directly copy the prototype from the **.h** header file that accompanies the DLL, or from the documentation to the C library. 
  * Additionally to the prototype, either add a **#define PRAGMA_API** line for initializing the function prototype, f.i. **#define PRAGMA_API MessageBox;user32!MessageBoxA** as described in the API example. Or, for dynamic initalization, add a **DefineApi** call in your main function. 

If you need certain structs or variable types that are not yet contained in **include\windows.h** or in the other standard include files, just add them from their original file either into your script. If you think that a certain function, struct, or variable type is often needed, suggest its inclusion on the Zorro future forum.

### See also:

[Pointers](apointer.htm), [Structs](structs.htm), [Functions](function.htm), [API](litec_api.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
