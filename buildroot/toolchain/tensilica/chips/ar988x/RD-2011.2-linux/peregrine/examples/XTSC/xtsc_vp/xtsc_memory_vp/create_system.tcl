# Customer ID=8327; Build=0x3b95c; Copyright (c) 2007-2008 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of
# Tensilica Inc.  They may be adapted and modified by bona fide
# purchasers for internal use, but neither the original nor any adapted
# or modified version may be disclosed or distributed to third parties
# in any manner, medium, or form, in whole or in part, without the prior
# written consent of Tensilica Inc.


variable module xtsc_core_vp
variable ISS    iss

if { "$env(COWARE_CXX_COMPILER)" == "gcc-4.1.2" } { set ISS iss-GCC-4.1 }


::pct::new_project
::pct::open_library $env(XTENSA_SW_TOOLS)/misc/xtsc_vp/xtsc_vp.xml
::pct::open_library $module/$module.xml



set module xtsc_core_vp
::pct::create_instance $module          /HARDWARE core0 xtsc_vp::$module xtsc_vp::$module\()
::pct::set_shape                        /HARDWARE/core0 core
::pct::set_param_value                  /HARDWARE/core0 "Scml Properties"    /Target/SimTargetOutput  core0_output.log
::pct::set_param_value                  /HARDWARE/core0 "Scml Properties"    /Target/SimTargetProgram target/memory_test.out

::pct::set_bounds                       /HARDWARE/core0 400 100 100 1050

::pct::set_orientation                  /HARDWARE/core0/iram0_req           left
::pct::set_orientation                  /HARDWARE/core0/iram0_rsp           left
::pct::set_location_on_owner            /HARDWARE/core0/iram0_req           22
::pct::set_location_on_owner            /HARDWARE/core0/iram0_rsp           55

::pct::set_orientation                  /HARDWARE/core0/iram1_req           left
::pct::set_orientation                  /HARDWARE/core0/iram1_rsp           left
::pct::set_location_on_owner            /HARDWARE/core0/iram1_req           172
::pct::set_location_on_owner            /HARDWARE/core0/iram1_rsp           205

::pct::set_orientation                  /HARDWARE/core0/dram0_req           left
::pct::set_orientation                  /HARDWARE/core0/dram0_rsp           left
::pct::set_location_on_owner            /HARDWARE/core0/dram0_req           322
::pct::set_location_on_owner            /HARDWARE/core0/dram0_rsp           355

::pct::set_orientation                  /HARDWARE/core0/dram1_req           left
::pct::set_orientation                  /HARDWARE/core0/dram1_rsp           left
::pct::set_location_on_owner            /HARDWARE/core0/dram1_req           622
::pct::set_location_on_owner            /HARDWARE/core0/dram1_rsp           655


::pct::set_orientation                  /HARDWARE/core0/pif_req             right
::pct::set_orientation                  /HARDWARE/core0/pif_rsp             right
::pct::set_location_on_owner            /HARDWARE/core0/pif_req             22
::pct::set_location_on_owner            /HARDWARE/core0/pif_rsp             55


set module xtsc_memory_vp
::pct::create_instance $module          /HARDWARE iram0 xtsc_vp::$module xtsc_vp::$module\()
::pct::set_param_value                  /HARDWARE/iram0 "Template Arguments" DATA_WIDTH                    32
::pct::set_param_value                  /HARDWARE/iram0 "Template Arguments" NUM_PORTS                     1
::pct::set_param_value                  /HARDWARE/iram0 "Scml Properties"    /Timing/block_read_delay      0
::pct::set_param_value                  /HARDWARE/iram0 "Scml Properties"    /Timing/block_write_delay     0
::pct::set_param_value                  /HARDWARE/iram0 "Scml Properties"    /Timing/block_write_repeat    0
::pct::set_param_value                  /HARDWARE/iram0 "Scml Properties"    /Timing/block_write_response  0
::pct::set_param_value                  /HARDWARE/iram0 "Scml Properties"    /Timing/rcw_response          0
::pct::set_param_value                  /HARDWARE/iram0 "Scml Properties"    /Timing/read_delay            0
::pct::set_param_value                  /HARDWARE/iram0 "Scml Properties"    /Timing/write_delay           0

