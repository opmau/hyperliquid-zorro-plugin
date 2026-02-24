---
title: version
url: https://zorro-project.com/manual/en/version.htm
category: Functions
subcategory: None
related_pages:
- [Licenses](restrictions.htm)
- [printf, print, msg](printf.htm)
- [run](run.htm)
- [wait](sleep.htm)
- [quit](quit.htm)
- [Status flags](is.htm)
---

# version

## version(): var 

Returns Zorro's version number with two digits behind the decimal. 

## require(var Version): int 

Returns nonzero when Zorro's version number is equal or higher than **Version**. Otherwise prints an error message, terminates the session, and returns 0. Pass a negative **Version** number when [Zorro S](restrictions.htm) is also required. 

### Examples:

```c
if(version() < 2.14)
  return quit("Zorro 2.14 or above needed for this script!");

if(!require(-2.14)) return; // need Zorro S 2.14 or above
```

### See also:

[printf](printf.htm), [run](run.htm), [wait](sleep.htm), [quit](quit.htm), [SPONSORED](is.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
