# Copyright (c) 2010-2011 Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.


# To get logging from pcsh:
# ::pct>  ::xtsc_vp::log_on

namespace eval ::xtsc_vp {

  variable has_xtsc_vp_instances_save   0
  variable log_level_initial            0
  variable log_level 



  proc log_on {} {
    global ::xtsc_vp::log_level
    set log_level 1
  }



  proc log_off {} {
    global ::xtsc_vp::log_level
    set log_level 0
  }



  proc log { message } {
    global ::xtsc_vp::log_level
    if { $log_level != 0 } {
      puts $message
    }
  }



  proc parameter_change_cb { instance parameter_set parameter_name old_value } {
    set new_value [::pct::get_param_value ${instance} ${parameter_set} ${parameter_name}]
    if { "${parameter_set}" == "Template Arguments" } {
      if { ("${parameter_name}" == "NUM_MASTERS") || ("${parameter_name}" == "NUM_SLAVES") || ("${parameter_name}" == "NUM_PORTS") } {
        # Limit xtsc_arbiter_vp, xtsc_router_vp, and xtsc_memory_vp to 8 ports
        if { (${new_value} >= 1) && (${new_value} <= 8) } {
          ::pct::update_port_array ${instance}
        } else {
          ::pct::display_warning_message  "${instance}: Template parameter '${parameter_name}' must be between 1 and 8 " TEN1001
          ::pct::set_param_value ${instance} ${parameter_set} ${parameter_name} ${old_value}
        }
      }
    }
  }



  proc add_parameter { parm_name parm_type default_value } {
    log "::xtsc_vp::add_parameter $parm_name $parm_type $default_value"
    set hardware [::pct::get_hardware_system]
    if { [lsearch [::pct::list_param_names $hardware "Extra properties"] /xtsc_vp/$parm_name] == -1 } { 
      ::pct::create_user_parameter       $hardware  /xtsc_vp/$parm_name  $parm_type
      ::pct::set_user_parameter_property $hardware  /xtsc_vp/$parm_name  default_value   $default_value
    }
  }



  proc remove_parameter { parm_name } {
    log "::xtsc_vp::remove_parameter $parm_name"
    set hardware [::pct::get_hardware_system]
    if { [lsearch [::pct::list_param_names $hardware "Extra properties"] /xtsc_vp/$parm_name] != -1 } { 
      ::pct::remove_user_parameter $hardware /xtsc_vp/$parm_name
    }
  }



