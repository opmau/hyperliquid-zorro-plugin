---
title: File functions
url: https://zorro-project.com/manual/en/file_.htm
category: Functions
subcategory: None
related_pages:
- [Time / Calendar](month.htm)
- [keys](keys.htm)
- [String handling](str_.htm)
- [HTTP functions](http.htm)
- [FTP transfer](ftp.htm)
- [putvar, getvar](putvar.htm)
---

# File functions

# File functions

The following functions can be used to read, write, or otherwise handle files:

## file_copy (string Dest, string Src)

Copies the file with the name **Src** to the file with the name **Dest**. If the **Dest** file already exists, it is overwritten. If **Dest** is a folder ending with a backslash **'\'** , the file is copied into the destination folder. This function can also be used to rename files by storing them under a different name. 

### Parameters:

**Dest** \- destination folder or file path (either absolute, or relative to the Zorro folder, e.g. **"Log\\\MyStrategy.txt"**).   
**Src** \- source file path 

## file_delete (string name)

Deletes a file. 

### Parameters:

**name** \- file path   
  

## file_length (string name): size_t 

Checks if a file with the given name exists, and returns its length in bytes. If the file was not found, **0** is returned. 

### Parameters:

**name** \- file path 

## file_date (string name): long 

Checks if a file with the given name exists, and returns its last modification date in UTC as the number of seconds since January 1, 1970. The [ mtu](month.htm) function can convert it to the **DATE** format. If the file was not found, **0** is returned. 

### Parameters:

**name** \- file path  

## file_select (string path, string filter): string 

Opens a file dialog box at a given directory that lets the user select a file name to open or save. Returns the selected file name including path. The returned string keeps its content until the next **file_select** call. 

### Parameters:

**path** \- initial directory to open, f.i. **"Data"** , or **0** for opening the Zorro directory.  
**filter** \- list of pairs of null-terminated filter strings /see below), or a 3-character file extension, or **0** for selecting all files. If the filter string has a **'#'** appended at the begin, it generates a save dialog, otherwise an open dialog.   
If a filter pair list, the last string must be terminated by two null characters (**"\0\0"**). The first string in each pair describes the filter (f.i. **"Parameter Files"**), and the second string specifies the filter pattern (f.i. **"*.par"**). Example: **"CSV Files\0*.csv\0\0"**. Multiple filter patterns can be separated with a semicolon (f.i. **"*.par;*.fac;*.c"**). The pattern must not contain spaces. Example of a filter list with three pairs: **"All files (*.*)\0*.*\0Par, Fac\0*.par;*.fac\0Text files\0*.txt\0\0"**.  

## file_next (string path): string

Enumerate files in a directory. Returns the first file name (without directory path) that matches the given **path**. Returns the next match when **path** is 0 or an empty string. Returns **0** after the last matching file. 

### Parameters:

**path** \- file path with wildcards (**"Data\\\MyStrategy*.par"**) for the first file; **0** for subsequent files.  
  

## file_content (string name): string (temporary)

Returns a null-terminated temporary string with the content of the file, or **0** when the file was not found. The predefined **var rLength** is set to the length of the file. The string keeps its content only until the next **file_content** call. For multiple files to be read at the same time, allocate buffers and use **file_read**. 

### Parameters:

**name** \- file path 

## file_read (string name, string content, size_t length): size_t 

Reads the content of a file into a string or array, and appends **0** at the end. Returns the number of read bytes. 

### Parameters:

**name** \- file path   
**content** \- **string** or array of any type to be filled with the content of the file; must have a size of at least **length+1**.   
**length** \- maximum number of characters or bytes to fill, or **0** for reading the whole file into a buffer of sufficient size. 

## file_write (string name, string content, size_t length)

Creates a file or folder, and stores the content of the given string or data array. 

### Parameters:

**name** \- file or folder path.  
**content** \- **string** or other data to be written, or **0** for creating a folder.  
**length** \- number of bytes to be written, or **0** for writing the complete content of a **string**. 

## file_append (string name, string content, size_t length)

## file_appendfront (string name, string content, size_t length)

Opens a file and appends text or other data either at the end (**file_append**) or at the begin (**file_appendfront**). If the file does not exist, it is created. 

### Parameters:

**name** \- file name with path.  
**content** \- text or other data to be appended at the end or begin of the file.   
**length** \- number of bytes to be written, or **0** for writing the content of a **string**.

## file_appendCSV (string name, string record, int field1, int field2)

Opens a CSV file and appends the given CSV record to the end. If the fields **field1** up to **field2** of the record match the corresponding fields in the file, the record is not appended, but replaces the original line in the file. In this way a CSV file can be updated with a new record. If the file does not exist, it is created. 

### Parameters:

**name** \- file name with path.  
**record** \- a line with comma separated fields to be appended or to replace a record in the file.  
**field1, field2** \- range of fields to compare. 

## file_sortCSV (string name, int field, int mode)

Opens a CSV file and sorts its records, except for the header line, by the numeric value in the given field. 

### Parameters:

**name** \- file name with path.  
**field** \- number ot fhe field with the sort criteria.  
**mode** \- **0** ascending, **1** descending sorting..  

### Remarks:

  * If a file to be read from or written into is not found, an error message will be printed to the message window, and the function returns 0. The error message can be suppressed by adding **'#'** at the begin of the file name (f.i. **file_content ("#myfilename.txt")**). 
  * Standard C file i/o functions - **fopen** , **fclose** , **fread** , **fwrite** , **_findfirst** , etc. - are also available through **#include <stdio.h>**.   

### Examples:

```c
//script for merging all WFO .par and .fac files
//from two strategies to build a combined strategy
 
string src1 = "Z1";    // first strategy, can be identical to dest
string src2 = "Z1add"; // second strategy, must be different to dest
string dest = "Z1";    // combined strategy

int file_merge(int n,string ext)
{
  char name1[40],name2[40],name3[40];
  if(n) {
    sprintf(name1,"Data\\%s_%i.%s",src1,n,ext);
    sprintf(name2,"Data\\%s_%i.%s",src2,n,ext);
    sprintf(name3,"Data\\%s_%i.%s",dest,n,ext);
  } else {
    sprintf(name1,"Data\\%s.%s",src1,ext);
    sprintf(name2,"Data\\%s.%s",src2,ext);
    sprintf(name3,"Data\\%s.%s",dest,ext);  
  }
  if(!file_date(name1)) 
    return 0; // file does not exist
  if(0 != strcmp(name3,name1))
    if(!file_copy(name3,name1))
      return 0;
  if(!file_append(name3,file_content(name2))) 
    return 0;
  return 1;
}

function main()
{
  int cycles = 1;
  for(; cycles < 100; cycles++)
    if(!file_merge(cycles,"par")) 
      break;
  
  cycles += file_merge(0,"par");
  cycles += file_merge(0,"fac");
  
  if(cycles > 3) 
    printf("%i files combined!",cycles);
  else
    printf("Error!");
}
```

```c
// set up strategy parameters from a .ini file
function run()
{
  static var Parameter1 = 0, Parameter2 = 0;
  if(is(INITRUN)) { // read the parameters only in the first run
    string setup = file_content("Strategy\\mysetup.ini");
    Parameter1 = strvar(setup,"Parameter1");
    Parameter2 = strvar(setup,"Parameter2");
  }
}
 
// mysetup.ini is a plain text file that contains
// the parameter values in a format like this:
Parameter1 = 123
Parameter2 = 456
```

### See also:

[keys](keys.htm), [sprintf](str_.htm), [string functions](str_.htm), [http functions](http.htm), [ftp functions](ftp.htm), [putvar](putvar.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
