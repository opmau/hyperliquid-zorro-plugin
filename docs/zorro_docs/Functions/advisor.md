---
title: Machine Learning: adviseLong, adviseShort
url: https://zorro-project.com/manual/en/advisor.htm
category: Functions
subcategory: None
related_pages:
- [R Bridge](rbridge.htm)
- [Python Bridge](python.htm)
- [set, is](mode.htm)
- [polyfit, polynom](polyfit.htm)
- [Candle Patterns](candle.htm)
- [fuzzy logic](fuzzy.htm)
- [W7 - Machine Learning](tutorial_pre.htm)
- [Deep Learning](deeplearning.htm)
- [Status flags](is.htm)
- [Walk-Forward optimization](numwfocycles.htm)
- [Training](training.htm)
- [algo](algo.htm)
- [Virtual Hedging](hedge.htm)
- [LookBack, UnstablePeriod, ...](lookback.htm)
- [BarMode](barmode.htm)
- [BarPeriod, TimeFrame](barperiod.htm)
- [normalize, zscore](norm.htm)
- [Indicators](ta.htm)
- [series](series.htm)
- [Testing](testing.htm)
- [DataSplit, DataSkip, ...](dataslope.htm)
- [Multiple cores](numcores.htm)
- [optimize](optimize.htm)
- [frechet](detect.htm)
- [crossOver, crossUnder](crossover.htm)
- [predict](predict.htm)
---

# Machine Learning: adviseLong, adviseShort

# Machine learning

