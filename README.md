# Taxi Queries

Course project for Database Training Spring 2018.

## Install

Dependencies:

* CMake (v >= 3.5)
* METIS
* Python 3 (v >= 3.5.2 recommended)
* web.py (v == 0.40)

You can type the following commands to build the project:

```bash
mkdir build && cd build
cmake ..
make
```

## Simple test

A small GPTree test tool will be generated inside `build/` folder during installation. Please feel free to run and check if everything is alright.

```
./gptreetest
```

## Run

Input the following commands to start the server (assuming that you are still inside `build/` folder):

```bash
cd ../web
python3 main.py
```

After the server is started, open your browser and access the page with the URL below:

```
http://${YOUR_IP_ADDRESS}:8080
```
