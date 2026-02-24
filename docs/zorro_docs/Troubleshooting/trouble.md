---
title: Troubleshooting
url: https://zorro-project.com/manual/en/trouble.htm
category: Troubleshooting
subcategory: None
related_pages:
- [Error Messages](errors.htm)
- [Strategy Coding, 1-8](tutorial_var.htm)
- [invalid](invalid.htm)
- [Log Messages](log.htm)
- [FXCM](fxcm.htm)
- [IB / TWS Bridge](ib.htm)
- [Oanda](oanda.htm)
- [MT4 / MT5 Bridge](mt4plugin.htm)
- [F.A.Q.](afaq.htm)
- [Expressions](expressions.htm)
- [watch](watch.htm)
- [Visual Studio](dlls.htm)
- [set, is](mode.htm)
- [Variables](aarray.htm)
- [series](series.htm)
- [trade management](trade.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [Functions](function.htm)
- [contract, ...](contract.htm)
- [Stop, Profit, Trail, Entry, ...](stop.htm)
- [Multiple cycles](numtotalcycles.htm)
- [What's New?](new.htm)
- [printf, print, msg](printf.htm)
- [Chart Viewer / Debugger](chart.htm)
- [Verbose](verbose.htm)
- [Dates, holidays, market](date.htm)
- [wait](sleep.htm)
- [Command Line Options](command.htm)
---

# Troubleshooting

# Troubleshooting. Bugs and fixing them. Support.

A script is a simple text file, so you can type anything in it, even complete nonsense. But even scripts that look perfectly fine at first glance can behave very different than the programmer intended. In the simplest case, the compiler throws an [error message](errors.htm) and displays the faulty line. You should be able to fix that unaided. Otherwise just go again through the [C tutorial](tutorial_var.htm). 

In less trivial cases you'll see an error message when the script is running, and it may be not as informative. The worst case is code that gives no error message at all, but just does not do what it should, or even crashes or freezes. Don't panic: Any programmer encounters such issues all the time. Beginners can often not imagine that they made a mistake, and suspect a [Zorro bug](bugs.htm). Seasoned programmers are used to bugs in their code and know how to find and fix them. So get out of beginner mode as soon as possible. Utilize the many books and online seminars about the C language and script writing with Zorro. If you cannof find a bug in your script, check also your  historical data \- maybe it has gaps, outliers, or irregular time stamps that cause strange results. If you're really stuck, we provide a script fixing service. But first try to solve the problem on your own.

### Clean code

The best way to avoid bugs is good coding style that allows quick reading and understanding the code. This should be no problem with strategy scripts, since they are normally very short and have a linear structure. Still, even a 20-line script can be written as a cluttered lump of spaghetti code. Some suggestions for avoiding bugs already when writing the script:

  * Have a naming convention for variables and functions. In the script examples, usually definitions are all caps (**DO_TEST**), variables begin with uppercase letters (**Price**) and functions with lowercase letters (**price()**). Variables with multiple elements, such as series, end with 's' (**vars Prices**). Some exceptions are by tradition, like lowercase one-letter variables (**int i**) and uppercase traditional indicators (**EMA**). Whatever naming convention you use, be familiar with it and stick to it.  

  * Keep similar things together. For instance, set up similar variables at one place, define all **series** at another place, put all **optimize** calls together, and open all trades at the same place in the code.   

  * Be careful about the order of statements. A script is run from begin to end, so statements that affect other statements must be placed earlier. Set up parameters that affect the generating of bars, such as **BarPeriod** or the **TICKS** flag, at the begin of your script _before_ loading assets. Select the asset _before_ setting asset specific parameters such as **MarginCost** or **Spread**. Set up trade parameters, such as **Stop** or **LifeTime** ,_ before_ entering the trade. Statements in wrong order have either no effect, or will produce error messages.
  * When functions open, create, or load something, always check for nonzero the returned pointer or handle, and manage the case that the requested file, trade, contract, or memory area was not found, not opened, or not available. 
  * Do not assume that prices are always available.  Do not divide by a price or indicator without making sure it's nonzero. Zorro has a convenient [fix0](invalid.htm) function for values of which you are not sure that they are always nonzero.
  * Make the script behavior transparent. Print any event in the log, such as any signal or condition that affects opening or closing trades. This way you can understand at a glance which variable values led to which trade decision in the backtest.  

  * Run the script at least once with **Verbose = 2** or **Verbose = 3** settings. Even though you'll then get a very verbose log file, you'll see more warnings about suspicious script behavior. **Verbose** at or above**2** will also print out function calls with invalid parameters.

### Error messages

Error messages at compiling the script are syntax errors. The line in question is printed in the error message. This makes those errors easy to identify and fix, even for a beginner. If you cannot find the error in the displayed line, scan the previous part of the code for missing brackets or semicolons. You should not need help with fixing syntax errors. If you do, start over with a C tutorial or book.

Typical causes of syntax errors:

  * Something is mistyped
  * A bracket, quotation mark, or semicolon is missing (often in a previous line!), 
  * A string contains a single backslash '\' instead of a double backslash '\\\', also often in a previous line,
  * An object got the same name as a pre-defined function or variable (all names can be found in **include\functions.h** and **include\variables.h**),
  * A variable had a wrong type,
  * A function had a wrong number or type of parameters. 

If an external DLL does not load, check if it a) exists, b) can load all needed modules (=> use Dependency Walker), and c) is not locked or access restricted by Windows. 

