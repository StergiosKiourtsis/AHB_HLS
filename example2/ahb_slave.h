/*	
	@author VLSI Lab, EE dept., Democritus University of Thrace
*/

#ifndef __AHB_SLAVE_H__
#define __AHB_SLAVE_H__

#include "../src/global_example2.h"
#include "../src/ahb.h"
#include "systemc.h"

#define DATA_WIDTH 32

SC_MODULE(ahb_slave) {

  typedef AHB_CTR_MA<DATA_WIDTH> M_REQ_TYPE;
  typedef AHB_RSP_MA<DATA_WIDTH> M_RSP_TYPE;
  typedef AHB_CTR_SL<DATA_WIDTH> S_REQ_TYPE;
  typedef AHB_RSP_SL<DATA_WIDTH> S_RSP_TYPE;


  sc_in_clk   clk;
  sc_in <bool> rst;

  Connections::In<  S_REQ_TYPE > slave_in;
  Connections::Out< S_RSP_TYPE > slave_out;


  void do_cycle(){
  slave_in.Reset();
  slave_out.Reset();

  S_REQ_TYPE req_in;
  S_RSP_TYPE rsp;

  bool verify = false;

  bool isError = false;
  bool onError = false;
  bool addWait;
  bool control;
  sc_uint<32> r;
  
  int timer = 2;

  while(1) {
    wait();

    // READ
    if (slave_in.PopNB(req_in)) {
      if(req_in.HSel){
       std::cout << "slave"<< req_in.HAddr << std::endl; 
        verify = (req_in.HReady == 1);
        if (verify) {
          timer = 2;
          control = 1; 
        }
      }
    }


    // DO
    if (onError) {
      rsp.HResp = ahb::AHB_Encoding::AHBRESP::ERROR;
      rsp.HReadyout = 1;
      onError = false;
    } else {
      if (timer == 1) {
        isError = ((rand() % 100) >= 100);
      }
      if (isError) {
        std::cout << "ERROR" << std::endl;
        rsp.HResp = ahb::AHB_Encoding::AHBRESP::ERROR;
        rsp.HReadyout = 0;
        isError = false;
        onError = true;
      } else {
        addWait = ((rand() % 100) >= 50);
        rsp.HResp = ahb::AHB_Encoding::AHBRESP::OKAY;
        rsp.HReadyout = (addWait) ? 0 : 1;
        
        // HRDATA need fixing --> depending on HTRANS
        rsp.HRData = req_in.HAddr;
      }
    }

    if (verify && req_in.HTrans != ahb::AHB_Encoding::AHBTRANS::IDLE
               && req_in.HTrans != ahb::AHB_Encoding::AHBTRANS::BUSY
               && rsp.HResp != ahb::AHB_Encoding::AHBRESP::ERROR
               && control) {

      control = 0;
    }


    // WRITE
    slave_out.PushNB(rsp);

    timer --;
  } // End of while(1)
}


  SC_HAS_PROCESS(ahb_slave);
  ahb_slave(sc_module_name nm)
  {
    SC_THREAD(do_cycle);
    sensitive << clk.pos();
    async_reset_signal_is(rst, false);
  };
};

#endif //__AHB_SLAVE_H__
