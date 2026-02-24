---
title: string functions
url: https://zorro-project.com/manual/en/str_.htm
category: Functions
subcategory: None
related_pages:
- [Variables](aarray.htm)
- [Asset Symbols](symbol.htm)
- [Dataset handling](data.htm)
- [Formats Cheat Sheet](format.htm)
- [System strings](script.htm)
- [File access](file_.htm)
- [printf, print, msg](printf.htm)
- [Time / Calendar](month.htm)
- [putvar, getvar](putvar.htm)
---

# string functions

# String functions

The following functions - mostly from the standard C library - can be used to manipulate [strings](aarray.htm#string):

## strcpy (string dest, string src): string 

Fills the char array **dest** with the content of string **src**. The buffer size of **dest** must not be exceeded. 

### Parameters:

**dest** \- destination string, either a [char array](aarray.htm) of sufficient length or a temporary string as returned from **strmid** , **strf** , or **strx**.   
**src** \- source string 

## strcat (string dest, string src): string 

Appends a copy of **src** to the end of the string **dest**. The buffer size of **dest** must be of sufficient length.

## strcatf (string dest, string src): string 

Like **strcat** , but appends to the front of **dest**. Can be used to insert a string into another string. This is not standard C string function. 

### Parameters:

**dest** \- destination string, a [char array](aarray.htm) of sufficient length or a temporary string as returned from **strmid** , **strf** , or **strx**.   
**src** \- source string  

## strlen (string str): int 

Returns the number of characters in the given string. 

### Parameters:

**str** \- string 

## strcspn (string str,string set): int 

Returns the index of the first occurrence of a character that belongs to a set of characters. Can be used to clip a string or to copy it up to a delimiter. 

### Parameters:

**str** \- string  
**set** \- character set, f.i. **" ,\n"** for the first occurrence of either blank, comma, or line feed. 

## strcount (string str, char c): int 

Returns the number of occurrences of the character **c** in the given string. This is not a standard C string function. 

### Parameters:

**str** \- string  
**c** \- character to be counted, f.i. **0x0a** for a line feed.   
  

## strcmp (string str1, string str2): int 

## strncmp (string str1, string str2, int count): int 

Compares all or the first **count** characters of the strings (case sensitive) and returns **0** if they have the same content. For comparing strings with text constants and case insensivity, the **==** and **!=** operators can alternatively be used. 

### Parameters:

**str1** \- first string to compare  
**str2** \- second string to compare  
**count** \- number of characters to compare  
  

## strstr (string str1, string str2): string

Returns a substring beginning with the first occurrence (case sensitive) of **str2** within **str1** , or **NULL** when **str2** is not contained in **str1**. This function is often used to check if a string can be found in another string, or to parse string content by keywords. 

### Parameters:

**str1** \- string to search within  
**str2** \- substring to be found   

## strchr (string str, int c): string 

## strrchr (string str, int c): string 

Returns a substring beginning with the first (**strchr**) or last (**strrchr**) occurrence of the character **c** in the string **str** , or **NULL** when **c** is not contained in **str**. This function can be used f.i. for getting the extension (search for **'.'**) or the file name (search for **'** \\\') from a path name. Another use is checking if a certain character is contained in a set of characters. 

### Parameters:

**str** \- string to search within  
**c** \- character to be found  

## strtok (string str, string delimiter): string

Returns a substring from the string **str** , or **0** when no more substring is found. The set of characters in the **delimiter** string specifies the characters that separate the substrings. This function can be used to parse CSV or similar files. 

### Parameters:

**str** \- string to separate the first substring from, or **0** for continuing the search with the next substring of the last **str** string. The string is modified by replacing the delimiters with **0** bytes.   
**delimiter** \- character or set of characters that separate the substrings, f.i. **","** for the comma that separates fields in a CSV file, or **"\n"** for a line feed.  

## strvar (string str, string name, var default): var

Parses a number from the string **str** that is preceded by the identifier **name**. If no identifier **name** is found in the string, the function returns the **default** value. Can be used to parse **.csv** , **.ini** , or **.json** files. This is not a standard C string function. 

### Parameters:

**str** \- string containing the number.  
**name** \- substring, variable identifier, or **0** to read the first number out of the string.    
**default** \- returned when no identifier **name** or no number was found in the string. Use **NIL** for detecting **name**. 

## strtext (string str, string name _,_ string default): string (temporary) 

Parses text from the string **str** and returns a temporary string containing the text preceded by the identifier **name**. If the text contains spaces, it should be in double quotes inside **str**. If no identifier **name** is found in the string **str** , the function returns **default**. Can be used to parse **.ini** files or JSON strings. This is not a standard C string function. 

### Parameters:

**str** \- string containing the text to be extracted.  
**name** \- substring, text identifier. For a JSON token, put it in double quotes followd by a colon (f.i. **"\"datetime\":"**).  
**default** \- returned when no identifier **name** was found in the string. 

## strvarCSV (string str, string name _,_ int field, var default): var 

Like **strvar** , but parses the number from a subsequent field in a CSV file. Can be used to parse the content of CSV files This is not a standard C string function. 

## strfield (string str, string name, int field): string 

Like **strtext** ,, but returns a pointer to the content of a subsequent field in a CSV file. Can be used to parse or modify the content of CSV files This is not a standard C string function. 

### Parameters:

**str** \- string containing the text to be extracted.  
**name** \- substring, token identifier, or **0** for counting the fields from the begin of the string.  
**field** \- the number of commas in the CSV record to skip before returning the field content  
. 

## strtr (TRADE*): string

Returns a temporary string (see remarks) with a trade identifier, as in the log messages. This is not a standard C string function. 

### Parameters:

**TRADE** pointer.  

## strcon (CONTRACT*): string

Returns a temporary string (see remarks) with the [contract symbol](symbol.htm). This is not a standard C string function. 

### Parameters:

**CONTRACT** pointer.  

## strmid (string str, int first, int count): string

Returns a temporary string (see remarks) that contains a substring of **str** , with length **count** and starting at position **first**. This is not a standard C string function. 

### Parameters:

**str** \- string containing the source text.   
**first** \- position of the first character of the copy; **first = 0** copies the string from the begin.  
**count** \- number of characters in the copy; 1000 characters maximum. If **count** exceeds the length of **str** , it is copied until the end.  

## strxc (string str, char orig, char repl): string (temporary)

## strxc (string out, string str, char orig, char repl)

## strx (string str, string orig, string repl): string (temporary)

## strx (string out, int size, string str, string orig, string repl)

Returns a temporary string (see remarks) or fills an existing string with a copy of **str** where all ocurrences of **orig** are exchanged with **repl**. This is not a standard C string function. For cutting off a string at a certain character, replace it with 0 (f.i. **strxc(Name, '.', 0)** cuts off the extension from a file name). 

### Parameters:

**out** \- target string to be filled, or **str** for filling the source string if the size does noch change. A taget string have been allocated before and have sufficient size.  
**size** \- size of the target string if different, or **0** for the same size as the source string. **  
str** \- string containing the source text, 1000 characters maximum.   
**orig** \- original text or char to be exchanged, or **0** for inserting **repl** at the begin.  
**repl** \- replacement text or char.   

## strw (string str): short* (temporary)

Converts an 8-bit char string to a temporary 16-bit wide string, i.e. an array of shorts. Useful for passing small strings, such as file names, to DLLs that require 16-bit characters. This is not a standard C string function. 

### Parameters:

**str** \- string containing the text to be converted, 1000 characters max.  

## stridx (string str, _int index)_ : int

Assigns the given **str** string to the given **index** number, a unique integer in the 1..1000 range. Useful for indexing [arrays](aarray.htm) or [datasets](data.htm) with strings, such as asset or algo names. If **index** is **0** , the function generates and return an index This is not a standard C string function. 

### Parameters:

**str** \- string to be converted to an index, 15 characters max.  
**index** \- number associated to the string. 

## strxid (int index): string

Returns the string associated to the given **index**. Example: **int i = stridx("EUR/USD",0), j = stridx("USD/JPY",0); printf("\n%i %i %s %s",i,j,strxid(i),strxid(j));**. This is not a standard C string function.  

## strf (string format, ...): string (temporary)

Returns a temporary string (see remarks) with data and variables formatted by the **format** string (see [format codes)](format.htm). This is not a standard C string function. 

### Parameters:

**format** \- format string, limited to a single line (no **"\n"** characters) and to maximum 1000 characters of the returned string.  

## sprintf (string dest, string format, ...): int 

Like **strf** , but fills a given char array with formatted data, and returns the number of characters written. The **dest** char array must have sufficient size. 

### Parameters:

**dest** \- destination string, a [char array](aarray.htm) of sufficient length.   
**format** \- format string (see [format codes](format.htm)).  

## sscanf (string str, string format, ...): int 

Reads formatted data from a string into variable pointers or substrings according to the **[format](format.htm)** string (see example). Returns the number of variables read from the string. The execution time of this function depends on the string length, so don't use it for very long strings such as the content of a whole file. Special format codes are supported, f.i. **"%[^\n]"** reads a substring until the end of the line, or **"%[^,]"** parses a line into comma-separated strings. Details can be found in any C/C++ manual or online documentation.  !!  Unlike **sprintf** and **printf** , for reading floating point numbers with **%f** the target variable must be **float** ; **var** or **double** require **%lf**. 

### Parameters:

**str** \- string to parse.  
**format** \- format string similar to [format codes](format.htm), but with additional fields.   
  

## atof (string str): var 

## atoi (string str): int 

Converts a number from a string to a **var** or **int** variable. If the string does not begin with a valid number, **0** is returned. 

### Parameters:

**str** \- string containing a number.  

## sftoa (var Value, int Digits): string 

Returns **Value** converted to a temporary string with the given number of significant **Digits**. For instance **sftoa(0.00123456, 4)** will return **"0.001235"** , **sftoa(12.3456, 4)** will return **"12.35"** , and **sftoa(123,0)** will return **"123"**. A negative **Digits** number enforces the number of decimals. This is not a standard C string function. 

### Parameters:

**Value** \- value to convert.  
**Digits** \- number of nonzero digits when positive, decimals when negative.   

## strselect (string* List, int NunStrings): int

Opens a dialog to select one of the strings from the given **List** of strings. Returns the index of the selected string, or -1 if no string was selected.This is not a standard C string function. 

### Parameters:

**List** \- array of strings**  
NumStrings** \- number of strings in the array.  
  

### Remarks:

  * Most of the string functions are standard C functions; a more extensive description with examples can be found in C/C++ books or tutorials. Standard C functions like **strcpy** , **strcat** , **sprintf** are normally not fool proof. They will crash when the original length of the destination string is exceeded. 
  * More C standard string functions are available when including ** <stdio.h>**.
  * For convenience, some string functions (such as **strf**) return a temporary string. Temporary strings are taken from a string pool and have a maximum length of 1023 characters and a lifetime of 10 function calls, i.e. they are recycled and overwritten after 10 subsequent calls. They can be used for passing string parameters to other functions, but cannot be used when a permanent string is required, f.i. for setting one of the [predefined strings](script.htm). See the example below for converting a temporary string to a permanent string.
  * Use **strf** or **strx** for temporarily extending the length of a string, f.i. **MyTempString = strf("%s",MyOriginalString);**. The returned temporary string can be extended to a length of up to 1023 characters. 

### Examples (see also **[file functions](file_.htm)and Simulate.c**):

```c
// read a variable out of a string
// with a format like "name = 123.456"
var readVar(string str,string name)
{
  float f = 0; // sscanf needs float
  string s = strstr(str,name);
  if(s) s = strstr(s,"=");
  if(s) sscanf(s+1,"%f",&f);
  return f;
}

function main()
{
  var Test1 = 0,Test2 = 0;
  string s = file_content("Strategy\\vars.ini");
  if(s) {
    Test1 = readVar(s,"Test1");
    Test2 = readVar(s,"Test2");
  }	
  printf("\nTest1 = %f, Test2 = %f",Test1,Test2);
}
```

```c
// copy a temporary string to a permanent string
...
static char MyPermanentString[100];
strcpy(MyPermanentString,strx("I like Ninja!","Ninja","Zorro"));
...
```

```c
// open a dialog to select an algorithm
...
string Items[4];
Items[0] = "Trend";
Items[1] = "Counter Trend";
Items[2] = "Channel";
Items[3] = "Cycle";
int N = strselect(Items,4);
if(N<0) printf("Error");
else printf("\n%s selected!",Items[N]);
```

### See also:

[printf](printf.htm), [string](aarray.htm), [strdate](month.htm), [file_functions](file_.htm), [putvar](putvar.htm), [format codes](format.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
