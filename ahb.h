/*	
	@author VLSI Lab, EE dept., Democritus University of Thrace
*/

#ifndef __AHB_H__
#define __AHB_H__

#include "systemc.h"
#include "ahb_encoding.h"
#include "nvhls_connections.h"


#define ADDR_WIDTH     32
#define WRITE_WIDTH    1
#define HSIZE_WIDTH    3
#define BURST_WIDTH    3
#define PROT_WIDTH     4
#define TRANS_WIDTH    2
#define MASTLOCK_WIDTH 1
#define READY_WIDTH    1
#define RESP_WIDTH     1
#define SEL_WIDTH      1
#define READYOUT_WIDTH 1

template<int DATA_WIDTH>
struct AHB_CTR_MA {

  sc_uint <ADDR_WIDTH>     HAddr;
  sc_uint <WRITE_WIDTH>    HWrite;
  sc_uint <HSIZE_WIDTH>    HSize;
  sc_uint <BURST_WIDTH>    HBurst;
  sc_uint <PROT_WIDTH>     HProt;
  sc_uint <TRANS_WIDTH>    HTrans;
  sc_uint <MASTLOCK_WIDTH> HMastLock;
  sc_uint <DATA_WIDTH>     HWData;

  AHB_CTR_MA() {
    HAddr = 0;
    HWrite = 0;
    HSize = 0;
    HBurst = 0;
    HProt = 0;
    HTrans = ahb::AHB_Encoding::AHBTRANS::IDLE;
    HMastLock = 0;
    HWData = 0;
  };

  static const unsigned int width = ADDR_WIDTH + WRITE_WIDTH + HSIZE_WIDTH +
    BURST_WIDTH + PROT_WIDTH + TRANS_WIDTH +
    MASTLOCK_WIDTH + DATA_WIDTH;

  inline friend std::ostream& operator<<(ostream& os, const AHB_CTR_MA& flit_tmp ) {
    os << flit_tmp.HAddr << " ";
    os << flit_tmp.HWrite << " ";
    os << flit_tmp.HSize << " ";
    os << flit_tmp.HBurst << " ";
    os << flit_tmp.HProt << " ";
    os << flit_tmp.HTrans << " ";
    os << flit_tmp.HMastLock << " ";
    os << flit_tmp.HWData << " ";

    #ifdef SYSTEMC_INCLUDED
      os << std::dec << "@" << sc_time_stamp();
    #else
      os << std::dec << "@" << "no-timed";
    #endif

    return os;
  };

#ifdef SYSTEMC_INCLUDED
  // Only for SystemC
  inline friend void sc_trace(sc_trace_file* tf, const AHB_CTR_MA& flit, const std::string& name) {
    sc_trace(tf, flit.HAddr, name + ".addr");
    sc_trace(tf, flit.HTrans, name + ".trans");
    sc_trace(tf, flit.HMastLock, name + ".mastlock");
  }
#endif

  // Matchlib Marshaller requirement
  template<unsigned int Size>
  void Marshall(Marshaller<Size>& m) {
    m& HAddr;
    m& HWrite;
    m& HSize;
    m& HBurst;
    m& HProt;
    m& HTrans;
    m& HMastLock;
    m& HWData;
  };

};

template<int DATA_WIDTH>
struct AHB_CTR_SL {

  sc_uint<SEL_WIDTH>      HSel;
  sc_uint<ADDR_WIDTH>     HAddr;
  sc_uint<WRITE_WIDTH>    HWrite;
  sc_uint<HSIZE_WIDTH>    HSize;
  sc_uint<BURST_WIDTH>    HBurst;
  sc_uint<PROT_WIDTH>     HProt;
  sc_uint<TRANS_WIDTH>    HTrans;
  sc_uint<MASTLOCK_WIDTH> HMastLock;
  sc_uint<READY_WIDTH>    HReady;
  sc_uint<DATA_WIDTH>     HWData;

  AHB_CTR_SL() {
    HSel = 0;
    HAddr = 0;
    HWrite = 0;
    HSize = 0;
    HBurst = 0;
    HProt = 0;
    HTrans = ahb::AHB_Encoding::AHBTRANS::IDLE;
    HMastLock = 0;
    HReady = 1;
    HWData = 0;
  }

  inline bool operator==(const AHB_CTR_MA<DATA_WIDTH>& rhs) const {
    bool eq = (rhs.HAddr == HAddr);
    eq = eq && (rhs.HWrite == HWrite);
    eq = eq && (rhs.HSize == HSize);
    eq = eq && (rhs.HBurst == HBurst);
    eq = eq && (rhs.HProt == HProt);
    eq = eq && (rhs.HTrans == HTrans);
    eq = eq && (rhs.HMastLock == HMastLock);
    eq = eq && (rhs.HWData == HWData);

    return eq;
  }

  inline AHB_CTR_SL& operator = (const AHB_CTR_MA<DATA_WIDTH>& rhs) {
    HAddr = rhs.HAddr;
    HWrite = rhs.HWrite;
    HSize = rhs.HSize;
    HBurst = rhs.HBurst;
    HProt = rhs.HProt;
    HTrans = rhs.HTrans;
    HMastLock = rhs.HMastLock;
    HWData = rhs.HWData;
    HSel = 1;

    return *this;
  }