::pct::set_bounds                       /HARDWARE/iram0 100 100 100 125

::pct::set_orientation                  /HARDWARE/iram0/m_request_exports\[0\]    right
::pct::set_orientation                  /HARDWARE/iram0/m_respond_ports\[0\]      right
::pct::set_orientation                  /HARDWARE/iram0/m_respond_ports\[0\]      top
::pct::set_location_on_owner            /HARDWARE/iram0/m_respond_ports\[0\]      55
::pct::set_location_on_owner            /HARDWARE/iram0/m_request_exports\[0\]    22
::pct::set_orientation                  /HARDWARE/iram0/m_respond_ports\[0\]      right
::pct::set_location_on_owner            /HARDWARE/iram0/m_respond_ports\[0\]      55



set module xtsc_memory_vp
::pct::create_instance $module          /HARDWARE iram1 xtsc_vp::$module xtsc_vp::$module\()
::pct::set_param_value                  /HARDWARE/iram1 "Template Arguments" DATA_WIDTH                    32
::pct::set_param_value                  /HARDWARE/iram1 "Template Arguments" NUM_PORTS                     1
::pct::set_param_value                  /HARDWARE/iram1 "Scml Properties"    /Timing/block_read_delay      0
::pct::set_param_value                  /HARDWARE/iram1 "Scml Properties"    /Timing/block_write_delay     0
::pct::set_param_value                  /HARDWARE/iram1 "Scml Properties"    /Timing/block_write_repeat    0
::pct::set_param_value                  /HARDWARE/iram1 "Scml Properties"    /Timing/block_write_response  0
::pct::set_param_value                  /HARDWARE/iram1 "Scml Properties"    /Timing/rcw_response          0
::pct::set_param_value                  /HARDWARE/iram1 "Scml Properties"    /Timing/read_delay            0
::pct::set_param_value                  /HARDWARE/iram1 "Scml Properties"    /Timing/write_delay           0

::pct::set_bounds                       /HARDWARE/iram1 100 250 100 125

::pct::set_orientation                  /HARDWARE/iram1/m_request_exports\[0\]    right
::pct::set_orientation                  /HARDWARE/iram1/m_respond_ports\[0\]      right
::pct::set_orientation                  /HARDWARE/iram1/m_respond_ports\[0\]      top
::pct::set_location_on_owner            /HARDWARE/iram1/m_respond_ports\[0\]      55
::pct::set_location_on_owner            /HARDWARE/iram1/m_request_exports\[0\]    22
::pct::set_orientation                  /HARDWARE/iram1/m_respond_ports\[0\]      right
::pct::set_location_on_owner            /HARDWARE/iram1/m_respond_ports\[0\]      55



set module xtsc_memory_vp
::pct::create_instance $module          /HARDWARE dram0 xtsc_vp::$module xtsc_vp::$module\()
::pct::set_param_value                  /HARDWARE/dram0 "Template Arguments" DATA_WIDTH                    32
::pct::set_param_value                  /HARDWARE/dram0 "Template Arguments" NUM_PORTS                     1
::pct::set_param_value                  /HARDWARE/dram0 "Scml Properties"    /Timing/block_read_delay      0
::pct::set_param_value                  /HARDWARE/dram0 "Scml Properties"    /Timing/block_write_delay     0
::pct::set_param_value                  /HARDWARE/dram0 "Scml Properties"    /Timing/block_write_repeat    0
::pct::set_param_value                  /HARDWARE/dram0 "Scml Properties"    /Timing/block_write_response  0
::pct::set_param_value                  /HARDWARE/dram0 "Scml Properties"    /Timing/rcw_response          0
::pct::set_param_value                  /HARDWARE/dram0 "Scml Properties"    /Timing/read_delay            0
::pct::set_param_value                  /HARDWARE/dram0 "Scml Properties"    /Timing/write_delay           0

