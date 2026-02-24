---
title: exec
url: https://zorro-project.com/manual/en/exec.htm
category: Functions
subcategory: None
related_pages:
- [keys](keys.htm)
- [Python Bridge](python.htm)
- [window](window.htm)
---

# exec

## **** exec(string Program, string Options, int Flags)

Execute an external program, open a document or URL, or run a script or batch file. 

### Parameters:

**Program** \- file name of the **** exe, batch file, document, URL to be opened. Use **"Editor"** for opening the script editor, **"Zorro"** for another Zorro instance.  
**Options** \- command line parameter string to be passed to the program, **** or **0** for no command line options.   
**Flags** \- **1** for waiting until the external program was terminated, **2** for hiding its window, otherwise **0**. 

### Returns:

**0** if the program was not found or could not be started, otherwise the return code of the program in mode **1** , or the process ID in mode **0**. 

### Remarks:

  * The **Program** parameter can specify a full path (from the root), a partial path (from the Zorro folder), or just a filename. In the latter case the **exec** function first looks for the file in the Zorro folder, and then in the in the folders specified by the system's PATH environment variable.
  * ****If the**Program** string does not have a filename extension, the **exec** function first tries the .COM extension, then the .EXE extension, then the .BAT extension, and finally the .CMD extension.
  * If the **Program** string contains a URL or the name of a document, the standard internet browser or the standard editor for that document is opened.
  * **'\'** characters in strings, like for file paths, have to be given in C-Notation as **"\\\"****.**
  * The external program can be controlled with the [keys](keys.htm) function. 
  * Data can be exchanged with the executed program through a text file; an example can be found under [Python](python.htm).  

### Examples (see also [Python](python.htm)):

```c
exec("notepad","test.txt",0); // open notepad
exec("Editor",strf("Log\\%s.csv",Script),0); // open a .csv file in the Log folder with the editor
exec("c:\\program files\\internet explorer\\iexplore.exe","https://zorro-project.com",0); // open an URL with Internet Explorer
exec("https://zorro-project.com",0,0); // open an URL with the standard browser
exec("Zorro","-run Myscript.c",1); // run a Zorro script in another instance
exec("Zorro",strf("-train %s",Script),0); // train current script in another Zorro instance
```

### See also:

[window](window.htm), [keys](keys.htm) [► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