  inline AHB_CTR_SL& operator = (const AHB_CTR_SL& rhs) {
    HAddr = rhs.HAddr;
    HWrite = rhs.HWrite;
    HSize = rhs.HSize;
    HBurst = rhs.HBurst;
    HProt = rhs.HProt;
    HTrans = rhs.HTrans;
    HMastLock = rhs.HMastLock;
    HWData = rhs.HWData;
    HSel = rhs.HSel;
    HReady = rhs.HReady;

    return *this;
  }


  static const int width = ADDR_WIDTH + WRITE_WIDTH + HSIZE_WIDTH +
    BURST_WIDTH + PROT_WIDTH + TRANS_WIDTH + SEL_WIDTH +
    READY_WIDTH + MASTLOCK_WIDTH + DATA_WIDTH;

  inline friend std::ostream& operator << ( std::ostream& os, const AHB_CTR_SL& flit_tmp ) {
    os << flit_tmp.HSel << " ";
    os << flit_tmp.HAddr << " ";
    os << flit_tmp.HWrite << " ";
    os << flit_tmp.HSize << " ";
    os << flit_tmp.HBurst << " ";
    os << flit_tmp.HProt << " ";
    os << flit_tmp.HTrans << " ";
    os << flit_tmp.HMastLock << " ";
    os << flit_tmp.HReady << " ";
    os << flit_tmp.HWData << " ";

#ifdef SYSTEMC_INCLUDED
    os << std::dec << "@" << sc_time_stamp();
#else
    os << std::dec << "@" << "no-timed";
#endif

    return os;
  }
//#endif

#ifdef SYSTEMC_INCLUDED
  // Only for SystemC
  inline friend void sc_trace(sc_trace_file* tf, const AHB_CTR_SL& flit, const std::string& name) {
    sc_trace(tf, flit.HAddr, name + ".addr");
    sc_trace(tf, flit.HSel, name + ".sel");
    sc_trace(tf, flit.HReady, name + ".ready");
    sc_trace(tf, flit.HMastLock, name + ".mastlock");
  }
#endif

  // Matchlib Marshaller requirement
  template<unsigned int Size>
  void Marshall(Marshaller<Size>& m) {
    m& HAddr;
    m& HWrite;
    m& HSize;
    m& HBurst;
    m& HProt;
    m& HTrans;
    m& HMastLock;
    m& HSel;
    m& HReady;
    m& HWData;
  };

};

template<int DATA_WIDTH>
struct AHB_RSP_SL {

  sc_uint<READYOUT_WIDTH> HReadyout;
  sc_uint<RESP_WIDTH>     HResp;
  sc_uint<DATA_WIDTH>     HRData;

  AHB_RSP_SL() {
    HReadyout =1;
    HResp = ahb::AHB_Encoding::AHBRESP::OKAY;
    HRData =88;
  }

  static const int width = READYOUT_WIDTH + RESP_WIDTH + DATA_WIDTH;

  inline friend std::ostream& operator << ( std::ostream& os, const AHB_RSP_SL& flit_tmp ) {
    os << flit_tmp.HReadyout << " ";
    os << flit_tmp.HResp << " ";
    os << flit_tmp.HRData << " ";

#ifdef SYSTEMC_INCLUDED
    os << std::dec << "@" << sc_time_stamp();
#else
    os << std::dec << "@" << "no-timed";
#endif

    return os;
  }
//#endif

#ifdef SYSTEMC_INCLUDED
  // Only for SystemC
  inline friend void sc_trace(sc_trace_file* tf, const AHB_RSP_SL& flit, const std::string& name) {
    sc_trace(tf, flit.HRData, name + ".addr");
    sc_trace(tf, flit.HReadyout, name + ".readyout");
    sc_trace(tf, flit.HResp, name + ".response");
  }
#endif

  // Matchlib Marshaller requirement
  template<unsigned int Size>
  void Marshall(Marshaller<Size>& m) {
    m& HReadyout;
    m& HResp;
    m& HRData;
  };
};

template<int DATA_WIDTH>
struct AHB_RSP_MA {

  sc_uint<READY_WIDTH> HReady;
  sc_uint<RESP_WIDTH>  HResp;
  sc_uint<DATA_WIDTH>  HRData;

  AHB_RSP_MA() {
    HReady = 1;
    HResp = ahb::AHB_Encoding::AHBRESP::OKAY;
    HRData = 88;
  }

  static const int width = READY_WIDTH + RESP_WIDTH + DATA_WIDTH;

  inline friend std::ostream& operator << ( std::ostream& os, const AHB_RSP_MA& flit_tmp ) {
    os << flit_tmp.HReady << " ";
    os << flit_tmp.HResp << " ";
    os << flit_tmp.HRData << " ";

#ifdef SYSTEMC_INCLUDED
    os << std::dec << "@" << sc_time_stamp();
#else
    os << std::dec << "@" << "no-timed";
#endif
    return os;
  }

  inline AHB_RSP_MA& operator = (const AHB_RSP_SL<DATA_WIDTH>& rhs) {
    HRData = rhs.HRData;
    HResp = rhs.HResp;
    HReady = rhs.HReadyout;

    return *this;
  }

#ifdef SYSTEMC_INCLUDED
  // Only for SystemC
  inline friend void sc_trace(sc_trace_file* tf, const AHB_RSP_MA& flit, const std::string& name) {
    sc_trace(tf, flit.HRData, name + ".addr");
    sc_trace(tf, flit.HReady, name + ".readyout");
  }
#endif

  // Matchlib Marshaller requirement
  template<unsigned int Size>
  void Marshall(Marshaller<Size>& m) {
    m& HReady;
    m& HResp;
    m& HRData;
  };
};


#endif //__AHB_H__
