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
#include "../src/ahb_encoding.h"

#include "drim4hls_datatypes.h"
#include "defines.h"
#include "globals.h"

#include <mc_connections.h>

#define DATA_WIDTH 32

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
    Connections::In < exe_out_t > CCS_INIT_S1(din);
    //Connections::In < dmem_out_t > CCS_INIT_S1(dmem_out);

    Connections::Out < mem_out_t > CCS_INIT_S1(dout);
    //Connections::Out < dmem_in_t > CCS_INIT_S1(dmem_in);
    
    Connections::Out < M_REQ_TYPE > CCS_INIT_S1(req_wb);
		Connections::In < M_RSP_TYPE > CCS_INIT_S1(rsp_wb);

    // Clock and reset signals
    sc_in < bool > CCS_INIT_S1(clk);
    sc_in < bool > CCS_INIT_S1(rst);
	
    // Member variables
    exe_out_t input;
    dmem_in_t dmem_dout;
    dmem_out_t dmem_din;
    mem_out_t output;
    
    M_REQ_TYPE req;
	  M_RSP_TYPE rsp;

    sc_uint < DATA_SIZE > mem_dout;
    sc_uint < XLEN > dmem_data;
    
    // Constructor
    SC_CTOR(writeback): din("din"), dout("dout"), req_wb("req_wb"),rsp_wb("rsp_wb"), clk("clk"), rst("rst") {
        SC_THREAD(writeback_th);
        sensitive << clk.pos();
        async_reset_signal_is(rst, false);
    }

    void writeback_th(void) {
        WRITEBACK_RST: {
            din.Reset();
            req_wb.Reset();

            dout.Reset();
            rsp_wb.Reset();
            
            req_wb.Reset();
			
            // Write dummy data to decode feedback.
            output.regfile_address = 0;
            output.regfile_data = 0;
            output.regwrite = 0;
            output.tag = 0;
            
            dmem_data = 0;
            mem_dout = 0;
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
            // WARNING: only supporting aligned memory accesses
            // Preprocess address
			
            unsigned int aligned_address = input.alu_res.to_uint();
            sc_uint< 5 > byte_index = (sc_uint< 5 >)((aligned_address & 0x3) << 3);
            sc_uint< 5 > halfword_index = (sc_uint< 5 >)((aligned_address & 0x2) << 3);

            aligned_address = aligned_address >> 2;
            sc_uint < BYTE > db = (sc_uint < BYTE >) 0;
            sc_uint < 2 * BYTE > dh = (sc_uint < 2 * BYTE >) 0;
            sc_uint < XLEN > dw = (sc_uint < XLEN >) 0;
            
            //stergios
            dmem_dout.data_addr = aligned_address;

            dmem_dout.read_en = false;
            dmem_dout.write_en = false;
            

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
			
			if (input.ld != NO_LOAD) { // a load is requested
                
                dmem_dout.read_en = true;
                //dmem_in.Push(dmem_dout);
                //stergios change
                req.HAddr = dmem_dout.data_addr+65536;
								req.HTrans = ahb::AHB_Encoding::AHBTRANS::NONSEQ;
								req.HSize = 2;//PC_LEN is 32bit,the code 32bit signal is 2
								req.HProt = 0; // temporary
								req.HMastLock = 0;
								req.HBurst = 0; // Signle transfer
								if(dmem_dout.read_en){
									req.HWrite = 0;//low is for read
									//req.HWData = 0;	
								}if(dmem_dout.write_en){
									req.HWrite = 1;//high is for write
									req.HWData = dmem_dout.data_in;	
								}
                req_wb.Push(req);
				
				rsp = rsp_wb.Pop();
				
                //dmem_din = dmem_out.Pop();
                //dmem_data = dmem_din.data_out;
                dmem_din.data_out = rsp.HRData;
                dmem_data = dmem_din.data_out;
                //stergios
                //freeze = false;
                switch (input.ld) { // LOAD
                case LB_LOAD:
                    db = dmem_data.range(byte_index + BYTE - 1, byte_index);
                    mem_dout = ext_sign_byte(db);

                    #ifndef __SYNTHESIS__
                    writeback_out_t.load_data = mem_dout;
                    writeback_out_t.load = "LB_LOAD";
                    #endif

                    break;
                case LH_LOAD:
                    dh = dmem_data.range(halfword_index + 2 * BYTE - 1, halfword_index);
                    mem_dout = ext_sign_halfword(dh);

                    #ifndef __SYNTHESIS__
                    writeback_out_t.load_data = mem_dout;
                    writeback_out_t.load = "LH_LOAD";
                    #endif

                    break;
                case LW_LOAD:
                    dw = dmem_data;
                    mem_dout = dw;

                    #ifndef __SYNTHESIS__
                    writeback_out_t.load_data = mem_dout;
                    writeback_out_t.load = "LW_LOAD";
                    #endif

                    break;
                case LBU_LOAD:
                    db = dmem_data.range(byte_index + BYTE - 1, byte_index);
                    mem_dout = ext_unsign_byte(db);

                    #ifndef __SYNTHESIS__
                    writeback_out_t.load_data = mem_dout;
                    writeback_out_t.load = "LBU_LOAD";
                    #endif

                    break;
                case LHU_LOAD:
                    //dh = dmem_data.range(halfword_index + 2 * BYTE - 1, halfword_index);
                    //dh = dmem_data.slc< 2 * BYTE >(halfword_index);
                    //dh.set_slc(0, dmem_data.slc< 2 * BYTE >(halfword_index));
                    dh = dmem_data.range(halfword_index + 2 * BYTE - 1, halfword_index);
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

                switch (input.st) { // STORE
                case SB_STORE: // store 8 bits of rs2
					
					db = input.mem_datain.range(BYTE - 1, 0).to_uint();
                    dmem_data.range(byte_index + BYTE - 1, byte_index) = db;

                    #ifndef __SYNTHESIS__
                    writeback_out_t.store_data = db;
                    writeback_out_t.store = "SB_STORE";
                    #endif
					
					break;
                case SH_STORE: // store 16 bits of rs2
					
					dh = input.mem_datain.range(2 * BYTE - 1, 0).to_uint();
                    dmem_data.range(byte_index + BYTE - 1, byte_index) = dh;
                    
                    #ifndef __SYNTHESIS__
                    writeback_out_t.store_data = dh;
                    writeback_out_t.store = "SH_STORE";
                    #endif
					
                    break;
                case SW_STORE: // store rs2
                    dw = input.mem_datain.to_uint();
                    dmem_data = dw;

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

                dmem_dout.data_in = dmem_data;
                //dmem_in.Push(dmem_dout);
								req.HAddr = dmem_dout.data_addr+65536;
								req.HTrans = ahb::AHB_Encoding::AHBTRANS::NONSEQ;
								req.HSize = 2;//PC_LEN is 32bit,the code 32bit signal is 2
								req.HProt = 0; // temporary
								req.HMastLock = 0;
								req.HBurst = 0; // Signle transfer
								if(dmem_dout.read_en){
									req.HWrite = 0;//low is for read
									//req.HWData = 0;	
								}if(dmem_dout.write_en){
									req.HWrite = 1;//high is for write
									req.HWData = dmem_dout.data_in;	
								}
                req_wb.Push(req);
                
            }
            // *** END of memory access.
            
            /* Writeback */
            output.regwrite = input.regwrite;
            output.regfile_address = input.dest_reg;
            output.regfile_data = (input.memtoreg[0] == 1) ? mem_dout : input.alu_res;
            output.tag = input.tag;
            output.pc = input.pc;
		
            // Put
		    dout.Push(output);
            #ifndef __SYNTHESIS__
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
            #endif
            wait();
        }
    }

    /* Support functions */

    // Sign extend byte read from memory. For LB
    sc_uint < XLEN > ext_sign_byte(sc_uint < BYTE > read_data) {
		if (read_data[7] == 1) {
			
			return (sc_uint < BYTE * 3 > (16777216), read_data);

		}
		else {

			return (sc_uint < BYTE * 3 > (0), read_data);
		}
    }

    // Zero extend byte read from memory. For LBU
    sc_uint < XLEN > ext_unsign_byte(sc_uint < BYTE > read_data) {

		return (sc_uint < BYTE * 3 > (0), read_data);       
    }

    // Sign extend half-word read from memory. For LH
    sc_uint < XLEN > ext_sign_halfword(sc_uint < BYTE * 2 > read_data) {
		        
        if (read_data[15] == 1) {

            return (sc_uint < BYTE * 2 > (65535), read_data);
        }
        else {

            return (sc_uint < BYTE * 2 > (0), read_data);
        }
    }

    // Zero extend half-word read from memory. For LHU
    sc_uint < XLEN > ext_unsign_halfword(sc_uint < BYTE * 2 > read_data) {

		return (sc_uint < BYTE * 2 > (0), read_data);
    }

};

#endif
