---
title: Matrix functions
url: https://zorro-project.com/manual/en/matrix.htm
category: Functions
subcategory: None
related_pages:
- [series](series.htm)
---

# Matrix functions

# Vector and matrix functions

The following functions can be used for vector and 2-dimensional matrix algebra. The matrix or vector type **mat** is a pointer to the.**MATRIX** struct.

## matrix(int rows,_int cols_): mat 

Creates a vector or a matrix with the given number of rows and columns. The matrix is initialized to zeros. If **cols** is omitted, an identity matrix with **1** s in the diagonal is created. 

### Parameters:

**rows** \- number of rows of the matrix, or **1** for defining a row vector.   
**cols** \- number of columns, or **1** for defining a column vector. Omit this for a square identity matrix. 

## me(mat X, int row, int col): var 

Macro for accessing an element of the matrix **X** at a given row and column, with no range check. 

## ve(mat V, int n): var 

Macro for accessing the **n** -th element of the row or column vector **V** , with no range check. 

### Parameters:

**X, V** \- matrix to be accessed.   
**row** \- row number of the element, starting with 0.   
**col** \- column number of the element, starting with 0. 

## matSet(mat X, mat A): mat 

Copies matrix **A** to matrix **X** , and returns **X**. **A** and **X** must have the same number of rows and columns. 

### Parameters:

**X** \- destination matrix or vector   
**A** \- source matrix or vector 

## matSet(mat X, int row, int col, mat A): mat 

Copies matrix **A** to a sub-range of matrix **X** that starts at the given row and colunm, and returns **X**. **A** and **X** can have different numbers of rows and columns. 

### Parameters:

**X** \- destination matrix  
**A** \- source matrix  
**row** \- row number of the block, starting with 0.   
**col** \- column number of the block. 

## matSet(mat X, var c): mat 

Sets all elements of matrix **X** to the value **c** , and returns **X**. 

### Parameters:

**X** \- destination matrix.  
**c** \- value to be set. 

## matTrans(mat X, mat A): mat 

Copies the transpose of **A** to **X** , and returns **X**. The number of columns of **X** must be identical to the number of rows of **A** , and vice versa. 

### Parameters:

**X** \- destination matrix.  
**A** \- source matrix. 

## matAdd(mat X, mat A, mat B): mat 

Adds**A** and **B** , stores the result in **X** , and returns **X**. **A** , **B** , and **X** must have the same number of rows and columns. 

### Parameters:

**X** \- destination matrix.  
**A,B** \- source matrices. 

## matSub(mat X, mat A, mat B): mat 

Subtracts **B** from **A** , stores the result in **X** , and returns **X**. **A** , **B** , and **X** must have the same number of rows and columns. 

### Parameters:

**X** \- destination matrix.  
**A,B** \- source matrices. 

## matMul(mat X, mat A, mat B): mat 

Multiplies **A** with **B** , stores the result in **X** , and returns **X**. **X** and **A** must have the same number of rows, **X** and **B** must have the same number of columns, and the **B** number of ****rows must be identical to the**A** number of ****columns.

### Parameters:

**X** \- destination matrix.  
**A,B** \- source matrices. 

## matScale(mat X, var c): mat 

Multiplies all elements of **X** with **c** , and returns **X**. 

### Parameters:

**X** \- destination matrix.  
**c** \- multiplication factor. 

## matSort(mat X, int col): mat 

Sorts all rows of the matrix by the given column in ascending or descending order, and returns the sorted matrix. 

### Parameters:

**X** \- destination matrix.  
**col** \- column number, negative for descending order.  

## matSaveCSV(mat X, string FileName) 

Saves the matrix **X** to a CSV file for further evaluation with Excel or other programs.

### Parameters:

**X** \- source matrix.  
**FileName** \- CSV file name.   
  

### Remarks:

  * Matrices are created in the **INITRUN** and released after the **EXITRUN**. Just like [series](series.htm), **matrix()** creation must happen in the same order at any bar, preferably at the begin of the **run** function. 
  * The **mat** type is a pointer of the **MATRIX** struct defined in **trading.h**. The numbers of rows and columns are stored in the **- >rows** and **- >cols** elements, a pointer to the content is stored in the **- >dat** element. 
  * Vectors can be defined as matrices with **rows = 1** or **cols = 1**. The dot product is the multiplication of a row vector with a column vector. 
  * If matrix arithmetics is not required and the row/column number is fixed, matrices and vectors can be alternatively defined as **var** [arrays](array.htm), like **var MyVector[3]** or **var MyMatrix[3][3]**. 

### Example:

```c
void matPrintf(mat X)
{
  int i,j;
  printf("\n");
  for(i=0; i < X->rows; i++) {
    for(j=0; j < X->cols; j++)
      printf("%.0f ",me(X,i,j));
    printf("\n");
  }
}

void main()
{
  int i,j;
  mat A = matrix(3,4);
  for(i=0; i < A->rows; i++)
    for(j=0; j < A->cols; j++)
      me(A,i,j) = i+j;
  matPrintf(A);

  mat B = matTrans(matrix(A->cols,A->rows),A);
  matPrintf(B);

  mat C = matrix(B->rows,A->cols);
  matMul(C,B,A);
  matPrintf(C);
  
  mat D = matrix(8);
  matSet(D,4,4,C);
  matPrintf(D);
}
```

### See also:

[series](series.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
