/*	
	@author VLSI Lab, EE dept., Democritus University of Thrace

	@brief Header file for writeback stage.

	@note Changes from HL5

		- Use of HLSLibs connections for communication with the rest of the processor.

		- Memory is outside of the processor

*/

#ifndef __WRITEBACK__H
#define __WRITEBACK__H

#ifndef __SYNTHESIS__
    #include <sstream>
#endif

#ifndef NDEBUG
    #include <iostream>
    #define DPRINT(msg) std::cout << msg;
#endif

#include "../src/ahb.h"
#include "../src/global_example1.h"

#include "drim4hls_datatypes.h"
#include "defines.h"
#include "globals.h"

#include <mc_connections.h>
#include <ac_int.h>

SC_MODULE(writeback) {
	
  typedef AHB_CTR_MA<DATA_WIDTH> M_REQ_TYPE;
  typedef AHB_RSP_MA<DATA_WIDTH> M_RSP_TYPE;	
	
    #ifndef __SYNTHESIS__
    struct writeback_out // TODO: fix all sizes
    {
        //
        // Member declarations.
        //		
        unsigned int aligned_address;
        sc_uint < XLEN > load_data;
        sc_uint < XLEN > store_data;
        std::string load;
        std::string store;

    }
    writeback_out_t;
    #endif

    // FlexChannel initiators
    Connections::In < exe_out_t > din;
    //Connections::In < dmem_out_t > dmem_out;

    Connections::Out < mem_out_t > dout;
    //Connections::Out < dmem_in_t > dmem_in;

    Connections::Out < M_REQ_TYPE > CCS_INIT_S1(req_wb);
		Connections::In < M_RSP_TYPE > CCS_INIT_S1(rsp_wb);
		
    // Clock and reset signals
    sc_in < bool > clk;
    sc_in < bool > rst;
	
    // Member variables
    exe_out_t input;
    dmem_in_t dmem_dout;
    dmem_out_t dmem_din;
    mem_out_t output;
    
    M_REQ_TYPE req;
	  M_RSP_TYPE rsp;
	  bool control;
	  bool skip_next;

    ac_int < DATA_SIZE, false > mem_dout;
    ac_int < DCACHE_LINE, false > dmem_data;
    ac_int < XLEN, false > dmem_data_offset;
        
    bool freeze;
    // Constructor
    SC_CTOR(writeback): din("din"), dout("dout"), req_wb("req_wb"), rsp_wb("rsp_wb"), clk("clk"), rst("rst") {
        SC_THREAD(writeback_th);
        sensitive << clk.pos();
        async_reset_signal_is(rst, false);
    }

       void wb2AHB(bool read , sc_int < PC_LEN > addr, sc_uint < PC_LEN > data){
                //dmem_in.Push(dmem_dout);
                //stergios change
                req.HAddr = addr;
								req.HTrans = 2;//ahb::AHB_Encoding::AHBTRANS::NONSEQ;
								req.HSize = 2;//PC_LEN is 32bit,the code 32bit signal is 2
								req.HProt = 0; // temporary
								req.HMastLock = 0;
								req.HBurst = 0; // Signle transfer
								req.HWrite = read;//low for read high for write
								if(read){
									req.HWData = dmem_dout.data_in;
								}
                req_wb.Push(req);		
                control = 0;	
                skip_next = 1;
               
			
		}

    void showIdle(){
                //dmem_in.Push(dmem_dout);
                //stergios change
								req.HTrans = 0;//ahb::AHB_Encoding::AHBTRANS::IDLE;
                req_wb.Push(req);				
		}

		void AHB2wb(bool read){
			bool skip=0;
	
			for(int i=0;i<4;i++){
				rsp = rsp_wb.Pop();
				if(!skip){
					if(rsp.HReady && !rsp.HResp){
						if(skip_next){
							skip_next = 0;
						}else{
							if(read){
								dmem_din.data_out = (Slong) rsp.HRData;
								dmem_data = dmem_din.data_out;
							}
							skip = 1;						
						}
					}
				}
			}
			
		}

    void writeback_th(void) {
        WRITEBACK_RST: {
            din.Reset();
            rsp_wb.Reset();
            
            req_wb.Reset();

            dout.Reset();
            
            // Write dummy data to decode feedback.
            output.regfile_address = 0;
            output.regfile_data = 0;
            output.regwrite = 0;
            output.tag = 0;
			
			
            dmem_data = 0;
            dmem_data_offset = 0;
            mem_dout = 0;
            
						freeze = false;
			      control = 0;
            skip_next = 0;
			
        }

        #pragma hls_pipeline_init_interval 1
        #pragma pipeline_stall_mode flush
        WRITEBACK_BODY: while (true) {

            // Get
			input = din.Pop();
			
            #ifndef __SYNTHESIS__
                writeback_out_t.aligned_address = 0;
                writeback_out_t.load_data = 0;
                writeback_out_t.store_data = 0;
                writeback_out_t.load = "NO_LOAD";
                writeback_out_t.store = "NO_STORE";
            #endif
            
            // Compute
            // *** Memory access.
			dmem_data = 0;
			dmem_data_offset = 0;
            // WARNING: only supporting aligned memory accesses
            // Preprocess address
			
            unsigned int aligned_address = input.alu_res.to_uint();
            ac_int< 5, false> byte_index = (ac_int< 5, false>)((aligned_address & 0x3) << 3);
            ac_int< 5, false> halfword_index = (ac_int< 5, false>)((aligned_address & 0x2) << 3);

            aligned_address = aligned_address >> 2;
            ac_int < BYTE, false > db = (ac_int < BYTE, false >) 0;
            ac_int < 2 * BYTE, false > dh = (ac_int < 2 * BYTE, false >) 0;
            ac_int < XLEN, false > dw = (ac_int < XLEN, false >) 0;

            dmem_dout.data_addr = aligned_address;

            
            ac_int < XLEN, false > addr = aligned_address;
            

            #ifndef __SYNTHESIS__
            if (sc_uint < 3 > (input.ld) != NO_LOAD || sc_uint < 2 > (input.st) != NO_STORE) {
                if (input.mem_datain.to_uint() == 0x11111111 ||
                    input.mem_datain.to_uint() == 0x22222222 ||
                    input.mem_datain.to_uint() == 0x11223344 ||
                    input.mem_datain.to_uint() == 0x88776655 ||
                    input.mem_datain.to_uint() == 0x12345678 ||
                    input.mem_datain.to_uint() == 0x87654321) {
                    std::stringstream stm;
                    stm << hex << "D$ access here2 -> 0x" << aligned_address << ". Value: " << input.mem_datain.to_uint() << std::endl;
                }
                //sc_assert(aligned_address < DCACHE_SIZE);
            }
            #endif
            
            
           /* if ((input.ld != NO_LOAD || input.st != NO_STORE) && !freeze) {
								dmem_dout.read_en = true;
                
                //dmem_in.Push(dmem_dout);
                
 								wb2AHB(0, dmem_dout.data_addr+2*map0,0);	
								
								showIdle();
								AHB2wb(1);
																
  
                    
								dmem_data_offset = dmem_data;
				
								dmem_dout.read_en = false;
                dmem_dout.write_en = false;
			}*/
            
            
            if (input.ld != NO_LOAD) { // a load is requested
								if(!freeze){
									wb2AHB(0, (Slong) dmem_dout.data_addr+2*map0,0);	
									#ifndef __SYNTHESIS__
										showIdle();
									#endif	
									AHB2wb(1); 
									dmem_data_offset = dmem_data;										
								}
                switch (input.ld) { // LOAD
                case LB_LOAD:                    
                    db.set_slc(0, dmem_data_offset.slc<BYTE>(byte_index));
                    mem_dout = ext_sign_byte(db);

                    #ifndef __SYNTHESIS__
                    writeback_out_t.load_data = mem_dout;
                    writeback_out_t.load = "LB_LOAD";
                    #endif

                    break;
                case LH_LOAD:
                    dh.set_slc(0, dmem_data_offset.slc< 2*BYTE >(halfword_index));
                    mem_dout = ext_sign_halfword(dh);

                    #ifndef __SYNTHESIS__
                    writeback_out_t.load_data = mem_dout;
                    writeback_out_t.load = "LH_LOAD";
                    #endif

                    break;
                case LW_LOAD:
                    dw = dmem_data_offset;
                    mem_dout = dw;

                    #ifndef __SYNTHESIS__
                    writeback_out_t.load_data = mem_dout;
                    writeback_out_t.load = "LW_LOAD";
                    #endif

                    break;
                case LBU_LOAD:
                    db.set_slc(0, dmem_data_offset.slc<BYTE>(byte_index));
                    mem_dout = ext_unsign_byte(db);

                    #ifndef __SYNTHESIS__
                    writeback_out_t.load_data = mem_dout;
                    writeback_out_t.load = "LBU_LOAD";
                    #endif

                    break;
                case LHU_LOAD:
                    dh.set_slc(0, dmem_data_offset.slc< 2 * BYTE >(halfword_index));
                    mem_dout = ext_unsign_halfword(dh);

                    #ifndef __SYNTHESIS__
                    writeback_out_t.load_data = mem_dout;
                    writeback_out_t.load = "LHU_LOAD";
                    #endif

                    break;
                default:

                    #ifndef __SYNTHESIS__
                    writeback_out_t.load_data = mem_dout;
                    writeback_out_t.load = "NO_LOAD";
                    #endif

                    break; // NO_LOAD
                }
            } else if (input.st != NO_STORE) { // a store is requested
                dmem_dout.write_en = true;
                dmem_dout.write_addr = aligned_address;
                
                switch (input.st) { // STORE
                case SB_STORE: // store 8 bits of rs2
					
					db.set_slc(0, (ac_int < BYTE, false >) input.mem_datain.slc<BYTE>(0));
					dmem_data_offset.set_slc(byte_index, (ac_int < BYTE, false >) db);

                    #ifndef __SYNTHESIS__
                    writeback_out_t.store_data = db;
                    writeback_out_t.store = "SB_STORE";
                    #endif
					
                    break;
                case SH_STORE: // store 16 bits of rs2

					dh.set_slc(0, input.mem_datain.slc<2*BYTE>(0));
					dmem_data_offset.set_slc(byte_index, dh);
                    
                    #ifndef __SYNTHESIS__
                    writeback_out_t.store_data = dh;
                    writeback_out_t.store = "SH_STORE";
                    #endif
					
                    break;
                case SW_STORE: // store rs2
                    dw = input.mem_datain;
                    dmem_data_offset = dw;

                    #ifndef __SYNTHESIS__
                    writeback_out_t.store_data = dw;
                    writeback_out_t.store = "SW_STORE";
                    #endif
					
					break;
                default:

                    #ifndef __SYNTHESIS__
                    writeback_out_t.store = "NO_STORE";
                    #endif
					
                    break; // NO_STORE
                }
                dmem_dout.data_in = dmem_data_offset;
								if(!freeze){
									wb2AHB(1, (Slong) dmem_dout.data_addr+2*map0, (Slong) dmem_dout.data_in);	
									#ifndef __SYNTHESIS__
										showIdle();
									#endif	
									AHB2wb(0);									
								}				
            
            }    
			
            // *** END of memory access.
            
            /* Writeback */
            output.regwrite = input.regwrite;
            output.regfile_address = input.dest_reg;
            output.regfile_data = (input.memtoreg[0] == 1) ? mem_dout : input.alu_res;
            output.tag = input.tag;
            output.pc = input.pc;

            // Put
            freeze = false;
		    dout.Push(output);
		    
            /*#ifndef __SYNTHESIS__
            DPRINT("@" << sc_time_stamp() << "\t" << name() << "\t" << "load= " << writeback_out_t.load << endl);
            DPRINT("@" << sc_time_stamp() << "\t" << name() << "\t" << "store= " << writeback_out_t.store << endl);
            DPRINT("@" << sc_time_stamp() << "\t" << name() << "\t" << std::hex << "input.regwrite=" << input.regwrite << endl);
            DPRINT("@" << sc_time_stamp() << "\t" << name() << "\t" << "regwrite=" << output.regwrite << endl);
            DPRINT("@" << sc_time_stamp() << "\t" << name() << "\t" << "aligned_address=" << aligned_address << endl);
            DPRINT("@" << sc_time_stamp() << "\t" << name() << "\t" << std::hex << "mem_dout=" << mem_dout << endl);
            DPRINT("@" << sc_time_stamp() << "\t" << name() << "\t" << std::hex << "input.alu_res=" << input.alu_res << endl);
            DPRINT("@" << sc_time_stamp() << "\t" << name() << "\t" << std::hex << "output.regfile_address=" << output.regfile_address << endl);
            DPRINT("@" << sc_time_stamp() << "\t" << name() << "\t" << std::hex << "output.regfile_data=" << output.regfile_data << endl);
            DPRINT("@" << sc_time_stamp() << "\t" << name() << "\t" << std::hex << "input.memtoreg=" << input.memtoreg << endl);
            DPRINT("@" << sc_time_stamp() << "\t" << name() << "\t" << std::hex << "writeback_out_t.store_data =" << writeback_out_t.store_data  << endl);
            DPRINT(endl);
            #endif*/
            wait();
        }
    }

    /* Support functions */

    // Sign extend byte read from memory. For LB
    ac_int < XLEN, false > ext_sign_byte(ac_int < BYTE, false > read_data) {
		ac_int <XLEN, false> extended = 0;
		if (read_data[7] == 1) {
			extended.set_slc(0, read_data);
			extended.set_slc(BYTE, (ac_int < BYTE * 3, false >) 16777216);
		}
		else {
			extended.set_slc(0, read_data);
			extended.set_slc(BYTE, (ac_int < BYTE * 3, false >) 0);		
		}
		return extended;
    }

    // Zero extend byte read from memory. For LBU
    ac_int < XLEN, false > ext_unsign_byte(ac_int < BYTE, false > read_data) {
		ac_int <XLEN, false> extended = 0;
		extended.set_slc(0, read_data);
		extended.set_slc(BYTE, (ac_int < BYTE * 3, false >) 0);
		return extended;        
    }

    // Sign extend half-word read from memory. For LH
    ac_int < XLEN, false > ext_sign_halfword(ac_int < BYTE * 2, false > read_data) {
		ac_int <XLEN, false> extended = 0;
		        
        if (read_data[15] == 1) {
			extended.set_slc(0, read_data);
			extended.set_slc(2*BYTE, (ac_int < BYTE * 2, false >) 65535);
        }
        else {
			extended.set_slc(0, read_data);
			extended.set_slc(2*BYTE, (ac_int < BYTE * 2, false >) 0);
        }
        return extended;
    }

    // Zero extend half-word read from memory. For LHU
    ac_int < XLEN, false > ext_unsign_halfword(ac_int < BYTE * 2, false > read_data) {
		ac_int <XLEN, false> extended = 0;
		extended.set_slc(0, read_data);
		extended.set_slc(2*BYTE, (ac_int < BYTE * 2, false >) 0);
		return extended;
    }
    

};

#endif
