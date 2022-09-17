# Customer ID=8327; Build=0x3b95c; Copyright (c) 2007-2011 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of
# Tensilica Inc.  They may be adapted and modified by bona fide
# purchasers for internal use, but neither the original nor any adapted
# or modified version may be disclosed or distributed to third parties
# in any manner, medium, or form, in whole or in part, without the prior
# written consent of Tensilica Inc.


variable module 
variable ISS    iss

if { "$env(COWARE_CXX_COMPILER)" == "gcc-4.1.2" } { set ISS iss-GCC-4.1 }


::pct::new_project
::pct::open_library $env(XTENSA_SW_TOOLS)/misc/xtsc_vp/xtsc_vp.xml



set module xtsc_master_vp
::pct::create_instance $module          /HARDWARE master xtsc_vp::$module xtsc_vp::$module\()
::pct::set_param_value                  /HARDWARE/master "Scml Properties"    /Misc/script_file ../../xtsc-run/xtsc_slave/request.vec

::pct::set_bounds                       /HARDWARE/master 100 100 150 150

::pct::set_orientation                  /HARDWARE/master/m_request_port        right
::pct::set_orientation                  /HARDWARE/master/m_respond_export      right
::pct::set_location_on_owner            /HARDWARE/master/m_request_port        22
::pct::set_location_on_owner            /HARDWARE/master/m_respond_export      55



set module xtsc_slave_vp
::pct::create_instance $module          /HARDWARE slave xtsc_vp::$module xtsc_vp::$module\()
::pct::set_param_value                  /HARDWARE/slave "Scml Properties"    /Misc/script_file ../../xtsc-run/xtsc_slave/respond.vec

::pct::set_bounds                       /HARDWARE/slave 400 100 150 150

::pct::set_orientation                  /HARDWARE/slave/m_request_export       left
::pct::set_orientation                  /HARDWARE/slave/m_respond_port         left
::pct::set_location_on_owner            /HARDWARE/slave/m_request_export       22
::pct::set_location_on_owner            /HARDWARE/slave/m_respond_port         55



::pct::create_connection req /HARDWARE    /HARDWARE/master/m_request_port    /HARDWARE/slave/m_request_export
::pct::create_connection rsp /HARDWARE    /HARDWARE/master/m_respond_export  /HARDWARE/slave/m_respond_port

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


