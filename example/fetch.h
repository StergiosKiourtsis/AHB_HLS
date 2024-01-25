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
#include <ac_int.h>

SC_MODULE(fetch) {
    public:
    // Clock and reset signals
    sc_in < bool > clk;
    sc_in < bool > rst;
    // Channel ports
    Connections::In < fe_in_t > fetch_din;
    //Connections::In < imem_out_t > imem_dout;
    //Connections::Out < imem_in_t > imem_din;
    Connections::Out < fe_out_t > dout;

		typedef AHB_CTR_MA<DATA_WIDTH> M_REQ_TYPE;
		typedef AHB_RSP_MA<DATA_WIDTH> M_RSP_TYPE;

    Connections::Out < M_REQ_TYPE > CCS_INIT_S1(req_fe);
    Connections::In < M_RSP_TYPE > CCS_INIT_S1(rsp_fe);
    
    // Trap signals. TODO: not used. Left for future implementations.
    sc_signal < bool > trap; //sc_out
    sc_signal < ac_int < LOG2_NUM_CAUSES, false > > trap_cause; //sc_out

    // *** Internal variables
    ac_int < PC_LEN, true > pc; // Init. to -4, then before first insn fetch it will be updated to 0.	 
    ac_int < PC_LEN, false > imem_pc; // Used in fetching from instruction memory
	ac_int < PC_LEN, false > pc_tmp; // Init. to -4, then before first insn fetch it will be updated to 0.
    // Custom datatypes used for retrieving and sending data through the channels
    imem_in_t imem_in; // Contains data for fetching from the instruction memory
    fe_out_t fe_out; // Contains data for the decode stage
    fe_in_t fetch_in; // Contains data from the decode stage used in incrementing the PC
    imem_out_t imem_out;

    M_REQ_TYPE req;
		M_RSP_TYPE rsp;
		bool control;
		bool skip_next;
		
    bool redirect;
    bool redirect_tmp;
    
    ac_int < PC_LEN, true > redirect_addr;
	
	ac_int < DATA_SIZE, false > mem_dout;
    ac_int < ICACHE_LINE, false > imem_data;
    ac_int < XLEN, false > imem_data_offset;
    
    ac_int < ICACHE_TAG_WIDTH + ICACHE_INDEX_WIDTH, false> buffer_addr;
	
    bool freeze;
    bool hit_buffer;
    int position;
    
    //sc_out < long int > fetch_cycles;
    //sc_out < long int > fetch_redirections;
	
    SC_CTOR(fetch): rsp_fe("rsp_fe"),
    fetch_din("fetch_din"),
    dout("dout"),
    req_fe("req_fe"),
    clk("clk"),
    rst("rst") {
        SC_THREAD(fetch_th);
        sensitive << clk.pos();
        async_reset_signal_is(rst, false);

    }

		void fe2AHB(){
  					req.HAddr = imem_in.instr_addr;
						req.HTrans = ahb::AHB_Encoding::AHBTRANS::NONSEQ;
						req.HSize = 2;//PC_LEN is 32bit,the code 32bit signal is 2
						req.HProt = 0; // temporary
						req.HMastLock = 0;
						req.HBurst = 0; // Signle transfer
						req.HWrite = 0;//low is for read
						req_fe.Push(req);
						control = 0;
						skip_next = 0;

		}
		
		void showIdle(){
						req.HTrans = 0;//ahb::AHB_Encoding::AHBTRANS::IDLE;
            req_fe.Push(req);				
		}
		
		bool AHB2fe(){			     
					 #ifndef __SYNTHESIS__
						showIdle();
					  #endif
			     rsp = rsp_fe.Pop();
							if(!skip_next){
										skip_next = 1;//orignal test fibi1
										return 0;
								}else{
									if(rsp.HReady && !rsp.HResp){
											imem_out.instr_data = (int) rsp.HRData;	
											return 1;							
									}else{
										return 0;
									}
							} 
		}

    void fetch_th(void) {
        FETCH_RST: {
            dout.Reset();
            fetch_din.Reset();
            req_fe.Reset();
            rsp_fe.Reset();
            
            position = 0;
									
            trap = 0;
            trap_cause = NULL_CAUSE;
            imem_in.instr_addr = 0;
            
            
            redirect_addr = 0;
			freeze = false;
			redirect = false;
            //  Init. pc to START_ADDRESS - 4 as on first fetch it will be incremented by
            //  4, thus fetching instruction at address 0
            pc = 0;
            pc_tmp = -4;
            buffer_addr = 0;
            
            control = 1;
            skip_next = 0;
			
			//fetch_cycles.write(0);
			//fetch_redirections.write(0);
            
            wait();
        }
        #pragma hls_pipeline_init_interval 1
        #pragma pipeline_stall_mode flush
        FETCH_BODY: while (true) {
            //sc_assert(sc_time_stamp().to_double() < 1500000);
			
            fe_out.pc = pc;
            
            unsigned int aligned_addr = pc >> 2;
            imem_in.instr_addr = aligned_addr;
            
            ac_int < XLEN, false > addr = aligned_addr;
			
			//fetch_cycles.write(fetch_cycles.read() + 1);
			
				   fe2AHB();

					 for(int i=0;i<4;i++){	
							if(!control){
									control = AHB2fe();
							}
						}
						
            imem_data = imem_out.instr_data;
					
			fe_out.instr_data = imem_data;
			
			//step2 read from backchannel (decode)
			if (position == 1 && !redirect) {
				fetch_in = fetch_din.Pop();
				redirect_addr = fetch_in.address;
			}else {
				position = 1;
			}
			// step3 if instruction correct send it, update btb, ras and get new pc
			if (redirect_addr == pc) {
				pc = (ac_int < PC_LEN, false >)(pc + 4);
				redirect = false;
				dout.Push(fe_out);
			}else { // step4 if instruction incorrect, redirect
				pc = redirect_addr;
				redirect = true;
				//fetch_redirections.write(fetch_redirections.read() + 1);
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
