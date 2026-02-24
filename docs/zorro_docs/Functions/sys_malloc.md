---
title: malloc
url: https://zorro-project.com/manual/en/sys_malloc.htm
category: Functions
subcategory: None
related_pages:
- [evaluate](evaluate.htm)
- [HTTP functions](http.htm)
- [Structs](structs.htm)
- [memory](memory.htm)
---

# malloc

# C Memory Functions

## malloc(long size): void* 

Allocates a contiguous memory area with the given size in bytes. This can be used to create arrays of dynamic size. This function is slow, so it is recommended to use it for allocating memory areas in the initial run.  If there is not enough memory, **0** is returned. 

## realloc(void* ptr, long size): void* 

Reallocates the memory area given by **ptr**. It must have been previously allocated by **malloc**. Returns a pointer to the new memory area. The old memory area is freed, the old content is copied over to the new area. This function is slow. If there is not enough memory, the old memory area is not freed and **0** is returned.

## zalloc(long size): void* (temporary)

Like **malloc** , but returns a pointer to a temporary memory area that is initialized to zero and preserved until the next **zalloc** call. This function is fast, and the memory area needs not be freed. Use this for allocating a temporary array inside functions. If you need mutliple arrays, use only a singe **zalloc** call and divide the area among the arrays. 

### Parameters:

**size** \- Size of the allocated memory area, in bytes. 

### Returns:

**void*** pointer to the allocated memory area, or **0** when the allocation failed.  

## free(void* ptr)

Releases a contiguous memory area that was allocated with [malloc](sys_malloc.htm). 

### Parameter:

**ptr - void*** pointer to the allocated memory area.   
  

## memcpy(void* dest, void* src, long size)

Copies **size** bytes from **src** to **dest**. Can be used to copy a whole struct or array with a single function. 

### Parameters:

**dest** \- Target address.   
**src -** Source address.  
**size** \- Number of bytes to copy.   
  

## memset(void* dest, int c, long size)

Sets the first **size** characters of **dest** to the character **c**. Can be used to initialize a whole struct or array to zero. 

### Parameters:

**dest** \- Target address.   
**c -** value to copy, 0..255.  
**size** \- Number of bytes to copy.   
  

## memcmp(void* buffer1, void* buffer2, long size)

Compares **buffer1** with **buffer2**. Returns **0** when both are identical, otherwise nonzero. 

### Parameters:

**buffer1** \- First buffer.   
**buffer2** \- Second buffer.   
**size** \- Number of bytes to compare.   

### Remarks:

  * Memory allocated in a function with **malloc** must be released \- normally at the end of the function - with **free**. This is not needed when allocating memory with **zalloc**. For conveniently freeing globally allocated memory areas, define a **[cleanup](evaluate.htm)** function that releases the memory. 
  * It is recommended to set pointers to released memory areas to zero. Accessing a memory area after it's released will result in a crash. 
  * Check the allocation size carefully. Exceeding the allocated size will result in a delayed crash, usually when freeing the memory.

### Example (see also [http](http.htm)):

```c
var calculate_something()
{
  var *Buffer = zalloc(100*sizeof(var));
  int i;
  for(i=1; i<100; i++)
    buffer[i] = buffer[i-1]+1;
  return Buffer[99]; // last element
}
```

### See also:

[sizeof](structs.htm), [memory](memory.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
