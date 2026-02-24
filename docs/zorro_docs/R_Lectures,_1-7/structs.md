---
title: Structs
url: https://zorro-project.com/manual/en/structs.htm
category: R Lectures, 1-7
subcategory: None
related_pages:
- [Pointers](apointer.htm)
- [malloc, free, ...](sys_malloc.htm)
- [Variables](aarray.htm)
- [Functions](function.htm)
---

# Structs

# Structs 

A **struct** is an assembled object that contains variables, pointers, or further structs. Members of a struct are individually accessed using the struct name, followed by a '.' and the member name. Example: 

```c
typedef struct { 
  int x; 
  int y; 
  string name;
} SPOT;  // defines a struct type named "SPOT"
...
SPOT myspot; // creates an uninitalized SPOT struct named "myspot"SPOT* pspot; // creates an uninitalized pointer to a SPOT struct
...
myspot.x = 10;
myspot.name = "test!";
```

A struct can contain [pointers](apointer.htm) to previously defined structs, and even pointers on itself, but no pointers to later defined structs: 

```c
typedef struct SPOT { 
  int x; 
  int y; 
  string name;
  struct SPOT* next; // pointer to a SPOT struct
} SPOT;  // defines a struct type named "SPOT"
```

In lite-C, struct pointers can be initialized to a static struct. Example**:**

```c
SPOT* myspot = { x = 1; y = 2; name = "my struct"; }  
// creates a new SPOT struct with initial values that is then referenced through the myspot pointer
```

**** Working with structs is explained in any C/C++ book, so we won't cover it here in detail. For creating structs or arrays of structs at run time, the standard C library functions **[malloc](sys_malloc.htm)** and **[free](sys_free.htm)** can be used. For initializing or copying structs, use the C library functions **memset()** and **memcpy()** :

```c
function foo()
{
  SPOT myspot; 
  memset(myspot,0,sizeof(myspot)); // set the struct content to zero (it's undefined after malloc)
  myspot.x = 1;  
  SPOT* spot_array = malloc(100*sizeof(myspot)); // creates an array of 100 SPOT structs
  memcpy(&spot_array[0],myspot,sizeof(myspot));  // copy the myspot struct to the first member of the array
  ...
  free(myspot_array); // removes the created structs
}
```

**** !!  There is plenty information about C, C++, or Windows library functions on the Internet or in online C courses. It is highly recommended for advanced lite-C programming to work through such a course or book and learn about **malloc** , **memcpy** and all the other library functions that you can use. 

## sizeof(struct)

The **sizeof()** macro **** that is used in the example above **** gives  the size of a variable or a struct instance in bytes. This can be used to initialize structs: 

```c
#define zero(struct) memset((void*)&struct,0,sizeof(struct))
...
SPOT speed;
zero(speed);	// initializes the SPOT struct "speed" to zero
```

!!  Unlike C/C++, **sizeof(some struct)** requires that at least one instance of the struct exists. Otherwise **sizeof** will return 0.

!!  Arrays are internally treated as a pointer to a memory area. So, **sizeof(some array)** and **sizeof(some pointer)** always  evaluates to **4** because that is the size of a pointer.

### See also:

[Variables](aarray.htm), [pointers](apointer.htm), [functions](function.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
