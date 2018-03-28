// ==============================================================
// RTL generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2017.2
// Copyright (C) 1986-2017 Xilinx, Inc. All Rights Reserved.
// 
// ===========================================================

#ifndef _md_HH_
#define _md_HH_

#include "systemc.h"
#include "AESL_pkg.h"

#include "doMD.h"

namespace ap_rtl {

struct md : public sc_module {
    // Port declarations 12
    sc_in_clk ap_clk;
    sc_in< sc_logic > ap_rst;
    sc_in< sc_logic > ap_start;
    sc_out< sc_logic > ap_done;
    sc_out< sc_logic > ap_idle;
    sc_out< sc_logic > ap_ready;
    sc_out< sc_lv<121> > stream_out_V_din;
    sc_in< sc_logic > stream_out_V_full_n;
    sc_out< sc_logic > stream_out_V_write;
    sc_in< sc_lv<121> > stream_in_V_dout;
    sc_in< sc_logic > stream_in_V_empty_n;
    sc_out< sc_logic > stream_in_V_read;


    // Module declarations
    md(sc_module_name name);
    SC_HAS_PROCESS(md);

    ~md();

    sc_trace_file* mVcdFile;

    ofstream mHdltvinHandle;
    ofstream mHdltvoutHandle;
    doMD* grp_doMD_fu_140;
    sc_signal< sc_lv<2> > ap_CS_fsm;
    sc_signal< sc_logic > ap_CS_fsm_state1;
    sc_signal< sc_logic > grp_doMD_fu_140_ap_start;
    sc_signal< sc_logic > grp_doMD_fu_140_ap_done;
    sc_signal< sc_logic > grp_doMD_fu_140_ap_idle;
    sc_signal< sc_logic > grp_doMD_fu_140_ap_ready;
    sc_signal< sc_lv<121> > grp_doMD_fu_140_stream_out_V_din;
    sc_signal< sc_logic > grp_doMD_fu_140_stream_out_V_write;
    sc_signal< sc_logic > grp_doMD_fu_140_stream_in_V_read;
    sc_signal< sc_logic > ap_reg_grp_doMD_fu_140_ap_start;
    sc_signal< sc_logic > ap_CS_fsm_state2;
    sc_signal< sc_lv<2> > ap_NS_fsm;
    static const sc_logic ap_const_logic_1;
    static const sc_logic ap_const_logic_0;
    static const sc_lv<2> ap_ST_fsm_state1;
    static const sc_lv<2> ap_ST_fsm_state2;
    static const sc_lv<32> ap_const_lv32_0;
    static const sc_lv<32> ap_const_lv32_1;
    static const bool ap_const_boolean_1;
    // Thread declarations
    void thread_ap_clk_no_reset_();
    void thread_ap_CS_fsm_state1();
    void thread_ap_CS_fsm_state2();
    void thread_ap_done();
    void thread_ap_idle();
    void thread_ap_ready();
    void thread_grp_doMD_fu_140_ap_start();
    void thread_stream_in_V_read();
    void thread_stream_out_V_din();
    void thread_stream_out_V_write();
    void thread_ap_NS_fsm();
    void thread_hdltv_gen();
};

}

using namespace ap_rtl;

#endif