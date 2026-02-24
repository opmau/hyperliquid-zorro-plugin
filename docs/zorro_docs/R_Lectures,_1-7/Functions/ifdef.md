---
title: #ifdef, #ifndef, #else, #endif
url: https://zorro-project.com/manual/en/ifdef.htm
category: R Lectures, 1-7
subcategory: Functions
related_pages:
- [#define, #undef](define.htm)
---

# #ifdef, #ifndef, #else, #endif

## #ifdef name

## #ifndef name

## #else

## #endif

Defined names can be used to to skip certain script lines dependent on previous [#define](define.htm)s. All script lines between **#ifdef** and **#endif** are skipped if **name** was not defined. Likewise, all lines between **#ifndef** and **#endif** are skipped if **name** was #defined. The **#else** statement reverses the line skipping or non-skipping. 

### Example: 

```c
#define SCALPING... #ifdef SCALPING  aut.nBarMinutes = 0;#endif
```

### See also:

[#define](define.htm) [► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
