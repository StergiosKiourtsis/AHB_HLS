# AMBA AHB PROTOCOL: designed for High Level Synthesis

We work on a new project about AHB protocol. This project has as structure the nvidia_connections tool and SystemC library.  
On this version we figure out the version that user includes templated number of masters(managers, requestors) and slaves(subordinates, completers). 

# Repository Directory Structure

`example` : In this folder there is an example were the top module is the version core of a 32-bit RISC-V processor that I take from here, https://github.com/ic-lab-duth/DRIM4HLS/tree/main .
We have a 2 masters 2 slaves interconnection.

`src` : In this folder there is the source code, the ahb_multi_master.h, ahb.h, ahb_encoding.h, global_example1.h

`images` : In this folder there is store the images of this git repository

`codeExamples` : In this folder we store code examples that is used for the example. This codes ara programms, that can be run in DRIM4HLS.
the default option is fibonacci/fibonacci.txt

# How to define the templated parameters 
In the global_example1.h we define the number of master and slaves and also the upper limit of memory address of slaves.
For example we have 3 slaves and the memory map for this example is Slave0 address 0-3000 Slave1 3001-4000 Slave2 4001-8000.
So in global_example1.h we must define the limits 3000,4000,8000 and then in  ahb_multi_master.h lines 90-92 implement this address limits in the decoder.
If we want to run example 2 we must go to ahb_multi_master.h  at line 11 and include the globals_example2.h and implement map0,map1 and map2 

# Schematic
Simple schematic with one master and three slaves: 
![alt text][logo]

[logo]: https://github.com/StergiosKiourtsis/AHB_HLS/blob/main/images/AHBOneMaster.png "Logo Title Text 2"


# Getting Started

To simulate the AHB Crossbar module you need to download this librαries: 
`MATCHLIB_CONNECTIONS` : click [here](https://github.com/hlslibs/matchlib_connections.git)

`MATCHLIB` : click [here](http://github.com/NVlabs/matchlib.git)

`BOOST_HOME` : copy following commands to terminal
   
    git clone http://github.com/boostorg/preprocessor
    git clone http://github.com/boostorg/static_assert
    mkdir -p boost_home/include/boost
    mv preprocessor/include/boost/* boost_home/include/boost
    mv static_assert/include/boost/* boost_home/include/boost
    rm -rf preprocessor static_assert

`AC_SIMUTILS` : click [here](https://github.com/hlslibs/ac_simutils.git)

`SYSTEMC-2.3.3` : click [here](//www.accellera.org/images/downloads/standards/systemc/systemc-$SYSCVER.tar.gz)

# Synthesis 
In the example folder there are 2 .tcl files. 
hls_to_synth_top_AHB-INTERCONNECT.tcl is a file which can be used in catapult to run the example. With this file the TOP module will be the AHB interconnect module.
If we want to run the example with the core as top module we must use hls_to_synth_top_drim4hls.tcl. The purpose of two options is to see both the performance of 
AHB module and the effect replacement of point to point connection with a whole interconnection protocol.

