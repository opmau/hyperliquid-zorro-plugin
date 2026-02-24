---
title: putvar, getvar
url: https://zorro-project.com/manual/en/putvar.htm
category: Functions
subcategory: None
related_pages:
- [lock, unlock](lock.htm)
- [File access](file_.htm)
- [String handling](str_.htm)
---

# putvar, getvar

## putvar (string FileName, string VarName, var Value) 

## getvar (string FileName, string VarName): var

Writes, updates, or reads a variable to or from a file or the Windows registry. Can be used for storing data globally or exchanging data between different scripts or Zorro instances. 

### Parameters:

**FileName** | Name of the file. If it does not exist, it is created. If **0** , the variable is stored in the Windows registry.  
---|---  
**VarName** | Name of the variable.   
**Value** | Value to be written or updated.   
  
### Returns:

Current variable value (**getvar**). If the variable does not yet exist, **0** is returned. 

### Remarks:

  * If several Zorro instances access the same variable at the same time, it should be [locked](lock.htm) between reading and writing. 
  * The variables are stored in a normal text file that can be viewed with a text editor or spreadsheet program. But it must not be manually edited or modified unless it is ensured that the file size does not change. Any variable in the file has a fixed line length for speedier access.

### Example:

```c
function increase_global_counter(string Name)
{
  string FileName = "\\Log\\Counters.csv";
  lock();
  int Counter = getvar(FileName,Name);
  putvar(FileName,Name,Counter+1);
  unlock();
}
```

### See also:

[file](file_.htm), [strvar](str_.htm), [lock](lock.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
