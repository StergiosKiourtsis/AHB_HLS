#include <iostream>

//#include "../src/ahb.h"
//#include "../src/global.h"
#include "../src/ahb_multi_master.h"

#include "drim4hls_datatypes.h"
#include "defines.h"
#include "globals.h"
#include "drim4hls.h"

#include <mc_scverify.h>
#include <ac_int.h>

#define DATA_WIDTH 32

class Top: public sc_module {
    public:
    
    typedef AHB_CTR_MA<DATA_WIDTH> M_TRAN;
    typedef AHB_RSP_MA<DATA_WIDTH> M_RESP;
    typedef AHB_CTR_SL<DATA_WIDTH> S_TRAN;
    typedef AHB_RSP_SL<DATA_WIDTH> S_RESP;


    CCS_DESIGN(drim4hls) CCS_INIT_S1(m_dut);
    
		ahb_if  CCS_INIT_S1(interconnect);
		//ahb_slave_converter_imem CCS_INIT_S1(converter_imem);
		//ahb_slave_converter_dmem CCS_INIT_S1(converter_dmem);
		//ahb_master_converter_fetch CCS_INIT_S1(converter_fetch);
		//ahb_master_converter_writeback CCS_INIT_S1(converter_writeback);

    sc_clock clk;
    SC_SIG(bool, rst);

    // End of simulation signal.
    #pragma hls_direct_input
    sc_signal < bool > CCS_INIT_S1(program_end);

    // Instruction counters
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(icount);
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(j_icount);
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(b_icount);
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(m_icount);
    #pragma hls_direct_input
    sc_signal < long int > CCS_INIT_S1(o_icount);

  Connections::Combinational< M_TRAN > CCS_INIT_S1(fe2AHB);
  Connections::Combinational< M_RESP > CCS_INIT_S1(AHB2fe);
  
  Connections::Combinational< M_TRAN > CCS_INIT_S1(wb2AHB);
  Connections::Combinational< M_RESP > CCS_INIT_S1(AHB2wb);

    /* The testbench, DUT, IMEM and DMEM modules. */    

		Connections::Combinational< S_TRAN > CCS_INIT_S1(AHB2imem);
		Connections::Combinational< S_RESP > CCS_INIT_S1(imem2AHB);

		Connections::Combinational< S_TRAN > CCS_INIT_S1(AHB2dmem);
		Connections::Combinational< S_RESP > CCS_INIT_S1(dmem2AHB);
  
    
    sc_uint < XLEN > imem[ICACHE_SIZE];
    
    S_TRAN req_imem;
		S_RESP rsp_imem;

    imem_out_t imem_dout;
    imem_in_t imem_din;

    sc_uint < XLEN > dmem[DCACHE_SIZE];

    S_TRAN req_dmem;
		S_RESP rsp_dmem;
		
    dmem_out_t dmem_dout;
    dmem_in_t dmem_din;

    const std::string testing_program;
    
    int wait_stalls;

