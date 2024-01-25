options set Input/CppStandard c++11
options set Input/SearchPath /home/skiourtsis/Desktop/matchlib_toolkit/matchlib_connections/include -append
options set Input/SearchPath /home/skiourtsis/Desktop/matchlib_toolkit/matchlib/cmod/include -append
options set Input/SearchPath /home/skiourtsis/Desktop/matchlib_toolkit/boost_home/include -append
options set Input/SearchPath /home/skiourtsis/Desktop/matchlib_toolkit/ac_simutils/include -append
#options set Input/SearchPath /home/skiourtsis/Desktop/matchlib_toolkit/ac_types/include -append
options set Input/SearchPath /home/skiourtsis/Desktop/matchlib_toolkit/systemc-2.3.3/src/sysc/packages -append
options set Input/SearchPath /opt/lib/matchlib_mentor_19-09-17/mentor/include -append
solution file add ../examplewithBlockingChannel/top.cpp  
go compile
solution library add nangate-45nm_beh -- -rtlsyntool OasysRTL -vendor Nangate -technology 045nm
solution library add ram_nangate-45nm-dualport_beh
solution library add ram_nangate-45nm-separate_beh
solution library add ram_nangate-45nm-singleport_beh
solution library add ram_nangate-45nm-register-file_beh
solution library add rom_nangate-45nm_beh
solution library add rom_nangate-45nm-sync_regin_beh
solution library add rom_nangate-45nm-sync_regout_beh
go libraries
directive set -CLOCKS {clk {-CLOCK_PERIOD 10 -CLOCK_HIGH_TIME 5 -CLOCK_OFFSET 0.000000 -CLOCK_UNCERTAINTY 0.0}}
go assembly
directive set /ahb4HLS/do_cycle/while -PIPELINE_INIT_INTERVAL 1
directive set /ahb4HLS/do_cycle/while:for -UNROLL yes
directive set /ahb4HLS/do_cycle/while:for#1 -UNROLL yes
directive set /ahb4HLS/do_cycle/while:for#2 -UNROLL yes
directive set /ahb4HLS/do_cycle/while:for#3 -UNROLL yes
directive set /ahb4HLS/do_cycle/while:for#3:for -UNROLL yes
directive set /ahb4HLS/do_cycle/while:for#3:if:if:for -UNROLL yes
directive set /ahb4HLS/do_cycle/while:for#4 -UNROLL yes
directive set /ahb4HLS/do_cycle/while:for#4:for -UNROLL yes
directive set /ahb4HLS/do_cycle/ahb4HLS::decoder:for -UNROLL yes
directive set /ahb4HLS/do_cycle/ahb4HLS::arbiter:if:for -UNROLL yes
directive set /ahb4HLS/do_cycle/ahb4HLS::arbiter:else:else:for -UNROLL yes
directive set /ahb4HLS/do_cycle/while:for#5 -UNROLL yes
directive set /ahb4HLS/do_cycle/while:for#6 -UNROLL yes
go architect
go allocate
go extract