Zorro can utilize native or external machine learning algorithms, for instance from [R packages](rbridge.htm) or [Python libraries](python.htm), for applying 'artificial intelligence' to algorithmic trading. Three native algorithms for price prediction are available: a decision tree, a simple neural network, and a signal pattern learning system. They generate trading rules separately for any WFO cycle, asset, algo, and long or short trades. You can also provide your own function for training an individual model, or use functions from R or Python packages. An introduction into various machine learning methods can be found on [ Financial Hacker](http://www.financial-hacker.com/build-better-strategies-part-4-machine-learning).  
  
The **advise** functions are used to implement a machine learning algorithm just like a standard indicator: 

## adviseLong (int Method, var Objective, var Signal0, var Signal1, var Signal2,_..., var Signal19_): var 

## adviseShort (int Method, var Objective, var Signal0, var Signal1, var Signal2,_... var Signal19_): var 

## adviseLong (int Method, var Objective, var* Signals, int NumSignals): var 

## adviseShort (int Method, var Objective, var* Signals, int NumSignals): var 

Call a machine learning algorithm for training or for prediction. In [Train] mode the function trains the algorithm to predict either a subsequent trade return or a training target (**Objective**). In [Test] or [Trade] mode the function calls the algorithm and returns the predicted value. Depending on algorithm, training can generate a binary model or a prediction function in C code that is stored in the **Data** folder. The only difference of the **..Long** and **..Short** variants is that they can train on the returns of long and short trades; otherwise they are identical. They can thus alternatively be used for generating two different models per algo/asset component.  

### Parameters:

**Method** |  **SIGNALS** \- don't train, but only export **Signals** and **Objective** in [Train] mode to a **Data\\*.csv** file.   
**NEURAL** \- train and predict with a user-supplied or external machine learning algorithm. See below.**  
DTREE** \- train and predict with a decision tree (20 signals max).  
**PERCEPTRON** \- train and predict with logistic regression (20 signals max).  
**PATTERN** \- train and predict with a signal pattern analyzer (20 signals max).   
**+FAST** \- fast pattern finding with large patterns (for **PATTERN**).   
**+FUZZY** \- fuzzy pattern finding for **PATTERN** ; analog output for **PERCEPTRON**.   
**+2 .. +6** \- number of pattern groups (for **PATTERN**).   
**+BALANCED** \- enforce similar numbers of positive and negative target values by replication (for **SIGNALS** , **NEURAL** , **DTREE** , **PERCEPTRON**).  
**+RETURNS** \- use the subsequent long or short trade for the training target, instead of the **Objective** parameter. The target is the profit or loss including trading costs of the immediately following trade with the same asset, algo, and matching long/short trade direction.   
**0** or omitted - use the method and signals of the last **advise** call. Only when signal parameters and trade returns are used.  
---|---  
**Objective** | The training target. For instance,.an indicator value at the next bar, or a binary value like +1 / -1 for the sign of the next price change. The value should be in the same range as the signals. The **Objective** parameter is only used in the training run and has no meaning in test or trade mode. Set the [PEEK](mode.htm) flag for accessing future prices in training.   
**Signal0,  
... Signal19 ** |  3..20 parameters that are used as features to the machine learning algorithm for training or prediction (less than 3 signals would be interpreted as a **Signals** array). Use signals that carry information about the current market situation, for instance candle patterns, price differences, indicators, filters, or statistics functions. All signal values should be in the same range, for instance **0..1** , **-1..+1** , or **-100..+100** , dependent on machine learning method (see remarks). Signals largely outside that range will generate a warning message. If the signals are omitted, the signals from the last **advise** call are used.   
**Signals** | Alternative input method, a **var** array of arbitrary length containing the features to the machine learning algorithm. Use this for less than 3 or more than 20 signals.  
**NumSignals** | Array length, the number of features in the **Signals** array.   
  
  
### Returns:

[Train] mode: **100** when trade results are to be trained, i.e. the **RETURNS** flag is set or **Objective** is zero. Otherwise **0**.  
[Test], [Trade] mode: Prediction value returned from the trained algorithm. Can be used as a signal for entering, exiting, or filtering trades. The **DTREE** , **PERCEPTRON** , **PATTERN** algorithms normally return a value in the **-100 .. +100** range.   

### DTREE (Decision Tree) 

A **decision tree** is a tree-like graph of decisions by comparing signals with fixed values. The values and the tree structure are generated in the training run. For this the training process iterates through the sets of signals and finds the signal values with the lowest information entropy. These values are then used to split the data space in a profitable and a non profitable part, then the process continues with iterating through the parts. Details about the decision tree algorithm can be found in books about machine learning.  
  
The signals should be normalized roughly to the **-100..100** range for best precision. They should be carefully selected so that the displayed prediction accuracy is well above 60% in all WFO cycles. Decision trees work best with signals that are independent of each other. They do not work very well when the prediction depends on a linear combination of the signals. In order to reduce overfitting, the resulting trees are pruned by removing non-predictive signals. The output of the tree is a number between **-100** .. **+100** dependent on the predictive quality of the current signal combination. 

The decision tree functions are stored in C source code in the **\Data\\*.c** file. The functions are automatically included in the strategy script and used by the **advise** function in test and trade mode. They can also be exported for using them in strategy scripts or expert advisors of other platforms.The example below is a typical Zorro-generated decision tree:

```c
int EURUSD_S(var* sig){  if(sig[1] <= 12.938) {    if(sig[3] <= 0.953) return -70;    else {      if(sig[2] <= 43) return 25;      else {        if(sig[3] <= 0.962) return -67;        else return 15;      }    }  }  else {    if(sig[3] <= 0.732) return -71;    else {      if(sig[1] > 30.61) return 27;      else {          if(sig[2] > 46) return 80;          else return -62;      }    }  }}
```

The **advise()** call used 5 signals, of which the first and the last one - **sig[0]** and **sig[4]** \- had no predictive power, and thus were pruned and do not appear in the tree. Unpredictive signals are displayed in the message window. 

Example of a script for generating a decision tree: 

```c
void run()
{
  BarPeriod = 60;
  LookBack = 150;
  TradesPerBar = 2;
  if(Train) Hedge = 2;
  set(RULES|TESTNOW);
// generate price series
  vars H = series(priceHigh()), 
    L = series(priceLow()),
    C = series(priceClose());

// generate some signals from H,L,C in the -100..100 range
  var Signals[2]; 
  Signals[0] = (LowPass(H,1000)-LowPass(L,1000))/PIP;
  Signals[1] = 100*FisherN(C,100);

// train and trade the signals 
  Stop = 4*ATR(100); 
  TakeProfit = 4*ATR(100);
  if(adviseLong(DTREE,0,Signals,2) > 0)
    enterLong();
  if(adviseShort(DTREE,0,Signals,2) > 0)
    enterShort();
}
```

  

### PERCEPTRON 

A **perceptron** , also called a **logistic regression** function, is a simple neural net consisting of one neuron with one output and up to 20 signal inputs. It calculates its predictions from a linear combination of weighted signals. It has some similarity to the [polyfit](polyfit.htm) algorithm, but with arbitrary variables instead of powers of a single variable, and with a binary output. A short preceptron algorithm description can be found on the [machine learning overview](http://www.financial-hacker.com/build-better-strategies-part-4-machine-learning). The signal weights are generated in the training run for producing the best possible prediction. The signals should be in the single digit range, like **-1...1**. 

The perceptron algorithm works best when the weighted sum of the signals has predictive power. It does not work well when the prediction requires a nonlinear signal combination, i.e. when trade successes and failures are not separated by a straight plane in the signal space. A classical example of a function that a perceptron can not emulate is a logical XOR. Often a perceptron can be used where a decision tree fails, and vice versa. 

The perceptron learning algorithm generates prediction functions in C source code in the **\Data\\*.c** file. The functions are automatically included in the strategy script and used by the **advise** function in test and trade mode. They can also be exported for using them in strategy scripts or expert advisors of other platforms. The output is **> 0** for a positive or **< 0** for a negative prediction. The output magnitude is the probability associated with the prediction, in percent, f.i. 70 for 70% estimated probability. A generated perceptron function with 3 signals and binary output looks like this: 

```c
int EURUSD_S(var* sig){  if(-27.99*sig[0]+1.24*sig[1]-3.54*sig[2] > -21.50)    return 70;  else    return -70;}
```

In **FUZZY** mode the output magnitude is equivalent to the prediction strength, thus allowing to ignore signals below a threshold. The scaling factor, **2.50** in the example below, is calculated so that the average perceptron return value of the training set has a magnitude of 50\. Example of a fuzzy perceptron:

```c
int EURUSD_S(var* sig){  return (-27.99*sig[0]+1.24*sig[1]-3.54*sig[2]-21.50)*2.50;}
```

Signals that do not contain predictive market information get a weight of **0**.   

### PATTERN (Pattern Analyzer)

The **pattern analyzer** is an intelligent version of classic [candle pattern indicators](candle.htm). It does not use predefined patterns, but learns them from historical price data. It's normally fed with up to 20 open, close, high or low prices of a number of candles. It compares every signal with every other signal, and uses the comparison results - greater, smaller, or equal - for classifying the pattern. 

The signals can be divided into groups with the **PATTERN+2** .. **PATTERN+6** methods. They divide the signals into up to six pattern groups and only compare signals within the same group. This is useful when, for instance, only the first two candles and the last two candles of a 3-candle pattern should be compared with each other, but not the first candle with the third candle. **PATTERN+2** requires an even number of signals, of which the first half belongs to the first and and the second half to the second group. **PATTERN+3** likewise requires a number of signals that is divisible by 3, and so on. Pattern groups can share signals - for instance, the open, high, low, and close of the middle candle can appear in the first as well as in the second group - as long as the total number of signals does not exceed 20. 

Aside from the grouping, Zorro makes no assumptions of the signals and their relations. Therefore the pattern analyzer can be also used for other signals than candle prices. All signals within a pattern group should have the same unit for being comparable, but different groups can have different units. For candle patterns, usually the high, low, and close of the last 3 bars is used for the signals - the open is not needed as it's normally identical with the close of the previous candle. More signals, such as the moving average of the price, can be added for improving the prediction (but in most cases won't).

For simple patterns with few signals, the pattern analyzer can generate direct pattern finding functions in plain C source code in the **\Data\\*.c** file. These functions are automatically included in the strategy script and used by the **advise** function in test and trade mode. They can also be exported to other platforms. They find all patterns that occurred 4 or more times in the training data set and had a positive profit expectancy. They return the pattern's information ratio - the ratio of profit mean to standard deviation - multiplied with 100. The better the information ratio, the more predictive is the pattern. A typical pattern finding function with 12 signals looks like this:

```c
int EURUSD_S(float* sig)
{
  if(sig[1]<sig[2] && eqF(sig[2]-sig[4]) && sig[4]<sig[0] && sig[0]<sig[5] && sig[5]<sig[3]
    && sig[10]<sig[11] && sig[11]<sig[7] && sig[7]<sig[8] && sig[8]<sig[9] && sig[9]<sig[6])
      return 19;
  if(sig[4]<sig[1] && sig[1]<sig[2] && sig[2]<sig[5] && sig[5]<sig[3] && sig[3]<sig[0] && sig[7]<sig[8]
    && eqF(sig[8]-sig[10]) && sig[10]<sig[6] && sig[6]<sig[11] && sig[11]<sig[9])
      return 170;
  if(sig[1]<sig[4] && eqF(sig[4]-sig[5]) && sig[5]<sig[2] && sig[2]<sig[3] && sig[3]<sig[0]
    && sig[10]<sig[7] && eqF(sig[7]-sig[8]) && sig[8]<sig[6] && sig[6]<sig[11] && sig[11]<sig[9])
      return 74;
  if(sig[1]<sig[4] && sig[4]<sig[5] && sig[5]<sig[2] && sig[2]<sig[0] && sig[0]<sig[3] && sig[7]<sig[8]
    && eqF(sig[8]-sig[10]) && sig[10]<sig[11] && sig[11]<sig[9] && sig[9]<sig[6])
      return 143;
  if(sig[1]<sig[2] && eqF(sig[2]-sig[4]) && sig[4]<sig[5] && sig[5]<sig[3] && sig[3]<sig[0]
    && sig[10]<sig[7] && sig[7]<sig[8] && sig[8]<sig[6] && sig[6]<sig[11] && sig[11]<sig[9])
      return 168;
  ....
  return 0;
}
```

The **eqF** function in the code above checks if two signals are equal. Signals that differ less than the [FuzzyRange](fuzzy.htm) are considered equal. 

There are two additional special methods for the pattern analyzer. The **FUZZY** method generates a pattern finding function that also finds patterns that can slightly deviate from the profitable patterns in the training data set. It gives patterns a higher score when they 'match better'. The deviation can be set up with **FuzzyRange**. A typical fuzzy pattern finding function looks like this:

```c
int EURUSD_S(float* sig)
{
  double result = 0.;
  result += belowF(sig[1],sig[4]) * belowF(sig[4],sig[2]) * belowF(sig[2],sig[5]) * belowF(sig[5],sig[3]) * belowF(sig[3],sig[0])
    * equF(sig[10],sig[11]) * belowF(sig[11],sig[7]) * belowF(sig[7],sig[8]) * belowF(sig[8],sig[9]) * belowF(sig[9],sig[6]) * 19;
  result += belowF(sig[4],sig[5]) * belowF(sig[5],sig[1]) * belowF(sig[1],sig[2]) * belowF(sig[2],sig[3]) * belowF(sig[3],sig[0])
    * belowF(sig[10],sig[7]) * belowF(sig[7],sig[11]) * belowF(sig[11],sig[8]) * belowF(sig[8],sig[9]) * belowF(sig[9],sig[6]) * 66;
  result += belowF(sig[4],sig[1]) * belowF(sig[1],sig[2]) * belowF(sig[2],sig[0]) * belowF(sig[0],sig[5]) * belowF(sig[5],sig[3])
    * belowF(sig[10],sig[11]) * belowF(sig[11],sig[7]) * belowF(sig[7],sig[8]) * belowF(sig[8],sig[6]) * belowF(sig[6],sig[9]) * 30;
  result += belowF(sig[1],sig[4]) * belowF(sig[4],sig[2]) * belowF(sig[2],sig[5]) * belowF(sig[5],sig[3]) * belowF(sig[3],sig[0])
    * belowF(sig[7],sig[10]) * belowF(sig[10],sig[11]) * belowF(sig[11],sig[8]) * belowF(sig[8],sig[6]) * belowF(sig[6],sig[9]) * 70;
  result += belowF(sig[4],sig[5]) * belowF(sig[5],sig[1]) * belowF(sig[1],sig[2]) * belowF(sig[2],sig[3]) * belowF(sig[3],sig[0])
    * belowF(sig[7],sig[10]) * belowF(sig[10],sig[8]) * belowF(sig[8],sig[11]) * belowF(sig[11],sig[9]) * belowF(sig[9],sig[6]) * 108;
  ...
  return result;
}
```

The **belowF** function is described on the [Fuzzy Logic](fuzzy.htm) page. 

The **FAST** method does not generate C code; instead it generates a list of patterns that are classified with alphanumeric names. For finding a pattern, it is classified and its name compared with the pattern list. This is about 4 times faster than the pattern finding function in C code, and can also handle bigger and more complex patterns with up to 20 signals. It can make a remarkable difference in backtest time or when additional parameters have to be trained. A pattern name list looks like this (the numbers behind the name are the information ratios):

```c
/* Pattern list for EURUSD_SHIECBFGAD   61BEFHCAIGD  152EHBCIFGAD   73BEFHCIGDA   69BHFECAIGD   95BHIFECGAD   86HBEIFCADG   67HEICBFGDA  108
...*/
```

The **FAST** method can not be used in combination with **FUZZY** or with **FuzzyRange.** But the **FAST** as well as the **FUZZY** method can be combined with pattern groups (f.i. **PATTERN+FAST+2**). 

The find rate of the pattern analyzer can be adjusted with two variables:

## PatternCount

The minimum number of occurrences of the found patterns in the analyzed price curve; default = **4**. 

## PatternRate

The minimum win rate of the found patterns, in percent; default = **50**. 

An example of a pattern trading script can be found in [Workshop 7](tutorial_pre.htm).   

### NEURAL (General Machine Learning)

The **NEURAL** method uses an external machine learning library, for instance a support vector machine, random forest, or a deep learning [neural network](deeplearning.htm) for predicting the next price or next trade return in an algo trading system. Many machine learning libraries are available in [R packages](rbridge.htm) or [Python libraries](python.htm); therefore the **NEURAL** method will often call **R** or **Python** functions for training and prediction. But any DLL-based machine learning library can be used as long as the **neural** function - see below - is adapted to it. 

## neural (int Status, ínt Model, int NumSignals, void* Data): var 

This function is automatically called several times during the training, test, or trade process. It has access to all global and predefined variables. Its behavior depends on **Status** :   
  
**Status** | **Parameters** | **Called** | **Description**  
---|---|---|---  
**NEURAL_INIT** | **\---** | After the [INITRUN](is.htm) | Initialize the machine learning library (f.i. by calling [Rstart](rbridge.htm)); return **0** if the initialization failed, otherwise **1**. The script is aborted if the system can not be initialized.   
**NEURAL_EXIT** | **\---** | After the [EXITRUN](is.htm) | Release the machine learning library if required.   
**NEURAL_TRAIN** | **Model, NumSignals, Data** | After any [WFOCycle](numwfocycles.htm) in train mode  | Batch training. Train the model with the given **Model** index with all data samples collected in the previous training run or WFO training cycle. The **Model** index is incremented for any asset, algo, and long/short combination. The list of samples is given in CSV format in the **Data** string. The columns of the CSV table are the signals, the last column is the **Objective** parameter or the trade result. The accuracy or loss in percent can be optionally returned by the **neural** function; otherwise return **1** if no accuracy or loss is calculated, or **0** for aborting the script when the training failed.   
**NEURAL_PREDICT** | **Model, NumSignals, Data** | By any **advise** call in test/trade mode | Return the value predicted by the model with the given **Model** index, using the signals contained in the **Data** double array.  
**NEURAL_SAVE** | **Data** | After any [WFOCycle](numwfocycles.htm) in train mode  | Save all models trained in the previous training run or WFO training cycle to the file with the name given by the string **Data**.  
**NEURAL_LOAD** | **Data** | Before any [WFOCycle](numwfocycles.htm) in test mode, at session begin in trade mode.  | Prepare the prediction by loading all trained models from the file with the name given ****by the string**Data**. Also called in trade mode whenever the model file was updated by re-training.   
  
**Model** is the number of the trained model, for instance a set of decision rules, or a set of weights of a neural network, starting with **0**. When several models are trained for long and short predictions and for different assets or algos, the index selects the suited model. The number of models is therefore normally equal to the number of **advise** calls per **run** cycle. All trained models are saved to a ***.ml** file at the end of every WFO cycle. In R, the models are normally stored in a list of lists and accessed through their index (f.i. **Models[[model+1]]**). Any aditional parameter set generated in the training process - for instance, a set of normalization factors, or selection masks for the signals - can be saved as part of the model. 

The **NumSignals** parameter is the number of signals passed to the **advise** function. It is normally identical to the number of trained features. 

The **Data** parameter provides data to the function. The data can be of different type. For **NEURAL_LEARN** /**NEURAL_PREDICT** it's a pointer to a **double** array of length **NumSignals+1** , containing the signal values plus the prediction objective or trade result at the end. Note that a plain data array has no "dim names" or other R gimmicks - if they are needed in the R training or predicting function, add them there. For **NEURAL_TRAIN** the **Data** parameter is a text string containing all samples in CSV format. The string can be stored in a temporary CSV file and then read by the machine learning algorithm for training the model. For **NEURAL_SAVE** /**NEURAL_LOAD** the **Data** parameter is the suggested file name for saving or loading all trained models of the current WFO cycle in the **Data** folder. Use the **slash(string)** function for converting backslashes to slashes when required for R file paths.

The code below is the default **neural** function in the **r.h** file for using a R machine learning algorithm. A 64-bit variant for Python can be found in **pynet.cpp**. If required for special purposes, the default **neural** function can be replaced by a user-supplied**** function.

```c
var neural(int Status, int model, int NumSignals, void* Data){  if(!wait(0)) return 0;// open an R script with the same name as the stratefy script    if(Status == NEURAL_INIT) {    if(!Rstart(strf("%s.r",Script),2)) return 0;    Rx("neural.init()");    return 1;  }// export batch training samples and call the R training function  if(Status == NEURAL_TRAIN) {    string name = strf("Data\\signals%i.csv",Core);    file_write(name,Data,0);    Rx(strf("XY <- read.csv('%s%s',header = F)",slash(ZorroFolder),slash(name)));    Rset("AlgoVar",AlgoVar,8);    if(!Rx(strf("neural.train(%i,XY)",model+1),2))       return 0;    return 1;  }// predict the target with the R predict function  if(Status == NEURAL_PREDICT) {    Rset("AlgoVar",AlgoVar,8);    Rset("X",(double*)Data,NumSignals);    Rx(strf("Y <- neural.predict(%i,X)",model+1));    return Rd("Y[1]");  }// save all trained models    if(Status == NEURAL_SAVE) {    print(TO_ANY,"\nStore %s",strrchr(Data,'\\')+1);    return Rx(strf("neural.save('%s')",slash(Data)),2);  }// load all trained models    if(Status == NEURAL_LOAD) {    printf("\nLoad %s",strrchr(Data,'\\')+1);    return Rx(strf("load('%s')",slash(Data)),2);  }  return 1;}
```

The default **neural** function requires an R script with same name as the strategy script, but extension **.r** instead of **.c**. There is also a Python version available. The R or Python script must contain the following functions:

**neural.init()** initializes the machine learning package, defines a model list, sets the initial seed, number of cores, GPU/CPU context, etc. 

**neural.train(Model, XY)** trains the the algorithm and stores the trained model in the model list. **XY** is a data frame with signals in the first **NumSignals** columns and the objective in the last (**NumSignals+1**) column. The samples are the rows of the data frame. **Model** is the number in the model list where the trained model is to be stored. The model can be any R object combined from the machine learning model and any additional data, f.i. a set of normalization factors. In the Python version, XY is the file name of a .csv file with the data.

**neural.save(FileName)** stores all models from the model list to a file with the given name and path. This function is called once per WFO cycle, so any cycle generates a file of all models of this cycle.

**neural.load(FileName)** loads back the previously stored model list for the current WFO cycle. If this optional function is missing, the model list is loaded with the R **load()** function. Otherwise make sure to load the list into the global R environment so that it is accessible to **neural.predict** , f.i. with **load(FileName, envir=.GlobalEnv)**.

**neural.predict(Model, X)** predicts the objective from the signals. **X** is a vector of size **NumSignals** , containing the signal values for a single prediction. **Model** is the number of the model in the model list. The **neural.predict** return value is returned by the **advise** function. The **neural.predict** function could also support batch predictions where**X** is a data frame with several samples and a vector **Y** is returned. Batch predictions are not used by Zorro, but useful for calibrating the algorithm with a training/test data set generated in **SIGNALS** mode. 

An example of such a deep learning system can be found on [Financial Hacker](http://www.financial-hacker.com/build-better-strategies-part-5-developing-a-machine-learning-system/). A simple machine learning strategy **DeepLearn.c** is included, together with R scripts for running it with the **Deepnet** , **H2O** , **MxNet** , or **Keras** deep learning packages. A .cpp version is available for **PyTorch**. The **neural** function is compatible with all Zorro trading, test, and training methods, including walk forward analysis, multi-core parallel training, and multiple assets and algorithms. A short description of installation and usage of popular deep learning packages can be found [here](deeplearning.htm).

### Remarks:

  * The [RULES](mode.htm) flag must be set for generating rules or training a machine learning algorithm. See the remarks under [Training](training.htm) for special cases when **RULES** and **PARAMETERS** are generated at the same time. 
  * All **advise** calls for a given asset/[algo ](algo.htm)combination must use the same **Method** , the same signals, and the same training target type (either **Objective** or the next trade return). However, different methods and signals can be used for different assets and algorithm identifiers, so call [algo()](algo.htm) for combining multiple **advise** methods in the same script. Ensemble or hybrid methods can also be implemented this way, using different [algo](algo.htm) identifiers.
  * The generated rules by **DTREE** , **PERCEPTRON** , **PATTERN** are stored in plain C code in ***.c** files in the **Data** folder. This allows to export the rules to other platforms, and to examine the prediction process to some degree for checking if the rules makes sense. Models by external machine learning algorithms are stored in ***.ml** files in the **Data** folder. If several models are trained, they are numbered in the order of advise calls and WFO cycles. F.i. for two advise calls, two assets, and 7 WFO cycles, 28 models are generated, 4 for any cycle. 
  * For considering the effects of stops, profit targets, and transactions costs in training, call the **advise** function with **Objective = 0** before entering a trade. Zorro will then use the result of the next trade for training the rules. When training long and short trades at the same time, set [Hedge](hedge.htm) to make sure that they can be opened simultaneously in training mode; otherwise any trade would close an opposite trade just entered before. Define an exit condition for the trade, such as a timed exit after a number of bars, or a stop/trailing mechanism for predicting a positive or negative excursion. Do not train rules with hedging explicitly disabled or with special modes such as [NFA](mode.htm#nfa) or [Virtual Hedging](hedge.htm).
  * Predictions are normally better with fewer signals, so use as less signals as possible, or pre-process the signals with an algorithm to reduce dimensionality. For more than 20 signals, use a **Signals** array. The signals for the **DTREE, PATTERN,** and **PERCEPTRON** methods are always limited to 20.
  * Most machine learning algorithms require balanced training data. That means the trades used for training should result in about 50% wins and 50% losses, and negative and positive **Objective** values should be equally distributed. The **+BALANCED** flag will duplicate samples until 50/50 balance is reached. The used algorithm balances the samples also locally, so arbitrary subsets ('batches') of the samples are still balanced.
  * The **SIGNALS** method exports data for testing and calibrating external machine learning algorithms. It is normally recommended to also use **BALANCED** data for that. The data is stored in a CSV file with the signals in the first columns and the objective in the last column.
  * During inactive time periods determined by [LookBack](lookback.htm), [BarMode](barmode.htm), a **[SKIP](mode.htm)** flag, or [TimeFrame](barperiod.htm), the **advise** functions won't train or predict and return **0**. 
  * Functions like [scale](norm.htm) or [Normalize](ta.htm) can be used to convert the signals to the same range. Some R machine learning algorithms require that signals and targets are in the **0..1** range; in that case negative values or values greater than 1 lead to wrong results. If a signal value is outside the +/-1000 range, a warning is issued. 
  * When using a future value for prediction, such as the price change of the next bars (f.i. **Objective = priceClose(-5) - priceClose(0)**), make sure to set the [PEEK](mode.htm) flag in train mode. Alternatively, use the price change of a past bar to the current bar, and pass past signals - f.i. from a [series](series.htm) \- to the advise function in train mode, f.i.:  
**Objective = priceClose(0) - priceClose(5)** ;   
**int Offset = ifelse(Train,5,0);  
var Prediction = adviseLong(Method,Objective,Signal0[Offset],Signal1[Offset],...);**
  * All machine learning functions have a tendency to overfit the strategy. For this reason, strategies that use the **advise** function should always be tested with unseen data, f.i. by using [Out-Of-Sample testing](testing.htm) or [Walk Forward Optimization](numwfocycles.htm). Use the [OPENEND](mode.htm) flag for preventing that trades prematurely close at the end of a WFO training period and thus reduce the training quality. When WFO training, make sure to set [DataHorizon](dataslope.htm) accordingly to prevent peeking bias in the subsequent test cycle.
  * A message like **"NEURAL_INIT: failed"** hints that the required R packages are not installed. Make sure that you can source the R script and that the train/predict functions run with no error message.
  * When WFO training a machine learning algorithm with [multiple cores](numcores.htm), check if the used R libraries are compatible with multiple processes. This is normally not the case when hardware resources, such as the GPU, or when multiple CPU cores are used. The R **neural.init** function is called at the begin of any process, which can lead to different results in single core and multi core when a random seed is set in the **neural.init** function.
  * For training multiple objectives with the **NEURAL** method, pass the further objectives as **Signal** parameters to the **advise** function in training mode. In test and trade mode, use a separate prediction function that loads the current model and returns the predictions from R in a vector.
  * For R debugging, use the R **sink** function for printing R output to a file. Or use the **save.image** / **load** functions for storing the current variable and function state and examining it later.
  * For internal machine learning algorithms, the estimated prediction accuracy or the number of found patterns is printed in the message window. The signals should be selected in a way that the prediction accuracy is well above 50% or that as many as possible profitable patterns are found. For external machine learning functions, use a confusion matrix or the built-in metrics for determining the prediction accuracy. 
  * When rules are stored in C code, the internal lite-C compiler compiles in 32-bit mode unless **FAST** is set. Since the compiler does not support to free parts of the code – only the complete code – the code is growing on any run cycle, thus increasing the memory footprint. The memory is only freed when the script is compiled again. 

### See also:

[optimize](optimize.htm), [frechet](detect.htm), [polyfit](polyfit.htm), [](crossover.htm)[predict](predict.htm), [Workshop 7](tutorial_pre.htm), [R Bridge](rbridge.htm), [deep learning](deeplearning.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
