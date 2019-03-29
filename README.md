# Distributed String and Suffix Sorting
[![Build Status](https://travis-ci.org/kurpicz/dsss.svg?branch=master)](https://travis-ci.org/kurpicz/dsss)

## What is it?
We implemented two distributed suffix array construction algorithms (SACAs) and distributed string sorting algorithsm.
A detailed description and benchmarks of the implemented algorithms can be found [here](https://epubs.siam.org/doi/10.1137/1.9781611975499.3).

	@inproceedings{DBLP:conf/alenex/0001K19,
  		author    = {Johannes Fischer and
		             Florian Kurpicz},
 		 title    = {Lightweight Distributed Suffix Array Construction},
  		booktitle = {Proceedings of the Twenty-First Workshop on
		             Algorithm Engineering and Experiments ({ALENEX}).},
  		pages     = {27--38},
  		publisher = {{SIAM}},
  		year      = {2019},
	}

## How to get it?
First clone this repository, then build all executables.
```sh
git clone https://github.com/kurpicz/dsss.git
cd dsss
git submodule update --init
mkdir build
cd build
cmake ..
make
```

Now there will be two executables in `build/benchmark`: (1) `dsaca` can be used to run the distributed suffix array construction algorithms and (2) `dss` can be used to run the distrubuted string sorting algorithms.
