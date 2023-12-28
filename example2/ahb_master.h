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
  for(int i=0;i<Slaves;i++){
    err_sb_resp_cnt[i] = 0;
    cor_sb_resp_cnt[i] = 0;
  }
  //err_sb_resp_cnt = 0;
  //cor_sb_resp_cnt = 0;

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
      if (rsp_in.HReady == 1) {
        if (rsp_in.HResp == OKAY) {
          if (!skip_next) {
            verify_resp(rsp_in);
          } else {
            skip_next = false;
          }
        }
        last_send = req_out;
        req_out = req;
        stored_trans.pop_front();
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
            //sb_lock->lock();

            //(*sb_tran[stored_decoder_for_req.front()]).push_back(req);
            //stored_decoder_for_req.pop_front();

            //sb_lock->unlock();
          }
        }
      }
    } else {
      req = stored_trans.front();
      skip_next = true;
    }


  }; // End of while(1)
}
  void verify_resp(M_RSP_TYPE &rsp) {
  /*
  bool found = false;
  sb_lock->lock();
  S_RSP_TYPE sb_rsp ;
  if (!(*sb_resp[stored_decoder_for_verify.front()]).empty()){
    sb_rsp = (*sb_resp[stored_decoder_for_verify.front()]).front();
    (*sb_resp[stored_decoder_for_verify.front()]).pop_front();

    if (sb_rsp.HResp == rsp.HResp && sb_rsp.HRData == rsp.HRData) {
      found = true;
    }
    if (!found) {
      err_sb_resp_cnt[stored_decoder_for_verify.front()]++;
      std::cout << "Received: " << rsp.HRData << ", Expected: " << sb_rsp.HRData << std::endl;
      std::cout << "Wrong response" << std::endl;
    } else{
      cor_sb_resp_cnt[stored_decoder_for_verify.front()]++;
          std::cout << "Correct response" << std::endl;
    }   
    stored_decoder_for_verify.pop_front();

  } else {
    std::cout << "No responses pushed into SB" << std::endl;
  } 
  sb_lock->unlock();
*/
	#ifndef __SYNTHESIS__
	std::cout << "Master Receided Response: " << rsp.HRData;
	std::cout << "   , Response: " << ((rsp.HResp ==0) ? "OKAY" : "ERROR") << std::endl;
	#endif
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

    sc_uint <ADDR_WIDTH> temp = decoder(addr,0);

    //stored_decoder_for_req.push_back( temp );
    //stored_decoder_for_verify.push_back( temp );
    
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
          
          //stored_decoder_for_req.push_back( temp );
          //stored_decoder_for_verify.push_back( temp );
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
          //stored_decoder_for_req.push_back( temp );
          //stored_decoder_for_verify.push_back( temp );
          i++;
        }
      }
    }

    tr = stored_burst.front();
    stored_burst.pop_front();

  }
  stored_trans.push_back(tr);


} // End of gen_write_trans

  sc_uint <ADDR_WIDTH> decoder(sc_uint <ADDR_WIDTH> address,sc_uint <ADDR_WIDTH> address_old){
  sc_uint <ADDR_WIDTH> temp;//not availiable
  if(address!=0){
    for(int i=0;i<Slaves;i++){
      //std::cout << " address :" << address << " --  memory["<<i<<"] :"<< memoryMap[i] << std::endl;
      if(address<memoryMap[i]){
        temp = i ;
        break;
      }
    }
  }else if(address == 0){
    temp = address_old;
  }
  return temp;
}



  int err_sb_resp_cnt[Slaves];
  int cor_sb_resp_cnt[Slaves];

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