  proc encap_build_script {} {
    log "::xtsc_vp::encap_build_script"
    variable preprocessor_opts  "-I \$(XTENSA_SW_TOOLS)/include"
    variable linker_opts        ""
    variable license_opts       ""
    variable compiler           [::scsh::get_backend_compiler]
    variable scsh_version       [::scsh::version]
    variable ISS                iss
    variable ALT_XTSC           CoWare-$scsh_version
    if { [info exists ::env(ALT_XTSC_COWARE_VERSION_OVERRIDE)] } {
      set ALT_XTSC CoWare-$::env(ALT_XTSC_COWARE_VERSION_OVERRIDE)
    } else {
      if { [string equal -length 8 $scsh_version "2010.1.1"  ] } { set ALT_XTSC CoWare-V2010.1.1 }
      if { [string equal -length 7 $scsh_version "2010.09"   ] } { set ALT_XTSC CoWare-E2010.09  }
      if { [string equal -length 9 $scsh_version "F-2011.06" ] } { set ALT_XTSC CoWare-F-2011.06 }
    }

    # Also see print_system_trailer ../../../SWConfig/xtsc/xtsc_vp.common.pl
    if { "${compiler}" == "msvc-9.0" } {
      if { [string equal -length 16 $ALT_XTSC "CoWare-F-2011.06" ] } {
        set license_opts " \
                           -LIBPATH:$::env(COWAREHOME)/IP_common/common_debug/lib/windows.${compiler} \
                           libipModuleInit.a \
                           libipdebug.a \
        "
      }
      set ISS iss-vc90
      set linker_opts " \
                        -LIBPATH:\$(XTENSA_SW_TOOLS)/lib/$ISS/$ALT_XTSC \
                        -LIBPATH:\$(XTENSA_SW_TOOLS)/lib/$ISS \
                        -LIBPATH:\$(XTENSA_SW_TOOLS)/lib \
                        xtsc_vp.lib \
                        xtsc_comp.lib \
                        xtsc.lib \
                        xtmp.lib \
                        simxtcore.lib \
                        log4xtensa.lib \
                        xtparams-msvc.lib \
                        ${license_opts} \
      "
    } elseif { ("${compiler}" == "gcc-3.4.4") || ("${compiler}" == "gcc-4.1.2") || ("${compiler}" == "gcc-4.2.2") } {
      if { [string equal -length 16 $ALT_XTSC "CoWare-F-2011.06" ] } {
        set license_opts " \
                           -L $::env(COWAREHOME)/IP_common/common_debug/lib/linux.${compiler} \
                           -l ipModuleInit \
                           -l ipdebug \
        "
      }
      if { "${compiler}" == "gcc-4.1.2" } { set ISS iss-GCC-4.1 }
      if { "${compiler}" == "gcc-4.2.2" } { set ISS iss-GCC-4.2 }
      set linker_opts " -m32 \
                        -L \$(XTENSA_SW_TOOLS)/lib/$ISS/$ALT_XTSC \
                        -L \$(XTENSA_SW_TOOLS)/lib/$ISS \
                        -L \$(XTENSA_SW_TOOLS)/lib \
                        -l xtsc_vp \
                        -l xtsc_comp \
                        -l xtsc \
                        -l xtmp \
                        -l simxtcore \
                        -l log4xtensa \
                        -l xtparams \
                        -Wl,-rpath,\$(XTENSA_SW_TOOLS)/lib/$ISS \
                        -Wl,-rpath,\$(XTENSA_SW_TOOLS)/lib/iss \
                        -Wl,-rpath,\$(XTENSA_SW_TOOLS)/lib \
                        ${license_opts} \
      "
    } else {
      error "Unsupported compiler: ${compiler}"
    }

    log "::xtsc_vp::encap_build_script scsh_version=$scsh_version compiler=$compiler ISS=$ISS ALT_XTSC=$ALT_XTSC"

    ::scsh::cwr_append_ipsimbld_opts preprocessor $preprocessor_opts
    ::scsh::cwr_append_ipsimbld_opts linker       $linker_opts
  }



  proc has_xtsc_vp_instances {} {
    foreach instance [::pct::get_instance_list] {
      if [::pct::is_leaf_instance $instance] {
        if [string match xtsc_core_vp* [::pct::get_block_from_instance $instance]] {
          log "::xtsc_vp::has_xtsc_vp_instances returning 1"
          return 1
        }
        foreach port [::pct::list_ports $instance] {
          foreach interface [::pct::list_possible_port_protocols $instance/$port] {
            if [string match xtsc_*_if $interface] {
              log "::xtsc_vp::has_xtsc_vp_instances returning 1"
              return 1
            }
          }
        }
      }
    }
    log "::xtsc_vp::has_xtsc_vp_instances returning 0"
    return 0
  }



  proc remove_global_xtsc_vp_parameters {} {
    log "::xtsc_vp::remove_global_xtsc_vp_parameters"

    ::xtsc_vp::remove_parameter  breakpoint_csv_file
    ::xtsc_vp::remove_parameter  call_sc_stop_on_finalize
    ::xtsc_vp::remove_parameter  clock_phase_delta_factors
    ::xtsc_vp::remove_parameter  constructor_log_level
    ::xtsc_vp::remove_parameter  coordinated_debugging
    ::xtsc_vp::remove_parameter  hex_dump_left_to_right
    ::xtsc_vp::remove_parameter  posedge_offset_factor
    ::xtsc_vp::remove_parameter  simcall_csv_file
    ::xtsc_vp::remove_parameter  simcall_csv_format
    ::xtsc_vp::remove_parameter  stop_after_all_cores_exit
    ::xtsc_vp::remove_parameter  system_clock_factor
    ::xtsc_vp::remove_parameter  target_memory_limit
    ::xtsc_vp::remove_parameter  text_logging_config_file
    ::xtsc_vp::remove_parameter  text_logging_delta_cycle_digits
    ::xtsc_vp::remove_parameter  text_logging_disable
    ::xtsc_vp::remove_parameter  text_logging_time_precision
    ::xtsc_vp::remove_parameter  text_logging_time_width
    ::xtsc_vp::remove_parameter  turbo
    ::xtsc_vp::remove_parameter  turbo_max_relaxed_cycles
    ::xtsc_vp::remove_parameter  turbo_min_sync
    ::xtsc_vp::remove_parameter  xtsc_finalize_unwind

  }



