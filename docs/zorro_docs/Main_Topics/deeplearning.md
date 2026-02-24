---
title: Deep Learning for Market Prediction | Zorro Project
url: https://zorro-project.com/manual/en/deeplearning.htm
category: Main Topics
subcategory: None
related_pages:
- [adviseLong, adviseShort](advisor.htm)
- [Visual Studio](dlls.htm)
- [Zorro.ini Setup](ini.htm)
- [Links & Books](links.htm)
- [Trading Strategies](strategy.htm)
---

# Deep Learning for Market Prediction | Zorro Project

# Deep learning for market prediction

Machine learning algorithms can be used for market prediction with Zorro's [advise](advisor.htm) functions. Due to the low signal-to-noise ratio and to ever-changing market conditions of price series, analyzing the market is an ambitious task for machine learning. But since the price curves are not completely random, even relatively simple machine learning methods, such as in the **DeepLearn** script, can predict the next price movement with a better than 50% success rate. If the success rate is high enough to overcome transactions costs - at or above 55% accuracy - we can expect a steadily rising profit curve. 

Compared with other machine learning algorithms, such as Random Forests or Support Vector Machines, deep learning systems combine a high success rate with litle programming effort. A linear neural network with 8 analog inputs and 4 binary outputs had a structure like this:

![](../images/deepnet4.png)

A neural network for trading has normally multiple inputs for price, volume, order book data, indicators etc. and a single analog output that predicts a future price or momentum. Deep learning uses linear or special neural network structures (convolution layers, LSTM) with a large number of neurons and hidden layers. Some parameters common for most neural networks:

  * **Hidden layers** of a linear network are usually defined with a vector. For instance, **c(50,100,50)** defines 3 hidden layers, the first with 50, second with 100, and third with 50 neurons.
  * The **Activation** function converts the sum of neuron input values to the neuron output. Most often used are a **Rectifier** (RELU = rectified linear unit) that has a linear slope from 0 to 1, **Sigmoid** that saturates to 0 or 1, **Tanh** that saturates to -1 or +1, or **SoftMax** that approximates the highest input.
  * An **Epoch** is a training iteration over the entire data set. Training will stop once the predefined number of epochs is reached. More epochs mean better prediction, but longer training.
  * The **Learning rate** controls the step size for the **gradient descent** algorithm used in training. A lower learning rate means finer steps and possibly more precise prediction, but longer training time. The **Larning rate scale** is a multiplication factor for changing the learning rate after each iteration.
  * **Momentum** adds a fraction of the previous step to the current one. It prevents the gradient descent from getting stuck at a tiny local minimum or saddle point.
  * The **Batch size** is a number of random samples – a **mini batch** – taken out of the data set for a single training run. Splitting the data into mini batches speeds up training since the weight gradient is then calculated from fewer samples. The higher the batch size, the better is the training, but the more time it will take.
  * **Dropout** is a number of randomly selected neurons that are disabled during a mini batch. This way the net learns only with a part of its neurons, which can effectively reduce overfitting.

Here's a short description of installation and usage of 5 popular R or Python based deep learning packages: Deepnet, Torch, Keras/Tensorflow, H2O, and MxNet. Each comes with an short example of a (not really deep) linear neural net with one hidden layer.

### Deepnet (R)

Deepnet is a lightweight and straightforward neural net library supporting a stacked autoencoder and a Boltzmann machine. Produces good results when the feature set is not too complex. Note that the current version does not support more than 3 hidden layers. The basic R script for using a deepnet autoencoder in a Zorro strategy:  

```c
library('deepnet')

neural.train = function(model,XY)
{
  XY <- as.matrix(XY)
  X <- XY[,-ncol(XY)]
  Y <- XY[,ncol(XY)]
  Y <- ifelse(Y > 0,1,0)
  Models[[model]] <<- sae.dnn.train(X,Y,
  hidden = c(30),
  learningrate = 0.5,
  momentum = 0.5,
  learningrate_scale = 1.0,
  output = "sigm",
  sae_output = "linear",
  numepochs = 100,
  batchsize = 100)
}

neural.predict = function(model,X)
{
  if(is.vector(X)) X <- t(X)
  return(nn.predict(Models[[model]],X))
}

neural.save = function(name)
{
  save(Models,file=name) 
}

neural.init = function()
{
  set.seed(365)
  Models <<- vector("list")
}
```

### Torch (Python)

