---
title: R bridge for algo trading | Zorro project
url: https://zorro-project.com/manual/en/rbridge.htm
category: Main Topics
subcategory: None
related_pages:
- [HTTP functions](http.htm)
- [Zorro.ini Setup](ini.htm)
- [Status flags](is.htm)
- [wait](sleep.htm)
- [quit](quit.htm)
- [rev](rev.htm)
- [series](series.htm)
- [Walk-Forward optimization](numwfocycles.htm)
- [adviseLong, adviseShort](advisor.htm)
- [Migrating Scripts](conversion.htm)
- [DLLs and APIs](litec_api.htm)
---

# R bridge for algo trading | Zorro project

# R Bridge

The R bridge connects Zorro scripts and algorithmic trading systems to the **R** environment. It allows R computations and access to R packages in your C or C++ based script. 

R is an interactive script language for data analysis and charting, developed in 1996 by Ross Ihaka and Robert Gentleman of Oakland University as successor to the S language. The latter was developed by John M. Chambers and his colleagues in the Bell Labs company in 1976. Today, R is still being improved by the R Development Core Team including John Chambers. Although R is an interpreted language and thus slow compared to 'real' programming languages such as C, it has many advantages:

  * It is the global standard for statistical research, data analysis, and machine learning algorithms. 
  * It is interactive and easy to use once you're familiar with its command syntax. R functions are short and effective. 
  * It is 'Zorro-like' - it has a minimalistic user interface and can be easily automatized for experiments and research tasks. 
  * It has ready-to-use extensions - "packages" - for all imaginable mathematical and statistical tasks, including support vector machines, genetic optimization, several kinds of neural networks, and deep learning algorithms. Meanwhile there are more than 4000 R packages.
  * It is continuously developed and supported by the global scientific community. The time between a research publication and its implementation in a R package is usually very short. In average, 15 new R packages come out every day. 
  * It is free. As Linus Torvalds said: "Software is like sex - it's better when it's free".

For any imaginable algorithm there's normally already one or several R packages available. This makes R an excellent tool for financial research or analyzing financial time series, using the newest published methods. However, R is not a 'clean' programming language such as C - it is full of traps to the beginner, so using it effectively needs some experience. It's also not suited for trading (although some packages provide rudimentary backtest, optimization, and even broker connection functions!). The solution to this is the **R bridge** that allows including R computations in a Zorro strategy and using their results for trade signals. The lite-C script can start R sessions, send price data or indicator values to it, and call training and predicting functions from R machine learning packages. The bridge uses the **R.dll** by Bernd Kreuss. Its original distribution with source code is contained in the **Zorro\Source** folder. The code that accesses the DLL is located in the **r.h** header file.

Many problems can be solved in lite-C as well as in R, but R often has already a dedicated command implemented for that. For instance, downloading and converting historical price data from an online source requires about 10 code lines in lite-C ([here](http.htm)), but often only a single line in R (**read.csv("http://ichart.finance.yahoo.com/table.csv?...")**). 

