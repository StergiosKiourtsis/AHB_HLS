options set Input/CppStandard c++11
options set Input/SearchPath /home/skiourtsis/Desktop/matchlib_toolkit/matchlib_connections/include -append
options set Input/SearchPath /home/skiourtsis/Desktop/matchlib_toolkit/matchlib/cmod/include -append
options set Input/SearchPath /home/skiourtsis/Desktop/matchlib_toolkit/boost_home/include -append
options set Input/SearchPath /home/skiourtsis/Desktop/matchlib_toolkit/ac_simutils/include -append
options set Input/SearchPath /home/skiourtsis/Desktop/matchlib_toolkit/ac_types/include -append
options set Input/SearchPath /home/skiourtsis/Desktop/matchlib_toolkit/systemc-2.3.3/src/sysc/packages -append
options set Input/SearchPath /opt/lib/matchlib_mentor_19-09-17/mentor/include -append
#set_working_dir .
#solution file add ../example1/fetch.h
#solution file add ../example1/drim4hls.h
#solution file add ../example1/top.cpp
#solution file add ../example1/writeback.h
#solution file add ../example1/execute.h
#solution file add ../example1/decode.h
#solution file set ../example1/top.cpp -exclude true
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
directive set /drim4hls/decode/sentinel.rom:rsc -MAP_TO_MODULE {[Register]}
directive set /drim4hls/decode/decode_th/regfile:rsc -MAP_TO_MODULE {[Register]}
directive set /drim4hls/decode/decode_th/sentinel:rsc -MAP_TO_MODULE {[Register]}
directive set /drim4hls/execute/csr.rom:rsc -MAP_TO_MODULE {[Register]}
directive set /drim4hls/execute/execute_th/csr:rsc -MAP_TO_MODULE {[Register]}
go architect
go allocate
go extract


