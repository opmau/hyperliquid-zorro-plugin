---
title: Lecture 4: Regression and Pairs Trading
url: https://zorro-project.com/manual/en/Lecture 4.htm
category: R Lectures, 1-7
subcategory: None
related_pages:
- [Lecture 5](Lecture 5.htm)
---

# Lecture 4: Regression and Pairs Trading

# Lecture 4 â Regression and Pairs Trading

In [Beginner R Tutorial](http://www.rfortraders.com/code/beginner-r-tutorial/), [FINC 621](http://www.rfortraders.com/code/finc-621/), [R Programming](http://www.rfortraders.com/code/r-programming/)

#### Regression Analysis

Regression is a very important topic. It is a widely used statistical tool in economics, ï¬nance and trading. R provides pre-written functions that perform linear regressions in a very straightforward manner. There exist multiple add-on packages that allow for more advanced functionality. In this class, we will only utilize the **lm()** function which is available in the base installation of R. The following example demonstrates the use of this function:

```r
x    <- rnorm(1000)
 y    <- (x - 2) + rnorm(1000)
 outR <- lm(y ~ x)
 summary(outR)
```  
---  
  
What we did above was creat an independent variable x and a dependent variable y. The call to the function **lm()** performed an OLS (Ordinary Least Squareâs) ï¬t to the function: y = b0 + b1x + e, where e was distributed as N(mu, sigma^2)

The **~** is used to separate the independent from the dependent variables. The expression **y ~ x** is a formula that speciï¬es the linear model with one independent variable and an intercept. If we wanted to ï¬t the same model, but without the intercept, we would specify the formula as **y ~ x â 1**. This tells R to omit the intercept (force it to zero). The following graph illustrates the best ï¬t lines for a model with and without an intercept. If you know that your model should contain an intercept term, then include it. Otherwise, do not include the intercept. For the subsequent pairs trading example, we are going to assume that the intercept term is equal to 0.

![Graph of regression in R](../images/regression_graph.png)

Whenever a regression is performed, it is very important to analyze the residuals (e) of the ï¬tted model. If everything goes according to plan, the residuals will be normally distributed with no visible pattern in the data, no auto-correlation and no heteroskedasticity. The residuals can be extracted from the regression object by using the **residuals** keyword.

```r
res <- outR$residuals
 par(mfrow = c(2,1))
 plot(res, col = "blue", main = "Residual Plot")
 acf(res)
```  
---  
  
Here is a graph of both the residuals themselves and their auto-correlation.  
![residual plot in R and autocorrelation](../images/residuals.png)

The **summary** keyword can be used to obtain the results of the linear regression model ï¬t.

```r
summary(outR)
```  
---  
  
The p-values and t-statistics can be used to evaluate the statistical signiï¬cance of the coefï¬cients. The smaller the p-value, the more certain we are that the coefï¬cient estimate is close to the actual population coefï¬cient. Notice how both the intercept and the independent variable coefï¬cient is signiï¬cant in this example. The extraction of the coefï¬cients can be accomplished via  
the **coefficients** keyword.

```r
outR$coefficients
```  
---  
  
#### Pairs Trading

What follows is a really simple version of a pairs trade between two equities. The main motivation for the following example is to expose you to obtaining data via the quantmod package and in using the **lm()** function that we covered above. 

We will explore a simple two-legged spread between AAPL and QQQ. Once we download and transform the timeseries for both stocks, we will define a simple trading rule and explore the trades that our signal generates. Various trade statistics and graphs will be presented.

##### Step 1: Obtain data via quantmod

```r
#Utilize quantmod to load the security symbols
 require(quantmod)
 symbols <- c("AAPL", "QQQ")
 getSymbols(symbols)
```  
---  
  
Now that our data frames for AAPL and QQQ are loaded into memory, letâs extract some prices.

##### Step 2: Extract prices and time ranges

```r
#define training set
 startT  <- "2007-01-01"
 endT    <- "2009-01-01"
 rangeT  <- paste(startT,"::",endT,sep ="")
 tAAPL   <- AAPL[,6][rangeT]
 tQQQ   <- QQQ[,6][rangeT]
 
 #define out of sample set
 startO  <- "2009-02-01"
 endO    <- "2010-12-01"
 rangeO  <- paste(startO,"::",endO,sep ="")
 oAAPL   <- AAPL[,6][rangeO]
 oQQQ   <- QQQ[,6][rangeO]
```  
---  
  
Notice how we defined and in-sample and out-of-sample range. We will use the in-sample data to compute a simple hedge ratio and then we will apply this hedge ratio to the out of sample data.

##### Step 3: Compute returns and find hedge ratio

```r
#compute price differences on in-sample data
 pdtAAPL <- diff(tAAPL)[-1]
 pdtQQQ <- diff(tQQQ)[-1]
 
 #build the model
 model  <- lm(pdtAAPL ~ pdtQQQ - 1)
 
 #extract the hedge ratio
 hr     <- as.numeric(model$coefficients[1])
```  
---  
  
##### Step 4: Construct the spread

```r
#spread price (in-sample)
 spreadT <- tAAPL - hr * tQQQ
 
 #compute statistics of the spread
 meanT    <- as.numeric(mean(spreadT,na.rm=TRUE))
 sdT      <- as.numeric(sd(spreadT,na.rm=TRUE))
 upperThr <- meanT + 1 * sdT
 lowerThr <- meanT - 1 * sdT
 
 #visualize the in-sample spread + stats
 plot(spreadT, main = "AAPL vs. QQQ spread (in-sample period)")
 abline(h = meanT, col = "red", lwd =2)
 abline(h = meanT + 1 * sdT, col = "blue", lwd=2)
 abline(h = meanT - 1 * sdT, col = "blue", lwd=2)
```  
---  
  
![trading in R pairs](../images/pairs_trade.png)

Letâs look at the distribution of the spread.

```r
hist(spreadT, col = "blue", breaks = 100, main = "Spread Histogram (AAPL vs. QQQ)")
 abline(v = meanT, col = "red", lwd = 2)
```  
---  
  
![distribution of spread in R](../images/spread_histogram.png)

##### Step 5: Define the trading rule

Once the spread exceeds our upper threshold, we sell AAPL and buy QQQ. Once the spread drops below our lower threshold, we buy AAPL and sell QQQ.

```r
indSell <- which(spreadT >= meanT + sdT)
 indBuy  <- which(spreadT <= meanT - sdT)
```  
---  
  
##### Step 6: Figure out the trades

```r
res <- outR$residuals
 par(mfrow = c(2,1))
 plot(res, col = "blue", main = "Residual Plot")
 acf(res)
```0  
---  
  
##### Step 7: Visualize trades

```r
res <- outR$residuals
 par(mfrow = c(2,1))
 plot(res, col = "blue", main = "Residual Plot")
 acf(res)
```1  
---  
  
![R spread prices](../images/spread_prices.png)

[Next: R Lecture 5](Lecture 5.htm)
