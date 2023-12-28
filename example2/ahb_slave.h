/*	
	@author VLSI Lab, EE dept., Democritus University of Thrace
*/

#ifndef __AHB_SLAVE_H__
#define __AHB_SLAVE_H__

#include "../src/global_example2.h"
#include "../src/ahb.h"
#include "systemc.h"

#define DATA_WIDTH 32

//template <int DATA_WIDTH>
//template <int Masters>
SC_MODULE(ahb_slave) {

  typedef AHB_CTR_MA<DATA_WIDTH> M_REQ_TYPE;
  typedef AHB_RSP_MA<DATA_WIDTH> M_RSP_TYPE;
  typedef AHB_CTR_SL<DATA_WIDTH> S_REQ_TYPE;
  typedef AHB_RSP_SL<DATA_WIDTH> S_RSP_TYPE;


  sc_in_clk   clk;
  sc_in <bool> rst;

  Connections::In<  S_REQ_TYPE > slave_in;
  Connections::Out< S_RSP_TYPE > slave_out;


  int err_sb_tran_cnt[Masters];
  int cor_sb_tran_cnt[Masters];

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
  
  for(int i=0;i<Masters;i++){
    err_sb_tran_cnt[i] = 0;
    cor_sb_tran_cnt[i] = 0;
  }
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

    //how to find which condition isn't accepted
    if (verify && req_in.HTrans != ahb::AHB_Encoding::AHBTRANS::IDLE
               && req_in.HTrans != ahb::AHB_Encoding::AHBTRANS::BUSY
               && rsp.HResp != ahb::AHB_Encoding::AHBRESP::ERROR
               && control) {
      verify_request(req_in);

      //sb_lock->lock();
      //(*sb_resp[0]).push_back(rsp);

      //sb_lock->unlock();
      control = 0;
    }


    // WRITE
    slave_out.PushNB(rsp);

    timer --;
  } // End of while(1)
}

  void verify_request(S_REQ_TYPE &req) {
  /*
  sb_lock->lock();
    if (!(*sb_tran[0]).empty()) {
      bool found = false;
      M_REQ_TYPE sb_req = (*sb_tran[0]).front();
      (*sb_tran[0]).pop_front();

      if (sb_req.HTrans == req.HTrans && sb_req.HAddr == req.HAddr && sb_req.HBurst == req.HBurst) {
        found = true;
      }

      if (!found) {
        err_sb_tran_cnt[0]++;
        std::cout << "Received: " << req.HAddr << ", Expected: " << sb_req.HAddr << std::endl;
        std::cout << "Wrong request" << std::endl;
      } else {
        cor_sb_tran_cnt[0]++;
        //std::cout << "Correct request" << std::endl;
      }
    } else {
      std::cout << "No Requests pushed into SB" << std::endl;
    }
  sb_lock->unlock();
	*/
	#ifndef __SYNTHESIS__
	std::cout << "Slave Receided Request: " << req.HAddr << std::endl;
	#endif

}

  SC_HAS_PROCESS(ahb_slave);
  ahb_slave(sc_module_name nm)
  {
    SC_THREAD(do_cycle);
    sensitive << clk.pos();
    async_reset_signal_is(rst, false);
  };
};

//template <int SIZE>
//void ahb_slave:: do_cycle()  // End of do_cycle



//template <int DATA_WIDTH>
//void ahb_slave:: verify_request(S_REQ_TYPE &req)

#endif //__AHB_SLAVE_H__
