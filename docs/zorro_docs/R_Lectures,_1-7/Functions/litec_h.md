---
title: Headers
url: https://zorro-project.com/manual/en/litec_h.htm
category: R Lectures, 1-7
subcategory: Functions
related_pages:
- [R Bridge](rbridge.htm)
- [contract, ...](contract.htm)
- [plotProfile, ...](profile.htm)
- [Visual Studio](dlls.htm)
- [DLLs and APIs](litec_api.htm)
- [Included Scripts](scripts.htm)
---

# Headers

# Header files

The following standard header files are used in Zorro strategy scripts and/or in C++ strategy DLLs:

### #include <litec.h>

Standard header for lite-C, contains standard variable type definitions. Automatically included in **default.c**. 

### #include <trading.h>

Standard header for strategy scripts, with definitions of the structs **GLOBALS** , **TRADE** , **ASSET** , **T6** , etc. Automatically included in **default.c** and **zorro.h**. 

### #include <variables.h>

Standard header for strategy scripts; contains #defines for all predefined trading variables. This file 'translates' the struct members into easier to memorize variable names. Automatically included in **default.c** and **zorro.h.**

### #include <functions.h>

Standard header for strategy scripts, with definitions of all functions. Automatically included in **default.c** and **zorro.h**. 

### #include <default.c>

Standard header for lite-C strategy scripts; includes all the headers above. Automatically included when no other **#include** statement is found in the main script. Otherwise it must be explicitly included. 

### #include <windows.h>

Optional lite-C header; contains common definitions and functions from the Windows API. Requires including **< default.c>** when trading functions are used. 

### #include <stdio.h>

Optional lite-C header; contains all usual file, directory, and sort/search functions from the standard C libraries **io.h** , **stdio.h** , **stdlib.h** , **direct.h**. Requires including **< default.c>** when trading functions are used. 

### #include <r.h>

Optional header for the [R bridge](rbridge.htm). Can be included in lite-C scripts and/or in C++ strategy DLLs. 

### #include <contract.c>

Optional header for [contract](contract.htm) functions.Can be included in lite-C scripts and/or in C++ strategy DLLs. 

### #include <profile.c>

Optional lite-C header for [histogram plotting](profile.htm) functions. 

### #include <zorro.h>

Header for [C++ strategy DLLs](dlls.htm). Not for lite-C.

### #include <legacy.h>

Header for deprecated and outdated lite-C keywords, for backwards compatibility to very old Zorro versions. If your script contains a deprecated keyword, you can include this header. But better remove or replace it. 

### See also:

[API](litec_api.htm), [scripts](scripts.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
