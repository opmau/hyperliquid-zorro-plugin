---
title: Lecture 3: Matrices and Libraries
url: https://zorro-project.com/manual/en/Lecture 3.htm
category: R Lectures, 1-7
subcategory: None
related_pages:
- [Lecture 4](Lecture 4.htm)
---

# Lecture 3: Matrices and Libraries

# Lecture 3 â Matrices and Libraries

#### Matrices in R

A matrix is a very useful mathematical construct. Matrices provide a mechanism for easily manipulating large collections of data. Matrix Mathematics is a vast topic and there exist numerous papers and publications that talk about all the possible uses of matrices. Suffice it to say that this class is only going to use a small subset of these theorems. In R, a matrix can be created in the following manner:

```r
#specify an empty marix with 3 rows and 3 columns
 emptyMat <- matrix(nrow = 3, ncol = 3)
```  
---  
  
Matrices are created column first. If you want to create the rows first, make sure to use the **byrow = TRUE** attribute.

```r
mat1 <- matrix(c(1,2,3,4,5,6), nrow = 2, ncol = 3, byrow = TRUE)
```  
---  
  
as opposed to:

```r
mat2 <- matrix(c(1,2,3,4,5,6), nrow = 2, ncol = 3)
```  
---  
  
##### Naming Convention for Matrices

Since a matrix is an object within R, one can change the name attribute of the matrix. Names are assigned to the rows and to the columns of a matrix. The following three snippets of code accomplish this.

```r
#Method 1
 mat3 <- matrix(rnorm(16,0,1), nrow = 4, ncol = 4)
 dimnames(mat3) <- list(c("Row1", "Row2", "Row3", "Row4"),
                        c("Col1", "Col2", "Col3", "Col4"))
 #Method 2
 mat4 <- matrix(rnorm(16,0,1), nrow = 4, ncol = 4, dimnames =
                list(c("Row1", "Row2", "Row3", "Row4"),
                     c("Col1", "Col2", "Col3", "Col4")))
 #Method 3
 myRowNames  <- c("r1", "r2", "r3", "r4")
 myColNames  <- c("c1", "c2", "c3", "c4")
 matrixNames <- list(myRowNames, myColNames)
 mat5        <- matrix(rnorm(16,0,1),nrow = 4, ncol = 4, dimnames = matrixNames)
```  
---  
  
#### Fun with Matrices

The following basic operations can be performed on matrices:

##### Addition

Provided that the number of rows and columns are the same for the matrices being added, once can do the following:

```r
m1 <- matrix(c(1,2,3,4), nrow = 2, ncol = 2)
 m2 <- matrix(c(5,4,3,2), nrow = 2, ncol = 2)
 m3 <- m1 + m2
```  
---  
  
##### Subtraction

```r
m1 <- matrix(c(7.8,2.4,3.3,4.0), nrow = 2, ncol = 2)
 m2 <- matrix(c(5,4,3,2), nrow = 2, ncol = 2)
 m3 <- m1 - m2
```  
---  
  
##### Multiplication

When multiplying together two matrices, make sure that the inner dimensions match. For example, it is fine to multiply a 2Ã3 with a 3Ã4 matrix. It is not ok to multiply together a 2Ã3 with a 4Ã4 matrix.

```r
m1 <- matrix(c(7.8,2.4,3.3,4.0), nrow = 2, ncol = 2)
 m2 <- matrix(c(5,4,3,2), nrow = 2, ncol = 2)
 m3 <- m1 %*% m2
```  
---  
  
##### Other

Matrix division is not defined. Rather, one can think of matrix division as multiplication by a matrix times the inverse of the second matrix. Remember also that, AB is not equal to BA in matrix land. Another operation that can be defined with matrices is that of exponentiation. This is a more involved topic and will not be covered in this class.

#### Determinant and Inverse

The determinant of a matrix A can be written as **det(A)** or |A|. The inverse of a matrix A can be written as inv(A) or A^-1.  
The determinant and the inverse of a matrix in R can be computed with the following functions: **det()** and **solve()**.

#### Sourcing Files

