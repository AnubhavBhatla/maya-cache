# maya-cache

This repository contains the artifact for evaluating the results of the Maya Cache paper from ISCA'24. Follow the steps below to generate the results from the paper:

> [!WARNING]
> We have discovered a bug in the implementation of the PRINCE cipher for both Mirage and Maya. Please note that the current configurations are functioning correctly. However, changing the configuration may lead to unexpected behavior. This is not due to a security vulnerability in either design but rather a result of the buggy cipher code. If needed, please replace our implementation with any other publicly available lightweight cipher implementation.

------------------

## Obtain the code

`$ git clone https://github.com/casper-iitb/maya-cache`

------------------

## Requirements

To install all python related requirements, run

`$ pip install -r requirements.txt`

------------------

## Performance evaluation

#### Enter the performance-analysis directory

`$ cd maya-cache/performance-analysis`

#### Data dependencies

The zip file for the required GAP traces can be downloaded from https://utexas.app.box.com/s/2k54kp8zvrqdfaa8cdhfquvcxwh7yn85/folder/132804598561. Download it to the traces directory. 

To download the required SPEC CPU2017 traces, run the command: 

`$ ./traces.sh`

This also validates if the GAP benchmarks have been extracted in the correct directory.

In case `unzip` is not installed, please install it using `sudo apt install unzip`.

### Generate the required binaries

Run the following command to generate all the requied binaries:

`$ ./compile.sh`

### Run the performance simulations

`$ ./run.sh`

This step will take significant amount of time to complete (5-6 days).

### Generate the performance plots

Once all the performance simulations have been completed, the plots can be generated using

`$ ./plot.sh 0`

We have also provided our simulation results which can be used to generate the plots using:

`$ ./plot.sh 1`

---------------

## Security evaluation

### Enter the security-analysis directory

`$ cd ../security-analysis`


### Generate the required binaries

`$ make`

### Run the security simulations

`$ ./run.sh`

### Generate the security plots

Once all the security simulations have been completed, the plots can be generated using

`$ python3 plot.py`

----------------

## Expected Results

In the performance-analysis directory, two graphs are generated, namely, `fig1.pdf`, `fig10.pdf`. 

In the security-analysis directory, two graphs are generated, namely, `fig7.pdf`, `fig8.pdf`.

----------------
