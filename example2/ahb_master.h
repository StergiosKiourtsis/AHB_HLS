/*	
	@author VLSI Lab, EE dept., Democritus University of Thrace
*/

#ifndef __AHB_MASTER_H__
#define __AHB_MASTER_H__

#include <vector>
#include "systemc.h"
#include "../src/global_example2.h"
#include "../src/ahb.h"
#include "nvhls_connections.h"

#define DATA_WIDTH 32

SC_MODULE(ahb_master) {

  typedef AHB_CTR_MA<DATA_WIDTH> M_REQ_TYPE;
  typedef AHB_RSP_MA<DATA_WIDTH> M_RSP_TYPE;
  typedef AHB_RSP_SL<DATA_WIDTH> S_RSP_TYPE;

  // In - Out Ports
  sc_in_clk   clk;
  sc_in <bool> rst;

  Connections::Out< M_REQ_TYPE > master_out;
  Connections::In<  M_RSP_TYPE > master_in;

  sc_uint <ADDR_WIDTH> memoryMap[Slaves];
  int start_point;
  bool mastlock_enable;

  // Transactions Queue (after Generation)
  std::deque< M_REQ_TYPE >    stored_trans;
  std::deque< M_REQ_TYPE >    stored_burst;

  // Functions
  void do_cycle(){

  master_in.Reset();
  master_out.Reset();

  sc_uint<1> ERROR = ahb::AHB_Encoding::AHBRESP::ERROR;
  sc_uint<1> OKAY  = ahb::AHB_Encoding::AHBRESP::OKAY;
  sc_uint<1> IDLE  = ahb::AHB_Encoding::AHBTRANS::IDLE;
  sc_uint<1> BUSY  = ahb::AHB_Encoding::AHBTRANS::BUSY;
  

  bool gen_new_trans = true;
  bool starter = true;

  sc_uint<32> address = start_point;
  sc_uint <ADDR_WIDTH> address_old = 0;
  
  sc_uint <ADDR_WIDTH> select;

  M_REQ_TYPE req, last_send, req_out;
  M_RSP_TYPE rsp_in;

  int count = 0;
  bool skip_next = false;

  while(1) {

    wait();
//    if (stored_trans.size() > 15) {
//      gen_new_trans = false;
//    }
std::cout << " loop start" << std::endl;
    if (!stored_burst.empty()) {
      stored_trans.push_back(stored_burst.front());
      stored_burst.pop_front();
    } else {
    //with mast lock
      if(mastlock_enable){
        if(count==10 || count==13 || count==14 || count ==20){
          if (gen_new_trans) {
            count++;
            gen_trans(address,1);
            address=100*(rand( )%70+1);
          }
        }else{
          if (gen_new_trans) {
            count++;
            gen_trans(address,0);
            address=100*(rand( )%70+1);
          }
        }
      }else{
      //without mast lock
        if(gen_new_trans){
          gen_trans(address,0);
          address=100*(rand( )%70+1);
        }        
      }
    }

    // Read Input
    if (master_in.PopNB(rsp_in)) {
      /*if(starter){
				last_send = req_out;
        req_out = req;
        stored_trans.pop_front();
        starter = 0;
			}*/
      if (rsp_in.HReady == 1 || starter) {
        /*if (rsp_in.HResp == OKAY) {
          if (!skip_next) {
            //verify_resp(rsp_in);
            C
          } else {
            skip_next = false;
          }
        }*/
        last_send = req_out;
        req_out = req;
        stored_trans.pop_front();
        starter = 0;
      } else if (rsp_in.HResp == ERROR) {
        stored_trans.push_front(req_out);
        stored_trans.push_front(last_send);
        req_out.HTrans = IDLE;
      }
    }
    // Push Output
    master_out.PushNB(req_out);

    if (rsp_in.HResp == OKAY) {
      if (rsp_in.HReady == 1 || req.HTrans == IDLE || req.HTrans == BUSY) {
        if (!stored_trans.empty()) {
          req = stored_trans.front();
          if (req.HTrans != IDLE && req.HTrans != BUSY) {
            std::cout << "Pushing into SB"<<" <-- " << req.HAddr << std::endl;
          }
        }
      }
    } else {
      req = stored_trans.front();
      skip_next = true;
    }


  }; // End of while(1)
}

  void gen_trans(sc_uint<ADDR_WIDTH> in_addr,bool mastlock){

  sc_uint<32> addr = in_addr;
  M_REQ_TYPE tr;

  bool idleTrans = (rand() % 100) >= 80;  // idle transfers
  sc_uint<1> wrt = ((rand() % 100) >= 50) ? 1 : 0;   // write or read transfer
  //sc_uint<1> lck = ((rand() % 100) >= 100) ? 1 : 0;   // locked transfer
  sc_uint<1> lck = mastlock;
  
  if (idleTrans) {
    // IDLE
    tr.HTrans = ahb::AHB_Encoding::AHBTRANS::IDLE;
  }
  else {
    // BURST
    //   sc_uint<3> burst_type = 0;
    sc_uint<3> burst_type = rand() % 8;

    int iter = (burst_type == 0) ? 0 :
               (burst_type == 1) ? 0 :
               (burst_type < 4)  ? 3 :
               (burst_type < 8)  ? 7 : 15;


    tr.HAddr = addr;
    tr.HTrans = ahb::AHB_Encoding::AHBTRANS::NONSEQ;
    tr.HSize = rand() % 8;
    tr.HProt = 0; // temporary
    tr.HMastLock = lck;
    tr.HBurst = burst_type;
    tr.HWrite = wrt;
    tr.HWData = rand() % 50;

    stored_burst.push_back(tr);
    
    if (burst_type == 1) {
      // undefined beats - INCR
      bool isBusy = false;
      bool keepGoing = true;
      while (keepGoing) {
        addr +=4;



        if (isBusy) {
          tr.HAddr = addr;
          tr.HTrans = ahb::AHB_Encoding::AHBTRANS::BUSY;
          tr.HBurst = burst_type;
          tr.HWData = rand() % 50;
        } else {
          tr.HAddr = addr;
          tr.HTrans = ahb::AHB_Encoding::AHBTRANS::SEQ;
          tr.HBurst = burst_type;
          tr.HWData = rand() % 50;
          
        }

        stored_burst.push_back(tr);
        isBusy = (rand() % 100) >= 80;
        keepGoing = false;
      }
    }
    else {
      // defined beats
      int i = 1;
      bool isBusy;
      while (i < iter) {
        addr +=4;



        isBusy = (rand() % 100) >= 80;

        if (isBusy) {
          tr.HAddr = addr;
          tr.HTrans = ahb::AHB_Encoding::AHBTRANS::BUSY;
          tr.HBurst = burst_type;
          tr.HWData = rand() % 50;
        } else {
          tr.HAddr = addr;
          tr.HTrans = ahb::AHB_Encoding::AHBTRANS::SEQ;
          tr.HBurst = burst_type;
          tr.HWData = rand() % 50;
        }

        stored_burst.push_back(tr);

        if (!isBusy) {
          i++;
        }
      }
    }

    tr = stored_burst.front();
    stored_burst.pop_front();

  }
  stored_trans.push_back(tr);


} // End of gen_write_trans

  // Constructor
  SC_HAS_PROCESS(ahb_master);
  ahb_master(sc_module_name nm)
  {
    SC_THREAD(do_cycle);
    sensitive << clk.pos();
    async_reset_signal_is(rst, false);
  };
};

#endif //__AHB_MASTER_H__
