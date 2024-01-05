/*	
	@author VLSI Lab, EE dept., Democritus University of Thrace
*/

#ifndef __IC_TOP_H__
#define __IC_TOP_H__

#include "systemc.h"
#include "ahb.h"
#include "global_example1.h"
//#include "global_example2.h"

#define DATA_WIDTH 32

//#pragma hls_design top
SC_MODULE(ahb4HLS) {

  enum STATES {FREE = 0, WAIT_CYCLE = 1, DATA = 2};
  
  sc_uint <ADDR_WIDTH> memoryMap[Slaves];
  

  typedef AHB_CTR_MA<DATA_WIDTH> M_REQ_TYPE;
  typedef AHB_RSP_MA<DATA_WIDTH> M_RSP_TYPE;
  typedef AHB_CTR_SL<DATA_WIDTH> S_REQ_TYPE;
  typedef AHB_RSP_SL<DATA_WIDTH> S_RSP_TYPE;

  sc_in <bool> clk;
  sc_in <bool> rst;

  Connections::In<  M_REQ_TYPE > req_from_master[Masters];
  Connections::Out< M_RSP_TYPE > rsp_to_master[Masters];

  Connections::Out< S_REQ_TYPE > req_to_slave[Slaves];
  Connections::In<  S_RSP_TYPE > rsp_from_slave[Slaves];
  

  void do_cycle(){

  M_REQ_TYPE req_in[Masters];
  M_RSP_TYPE rsp_out[Masters];

  S_RSP_TYPE rsp_in[Slaves];
  S_REQ_TYPE req_out[Slaves];

  S_REQ_TYPE internal_req_out[Slaves][Masters];

  sc_uint<READY_WIDTH> ready[Masters];
  sc_uint <ADDR_WIDTH> decoder_result[Masters];
  sc_uint <ADDR_WIDTH> decoder_result_old[Masters];
  bool idle_transaction[Masters];
  
  int reqs[Slaves];
  int matrix_masters_control[Masters];
  int matrix_slaves_control[Slaves];
  int address[Masters];
  int old_address[Masters];
  int mastlock;
  int counter;
  STATES state[Masters];

   //reset
  for(int i=0;i<Masters;i++){
    rsp_to_master[i].Reset();
    req_from_master[i].Reset();
  }
  for(int i=0;i<Slaves;i++){
    rsp_from_slave[i].Reset();
    req_to_slave[i].Reset();
  }

  mastlock=Masters+1;
  counter = 0 ;

  for(int i=0;i<Masters;i++){
    ready[i]=1;
    decoder_result[i]=Slaves+1;
    decoder_result_old[i]=Slaves+1;
    state[i] = FREE;
    matrix_masters_control[i]=Masters+1;
    address[i]=1;
    old_address[i]=1;
  }
  for(int i=0;i<Slaves;i++){
    matrix_slaves_control[i]=Slaves+1;
  }
  ///////////////////////////////////////
  //-------memory map definition-------//
  ///////////////////////////////////////
  memoryMap[0]=map0;
  memoryMap[1]=map1;
  //memoryMap[2]=map2;
  
  #pragma hls_pipeline_init_interval 1
  while(1) {
      wait();
    // Read inputs from master and slave.
    for(int i=0;i<Masters;i++){
        req_from_master[i].PopNB(req_in[i]);
    }
    for(int i=0;i<Slaves;i++){
      rsp_from_slave[i].PopNB(rsp_in[i]);
    }
    for(int i=0;i<Masters;i++){
      if(req_in[i].HMastLock){
        mastlock=i;
        if(counter!=1){
          counter=2;
        }
        break;
      }else{
        mastlock=Masters+1;
      }
      if(i==1){
        counter = 0;
      }
    }
   
      for(int i=0;i<Masters;i++){
          if(req_in[i].HTrans!=0 && req_in[i].HTrans!=1){
          //if(req_in[i].HAddr!=address[i]){
            old_address[i] = address[i];
            address[i]=req_in[i].HAddr;
          }
        

        if (address[i]!=old_address[i]) {
          decoder_result_old[i] = decoder_result[i];
          decoder_result[i] = decoder(address[i],decoder_result_old[i]);
          internal_req_out[decoder_result[i]][i] = req_in[i];
        }

        //set select signals
        for(int j=0;j<Slaves;j++){
          if(j == decoder_result[i]){
            internal_req_out[j][i].HSel = 1;
          }else{
            internal_req_out[j][i].HSel = 0;
          } 
        }

        if (state[i] == FREE) {
          // FREE: There isn't any transaction active.

          rsp_out[i].HResp = ahb::AHB_Encoding::AHBRESP::OKAY;
          rsp_out[i].HRData = 89;
          
          // if Master sent a request, move to next state,
          // else stay idle and send OKAY response.
          idle_transaction[i] = (req_in[i].HTrans == ahb::AHB_Encoding::AHBTRANS::IDLE || req_in[i].HTrans == ahb::AHB_Encoding::AHBTRANS::BUSY);
          ready[i] = idle_transaction[i];
          state[i] = (idle_transaction[i]) ? FREE : WAIT_CYCLE;
          rsp_out[i].HReady =  idle_transaction[i];
          //rsp_out[i].HReady =  0;
        } else if (state[i] == WAIT_CYCLE) {
          // WAIT_CYCLE: Master has pushed a request into the bus and 
          //             is now waiting for the Slave's response. 

          rsp_out[i].HResp = ahb::AHB_Encoding::AHBRESP::OKAY;
          rsp_out[i].HRData = 90;
          //4864
          ready[i] = 0;
          state[i] = DATA;
          rsp_out[i].HReady = 0 ;
        } else {
          // DATA: Slave has read the request and now the bus is 
          //       pushing slave's responses to Master, until 
          //       HReadyout = 1 (Correct data send).

          //there is a problem sometimes when change slave , select signal change but responde delay a cycle to read
          //this is working
          std::cout<<"see decoder : "<<decoder_result[i]<<" , and matrix control : "<< matrix_masters_control[i] <<std::endl;
          if(matrix_masters_control[i]==decoder_result[i]){
						
						std::cout<<"rsp_out[ "<<i<<"] is :"<<rsp_out[i]<<"and rsp_in[decoder_result[i]] is "<<rsp_in[decoder_result[i]]<<std::endl;
            rsp_out[i] = rsp_in[decoder_result[i]];
            
            ready[i] = rsp_in[decoder_result[i]].HReadyout;
            std::cout<<"After assigment"<<std::endl;
            std::cout<<"rsp_out[ "<<i<<"] is :"<<rsp_out[i]<<"and ready ["<<i<<"] is "<<ready[i]<<std::endl;
            if (rsp_in[decoder_result[i]].HReadyout == 1) {
              state[i] = FREE;
              matrix_masters_control[i]=Masters+1;
              matrix_slaves_control[decoder_result[i]]=0;
            }
          }else{
            
            rsp_out[i].HResp = ahb::AHB_Encoding::AHBRESP::OKAY;
            rsp_out[i].HRData = 91;
            ready[i] = 0;
            state[i] = WAIT_CYCLE;
            rsp_out[i].HReady = 0;
          }
      }//end state control


        //rsp_out[i].HReady = ready[i];

        for(int j=0;j<Slaves;j++){
         if(j==decoder_result[i]){
            //internal_req_out[j][i].HReady = ready[i];
            //if(state[i] == WAIT_CYCLE){
							internal_req_out[j][i].HReady = internal_req_out[j][i].HSel;
						//}else{
							//internal_req_out[j][i].HReady = 0;
						//}
          }else{
            internal_req_out[j][i].HReady = 0;
          }
        }
        				
      }//end of main master loop

    //arbitration 
    for(int i=0;i<Slaves;i++){
      bool flag=0;
      for(int k =0;k<Masters;k++){
        if(internal_req_out[i][k].HSel){
          flag = 1; 
          break;
        }
      }
      if(flag){
        arbiter(internal_req_out[i],req_out[i],reqs[i],matrix_masters_control,i,matrix_slaves_control[i],mastlock);
      }
    }
        
    // Push response to Master and request to Slave.
    for(int i=0;i<Masters;i++){
				rsp_to_master[i].PushNB(rsp_out[i]);
    }

    for(int i=0;i<Slaves;i++){
				req_to_slave[i].PushNB(req_out[i]);
    }
    
    #ifndef __SYNTHESIS__
    std::cout << " -- Simulation Stopped @ " << sc_time_stamp() << " -- " << std::endl;
    for(int i=0;i<Masters;i++){
      if(decoder_result[i]!=4){
        std::cout << "\t\t Master"<<i<<std::endl;
        std::cout << "\tReceived from MASTER "<< i <<" --> Transfer Type: " << ((req_in[i].HTrans==0) ? "IDLE" :
                                                                  (req_in[i].HTrans==1) ? "BUSY" : "PCKT");
        std::cout << ", Address: " << req_in[i].HAddr << std::endl;
        std::cout << "\tResponse to MASTER  "<< i <<"  <-- DATA: " << rsp_out[i].HRData << ", Ready: " << /*req_out[decoder_result[i]]*/rsp_out[i].HReady;
        std::cout << ", Response: " << ((rsp_out[i].HResp == 0) ? "OKAY" : "ERROR") << std::endl;

        std::cout << "\tSending to SLAVE"<< decoder_result[i] <<  " <-- Transfer Type: "<<((req_out[decoder_result[i]].HTrans==0) ? "IDLE" :
                                                                  (req_out[decoder_result[i]].HTrans==1) ? "BUSY" : "PCKT");
        std::cout << ", Address: " << req_out[decoder_result[i]].HAddr << std::endl;
        std::cout << "\tResponse from SLAVE" << decoder_result[i] <<  "  --> DATA: " << rsp_in[decoder_result[i]].HRData << ", Ready: " << rsp_in[decoder_result[i]].HReadyout;
        std::cout << ", Response: " << ((rsp_in[decoder_result[i]].HResp ==0) ? "OKAY" : "ERROR") << std::endl;
      }
    }
    #endif
  }//end while loop
}//end do loop

  sc_uint <ADDR_WIDTH> decoder(sc_int <ADDR_WIDTH> address,sc_uint <ADDR_WIDTH> address_old){
  sc_uint <ADDR_WIDTH> temp;
  for(int i=0;i<Slaves;i++){
    if(address<memoryMap[i]){
      temp = i ;
      break;
    }
  }
  return temp;
}

  void arbiter(S_REQ_TYPE masters_req[Masters],S_REQ_TYPE &req_out,int &reqs,int matrix_masters_control[Masters],int slave_number,int &matrix_slaves_control,int mastlock){
  if(mastlock==Masters+1){
    for(int i=0;i<Masters;i++){
      if(masters_req[i].HSel){
        req_out = masters_req[i];
        //reqs = i;
        matrix_slaves_control=i;
        matrix_masters_control[i]=slave_number;
        break;
      }
    }
  }else{
    if(masters_req[mastlock].HSel){
      req_out = masters_req[mastlock];
      //reqs = i;
      matrix_slaves_control=mastlock;
      matrix_masters_control[mastlock]=slave_number;
    }else{
      for(int i=0;i<Masters;i++){
        if(masters_req[i].HSel){
          req_out = masters_req[i];
          //reqs = i;
          matrix_slaves_control=i;
          matrix_masters_control[i]=slave_number;
          break;
        }
      }
    }
  }   
   
}
  
  SC_HAS_PROCESS(ahb4HLS);
  ahb4HLS(sc_module_name nm)
  {
    SC_THREAD(do_cycle);
    sensitive << clk.pos();
    async_reset_signal_is(rst, false);

  };
};

#endif // __IC_TOP_H__