  proc add_global_xtsc_vp_parameters {} {
    log "::xtsc_vp::add_global_xtsc_vp_parameters"

    ::xtsc_vp::add_parameter  breakpoint_csv_file                  string          ""
    ::xtsc_vp::add_parameter  call_sc_stop_on_finalize             boolean         true
    ::xtsc_vp::add_parameter  clock_phase_delta_factors            string          200,100,600
    ::xtsc_vp::add_parameter  constructor_log_level                string          INFO
    ::xtsc_vp::add_parameter  coordinated_debugging                boolean         true
    ::xtsc_vp::add_parameter  hex_dump_left_to_right               boolean         true
    ::xtsc_vp::add_parameter  posedge_offset_factor                integer         0xFFFFFFFF
    ::xtsc_vp::add_parameter  simcall_csv_file                     string          ""
    ::xtsc_vp::add_parameter  simcall_csv_format                   string          ""
    ::xtsc_vp::add_parameter  stop_after_all_cores_exit            boolean         true
    ::xtsc_vp::add_parameter  system_clock_factor                  integer         1000
    ::xtsc_vp::add_parameter  target_memory_limit                  integer         512
    ::xtsc_vp::add_parameter  text_logging_config_file             string          ""
    ::xtsc_vp::add_parameter  text_logging_delta_cycle_digits      integer         3
    ::xtsc_vp::add_parameter  text_logging_disable                 boolean         false
    ::xtsc_vp::add_parameter  text_logging_time_precision          integer         1
    ::xtsc_vp::add_parameter  text_logging_time_width              integer         10
    ::xtsc_vp::add_parameter  turbo                                boolean         false
    ::xtsc_vp::add_parameter  turbo_max_relaxed_cycles             integer         10000000
    ::xtsc_vp::add_parameter  turbo_min_sync                       boolean         false
    ::xtsc_vp::add_parameter  xtsc_finalize_unwind                 boolean         true

  }



  proc create_instance_pre { args } {
    log "::xtsc_vp::create_instance_pre"
    global ::xtsc_vp::has_xtsc_vp_instances_save
    set has_xtsc_vp_instances_save [has_xtsc_vp_instances]
  }



  proc create_instance_post { args } {
    log "::xtsc_vp::create_instance_post"
    if { ($::xtsc_vp::has_xtsc_vp_instances_save == 0) && ([ has_xtsc_vp_instances ] == 1) } {
      add_global_xtsc_vp_parameters
    }
  }



  proc remove_instance_pre { args } {
    log "::xtsc_vp::remove_instance_pre"
    global ::xtsc_vp::has_xtsc_vp_instances_save
    set has_xtsc_vp_instances_save [has_xtsc_vp_instances]
  }



  proc remove_instance_post { args } {
    log "::xtsc_vp::remove_instance_post"
    if { ($::xtsc_vp::has_xtsc_vp_instances_save == 1) && ([ has_xtsc_vp_instances ] == 0) } {
      remove_global_xtsc_vp_parameters
    }
  }



  proc add_callback { command callback pre } {
    log "::xtsc_vp::add_callback $command $callback $pre"
    if { [lsearch [::pct::list_callbacks] $callback] == -1 } { 
      ::pct::add_callback $command $callback $pre 
    }
  }



} ;  #  namespace ::xtsc_vp





proc open_library_callback  { library_name } {
  ::xtsc_vp::log "::xtsc_vp::Opening library $library_name"

  ::xtsc_vp::add_callback ::pct::create_instance ::xtsc_vp::create_instance_pre  1
  ::xtsc_vp::add_callback ::pct::create_instance ::xtsc_vp::create_instance_post 0

  ::xtsc_vp::add_callback ::pct::instantiate_block ::xtsc_vp::create_instance_pre  1
  ::xtsc_vp::add_callback ::pct::instantiate_block ::xtsc_vp::create_instance_post 0

  ::xtsc_vp::add_callback ::pct::remove_instance ::xtsc_vp::remove_instance_pre  1
  ::xtsc_vp::add_callback ::pct::remove_instance ::xtsc_vp::remove_instance_post 0
}



proc close_library_callback { library_name } {
  ::xtsc_vp::log "::xtsc_vp::Closing library $library_name"
}





if { [info exists ::xtsc_vp::log_level] == 0 } { set ::xtsc_vp::log_level $::xtsc_vp::log_level_initial }

