---
title: Zorro command line
url: https://zorro-project.com/manual/en/command.htm
category: Main Topics
subcategory: None
related_pages:
- [Zorro.ini Setup](ini.htm)
- [Included Scripts](scripts.htm)
- [Status flags](is.htm)
- [Multiple cores](numcores.htm)
- [Retraining](retraining.htm)
- [Licenses](restrictions.htm)
- [Error Messages](errors.htm)
- [Asset and Account Lists](account.htm)
- [Command](cmd.htm)
- [#define, #undef](define.htm)
- [report](report.htm)
- [set, is](mode.htm)
- [Verbose](verbose.htm)
- [ExitCode](exitcode.htm)
- [Testing](testing.htm)
- [Training](training.htm)
- [Live Trading](trading.htm)
---

# Zorro command line

# Automatization with the command line (Zorro S)

Zorro S can be started directly from an external program, a shortcut, a batch file, the Windows command shell, or a PHP **exec** call on a Windows server. In this way, Zorro training or tests can be automatized. The command line looks like this:

**"C:\Users\YourName\Zorro\Zorro.exe"_[filename]_ ****_[options]_**

For starting it manually with command line options,  use the Windows command prompt, navigate to your Zorro folder (**cd** command), type in a command line, for instance **Zorro -run MyScript** , and hit the [**Enter**] key.

If only a script name is given with no further options, Zorro will select it in the **[Script]** scrollbox. **filename** (without blanks or special characters) must be either an existing script in the **StrategyFolder** given in [Zorro.ini](ini.htm), or a historical data file in **.t1** , **.t6** , or **.t8** format. If a historical data file is given, the [Chart](scripts.htm) or [History](scripts.htm) script is started with that file. When a script is started from the command line, the [COMMAND](is.htm) status flag is set. Several Zorro functions use this way to start other Zorro processes, f.i. for [multicore training](numcores.htm), [ retraining](retraining.htm), or [retesting](retraining.htm). External tools can also use the command line.

You can give a command line option either directly in the Windows command prompt, or with the Windows [Run] function, or by editing the properties of a Zorro shortcut on the Windows desktop. For this, right click the shortcut icon and select Properties (for icons in the task bar you need to hold the [Shift] key). Under the shortcut tab you'll see the Target field containing the exact location of Zorro.exe within quotation marks. Add command line options, such as '**-diag** ', after the last quotation mark, and save the modified shortcut with [Apply].

Besides the file name, the following command line options can be given ([Zorro S](restrictions.htm) only): 

### -run

Run the given script in [Test] mode, and exit afterwards. 

### -train

Run the given script in [Train] mode, and exit afterwards. 

### -trade

Run the given script in [Trade] mode.

### -edit

Select the script and open it in the editor. 

### -h

In combination with **-run** , **-train** , or **-trade** : run with minimized Zorro window. 

### -stay

In combination with **-run** , **-train** , or **-trade** : don't close Zorro afterwards. 

### -quiet

Don't open a message box when Zorro encounters a fatal [error](errors.htm); print the error in the message window instead. Don't wait 4 seconds after a command line run before closing the Zorro window. 

### -a assetname 

Select the given asset from the [Assets] scrollbox. 

### -c accountname 

Selects the account with the given name from a user defined [account list](account.htm). 

### -d string 

Stores the given string (no blanks or special characters) in the [Define](cmd.htm) string. In lite-C, it also generates a [#define](define.htm) statement. This way, a script started with the command line can get file names or other text based information, and in case of lite-C compile differently. 

### -u string 

Allows to enter more strings or other content that can be parsed from the command line with the [report](report.htm) funcrion. 

### -i number

Passes an integer number to the script that can be read through the [Command](cmd.htm) variable. Up to 4 numbers can be transferred to the script, each preceded by **"-i"**. This way, the same script started with the command line can behave in different ways.

### -x 

Compiles the selected script to an executable, like the [EXE](mode.htm) flag, and exit afterwards.  

### -diag

Starts Zorro in 'black box' mode, as if the [DIAG](verbose.htm) flag was set. Used for debugging the startup behavior. A message "diagnostics mode" will appear in the message window at startup, and the [DIAG](verbose.htm) flag is automatically set in subsequent scripts. Startup events are recorded to a file **"diag.txt"** in the Zorro main folder. Black box recording strongly reduces the program speed, so do not use this feature unnecessarily. 

### -w offset

Shift the Zorro window by the given number of pixels to the right.  
  
  
The command line can be evaluated with the [report](report.htm) function, so arbitrary user-defined command line options can be added. Use the [ ExitCode](exitcode.htm) variable for returning with a script-defined exit code or error level.  

### Examples

Start a re-training run with a strategy. 

```c
Zorro.exe -train MyStrategy
```

From outside the Zorro folder, run the script **pricedownload.c** with the selected asset **"USD/CAD"** in test mode.

```c
"c:\Users\MyName\Zorro\Zorro.exe" -run pricedownload -a USD/CAD
```

Start Zorro in diagnostics mode. A file **diag.txt** is generated in the Zorro folder.

```c
Zorro.exe -diag
```

A **.bat** file in the Zorro folder that trains 3 scripts when clicked on.

```c
Zorro -train MyStrategy1
Zorro -train MyStrategy2
Zorro -train MyStrategy3
```

A **.bat** file that runs a Zorro script 50 times in 2 nested loops and passes the loop numbers to **[Command[0]](cmd.htm)** and **[Command[1]](cmd.htm)**.

```c
for /l %%x in (1, 1, 5) do (
for /l %%y in (1, 1, 10) do (
  @echo Loop %%x %%y
  Zorro -run MyScript -i %%x -i %%y
)
)
pause
```

A **.bat** file that executes a script with any **.csv** file name in a certain directory passed to the [ Define](cmd.htm) string:

```c
for %%f in (Data\MyFolder\*.csv) do Zorro -run MyScript -d %%f
```

### See also: 

[Testingg](testing.htm), [Training](training.htm), [Trading](trading.htm), [Zorro.ini](ini.htm), [ExitCode](exitcode.htm), [report](report.htm), [Command](cmd.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
