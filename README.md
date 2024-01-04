# AMBA AHB PROTOCOL: designed for High Level Synthesis

We work on a new project about AHB protocol. This project has as structure the nvidia_connections tool and SystemC library.  
On this version we figure out the version that user includes templated number of masters(managers,requestors) and slaves(subordinates,completers).
The definition of masters and slave numbers is implemented in src folder global.h file

# Repository Directory Structure

`example1/` :

`example2/` :

`src/` :


Simple schematic with one master and three slaves: 
![alt text][logo]

[logo]: https://github.com/StergiosKiourtsis/AHB_HLS/blob/main/images/AHBOneMaster.png "Logo Title Text 2"


# Getting Started

To simulate the AHB Crossbar module you need to download this libriries: 
`MATCHLIB_CONNECTIONS` : click [here](https://github.com/hlslibs/matchlib_connections.git)

`MATCHLIB` : 

`BOOST_HOME` : copy following commands to terminal
    git clone http://github.com/boostorg/preprocessor
    git clone http://github.com/boostorg/static_assert
    mkdir -p boost_home/include/boost
    mv preprocessor/include/boost/* boost_home/include/boost
    mv static_assert/include/boost/* boost_home/include/boost
    rm -rf preprocessor static_assert

# Any Header/subtitle or any text you want starting with hash
        Your text
    
`AC_SIMUTILS` : click [here](https://github.com/hlslibs/ac_simutils.git)

`SYSTEMC-2.3.3` :
