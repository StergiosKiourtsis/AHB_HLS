/*	
	@author VLSI Lab, EE dept., Democritus University of Thrace

	@brief Header file for fetch stage

	@note Changes from HL5
		- Implements the logic only for the fetch part from fedec.hpp.

		- Use of HLSLibs connections for communication with the rest of the processor.

		- Increment program counter based on new stall functionality.


*/

#ifndef __FETCH__H
#define __FETCH__H

#ifndef NDEBUG
    #include <iostream>
    #define DPRINT(msg) std::cout << msg;
#endif

#include "../src/ahb.h"
#include "../src/ahb_encoding.h"

#include "drim4hls_datatypes.h"
#include "defines.h"
#include "globals.h"

#include <mc_connections.h>

#define DATA_WIDTH 32

SC_MODULE(fetch) {
    public:
    
  typedef AHB_CTR_MA<DATA_WIDTH> M_REQ_TYPE;
  typedef AHB_RSP_MA<DATA_WIDTH> M_RSP_TYPE;
    
    
    // Clock and reset signals
    sc_in < bool > CCS_INIT_S1(clk);
    sc_in < bool > CCS_INIT_S1(rst);
    // Channel ports
    Connections::In < fe_in_t > CCS_INIT_S1(fetch_din);
    //Connections::In < imem_out_t > CCS_INIT_S1(imem_dout);
    //Connections::Out < imem_in_t > CCS_INIT_S1(imem_din);
    Connections::Out < fe_out_t > CCS_INIT_S1(dout);
    Connections::Out < imem_out_t > CCS_INIT_S1(imem_de);
    
    Connections::Out < M_REQ_TYPE > CCS_INIT_S1(req_fe);
    Connections::In < M_RSP_TYPE > CCS_INIT_S1(rsp_fe);

    // Trap signals. TODO: not used. Left for future implementations.
    sc_signal < bool > CCS_INIT_S1(trap); //sc_out
    sc_signal < ac_int < LOG2_NUM_CAUSES, false > > CCS_INIT_S1(trap_cause); //sc_out

    // *** Internal variables
    sc_int < PC_LEN > pc; // Init. to -4, then before first insn fetch it will be updated to 0.	 
    sc_uint < PC_LEN > imem_pc; // Used in fetching from instruction memory
	sc_uint < PC_LEN > pc_tmp; // Init. to -4, then before first insn fetch it will be updated to 0.	 
    // Custom datatypes used for retrieving and sending data through the channels
    imem_in_t imem_in; // Contains data for fetching from the instruction memory
    fe_out_t fe_out; // Contains data for the decode stage
    fe_in_t fetch_in; // Contains data from the decode stage used in incrementing the PC
    imem_out_t imem_out;
    
    M_REQ_TYPE req;
		M_RSP_TYPE rsp;
		bool control;
		
		
    bool redirect;
    bool redirect_tmp;
    
    
    sc_uint < PC_LEN > redirect_addr;
	  sc_uint < PC_LEN > redirect_addr_tmp;
	
    bool freeze;
	bool freeze_tmp;
	int position;
    SC_CTOR(fetch): rsp_fe("rsp_fe"),
    fetch_din("fetch_din"),
    dout("dout"),
    req_fe("req_fe"),
    imem_de("imem_de"),
    clk("clk"),
    rst("rst") {
        SC_THREAD(fetch_th);
        sensitive << clk.pos();
        async_reset_signal_is(rst, false);

    }
		
		void fe2AHB(sc_int < PC_LEN > addr){
						req.HAddr = addr;
						req.HTrans = ahb::AHB_Encoding::AHBTRANS::NONSEQ;
						req.HSize = 2;//PC_LEN is 32bit,the code 32bit signal is 2
						req.HProt = 0; // temporary
						req.HMastLock = 0;
						req.HBurst = 0; // Signle transfer
						req.HWrite = 0;//low is for read
						req_fe.Push(req);
			
		}
		bool AHB2fe(){
			     rsp = rsp_fe.Pop();
           if(rsp.HReady && !rsp.HResp){
							imem_out.instr_data = rsp.HRData;
							imem_de.Push(imem_out);
              dout.Push(fe_out);
            	return 1;
					 }else{
							return 0;
					 }
		}
		
    void fetch_th(void) {
        FETCH_RST: {
            dout.Reset();
            fetch_din.Reset();
            rsp_fe.Reset();
            req_fe.Reset();
            imem_de.Reset();
									
            trap = 0;
            trap_cause = NULL_CAUSE;
            imem_in.instr_addr = 0;
            
            redirect_addr = 0; 
			freeze = false;
			redirect = false;
            //  Init. pc to START_ADDRESS - 4 as on first fetch it will be incremented by
            //  4, thus fetching instruction at address 0
            pc = -4;
            pc_tmp = -4;
            position = 0;
            
            //stergios
						control = 0;
            //stergios
            
            wait();
        }
        #pragma hls_pipeline_init_interval 1
        #pragma pipeline_stall_mode flush
        FETCH_BODY: while (true) {
			
            //sc_assert(sc_time_stamp().to_double() < 1500000);
            if (fetch_din.PopNB(fetch_in)) {
                // Mechanism for incrementing PC
                redirect = fetch_in.redirect;
                redirect_addr = fetch_in.address;
                freeze = fetch_in.freeze;
            }
          
          
		  
            // Mechanism for incrementing PC
            if ((redirect && redirect_addr != pc) || freeze) {
                pc = redirect_addr;
            } else if (!freeze) {
                pc = (pc + 4);
            }
			
            imem_in.instr_addr = pc;

            fe_out.pc = pc;

						//imem_din.Push(imem_in);
						//stergios
						fe2AHB(pc);
						
						//control = AHB2fe();
            //imem_out = imem_dout.Pop();
						//stergios

						while(!control){
							control = AHB2fe();
						}
						
			#ifndef __SYNTHESIS__
            DPRINT("@" << sc_time_stamp() << "\t" << name() << "\t" << std::hex << "pc= " << pc << endl);
            DPRINT(endl);
            #endif
            
            
            wait();

        } // *** ENDOF while(true)
    } // *** ENDOF sc_cthread
};

#endif
