#include <iostream>
#include <math.h>

#include "../src/ahb.h"
#include "../src/ahb_multi_master.h"
#include "../src/global_example1.h"

#include "drim4hls_datatypes.h"
#include "defines.h"
#include "globals.h"
#include "drim4hls.h"

#include <mc_scverify.h>
#include <ac_int.h>

class Top: public sc_module {
    public:

  typedef AHB_CTR_MA<DATA_WIDTH> M_REQ_TYPE;
  typedef AHB_RSP_MA<DATA_WIDTH> M_RSP_TYPE;
  typedef AHB_CTR_SL<DATA_WIDTH> S_REQ_TYPE;
  typedef AHB_RSP_SL<DATA_WIDTH> S_RSP_TYPE;

    //CCS_DESIGN(drim4hls) CCS_INIT_S1(m_dut);
		//ahb4HLS  CCS_INIT_S1(interconnect);
		drim4hls CCS_INIT_S1(m_dut);
		CCS_DESIGN(ahb4HLS)  CCS_INIT_S1(interconnect);

    sc_clock clk;
    sc_signal < bool > rst;
    //SC_SIG(bool, rst);

    // End of simulation signal.
    #pragma hls_direct_input
    sc_signal < bool > program_end;

    // Instruction counters
    #pragma hls_direct_input
    sc_signal < long int > icount;
    #pragma hls_direct_input
    sc_signal < long int > j_icount;
    #pragma hls_direct_input
    sc_signal < long int > b_icount;
    #pragma hls_direct_input
    sc_signal < long int > m_icount;
    #pragma hls_direct_input
    sc_signal < long int > o_icount;
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(dec_stalls);
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(raw_stalls);
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(jump_stalls);
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(branch_stalls);
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(total_cycles);

		Connections::Combinational<  M_REQ_TYPE > CCS_INIT_S1(fe2AHB);
		Connections::Combinational<  M_RSP_TYPE > CCS_INIT_S1(AHB2fe);
  
		Connections::Combinational<  M_REQ_TYPE > CCS_INIT_S1(wb2AHB);
		Connections::Combinational<  M_RSP_TYPE > CCS_INIT_S1(AHB2wb);

    /* The testbench, DUT, IMEM and DMEM modules. */    

		Connections::Combinational< S_REQ_TYPE > CCS_INIT_S1(AHB2imem);
		Connections::Combinational< S_RSP_TYPE > CCS_INIT_S1(imem2AHB);

		Connections::Combinational< S_REQ_TYPE > CCS_INIT_S1(AHB2dmem);
		Connections::Combinational< S_RSP_TYPE > CCS_INIT_S1(dmem2AHB);

    ac_int < XLEN, false > imem[ICACHE_SIZE];

    S_REQ_TYPE req_imem;
		S_RSP_TYPE rsp_imem;
		
    imem_out_t imem_dout;
    imem_in_t imem_din;

    ac_int < XLEN, false > dmem[DCACHE_SIZE];

    S_REQ_TYPE req_dmem;
		S_RSP_TYPE rsp_dmem;
		
    dmem_out_t dmem_dout;
    dmem_in_t dmem_din;
    
    int wait_stalls;

    const std::string testing_program;

    SC_CTOR(Top);
    Top(const sc_module_name &name, const std::string &testing_program): 
    clk("clk", 10, SC_NS, 5, 0, SC_NS, true),
    m_dut("drim4hls"),    
    interconnect("interconnect"),
    testing_program(testing_program) {
        
        //Connections::set_sim_clk( & clk);

        // Connect the design module
        m_dut.clk(clk);
        m_dut.rst(rst);
        m_dut.program_end(program_end);

        m_dut.icount(icount);
        m_dut.j_icount(j_icount);
        m_dut.b_icount(b_icount);
        m_dut.m_icount(m_icount);
        m_dut.o_icount(o_icount);
        m_dut.dec_stalls(dec_stalls);
        m_dut.raw_stalls(raw_stalls);
        m_dut.jump_stalls(jump_stalls);
        m_dut.branch_stalls(branch_stalls);
        m_dut.total_cycles(total_cycles);

    		m_dut.req_fe(fe2AHB);
				m_dut.rsp_fe(AHB2fe);
  
				m_dut.req_wb(wb2AHB);
				m_dut.rsp_wb(AHB2wb);
		        
        
        interconnect.clk(clk);
        interconnect.rst(rst);
        interconnect.req_from_master[0](fe2AHB);
        interconnect.req_from_master[1](wb2AHB);
        interconnect.rsp_to_master[0](AHB2fe);
        interconnect.rsp_to_master[1](AHB2wb);
        interconnect.req_to_slave[0](AHB2imem);
        interconnect.req_to_slave[1](AHB2dmem);
        interconnect.rsp_from_slave[0](imem2AHB);
        interconnect.rsp_from_slave[1](dmem2AHB);

        SC_CTHREAD(run, clk);

        SC_THREAD(imemory_th);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst, false);

