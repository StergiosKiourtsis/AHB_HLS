In this folder there is sourse code and the file global_example1.h,which can be used to customize the protocol.
►In ahb.h file are created 4 custom data types.
►In ahb_encoding.h file are created 3 enum data types to encode some situation of the protocol such as 4 HTrans option IDLE(0), BUSY(1), SEQ(2), NONSEQ(3)  
►In global_example1.h use can customize the templated module of ahb_multi_master.h file. The number of masters, slaves and memory map can be changed
►In ahb_multi_master.h is contained the basic design of the module ahb4hls. This is a crossbar that uses AHB protocol to connect master to slaves.