    SC_CTOR(Top);
    Top(const sc_module_name &name, const std::string &testing_program): 
    clk("clk", 10, SC_NS, 5, 0, SC_NS, true),
    m_dut("drim4hls"),
    interconnect("interconnect"),
    testing_program(testing_program) {
        
        Connections::set_sim_clk( & clk);

        // Connect the design module
        m_dut.clk(clk);
        m_dut.rst(rst);
        m_dut.program_end(program_end);

        m_dut.icount(icount);
        m_dut.j_icount(j_icount);
        m_dut.b_icount(b_icount);
        m_dut.m_icount(m_icount);
        m_dut.o_icount(o_icount);
    		
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
            //imem_din = converter2imem.Pop();

			std::cout<<"Iaddr pop : "<< req_imem.HAddr << std::endl;
			if(req_imem.HSel && req_imem.HReady && req_imem.HTrans!=0 && req_imem.HTrans!=1){
						imem_din.instr_addr = req_imem.HAddr;
			std::cout<<"Iaddr is : "<< imem_din.instr_addr << std::endl;			
            unsigned int addr_aligned = imem_din.instr_addr >> 2;
						std::cout <<"shifted";
						std::cout << " imem addr= " << addr_aligned << endl;
            
            imem_dout.instr_data = imem[addr_aligned];
			std::cout<<"I instr read : "<< imem_dout.instr_data << std::endl;
            unsigned int random_stalls = (rand() % 2) + 1;
            //unsigned int random_stalls = 1;
            wait(random_stalls);

            rsp_imem.HReadyout = 1;
			rsp_imem.HResp = 0;
			rsp_imem.HRData = imem_dout.instr_data;
            imem2AHB.Push(rsp_imem);
            std::cout<<"I instr push : "<< rsp_imem.HRData << std::endl;
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
						std::cout<<"Daddr pop : "<< req_dmem.HAddr << std::endl;
           if(req_dmem.HSel && req_dmem.HReady){
            if(req_dmem.HAddr - 65536 <= 65536){//
                        dmem_din.data_addr = req_dmem.HAddr-65536;
                        std::cout<<"Daddr is : "<< dmem_din.data_addr << std::endl;	
				
						dmem_din.data_in = req_dmem.HWData;
						if(req_dmem.HWrite){
							dmem_din.read_en = 0;
							dmem_din.write_en = 1;
						}else if(!req_dmem.HWrite){
							dmem_din.read_en = 1;
							dmem_din.write_en = 0;		
						}
            unsigned int addr = dmem_din.data_addr;
			//std::cout << "dmem addr= " << addr << endl;
            unsigned int random_stalls = (rand() % 25) + 1;
            
            //std::cout << "wait=" << random_stalls << endl;
            //unsigned int random_stalls = 15;
            wait_stalls += random_stalls;
            wait(random_stalls);
            std::cout << "wait= " << random_stalls << endl;
            
            if (dmem_din.read_en) {
								std::cout << "dmem read" << endl;
                                dmem_dout.data_out = dmem[addr];
								
								rsp_dmem.HReadyout = 1;
								rsp_dmem.HResp = 0;
								rsp_dmem.HRData = dmem_dout.data_out;               
                
                                dmem2AHB.Push(rsp_dmem);
            } else if (dmem_din.write_en) {
								std::cout << "dmem write" << endl;
                dmem[addr] = dmem_din.data_in;
                dmem_dout.data_out = dmem_din.data_in;
            }
        
            // REMOVE
            std::cout << "dmem[" << addr << "]=" << dmem[addr] << endl;
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
            imem[index] = (ac_int<32, false>) data;
            std::cout << "imem[" << index << "]=" << imem[index] << endl;
            dmem[index] = imem[index];
        }

        load_program.close();

        rst.write(0);
        wait(5);
        rst.write(1);
        wait();

        do {
            wait();
        } while (!program_end.read());
        wait(5);
        
        sc_stop();
        int dmem_index;
        for (dmem_index = 0; dmem_index < 400; dmem_index++) {
            std::cout << "dmem[" << dmem_index << "]=" << dmem[dmem_index] << endl;
        }
        std::cout << "wait_stalls " << wait_stalls << endl;

        long icount_end, j_icount_end, b_icount_end, m_icount_end, o_icount_end, pre_b_icount_end;

        icount_end = icount.read();
        j_icount_end = j_icount.read();
        b_icount_end = b_icount.read();
        m_icount_end = m_icount.read();
        o_icount_end = o_icount.read();

        SC_REPORT_INFO(sc_object::name(), "Program complete.");

        std::cout << "INSTR TOT: " << icount_end << std::endl;
        std::cout << "   JUMP  : " << j_icount_end << std::endl;
        std::cout << "   BRANCH: " << b_icount_end << std::endl;
        std::cout << "   MEM   : " << m_icount_end << std::endl;
        std::cout << "   OTHER : " << o_icount_end << std::endl;

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
    std::string testing_program = "/home/skiourtsis/Desktop/AHBwithMastLockWithoutCode/example2o/fibonacci/fibonacci.txt";

    Top top("top", testing_program);
    sc_start();
    return 0;
}