        SC_THREAD(dmemory_th);
        sensitive << clk.posedge_event();
        async_reset_signal_is(rst, false);
    }

    void imemory_th() {
        IMEM_RST: {
						imem2AHB.ResetWrite();
						AHB2imem.ResetRead();

            wait();
        }
        IMEM_BODY: while (true) {
            req_imem = AHB2imem.Pop();
       
						if(req_imem.HSel){
							if(req_imem.HReady && req_imem.HTrans!=1){
													imem_din.instr_addr = (int) req_imem.HAddr;		
													//unsigned int addr_aligned = imem_din.instr_addr >> 2;
													unsigned int addr_aligned = imem_din.instr_addr;
													imem_dout.instr_data = imem[addr_aligned];
													rsp_imem.HReadyout = 1;
													rsp_imem.HResp = 0;
													rsp_imem.HRData = imem_dout.instr_data;
													imem2AHB.Push(rsp_imem);  
							}else{
							          rsp_imem.HReadyout = 0;
												rsp_imem.HResp = 0;
												rsp_imem.HRData = 1111;
												imem2AHB.Push(rsp_imem);
							}
						}
            wait();
        }
    }

    void dmemory_th() {
        DMEM_RST: {
						dmem2AHB.ResetWrite();
						AHB2dmem.ResetRead();
			wait_stalls = 0;
            wait();
        }
        DMEM_BODY: while (true) {
						req_dmem = AHB2dmem.Pop();
           if(req_dmem.HSel && req_dmem.HReady  && req_imem.HTrans!=1){
               dmem_din.data_addr = req_dmem.HAddr-2*map0;
                
				
							 dmem_din.data_in = (int) req_dmem.HWData;
							 if(req_dmem.HWrite){
								dmem_din.read_en = 0;
								dmem_din.write_en = 1;
						   }else if(!req_dmem.HWrite){
							  dmem_din.read_en = 1;
							  dmem_din.write_en = 0;		
						   }
               unsigned int addr = dmem_din.data_addr;

            
							if (dmem_din.read_en) {
								 //std::cout << "dmem read" << endl;
                 dmem_dout.data_out = dmem[addr];
								
								 rsp_dmem.HReadyout = 1;
								 rsp_dmem.HResp = 0;
								 rsp_dmem.HRData = dmem_dout.data_out;               
                
                 dmem2AHB.Push(rsp_dmem);
              } else if (dmem_din.write_en) {
								//std::cout << "dmem write" << endl;
                dmem[addr] = dmem_din.data_in;
                dmem_dout.data_out = dmem_din.data_in;
								
								rsp_dmem.HReadyout = 1;
								rsp_dmem.HResp = 0;
								rsp_dmem.HRData = dmem_dout.data_out;               
                
                dmem2AHB.Push(rsp_dmem);
            
							}
        
					 }
            wait();
        }
    }

    void run() {

        std::ifstream load_program;
        load_program.open(testing_program, std::ifstream:: in );
        unsigned index;
        unsigned address;
        unsigned data;
        
        while (load_program >> std::hex >> address) {

            index = address >> 2;
            if (index >= ICACHE_SIZE) {
                SC_REPORT_ERROR(sc_object::name(), "Program larger than memory size.");
                sc_stop();
                return;
            }
            load_program >> data;
            //load_program >> std::hex >> imem[index];
            imem[index] = (ac_int<32, false>) data;
            std::cout << "imem[" << index << "]=" << imem[index] << endl;
            dmem[index] = imem[index];
        }

        load_program.close();

        rst.write(0);
        wait(5);
        rst.write(1);
        wait();
		
		long total_cycles_top = 0;
        do {
            wait();
            total_cycles_top += 1;
        } while (!program_end.read());
        wait(5);
        
        sc_stop();
        int dmem_index;
        for (dmem_index = 0; dmem_index < 500; dmem_index++) {
            std::cout << "dmem[" << dmem_index << "]=" << dmem[dmem_index] << endl;
        }
		std::cout << "wait stalls " << wait_stalls << endl;
        long total_cache_hit_end, total_cache_miss_end, icount_end, j_icount_end, b_icount_end, m_icount_end, o_icount_end, pre_b_icount_end, dec_stalls_end, total_cycles_end, raw_stalls_end, jump_stalls_end, branch_stalls_end;

        icount_end = icount.read();
        j_icount_end = j_icount.read();
        b_icount_end = b_icount.read();
        m_icount_end = m_icount.read();
        o_icount_end = o_icount.read();
        dec_stalls_end = dec_stalls.read();
        raw_stalls_end = raw_stalls.read();
        branch_stalls_end = branch_stalls.read();
        jump_stalls_end = jump_stalls.read();
        total_cycles_end = total_cycles.read();

        SC_REPORT_INFO(sc_object::name(), "Program complete.");

        std::cout << "INSTR TOT: " << icount_end << std::endl;
        std::cout << "   JUMP  : " << j_icount_end << std::endl;
        std::cout << "   BRANCH: " << b_icount_end << std::endl;
        std::cout << "   MEM   : " << m_icount_end << std::endl;
        std::cout << "   OTHER : " << o_icount_end << std::endl;
        std::cout << "   DEC STALLS : " << dec_stalls_end << std::endl;
        std::cout << "   RAW STALLS : " << raw_stalls_end << std::endl;
        std::cout << "   JUMP STALLS : " << jump_stalls_end << std::endl;
        std::cout << "   BRANCH STALLS : " << branch_stalls_end << std::endl;
        std::cout << "   TOTAL DECODE CYCLES : " << total_cycles_end << std::endl;
        std::cout << "   TOTAL CYCLES : " << total_cycles_top << std::endl;

    }

};

int sc_main(int argc, char * argv[]) {

    // if (argc == 1) {
    //     std::cerr << "Usage: " << argv[0] << " <testing_program>" << std::endl;
    //     std::cerr << "where:  <testing_program> - path to .txt file of the testing program" << std::endl;
    //     return -1;
    // }

    //std::string testing_program = argv[1];
    // USE IN QUESTASIM
    std::string testing_program ="../codeExamples/fibonacci/fibonacci.txt";

    Top top("top", testing_program);
    sc_start();
    return 0;
};






