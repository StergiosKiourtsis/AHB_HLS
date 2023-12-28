/*	
	@author VLSI Lab, EE dept., Democritus University of Thrace
*/

#include "../src/ahb.h"
#include "../src/global_example2.h"
#include "ahb_master.h"
#include "ahb_slave.h"
#include "../src/ahb_multi_master.h"

#include <mc_trace.h>

#include <mc_scverify.h>

#define DATA_WIDTH 32

SC_MODULE(Top) {

  typedef AHB_CTR_MA<DATA_WIDTH> M_TRAN;
  typedef AHB_RSP_MA<DATA_WIDTH> M_RESP;
  typedef AHB_CTR_SL<DATA_WIDTH> S_TRAN;
  typedef AHB_RSP_SL<DATA_WIDTH> S_RESP;

  sc_clock clk;
  sc_signal<bool> rst;

  

  ahb_slave  INIT_S1(slave0);
  ahb_slave  INIT_S1(slave1);
  ahb_slave  INIT_S1(slave2);
  ahb_master INIT_S1(master0);
  ahb_master INIT_S1(master1);
  
  
  CCS_DESIGN(ahb_if) INIT_S1(interconnect);


  Connections::Combinational< M_TRAN > INIT_S1(from_master0_req);
  Connections::Combinational< M_RESP > INIT_S1(to_master0_rsp);
  
  Connections::Combinational< M_TRAN > INIT_S1(from_master1_req);
  Connections::Combinational< M_RESP > INIT_S1(to_master1_rsp);

  Connections::Combinational< S_TRAN > INIT_S1(to_slave_req0);
  Connections::Combinational< S_RESP > INIT_S1(from_slave_rsp0);

  Connections::Combinational< S_TRAN > INIT_S1(to_slave_req1);
  Connections::Combinational< S_RESP > INIT_S1(from_slave_rsp1);

  Connections::Combinational< S_TRAN > INIT_S1(to_slave_req2);
  Connections::Combinational< S_RESP > INIT_S1(from_slave_rsp2);
 
  

  void do_cycle(){
    
  std::cout << " -- Simulation Starting @ " << sc_time_stamp() << " -- " << std::endl;
  wait();
  rst.write(0);
  wait(2);
  rst.write(1);

  for (int i=0; i<100; i++) {
      std::cout << " -- Simulatiom @ " << sc_time_stamp() << " -- " << std::endl;
      wait();
      //std::cout << "" << std::endl;
  }

  sc_stop();
  std::cout << "\n" << std::endl;
  std::cout << " -- Simulation Stopped @ " << sc_time_stamp() << " -- " << std::endl;
}

  SC_HAS_PROCESS(Top);
  Top(sc_module_name nm)
    : clk("clk", 1, SC_NS, 0.5, 0, SC_NS, true)
  {
    //sc_object_tracer<sc_clock> trace_clk(clk);
    Connections::set_sim_clk(&clk);


    master0.start_point = 500;
    master0.mastlock_enable = 0;

    master0.clk(clk);
    master0.rst(rst);
    master0.master_out(from_master0_req);
    master0.master_in(to_master0_rsp);
    /////////////

    master1.start_point = 4500;
    master1.mastlock_enable = 1;

    master1.clk(clk);
    master1.rst(rst);
    master1.master_out(from_master1_req);
    master1.master_in(to_master1_rsp);
    /////////////

    slave0.clk(clk);
    slave0.rst(rst);
    slave0.slave_out(from_slave_rsp0);
    slave0.slave_in(to_slave_req0);
    /////////////
    
    slave1.clk(clk);
    slave1.rst(rst);
    slave1.slave_out(from_slave_rsp1);
    slave1.slave_in(to_slave_req1);
    ////////////

    slave2.clk(clk);
    slave2.rst(rst);
    slave2.slave_out(from_slave_rsp2);
    slave2.slave_in(to_slave_req2);
    ////////////
    interconnect.clk(clk);
    interconnect.rst(rst);
    interconnect.req_from_master[0](from_master0_req);
    interconnect.req_from_master[1](from_master1_req);
    interconnect.req_to_slave[0](to_slave_req0);
    interconnect.rsp_from_slave[0](from_slave_rsp0);
    interconnect.rsp_to_master[0](to_master0_rsp);
    interconnect.rsp_to_master[1](to_master1_rsp);
    interconnect.req_to_slave[1](to_slave_req1);
    interconnect.rsp_from_slave[1](from_slave_rsp1);
    interconnect.req_to_slave[2](to_slave_req2);
    interconnect.rsp_from_slave[2](from_slave_rsp2);
    
    //interconnect.memoryMap[0]=map0;
    //interconnect.memoryMap[1]=map1;
    //interconnect.memoryMap[2]=map2;
    master0.memoryMap[0] = map0;
    master0.memoryMap[1] = map1;
    master0.memoryMap[2] = map2;
    master1.memoryMap[0] = map0;
    master1.memoryMap[1] = map1;
    master1.memoryMap[2] = map2;


    SC_THREAD(do_cycle);
    sensitive <<clk.posedge_event();
  };
};
 

//sc_trace_file* trace_file_ptr;

int sc_main(int argc, char *argv[]) {

  //trace_file_ptr = sc_create_vcd_trace_file("trace");
 
  Top top("top");

  //trace_hierarchy(&top, trace_file_ptr);

  //channel_logs logs;
  //logs.enable("chan_log");
  
  sc_start();

  return 0;
}