::pct::set_bounds                       /HARDWARE/dram0 100 400 100 125

::pct::set_orientation                  /HARDWARE/dram0/m_request_exports\[0\]    right
::pct::set_orientation                  /HARDWARE/dram0/m_respond_ports\[0\]      right
::pct::set_orientation                  /HARDWARE/dram0/m_respond_ports\[0\]      top
::pct::set_location_on_owner            /HARDWARE/dram0/m_respond_ports\[0\]      55
::pct::set_location_on_owner            /HARDWARE/dram0/m_request_exports\[0\]    22
::pct::set_orientation                  /HARDWARE/dram0/m_respond_ports\[0\]      right
::pct::set_location_on_owner            /HARDWARE/dram0/m_respond_ports\[0\]      55



set module xtsc_memory_vp
::pct::create_instance $module          /HARDWARE dram1 xtsc_vp::$module xtsc_vp::$module\()
::pct::set_param_value                  /HARDWARE/dram1 "Template Arguments" DATA_WIDTH                    32
::pct::set_param_value                  /HARDWARE/dram1 "Template Arguments" NUM_PORTS                     1
::pct::set_param_value                  /HARDWARE/dram1 "Scml Properties"    /Timing/block_read_delay      0
::pct::set_param_value                  /HARDWARE/dram1 "Scml Properties"    /Timing/block_write_delay     0
::pct::set_param_value                  /HARDWARE/dram1 "Scml Properties"    /Timing/block_write_repeat    0
::pct::set_param_value                  /HARDWARE/dram1 "Scml Properties"    /Timing/block_write_response  0
::pct::set_param_value                  /HARDWARE/dram1 "Scml Properties"    /Timing/rcw_response          0
::pct::set_param_value                  /HARDWARE/dram1 "Scml Properties"    /Timing/read_delay            0
::pct::set_param_value                  /HARDWARE/dram1 "Scml Properties"    /Timing/write_delay           0

::pct::set_bounds                       /HARDWARE/dram1 100 700 100 125

::pct::set_orientation                  /HARDWARE/dram1/m_request_exports\[0\]    right
::pct::set_orientation                  /HARDWARE/dram1/m_respond_ports\[0\]      right
::pct::set_orientation                  /HARDWARE/dram1/m_respond_ports\[0\]      top
::pct::set_location_on_owner            /HARDWARE/dram1/m_respond_ports\[0\]      55
::pct::set_location_on_owner            /HARDWARE/dram1/m_request_exports\[0\]    22
::pct::set_orientation                  /HARDWARE/dram1/m_respond_ports\[0\]      right
::pct::set_location_on_owner            /HARDWARE/dram1/m_respond_ports\[0\]      55



set module xtsc_memory_vp
::pct::create_instance $module          /HARDWARE pif xtsc_vp::$module xtsc_vp::$module\()
::pct::set_param_value                  /HARDWARE/pif "Template Arguments" DATA_WIDTH                    32
::pct::set_param_value                  /HARDWARE/pif "Template Arguments" NUM_PORTS                     1
::pct::set_param_value                  /HARDWARE/pif "Scml Properties"    /Timing/block_read_delay      1
::pct::set_param_value                  /HARDWARE/pif "Scml Properties"    /Timing/block_write_delay     1
::pct::set_param_value                  /HARDWARE/pif "Scml Properties"    /Timing/block_write_repeat    1
::pct::set_param_value                  /HARDWARE/pif "Scml Properties"    /Timing/block_write_response  1
::pct::set_param_value                  /HARDWARE/pif "Scml Properties"    /Timing/rcw_response          1
::pct::set_param_value                  /HARDWARE/pif "Scml Properties"    /Timing/read_delay            1
::pct::set_param_value                  /HARDWARE/pif "Scml Properties"    /Timing/write_delay           1

::pct::set_bounds                       /HARDWARE/pif 700 100 100 125