### Installation and test 

  * Install R from [http://cran.r-project.org](http://cran.r-project.org/).   

  * Open **[ZorroFix.ini](ini.htm)** and make sure that the entry **RTermPath** contains the correct path to the R terminal **RTerm.exe**.  

  * Start Zorro and select the script **RTest**. Click [Test] for running **RTest**. If everything went well, you should see a result in your Zorro window, like this:  
  
![](../images/rbridge1.png)   

  * For using the R bridge in your scripts, include the**r.h** header at the begin of the **.c** or **.cpp** script. You need the 32 bit Zorro for running it - Zorro64 has no R bridge (yet). 

A thorough intro to using R with Zorro can be found on [ RobotWealth](https://robotwealth.com/integrating-r-with-the-zorro-backtesting-and-execution-platform/).

# R bridge functions 

The **r.h** header defines several functions to directly assign and read data types without the need for manually formatting and executing snippets of R code for these purposes. Not all possible data types are directly supported, the main emphasis is on vectors and matrices containing floating point values. Once you have transferred the bulk of your data as vectors or matrices into the R session, you can execute snippets of R code to combine them or convert them into something more complex if needed. 

Normally you want to get huge amounts of numeric data into the R session quickly, but not the other way. The assumption is that you want to feed it with price data on order to crunch numbers, and only need to get back a single value or a vector as a result.

All vector functions have a parameter for the number of elements. Be careful that the array you supply has the correct size, or it will crash the R console. Expressions or code snippets have a maximum size of 1000 characters per command; larger pieces of code must be sent with several commands or - much better - be called as a function in the R environment. See **RTest.c** for example usage.

## Rstart(string Filename, int debuglevel): int 

Start a new R session, and load R functions and variables from the file **Filename** in the **Strategy** folder. This must be called in the [INITRUN](is.htm) before any R computations can be done. Returns **0** when the R session could not be started, otherwise nonzero. The R session will stay alive until the end of the simulation or Zorro session. 

### Parameters:

**Filename** | Name of a **.r** file in the **Strategy** folder containing all needed R functions and variables (f.i. **"MySource.r"**), or **""** (default) for not sourcing a file.   
---|---  
**debuglevel** | **0** \- only output fatal errors (default)  
**1** \- output warnings and notes   
**2** \- output every R message  
The R output goes to the system debug monitor. Use the free **DebugView** tool from Microsoft, or call **Rx(..., 3)** to view R output).  
  
## Rrun(): int 

Return **1** if the R session is ready for input, **2** if the session is busy with a computation, and **0** if the session is terminated. R will terminate on any fatal error in the code. You should check this regularly. The last command prior to the termination will be found in the debug output. If R is not running anymore, the R bridge won't emit any more messages and will silently ignore all commands.   

## Rx(string code,_int mode_): int 

Execute a line of R code. Similar to the **Ri** / **Rd** / **Rv** functions below, as evaluating an expression is also just executing code; the difference is that **Rx()** can send commands asynchronously, allows R and Zorro to do computations in parallel, and prints R output to the Zorro window.  

### Parameters:

**code** | R code line to be executed (1000 characters max).  
---|---  
**mode** | **0** \- synchronous. Wait until the computation was finished (default).  
**1** \- asynchronous. Return immediately. **Rrun()** can be used to check when the next R command can be sent.  
**2** \- synchronous, using a [wait](sleep.htm) loop to react on events during computation. Return **1** when finished, or [quit](quit.htm) with returning **0** when the session was aborted due hitting [Stop] or due to a fatal R error.  
**3** \- synchronous using with a [wait](sleep.htm) loop as above, but direct R **print** output to the Zorro window. The **debuglevel** must be set accordingly.   
  
## Rset(string name, int n)

## Rset(string name, var d)

## Rset(string name, var *v, int elements)

## Rset(string name, var *m, int rows, int cols)

## Rset(string name, string s)

Store an int, var, series, matrix, or string in the R variable **name**. Since the target type depends on whether the variable is **int** , **var** , or a pointer, make sure to use the correct type especially when storing constants, and typecast it with **(int)** or **(var)** if in doubt. 

### Parameters:

**name** | Name of the R variable.  
---|---  
**i** | **int** value to be assigned to a R integer variable.  
**d** | **var** value to be assigned to a R floating point variable.  
**v** | Pointer of the **var** array or series to be assigned to a R vector.  
**m** | Pointer of the **var** array to be assigned to a R matrix, by row order.  
**elements** | Length of the vector or series. Make sure to give the exact number of elements.   
**rows, cols** | Number of rows and columns of the matrix.  
  
## Ri(string expression): int

## Rd(string expression): var

## Rv(string expression, var *v, int elements)

Evaluate the given R expression, and return the result as an int, double, or vector. The expression can be a single variable, a R function, or any R code that will evaluate to the required variable type. 

### Parameters:

**expression** | R expression to be evaluated (1000 characters max).  
---|---  
**v** | Pointer of the **var** array to be filled with the R vector.  
**elements** | Number of elements of the vector; must be identical in the R code and in the lite-C script.   
  
### Remarks:

  * Test your R commands and functions carefully on the R console before executing them in a lite-C script. A command error - wrong syntax, a wrong path, a missing R package, or a bad parameter for a R function - will cause the session to abort and all subsequent R commands to fail. You'll normally see the error reason printed in the Zorro window. Use the **Rrun()** function at the end of every bar of a strategy to determine if the session is still running. When live trading, stop the strategy and raise the alarm when **Rrun()** fails.
  * When passing file names to R, use forward slashes for directories, f.i. **"C:/Zorro/Data/Data.csv"**. You can use the **slash** helper function in **r.h** for converting backslashes to forward slashes (f.i. **slash(ZorroFolder)**). 
  * For checking the R console output while under lite-C control, print it to a file, f.i. with the **sink** function that prints all subsequent R output to the given file. Example:  
**Rx("sink(\"C:/Zorro/Log/RTest.txt\")");**
  * While the R bridge is busy, Zorro can't react on user actions such as clicking the [Stop] button. For avoiding this on long R computations, use **Rx()** with **mode=2**. Alternatively, call **Rx()** with **mode=1** and check **Rrun()** in a [wait()](sleep.htm) loop (f.i. **while(Rrun() == 2) wait(100)**).
  * Make sure to send complete commands. If a R command is incomplete f.i. due to a missing bracket at the end, the R session will freeze and consequently Zorro will also freeze. 
  * If you have the choice, perform script-based computations on the lite-C side, as lite-C is up to 200 times faster than R. Loops in R are awfully slow. However R is preferable for vector and matrix operations. 
  * R functions usually expect time series in straight order with newest elements last. Use the [rev](rev.htm) function for reversing the order of a [series](series.htm).
  * Use [Train] mode and [WFO](numwfocycles.htm) for training machine learning algorithms, f.i. a SVM or neural network. Send training data to R at the end of every WFO cycle, and store the trained models in files for later use in the [Test] or [Trade] session. The [advise(NEURAL)](advisor.htm) function contains a framework for that. 
  * Install all needed R packages before starting the script. Often-used packages for financial computations are caret, zoo, xts, ttr, quantmod, rminer. There are also several popular packages for machine learning. The **e1071** package contains a support vector machine, and the **deepnet** package contains several deep learning network algorithms, such as sparse autoencoders and Boltzman machines. If the package of a called R function is not installed, the R session will abort with an error message. 
  * For conveniently working with R, we recommend the free programming environment **RStudio** (<http://www.rstudio.org>) that contains a simple debugger. 
  * The [R lectures](Lecture%201.htm) by Harry Georgakopoulos have been included in this manual. There are also several e-books about R (f.i. [R in a nutshell](http://web.udl.es/Biomath/Bioestadistica/R/Manuals/r_in_a_nutshell.pdf)). Scanning through a R book or tutorial is highly recommended, as it takes a while to get used to the R language. Variables are generated by assignment, and need not be declared. The most common variables are a **numeric** (= var, int, bool), a **vector** (= series, array), a **matrix** (= two-dimensional array), a **data frame**(= array of structs), and a **list** (= arbitrary collection of data). A series element can be accessed by appending **[n]** just as in lite-C, but in R the index begins with **1** , not with **0**. List elements are indexed with double brackets **[[n]]**. Although R also recognizes the '=' assignment operator, R assignments are usually made with '<-' or '->' depending on the assignment direction. Functions can write into global variables with '<<-'. Boolean operators are similar to C, but mind the difference between '&' and '&&': the former operates on all vector elements, the latter only on the first - a common trap for C programmers who write their first R code. 
  * An introduction to exchanging data between Zorro and R can be found on <https://robotwealth.com/integrating-r-with-the-zorro-backtesting-and-execution-platform/>.

### Examples (see also [advise](advisor.htm)): 

```c
// do some R calculations
#include <default.c>#include <r.h>

function main()
{
  Rstart("",2); // enable output 
  
  var vecIn[5],vecOut[5];
  int i;
  for(i=0; i<5; i++) 
    vecIn[i] = i;
  
  Rset("rin",vecIn,5); // set up a vector
  Rx("rout <- rin * 10"); // perform some arithmetics
  Rx("print(rout)",3); // print rout to the Zorro window 
  Rv("rout",vecOut,5); // read it back
   
   if(!Rrun()) 
     printf("Error - R session aborted!");
   else 
    for(i=0; i<5; i++) 
      printf("%.0f ",vecOut[i]);
}
```

```c
// Run a statistics test (ADF)
function run()
{
  set(PLOTNOW);
  BarPeriod = 1440;
  LookBack = 100;
  if(Init) {
    if(!Rstart())
      return quit("Error - R won't start!");
    Rx("rm(list = ls());"); // clear the workspace
    Rx("library(tseries)"); // load time series library
  }
  if(is(LOOKBACK)) return;
	
  Rset("Data",rev(seriesC()),LookBack); // send Close series to R
  Rx("ADF = adf.test(Data)"); // Augmented Dickey-Fuller test
  plot("ADF p-value",Rd("ADF$p.value"),NEW,RED); //display p-value
}
```

```c
// template for computing trade signals in R
#include <r.h>

int Size = 200; // number of candles needed by the R algorithm  

bool Rcheck()
{
   if(!Rrun())
     return quit("R session aborted!");
   return true;
}

function run()
{
  BarPeriod = 60;
  LookBack = Size;
  asset("EUR/USD");  
  
  if(Init)) {
    Rstart("MySignals.r",1);
// load all required R objects from a file in the Zorro Data folder 
    Rx(strf("load('%sData/MyObjects.bin')",slash(ZorroFolder)));
// make sure everything loaded ok
    if(!Rcheck()) return;
  }

// generate reverse price series (latest price comes last)
  vars O = rev(seriesO())),
    H = rev(seriesH())),
    L = rev(seriesL())),
    C = rev(seriesC()));
    
  if(!is(LOOKBACK)) {
// send the last 200 candles to R
    Rset("Open",O,Size);
    Rset("High",H,Size);
    Rset("Low",L,Size);
    Rset("Close",C,Size);

// let R compute the signal
    var Signal = Rd("Compute(Open,High,Low,Close)");
    if(!Rcheck()) return; // Compute() error
    
    if(Signal > 0 && !NumOpenLong)
      enterLong();
    if(Signal < 0 && !NumOpenShort)
      enterShort();
  }
}
```

```c
// export historical price data to R
string name = "Data\\Export.csv";

function run()
{
  BarPeriod = 60;
  StartDate = 20140101;

  if(Init) // write the header
    file_write(name,"Date, Open, High, Low, Close",0);
  else
    file_append(name,strf(
      "\n%04i-%02i-%02i %02i:%02i, %.5f, %.5f, %.5f, %.5f",
      year(),month(),day(),hour(),minute(),
      priceOpen(),priceHigh(),priceLow(),priceClose()));
}

/* in R:
> Data <- read.csv('C:/Projects/Zorro/Data/export.csv') # mind the forward slashes
> plot(Data$Close)
> ...
*/
```

### See also:

[Code conversion](conversion.htm), [DLL interface](litec_api.htm), [advise](advisor.htm), [R course](Lecture%201.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