R code can be composed entirely within a simple text file. For more advanced editing capability, check out the following links:

  * [Tinn-R](http://www.sciviews.org/Tinn-R/)
  * [Notepad++](http://notepad-plus-plus.org/)
  * [Sublime 2](http://www.sublimetext.com/2)
  * [RStudio](http://www.rstudio.org/)

As mentioned in Class 1, there are 3 ways to get code into R.  
1\. Write code directly into the R console  
2\. Write code into a text file and copy/paste it into the R console  
3\. Write code into a text file, save the text file as a .R file and then invoke the **source()** command to load the contents of that file into memory.

```r
#specify the path of the .R file
 fileName <- "c:/myCode.R"
 #load the code into memory
 source(fileName)
```  
---  
  
#### Finding Packages

One of the benefits of the R environment is the abundance of open-source code in the form of external libraries/packages. The vast majority of these add-ons can be found here:  
[http://cran.r-project.org](http://cran.r-project.org/). Libraries are typically organized by subject matter. For a useful breakdown, click on _Packages - > CRAN Task Views_.

#### Installing Packages

There are two ways to install packages in R. The first way is via the GUI, and the second way is by issuing the appropriate command in the console.

##### Installing via the GUI

![Package loading in R](../images/loadPackages.png)

Package Loading in R

  1. Click on Packages -> Install package(s) 
  2. Select a CRAN mirror site from the drop-down window. 
  3. Select the appropriate package from the drop-down window and click OK. 
  4. A diagnostic message will appear on the screen and the package will be loaded into the appropriate library folder. 

There is a difference between installing a package and loading a package. The installation procedure will expose the new library/package to the R environment. This task only needs to occur once. In order to use the functions and classes within the newly installed package, the **library()** or **require()** commands need to be specified. This needs to occur every time the R workspace is re-loaded. The following command loads the newly installed package into memory.

```r
library(xts)
```  
---  
  
Here, we have made the assumption that the xts package was installed. If all goes well, nothing will appear on the screen. If the package has not been previously installed, R will issue an error message.

##### Installing via the Command Prompt

To install a package from the command prompt, simply issue the following command.

```r
install.packages("xts")
```  
---  
  
Like most functions, the **install.packages()** function takes multiple arguments. Various repositories and alternate file locations can be specified.

#### Useful Financial Packages

This class will briefly cover 2 packages. These are xts and quantmod. The xts package is a timeseries package and comes in very handy when dealing with ordered observations. The quantmod package allows for some extended graphing functionality and works well with xts.

##### xts()

Over the years, various practitioners and academics have written functions in R that deal with financial timeseries data. Given that the bulk of xts is written in C, it is ideal for fast lookups and indexing.  
An xts timeseries obect is composed of an index and coredata. The index contains the time information and the coredata contains the raw data. The following examples illustrate the creation and manipulation of xts objects. The first example is taken directly from the ?xts help file.

```r
mat1 <- matrix(c(1,2,3,4,5,6), nrow = 2, ncol = 3, byrow = TRUE)
```0  
---  
  
The first line of the previous example invokes the **data()** command. Typically, external packages include both functions and supporting data. The included data is meant to assist the user in understanding the functionality of the package. The **as.xts()** command casts the matrix into an xts object. In this example, the row-names of the matrix are converted into an index object and the rest of the data into the coredata. The **::** operator is used to extract specific data from the xts object.  
The next example extracts the index and the coredata from myXts.

```r
mat1 <- matrix(c(1,2,3,4,5,6), nrow = 2, ncol = 3, byrow = TRUE)
```1  
---  
  
The timeInfo object should now only contain the time-information. The command **class(timeInfo)** reveals that we are dealing with a POSIXct object. It is good practice to convert any timestamps into POSIXct from now on.  
Before we move on to more intricate timeseries examples, we need to address the conversion of strings into POSIXct objects. Typically, timestamps are formatted as strings initially when read in from Excel or other databases. Before we can convert the strings into POSIXct, we need to let R know what format we are dealing with. The next example illustrates this.

```r
mat1 <- matrix(c(1,2,3,4,5,6), nrow = 2, ncol = 3, byrow = TRUE)
```2  
---  
  
After converting a regular timeseries into an xts object, it becomes fairly easy to perform sub-setting, indexing and merging operations.

```r
mat1 <- matrix(c(1,2,3,4,5,6), nrow = 2, ncol = 3, byrow = TRUE)
```3  
---  
  
##### quantmod()

After installing and loading quantmod, we can use the following functions to visualize financial timeseries data. The following link provides some useful information about quantmod. [http://www.quantmod.com](http://www.quantmod.com/). The examples that follow are taken directly from the quantmod website.

```r
mat1 <- matrix(c(1,2,3,4,5,6), nrow = 2, ncol = 3, byrow = TRUE)
```4  
---  
  
[Next: R Lecture 4](Lecture 4.htm)

#### References

  * CRAN, CRAN Packages, http://cran.r-project.org 
  * Rmetrics, Rmetrics, http://www.rmetrics.org 
  * Quantmod, Package by Jeff Ryan, URL http://www.quantmod.com 
  * [Sample stock prices](http://www.rfortraders.com/wp-content/uploads/2012/11/stockPrices.txt)