::pct::set_orientation                  /HARDWARE/pif/m_request_exports\[0\]    left
::pct::set_orientation                  /HARDWARE/pif/m_respond_ports\[0\]      left
::pct::set_orientation                  /HARDWARE/pif/m_respond_ports\[0\]      top
::pct::set_location_on_owner            /HARDWARE/pif/m_respond_ports\[0\]      55
::pct::set_location_on_owner            /HARDWARE/pif/m_request_exports\[0\]    22
::pct::set_orientation                  /HARDWARE/pif/m_respond_ports\[0\]      left
::pct::set_location_on_owner            /HARDWARE/pif/m_respond_ports\[0\]      55




::pct::create_connection iram0_req /HARDWARE    /HARDWARE/core0/iram0_req                 /HARDWARE/iram0/m_request_exports\[0\]
::pct::create_connection iram0_rsp /HARDWARE    /HARDWARE/core0/iram0_rsp                 /HARDWARE/iram0/m_respond_ports\[0\]

::pct::create_connection iram1_req /HARDWARE    /HARDWARE/core0/iram1_req                 /HARDWARE/iram1/m_request_exports\[0\]
::pct::create_connection iram1_rsp /HARDWARE    /HARDWARE/core0/iram1_rsp                 /HARDWARE/iram1/m_respond_ports\[0\]

::pct::create_connection dram0_req /HARDWARE    /HARDWARE/core0/dram0_req                 /HARDWARE/dram0/m_request_exports\[0\]
::pct::create_connection dram0_rsp /HARDWARE    /HARDWARE/core0/dram0_rsp                 /HARDWARE/dram0/m_respond_ports\[0\]

::pct::create_connection dram1_req /HARDWARE    /HARDWARE/core0/dram1_req                 /HARDWARE/dram1/m_request_exports\[0\]
::pct::create_connection dram1_rsp /HARDWARE    /HARDWARE/core0/dram1_rsp                 /HARDWARE/dram1/m_respond_ports\[0\]

::pct::create_connection pif_req /HARDWARE    /HARDWARE/core0/pif_req                 /HARDWARE/pif/m_request_exports\[0\]
::pct::create_connection pif_rsp /HARDWARE    /HARDWARE/core0/pif_rsp                 /HARDWARE/pif/m_respond_ports\[0\]


::pct::save_system system.xml



::pct::create_simulation_build_project .

::pct::add_path_variable LIB_DIR $env(XTENSA_SW_TOOLS)/lib
::pct::add_path_variable INC_DIR $env(XTENSA_SW_TOOLS)/include
::pct::add_path_variable SRC_DIR $env(PWD)/SRC_DIR

::pct::set_simulation_build_project_setting Debug "Disable Elaboration" true
::pct::set_simulation_build_project_setting Debug "Include Path Substitution" false
::pct::set_simulation_build_project_setting Debug "Enable Instrumentation" false
::pct::set_simulation_build_project_setting Debug "Backdoor Mode" true

::pct::set_simulation_build_project_setting Debug "Include Paths" " \
                                     $env(XTENSA_SW_TOOLS)/include \
                                     "

::pct::set_simulation_build_project_setting Debug Libraries " \
                                     xtsc_vp \
                                     xtsc_comp \
                                     xtsc \
                                     xtmp \
                                     simxtcore \
                                     log4xtensa \
                                     xtparams \
                                     "

::pct::set_simulation_build_project_setting Debug "Library Search Paths" " \
                                     $env(XTENSA_SW_TOOLS)/lib/$ISS/CoWare-E2010.09 \
                                     $env(XTENSA_SW_TOOLS)/lib/$ISS \
                                     $env(XTENSA_SW_TOOLS)/lib \
                                     "

::pct::set_simulation_build_project_setting Debug "Linker Flags" " \
                                     -m32 \
                                     -Wl,-rpath,$env(XTENSA_SW_TOOLS)/lib/$ISS \
                                     -Wl,-rpath,$env(XTENSA_SW_TOOLS)/lib/iss \
                                     -Wl,-rpath,$env(XTENSA_SW_TOOLS)/lib \
                                     "

::pct::set_preference_value /Messages/FilterIDs ESA0022
::pct::export_simulation