Torch is an open-source machine learning library and a scientific computing framework, created by the [Idiap Research Institute](https://en.wikipedia.org/wiki/IDIAP). Torch development moved in 2017 to [PyTorch](https://en.wikipedia.org/wiki/PyTorch), a port of the library to Python. It is currently the library that we recommend for high-end machine learning systems. You will need to program your script in [C++](dlls.htm) for using Torch.  
  
The installation: 

  * Download and install a 64 bit Python version. 
  * Edit **Zorro.ini** and enter the path to 64 bit Python on your PC, f.i. "**PythonPath64 = "C:\Users\YourName\AppData\Local\Programs\Python\Python312** ". 
  * Install Torch as described on [PyTorch.org](https://PyTorch.org). Usually, open the Windows command prompt, enter **pip3 install torch** and hit <Return>**.**

For your convenience, we've uploaded an image of a 64-bit Python installation with Torch and all required modules to [ https://opserver.de/down/Python312.zip](https://opserver.de/down/Python312.zip). Unzip it into some folder with full access, like your **Documents** folder, then enter its path under **PythonPath64** in [ZorroFix.ini](ini.htm). You need not run a setup or modify the enviroment. Other Python installations on your PC are not affected.

For using Torch, write a .cpp script and run it with the 64-bit Zorro version. Include **pynet.cpp** , which contains the **neural** function for Python. Then write the correcponding PyTorch script, with the same name as your strategy, but extension **.py**. Example:

```c
import torch
from torch import nn
import math
import numpy as np

Path = "Data"
Device = "cpu"
NumSignals = 8
Batch = 15
Split = 90
Epochs = 30
Neurons = 256
Rate = 0.001
Models = []

def neural_init():
    global Device
    Device = (
        "cuda"
        if torch.cuda.is_available()
        else "mps"
        if torch.backends.mps.is_available()
        else "cpu"
    )
    
def network():  
    model = nn.Sequential(
        nn.Linear(NumSignals,Neurons),
        nn.ReLU(),
        nn.Linear(Neurons,Neurons),
        nn.ReLU(),
        nn.Linear(Neurons,1),
        nn.Sigmoid()
    ).to(Device)
    loss = nn.BCELoss()
    optimizer = torch.optim.Adam(model.parameters(),lr=Rate)
    return model,loss,optimizer 

def data(Path):
    global NumSignals
    Xy = np.loadtxt(Path,delimiter=',')
    NumSignals = len(Xy[0,:])-1
    return Xy

def split(Xy):
    X = torch.tensor(Xy[:,0:NumSignals],dtype=torch.float32,device=Device)
    y = torch.tensor(Xy[:,NumSignals].reshape(-1,1),dtype=torch.float32,device=Device)
    y = torch.where(y > 0.,1.,0.)
    return X,y

def train(N,Xy):
    global Models
    model,loss,optimizer = network()
    X,y = split(Xy)
    for j in range(Epochs):
        for i in range(0,len(X),Batch):
            y_pred = model(X[i:i+Batch])
            Loss = loss(y_pred,y[i:i+Batch])
            optimizer.zero_grad()
            Loss.backward()
            optimizer.step()
        print(f"Epoch {j}, Loss {Loss.item():>5f}")
    Models.append(model)
    return Loss.item()*100

def neural_train(N,Path):
    Xy = data(Path)
    return train(N,Xy)

def neural_predict(N,X):
    if N >= len(Models):
        print(f"Error: Predict {N} >= {len(Models)}")
        return 0
    model = Models[N]
    X = torch.tensor(X,dtype=torch.float32,device=Device)
    with torch.no_grad():
        y_pred = model(X)
    return y_pred

def neural_save(Path):
    global Models
    torch.save(Models,Path)
    print(f"{len(Models)} models saved")
    Models.clear()

def neural_load(Path):
    global Models
    Models.clear()
    Models = torch.load(Path)
    for i in range(len(Models)):
        Models[i].eval()
    print(f"{len(Models)} models loaded")
```

### Keras / Tensorflow (R)

Tensorflow was a neural network kit by Google. They lately seem to have abanoned its further development, but it is still available. It supports CPU and GPU and comes with all needed modules for tensor arithmetics, activation and loss functions, covolution kernels, and backpropagation algorithms. So you can build your own neural net structure. Keras is a neural network shell that offers a simple interface for that, and is well described in a [book](links.htm).   
  
Keras is available as a R library, but installing it with Tensorflow requires also a Python environment such as Anaconda or Miniconda. The step by step installation for the CPU version: 

  * Download and install R 64 bit for Windows from [ cran.r-project.org/bin/windows/](https://cran.r-project.org/bin/windows/)..
  * Edit **Zorro.ini** and enter the path to the R terminal on your PC, f.i. "**C:\Program Files\R\R-4.2.1\bin\x64\Rterm.exe** ". 
  * Start the **R x64** console. Enter the command **install.packages("keras")** and hit <Return>**.**
  * Now install let Keras automatically install Miniconda and Tensorflow. The commands: **library('keras') -** **install_keras()**.

Installing the GPU version is more complex, since Tensorflow supports Windows only in CPU mode. So you need to either run Zorro on Linux, or run Keras and Tensorflow under WSL for GPU-based training. Multiple cores are only available on a CPU, not on a GPU. 

Example to use Keras in your strategy:

```c
library('keras')

neural.train = function(model,XY)
{
  X <- data.matrix(XY[,-ncol(XY)])
  Y <- XY[,ncol(XY)]
  Y <- ifelse(Y > 0,1,0)
  Model <- keras_model_sequential()
  Model %>%
    layer_dense(units=30,activation='relu',input_shape = c(ncol(X))) %>%
    layer_dropout(rate = 0.2) %>%
    layer_dense(units = 1, activation = 'sigmoid')

  Model %>% compile(
    loss = 'binary_crossentropy',
    optimizer = optimizer_rmsprop(),
    metrics = c('accuracy'))

  Model %>% fit(X, Y,
    epochs = 20, batch_size = 20,
    validation_split = 0, shuffle = FALSE)

  Models[[model]] <<- Model
}

neural.predict = function(model,X)
{
  if(is.vector(X)) X <- t(X)
  X <- as.matrix(X)
  Y <- Models[[model]] %>% predict_proba(X)
  return(ifelse(Y > 0.5,1,0))
}

neural.save = function(name)
{
  for(i in c(1:length(Models)))
    Models[[i]] <<- serialize_model(Models[[i]])
  save(Models,file=name) 
}

neural.load = function(name)
{
  load(name,.GlobalEnv)
  for(i in c(1:length(Models)))
    Models[[i]] <<- unserialize_model(Models[[i]])
}

neural.init = function()
{
  set.seed(365)
  Models <<- vector("list")
}
```

### MxNet (R)

MxNet was Amazon’s answer on Google’s Tensorflow, but did not gain the same popularity and is currently not further developed. It offers also tensor arithmetics and neural net building blocks on CPU and GPU, as well as high level network functions similar to Keras; a future Keras version will also support MxNet. Just as with Tensorflow, CUDA is supported, but not (yet) OpenCL, so you’ll need a Nvidia graphics card to enjoy GPU support. The code for installing the MxNet CPU version:

```c
cran <- getOption("repos")
cran["dmlc"] <- "https://s3-us-west-2.amazonaws.com/apache-mxnet/R/CRAN/"
options(repos = cran)
install.packages('mxnet')
```

The MxNet R script:  

```c
library('mxnet')

neural.train = function(model,XY)
{
  X <- data.matrix(XY[,-ncol(XY)])
  Y <- XY[,ncol(XY)]
  Y <- ifelse(Y > 0,1,0)
  Models[[model]] <<- mx.mlp(X,Y,
    hidden_node = c(30),
    out_node = 2,
    activation = "sigmoid",
    out_activation = "softmax",
    num.round = 20,
    array.batch.size = 20,
    learning.rate = 0.05,
    momentum = 0.9,
    eval.metric = mx.metric.accuracy)
}

neural.predict = function(model,X)
{
  if(is.vector(X)) X <- t(X)
  X <- data.matrix(X)
  Y <- predict(Models[[model]],X)
  return(ifelse(Y[1,] > Y[2,],0,1))
}

neural.save = function(name)
{
  save(Models,file=name) 
}

neural.init = function()
{
  mx.set.seed(365)
  Models <<- vector("list")
}
```

  

### Remarks:

  * !!  Machine learning packages (with exception of **Deepnet** , which usually always works) are often heavily optimized to a particular R and/or Python version. This can have the side effect that you'll end up with a slightly incompatible library that won't fully work with your R or Python version. In that case the training process fails with an error message like **NEURAL_INIT: failed**. Check out the support or user forums for suggestions and solutions. Sometimes you need to fall back to an earlier R, Python, or library version.
  * Most deep learning packages are optimized for large batch of data samples. Algo trading methods however normally train with many samples, but predict with only a single sample derived from the current price curve. This is ok for live trading, but can render the backtest very slow. Especially with the H2O package, but to some extent also with other packages. If you can choose, use GPU support only for training, and CPU for testing and trading. 
  * Batch prediction can speed up backtests enormously. An example with code can be found on the [ Zorro user forum](https://opserver.de/ubb7/ubbthreads.php?ubb=showflat&Number=485210#Post485210).

### Read on:

[Training](https://zorro-project.com/training.php), [Strategies](strategy.htm), [advise](advisor.htm)

[► latest version online](javascript:window.location.href = 'https://zorro-project.com/manual/en' + window.location.href.slice\(window.location.href.lastIndexOf\('/'\)\))