If the script compiles with no error messages, it can still have bugs and produce runtime errors (like **"Error 123: ...."**). Under [error messages](errors.htm) you'll find a list of all such errors and warnings, and their likely reasons. Most runtime errors will terminate the script; some are mere hints that something in your code is likely wrong and you should look into it. Messages related to opening and closing positions are listed under [Log](log.htm); known issues with brokers are described on the related page ([FXCM](fxcm.htm), [IB](ib.htm), [Oanda](oanda.htm), [MT4](mt4plugin.htm), etc). The answers to many typical issues of script development can be found in the [FAQ](afaq.htm) list. 

If you see a strange number like like **"1.#J"** or **"1.#IND"** in the log or message window, or got [Error 037](errors.htm), [ Error 011](errors.htm), or similar errors, you're likely having an error in an [expression](expressions.htm). Such as a division by zero, or the square root of a negative number. A quick workaround for division by zero errors is replacing **A / B** with **A / fix0(B)**. 

If you see **"(Null)"** in the log, it's usually attempting to print a nonexisting string.

If you see a runtime error message in the message window, but have no clue at which part of your code the problem happens, the quickest way to find it is placing **[watch("?...")](watch.htm)** statements at suspicious places. The given parameter is then displayed at the end of the error mesage. For instance, **watch("?10")** will add a **(10)** at the end of all subsequent error messages, and **watch("?TEST")** will add **(TEST)**. You can remove the statements when the error is found and fixed.

A great alternative to **watch** statements is writing your code in C++. You can then use the **Microsoft™ Visual Debugger** for single stepping through the code and observing its behavior directly. For complex strategies with more than 100 lines of code, C++ is strongly recommended. See [Coding in C++](dlls.htm).

If you trade live and see an error message beginning with an exclamation mark **"!"** , it's a message from your broker, often giving the reason for not opening or closing a trade. For details, see [Log](log.htm) about a list of all possible messages in a live trading session. 

### Unexpected results or differences

If backtest results seem not to be right, first look into the log ([LOGFILE](mode.htm) must be set). Select one or two trades and check them from begin to end. Script parameters can affect trade results in many different ways - make sure to check them all, and also read the remarks on their manual pages. Look for outlier trades with extremely high profits or losses. If needed, make sure that your script can deal with events such as price shocks or [stock splits](outlier.htm).

Most typical coding errors are easy to spot:

  * Confusing **[var](aarray.htm)** and **[vars](series.htm)** (the classic)
  * Using [trade variables](trade.htm) without a trade (another classic)
  * Reading or writing past the end of a [series](series.htm)
  * A too short [LookBack](lookback.htm) period for an indicator
  * Forgetting the parentheses of [function calls](function.htm)
  * Using [contracts](contract.htm) without loading a contract chain
  * Setting [Stop](stop.htm) after entering a trade, instead of before

