/*	
	@author VLSI Lab, EE dept., Democritus University of Thrace

	@brief 
	Header file for the drim4hls CPU container.
	This module instantiates the stages and interconnects them.

	@note Changes from HL5

		- Use of HLSLibs connections for communication with the rest of the processor.

		- Connection with memories outside of the processor.


*/

#ifndef __DRIM4HLS__H
#define __DRIM4HLS__H

#include "fetch.h"
#include "decode.h"
#include "execute.h"
#include "writeback.h"

#include "../src/ahb.h"

#include "drim4hls_datatypes.h"
#include "defines.h"
#include "globals.h"

#include <mc_connections.h>

#define DATA_WIDTH 32

#pragma hls_design top
SC_MODULE(drim4hls) {
    public:
    // Declaration of clock and reset signals
    sc_in < bool > clk;
    sc_in < bool > rst;

    //End of simulation signal.
    sc_out < bool > CCS_INIT_S1(program_end);
    
    typedef AHB_CTR_MA<DATA_WIDTH> M_TRAN;
    typedef AHB_RSP_MA<DATA_WIDTH> M_RESP;

    // Instruction counters
    sc_out < long int > CCS_INIT_S1(icount);
    sc_out < long int > CCS_INIT_S1(j_icount);
    sc_out < long int > CCS_INIT_S1(b_icount);
    sc_out < long int > CCS_INIT_S1(m_icount);
    sc_out < long int > CCS_INIT_S1(o_icount);

    // Inter-stage Channels and ports.
    Connections::Combinational < fe_out_t > CCS_INIT_S1(fe2de_ch);
    Connections::Combinational < de_out_t > CCS_INIT_S1(de2exe_ch);
    Connections::Combinational < fe_in_t > CCS_INIT_S1(de2fe_ch);
    Connections::Combinational < mem_out_t > CCS_INIT_S1(wb2de_ch); // Writeback loop
    Connections::Combinational < exe_out_t > CCS_INIT_S1(exe2mem_ch);
    Connections::Combinational < imem_out_t > CCS_INIT_S1(fe2de_imem_ch);
    
		Connections::Out< M_TRAN > CCS_INIT_S1(req_fe);
		Connections::In< M_RESP > CCS_INIT_S1(rsp_fe);
  
		Connections::Out< M_TRAN > CCS_INIT_S1(req_wb);
		Connections::In< M_RESP > CCS_INIT_S1(rsp_wb);


    // Forwarding
    Connections::Combinational < reg_forward_t > CCS_INIT_S1(fwd_exe_ch);

    // Instantiate the modules
    fetch CCS_INIT_S1(fe);
    decode CCS_INIT_S1(dec);
    execute CCS_INIT_S1(exe);
    writeback CCS_INIT_S1(wb);
        

    SC_CTOR(drim4hls): clk("clk"),
    rst("rst"),
    program_end("program_end"),
    fe2de_ch("fe2de_ch"),
    de2exe_ch("de2exe_ch"),
    de2fe_ch("de2fe_ch"),
    exe2mem_ch("exe2mem_ch"),
    wb2de_ch("wb2de_ch"),
    fwd_exe_ch("fwd_exe_ch"),
    req_fe("req_fe"),
    rsp_fe("rsp_fe"),
    req_wb("req_wb"),
    rsp_wb("rsp_wb"),
    fe("Fetch"),
    dec("Decode"),
    exe("Execute"),
    wb("Writeback"){
        // FETCH
        fe.clk(clk);
        fe.rst(rst);
        fe.dout(fe2de_ch);
        fe.imem_de(fe2de_imem_ch);
        fe.req_fe(req_fe);
        fe.rsp_fe(rsp_fe);
        fe.fetch_din(de2fe_ch);

        // DECODE
        dec.clk(clk);
        dec.rst(rst);
        dec.dout(de2exe_ch);
        dec.feed_from_wb(wb2de_ch);
        dec.fetch_din(fe2de_ch);
        dec.fetch_dout(de2fe_ch);
        dec.program_end(program_end);
        dec.fwd_exe(fwd_exe_ch);
        dec.icount(icount);
        dec.j_icount(j_icount);
        dec.b_icount(b_icount);
        dec.m_icount(m_icount);
        dec.o_icount(o_icount);
        dec.imem_out(fe2de_imem_ch);

        // EXE
        exe.clk(clk);
        exe.rst(rst);
        exe.din(de2exe_ch);
        exe.dout(exe2mem_ch);
        exe.fwd_exe(fwd_exe_ch);

        // MEM
        wb.clk(clk);
        wb.rst(rst);
        wb.din(exe2mem_ch);
        wb.dout(wb2de_ch);

        wb.req_wb(req_wb);
        wb.rsp_wb(rsp_wb);
        

    }

};

#endif // end __DRIM4HLS__H