Other errors are not as easy to find. Typical problems:

  * A small change to the script has a large and unexpected effect on the backtest. This can be a random effect - in that case, your strategy is probably not good - or it has a particular reason. First, run a test of the original and of the changed script, or run it twice with and without change with a different [LogNumber](numtotalcycles.htm). If necessary, prevent training by temporarily disabling the **PARAMETERS** , **RULES** , or **FACTORS** flags. Also disable reinvesting, which can cause tiny profit differences to strongly affect the equtiy curve. Then compare the two logs, preferably with a compare tool such as BeyondCompare™. You can then directly see where prices, costs, or signals start to differ. If trades opened and closed at different time, the modification likely affected the trade signals. If the trades entered and exited at the same time but with different prices, the modification might have affected slippage, spread, or bar composition. If entry and exit times and prices are the same, but the profit is different, something was changed with the volume or trading costs.  

  * A new Zorro version produces a very different result in the backtest. This is often a hint that something is wrong with the script. Check first under [What's new](new.htm) the list of differences that might have an effect on test results, such as a modified indicator algorithm or performance metric. Also check if asset lists have been updated, resulting in different spreads and transaction costs. Otherwise look for the reason of the differences by running the old and new Zorro version from two different folders and comparing both logs with a compare tool, as described above.   

  * A part of the code seems to have no effect. If it's code after a conditional statement, check if the condition is ever fulfilled, for instance by placing [watch](watch.htm) or [printf](printf.htm) statements behind it. If the code writes something into a file or on a chart, check if the file is accessible (not opened in another application) and if the parameters to the plot command are correct.

### Bugs

You want your script to do something, but it refuses to obey and does something else instead. That's a bug. The worst bugs are those that go unnoticed. That's why you should always carefully check the log, even if you don't get any error messages and all seems ok. If you notice something wrong in the log - for instance, the script trades too often, or not often enough, or at the wrong time - first go through your code. Sometimes you will directly see what's wrong, sometimes not. Then you must debug it. 

Debugging strategy scripts is a special case of program debugging. You're normally interested in the behaviour of a variable or signal from bar to bar. There are several methods to observe the behavior of variables, either from code line to code line, or from bar to bar, or during the whole test run: 

  * Use the [Zorro Debugger](chart.htm) to single-step through parts of code, or through loops, while observing variables and trade signals. Place [watch](watch.htm)**("!...",...)** statements with exclamation mark and with the variables to observe inside the loop to check, or behind suspicious code lines.   

  * Set the [LOGFILE](mode.htm) flag, and set [Verbose](verbose.htm) to a high value, like **3** or **7**. Use [printf](printf.htm) statements to print variables and other information to the log file. Run a test, open the **.log** file and examine the trades in detail. If the problem happens only when training a script, use **print(TO_FILE,...)** for printing information to the training log.   

  * Display the behavior of suspicious variables or indicators in the message window or as curves on the chart. For instance, in order to check the overall behavior of a variable **TradeSignal** that determines when a trade is triggered, use this line: **plot("TradeSignal", TradeSignal, NEW, RED);.**

![Single Step Debugging](../images/debugging2.png)  
A strategy in the visual debugger

  * For testing the live trading behavior with no broker connection, use the **Offline** plugin. A live broker connection with a certain behavior can be simulated with the **Simulation** plugin that's available in the Source folder. Since live testing can take a long time when you need to wait for a particular day or hour, use the [DayOffset](date.htm) variable to skip wait periods. For live testing in 'fast forward' mode, add a certain time amount to **DayOffset** at any bar or in regular intervals. Examples: 

```c
DayOffset += 100./1440;  // in a tock function: fast forward by factor 100
DayOffset += 10*BarPeriod/1440; // in the run function: fast forward by factor 10
DayOffset = FRIDAY - dow(NOW); // pretend it's Friday
```

  * For debugging the script with Visual Studio™, convert it to a [C++ project](dlls.htm), then use the VS Debugger for stepping into the code. 

### Crashes and freezes

They seem more serious, but are often easier to identify and fix than the bugs that only cause wrong behavior. You can encounter three types of crashes:

  * Zorro stops the script with a message like **"Crash in (_function name_)"**. Typical crash reasons: a numeric error, like taking the square root of a negative number, or dividing by zero; calling **printf** or **strf** with missing or wrong arguments; exceeding the length of an array, string, or buffer, either by script or already in the definition; wrongly assuming that a string or pointer returned by a function (like **ThisTrade**) is nonzero and its elements accessible; or exceeding the stack size by using too large local arrays. If a crash is caused by a memory error, Zorro must be restarted.   

  * Zorro terminates suddenly either without any further message or with a Windows Runtime Error. This is usually caused by a preceding script crash or by script code that overwrites internal structs or accesses array elements past the end of the array. Other possible causes are a crash in an external module, such as the broker API or some system component, a malfunctioning malware program, a faulty Windows library, running out of memory, or even a PC hardware issue.   

  * Zorro freezes permanently and must be manually closed with the Task Manager. The reasons can be the same as for a sudden termination. If they are caused by the script, look for a never-terminating **while** , **do** , or **for** loop in the script, or some endless recursion, such as a trade that opens further trades in its TMF. 

For fixing a freeze, first check all loops and recursions in your script and make sure that they terminate. Loops waiting for user input should check with a [wait()](sleep.htm) call if the [Stop] button was hit. Most crashes are normally easy to fix when they happen regularly or always at the same bar or script position. Typical causes of crashes:

  * [Printing](printf.htm) a string with wrong variable types, such as **float** instead of **double**
  * Accessing elements of nonexistent objects, such as trade or contract variables
  * Reading or writing past the end of a [series](series.htm) or of an array
  * Calling a wrong defined or empty [function](function.htm)

Crashes can be harder to find when they happen irregularly and their reason is not immediately obvious. Zorro has several mechanisms for detecting such problems:

  * For finding out at which line in your script a crash happened, place **watch("?...")** statements before suspicious places. The text after the question mark is then displayed at the end of the crash message. For instance, **watch("?10");** will add a **(10)** at the end of subsequent crash messages, and **watch("?TEST");** will add **(TEST)**. You can remove the statements when the error is fixed.  

  * Set [Verbose](verbose.htm) to **15** or **31** in the script. This activates diagnostics mode. If you have no access to the script, alternatively start Zorro with the [-diag](command.htm) command line option. Let then Zorro run with the script in question until it crashes again. You'll now see a file ending with **"..diag.txt"** in the **Log** folder. This file is a 'black box' recorder containing the last 1000 internal events or [printf](printf.htm) or [watch](watch.htm) commands. The last line in the file is the last event immediately before the crash.  

  * For observing variables that possibly cause a crash, use the diagnostics mode and place [watch](watch.htm)**("#...")** or [printf](printf.htm)**("#\n...")** statements between the lines. You can then see from the last such printed line in the log file or the **diag.txt** file with which variable values and at which line the crash happened.   

  * The final method to fix a crash in a script is out-commenting parts of the script until it does not crash anymore. The bug leading to the crash can then usually be found in the last outcommented lines. 

If the problem is not caused by the script - for instance, when it happend while trading a Z system - you need to look for an external reason. Activate diagnostics mode as above. If the program then freezes or crashes again, you can see in the last line of the **diag.txt** at which place it happened. This can indicate which module on your PC is possibly malfunctioning. There are websites and forums on the Internet with more hints for PC troubleshooting. 

### Zorro bugs

The hopefully least likely reason of script troubles is a bug in Zorro. Look first in the [bug list](bugs.htm). If you think you've encountered a new Zorro bug that no one else has seen before, please report it to **support@opgroup.de**. Please describe what's happening under which circumstances and how to reproduce the problem; please include your script, the log, and all related data such as asset list and asset history. Do NOT send screenshots, videos, or photos that you took from your monitor (unless we ask for them). You need no support ticket for bug reports. If the bug can be confirmed, it will appear on the bug list and will normally be fixed within a few days.

Don't be disappointed when your bug report is quickly returned with the remark "no Zorro bug", since this happens with most of them. You can then safely assume that you'll be able to fix that bug. Use the methods described above, or hire our script fixing service.

### Getting help

If you're really stuck, you can subscribe a [support ticket](https://zorro-project.com/docs.php) and contact **support** @**opgroup.de**. There are two levels of support. Standard support will answer Zorro-related questions by email usually within 24 hours on business days, and suggest solutions for technical problems.  Priority support helps with coding and with urgent problems. It includes reviewing small code snippets and providing answers by Skype chat, usually within 2 hours from 8:00 - 18:00 CET on business days. The subscriptions can be cancelled anytime. Licenses include free standard support periods.

You don't need a support ticket:

  * during the free support period of a Zorro S license,
  * for contributions to the user community,
  * for general questions about licensing, 
  * for suggestions or feature requests, 
  * for reporting a documentation error or a Zorro bug,
  * or when the needed information is missing in this manual. 

Zorro support does not cover teaching the C language or debugging, fixing, or programming your script. For C, there are lots of books, tutorials, and online courses available. For fixing your script, you can hire our script fixing service at **support** @**opgroup.de**. Depending on code complexity, it takes usually 1-2 hours to find and fix a bug. You can also hire a service for installing a ready-to-run system on your PC or VPS, or for writing specific asset lists and data conversion scripts. The most frequent support questions are listed [here](afaq.htm). 

### See also:

[watch](watch.htm), [debugger](chart.htm), [error messages](errors.htm), [bug list](bugs.htm)  
[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
