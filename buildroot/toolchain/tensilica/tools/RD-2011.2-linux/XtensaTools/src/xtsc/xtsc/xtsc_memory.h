#ifndef _XTSC_MEMORY_H_
#define _XTSC_MEMORY_H_

// Copyright (c) 2005-2010 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.

/**
 * @file 
 */

#include <xtsc/xtsc_request_if.h>
#include <xtsc/xtsc_respond_if.h>
#include <xtsc/xtsc_request.h>
#include <xtsc/xtsc_response.h>
#include <xtsc/xtsc_parms.h>
#include <xtsc/xtsc_memory_base.h>
#include <cstring>
#include <vector>


namespace xtsc {
class xtsc_core;
class xtsc_cohctrl;
class xtsc_fast_access_if;
}

namespace xtsc_component {

class xtsc_arbiter;
class xtsc_dma_engine;
class xtsc_master;
class xtsc_memory_trace;
class xtsc_pin2tlm_memory_transactor;
class xtsc_router;


/**
 * Constructor parameters for a xtsc_memory object.
 *
 * This class contains the constructor parameters for an xtsc_memory object.
 *
 *  \verbatim
   Name                   Type  Description
   ------------------     ----  -------------------------------------------------------

   "num_ports"            u32   The number of slave port pairs this memory has.  This
                                defines how many memory interface master devices will be
                                connected with this memory.
                                Default = 1.
                                Minimum = 1.

   "byte_width"           u32   Memory data interface width in bytes.  Valid values are
                                0, 4, 8, 16, 32, and 64.  A value of 0 indicates that this 
                                memory supports all of the valid data interface widths.
                                This can be useful, for example, to model a memory that 
                                is connected to multiple Xtensa cores that do not all
                                have the same PIF data interface width.
  
   "start_byte_address"   u32   The starting byte address of this memory in the 4GB
                                address space.
  
   "memory_byte_size"     u32   The byte size of this memory.  0 means the memory
                                occupies all of the 4GB address space at and above
                                "start_byte_address".
  
   "clock_period"         u32   This is the length of this memory's clock period
                                expressed in terms of the SystemC time resolution
                                (from sc_get_time_resolution()).  A value of 
                                0xFFFFFFFF means to use the XTSC system clock 
                                period (from xtsc_get_system_clock_period()).
                                A value of 0 means one delta cycle.
                                Default = 0xFFFFFFFF (i.e. use the system clock 
                                period).

   "request_fifo_depth"   u32   The request fifo depth.
                                Default = 2.
                                Minimum = 1.

   "check_alignment"      bool  If true, requests whose address is not size-aligned or
                                whose size is not a power of 2 will get an
                                RSP_ADDRESS_ERROR response.  If false, this check is not
                                performed.  
                                Default = false (don't check alignment).

   "delay_from_receipt"   bool  If false, the following delay parameters apply from 
                                the start of processing of the request (i.e. after 
                                all previous requests have been responded to).  This
                                models a memory that can only service one request
                                at a time.  If true, the following delay parameters 
                                apply from the time of receipt of the request.  This
                                more closely models a memory with pipelining; however,
                                requests are always completed (i.e. the response(s) 
                                is/are sent) in the order that their corresponding 
                                requests were received.
                                Default = true.

   "recovery_time"        u32   If "delay_from_receipt" is true, this specifies the 
                                minimum number of clock periods after a response is 
                                sent (except for non-last BLOCK_READ responses) or a 
                                first RCW is handled or a non-last BLOCK_WRITE is 
                                handled before the next response will be sent or the 
                                next BLOCK_WRITE will be handled ("block_read_repeat"
                                is used in lieu of "recovery_time" to specify the
                                delay between consecutive BLOCK_READ responses to
                                the same BLOCK_READ request).  If "delay_from_receipt"
                                is false, this parameter is ignored.  
                                Default = 1.

   "read_delay"           u32   If "delay_from_receipt" is false, the number of clock 
                                periods between starting to process a READ request and 
                                sending xtsc::xtsc_response.  If "delay_from_receipt" is true,
                                the minimum number of clock periods between receipt of
                                a READ request and sending xtsc::xtsc_response.
                                Default = 0.

   "block_read_delay"     u32   If "delay_from_receipt" is false, the number of clock 
                                periods between starting to process a BLOCK_READ request 
                                and sending the first xtsc::xtsc_response.  If 
                                "delay_from_receipt" is true, the minimum number of
                                clock periods between receipt of a BLOCK_READ request
                                and sending the first xtsc::xtsc_response.
                                Default = 0.

   "block_read_repeat"    u32   Number of clock periods between each BLOCK_READ response.
                                Default = 1.

   "burst_read_delay"     u32   If "delay_from_receipt" is false, the number of clock 
                                periods between starting to process a BURST_READ request 
                                and sending the first response.  If "delay_from_receipt"
                                is true, the minimum number of clock periods between
                                receipt of a BURST_READ request and sending the first
                                response.  This parameter is ignored unless
                                "memory_interface" is "PIF".
                                Default = 0.

   "burst_read_repeat"    u32   Number of clock periods between each BURST_READ response.
                                This parameter is ignored unless "memory_interface" is
                                "PIF".
                                Default = 1.

   "rcw_repeat"           u32   If "delay_from_receipt" is false, the minimum number of 
                                clock periods between starting to process the first RCW 
                                request and starting to process the second RCW request.
                                If "delay_from_receipt" is true, the minimum number of 
                                clock periods between receipt of the first RCW request 
                                and starting to process the second RCW request.
                                Default = 1.

   "rcw_response"         u32   If "delay_from_receipt" is false, the number of clock 
                                periods between starting to process the second RCW 
                                request and sending the xtsc::xtsc_response.  If
                                "delay_from_receipt" is true, the minimum number of 
                                clock periods between receipt of the second RCW 
                                request and sending the xtsc::xtsc_response.
                                Default = 0.

   "write_delay"          u32   If "delay_from_receipt" is false, the number of clock 
                                periods between starting to process a WRITE request 
                                and sending xtsc::xtsc_response.  If "delay_from_receipt"
                                is true, the minimum number of clock periods between
                                receipt of a WRITE request and sending xtsc::xtsc_response.
                                Default = 0.

   "block_write_delay"    u32   If "delay_from_receipt" is false, the minimum number 
                                of clock periods between starting to process the first
                                BLOCK_WRITE request and starting to process the second 
                                BLOCK_WRITE request.  If "delay_from_receipt" is true,
                                the minimum number of clock periods between receipt
                                of  the first BLOCK_WRITE request and starting to 
                                process the second BLOCK_WRITE request.
                                Default = 0.

   "block_write_repeat"   u32   If "delay_from_receipt" is false, the minimum number 
                                of clock periods between starting to process a 
                                BLOCK_WRITE request (except the first or last one) 
                                and starting to process the following BLOCK_WRITE 
                                request.  If "delay_from_receipt" is true, the 
                                minimum number of clock periods between receipt of 
                                a BLOCK_WRITE request (except the first or last one) 
                                and starting to process the following BLOCK_WRITE 
                                request.
                                Default = 0.

   "block_write_response" u32   If "delay_from_receipt" is false, the number of clock 
                                periods between starting to process the last 
                                BLOCK_WRITE request and sending the xtsc::xtsc_response.
                                If "delay_from_receipt" is true, the minimum number
                                of clock periods between receipt of the last 
                                BLOCK_WRITE request and sending the xtsc::xtsc_response.
                                Default = 0.

   "burst_write_delay"  u32     If "delay_from_receipt" is false, the minimum number 
                                of clock periods between starting to process the first
                                BURST_WRITE request and starting to process the second 
                                BURST_WRITE request.  If "delay_from_receipt" is true,
                                the minimum number of clock periods between receipt
                                of  the first BURST_WRITE request and starting to 
                                process the second BURST_WRITE request.  This parameter
                                is ignored unless "memory_interface" is "PIF".
                                Default = 0.

   "burst_write_repeat" u32     If "delay_from_receipt" is false, the minimum number 
                                of clock periods between starting to process a 
                                BURST_WRITE request (except the first or last one) 
                                and starting to process the following BURST_WRITE 
                                request.  If "delay_from_receipt" is true, the 
                                minimum number of clock periods between receipt of 
                                a BURST_WRITE request (except the first or last one) 
                                and starting to process the following BURST_WRITE 
                                request.  This parameter is ignored unless
                                "memory_interface" is "PIF".
                                Default = 0.

   "burst_write_response" u32   If "delay_from_receipt" is false, the number of clock 
                                periods between starting to process the last BURST_WRITE
                                request and sending the response.  If
                                "delay_from_receipt" is true, the minimum number of
                                clock periods between receipt of the last BURST_WRITE
                                request and sending the response.  This parameter is
                                ignored unless "memory_interface" is "PIF".
                                Default = 0.

   "response_repeat"      u32   The number of clock periods after a response is sent 
                                and rejected before the response will be resent.  A 
                                value of 0 means one delta cycle.  
                                Default = 1.

   "immediate_timing"     bool  If true, the above delays parameters are ignored and
                                the memory model responds to all requests immediately
                                (without any delay--not even a delta cycle).  If false,
                                the above delay parameters are used to determine
                                response timing.
                                Default = false.

   "page_byte_size"       u32   The byte size of a page of memory.  In the model,
                                memory is not allocated until it is accessed.  This
                                parameter specifies the allocation size.
                                Default is 16 Kilobytes (1024*16=16384=0x4000).
                                Minimum page size is 16*byte_width (or 256 if 
                                "byte_width" is 0).

   "initial_value_file"   char* If not NULL or empty, this names a text file from which
                                to read the initial memory contents as byte values.
                                Default = NULL.
                                The text file format is:

                                ([@<Offset>] <Value>*)*

                                1.  Any number (<Offset> or <Value>) can be in decimal
                                    or hexadecimal (using '0x' prefix) format.
                                2.  @<Offset> is added to "start_byte_address".
                                3.  <Value> cannot exceed 255 (0xFF).
                                4.  If a <Value> entry is not immediately preceeded in
                                    the file by an @<Offset> entry, then its offset is
                                    one greater than the preceeding <Value> entry.
                                5.  If the first <Value> entry in the file is not 
                                    preceeded by an @<Offset> entry, then its offset
                                    is zero.
                                6.  Comments, extra whitespace, and blank lines are
                                    ignored.  See xtsc::xtsc_script_file.

                                Example text file contents:

                                   0x01 0x02 0x3    // First three bytes of the memory,
                                                    // 0x01 is at "start_byte_address"
                                   @0x1000 50       // The byte at offset 0x1000 is 50
                                   51 52            // The byte at offset 0x1001 is 51
                                                    // The byte at offset 0x1002 is 52
                             

   "memory_fill_byte"     u32   The low byte specifies the value used to initialize 
                                memory contents at address locations not initialize
                                from "initial_value_file".
                                Default = 0.

   "read_only"            bool  If true this memory model represents a ROM which
                                does not support WRITE, BLOCK_WRITE, and RCW 
                                transactions.  Use "initial_value_file" to initialize
                                the ROM.  The nb_poke() method may also be used to
                                initialize or change the ROM contents.
                                Default = false.

   "use_fast_access"      bool  If true, this memory will support fast access
                                for the turboxim simulation engine.

   "deny_fast_access"  vector<u32>   A std::vector containing an even number of
                                addresses.  Each pair of addresses specifies a range of
                                addresses that will be denied fast access.  The first
                                address in each pair is the start address and the second
                                address in each pair is the end address.

   "use_raw_access"       bool  If fast access is supported, when this is true
                                the memory will give direct raw pointer access
                                to the turboxim simulation engine.  When false,
                                only function access is supported.  This is
                                primarily for testing purposes.

   "script_file"        char*   An optional file to read scripted responses from.
                                Normally xtsc_memory, baring an error in a request and
                                baring requests coming in too fast, performs the
                                requested action (i.e. it reads or writes the memory)
                                and responds with an RSP_OK.  This script file allows
                                you to specify other responses.  This can be useful for
                                test scenarios or for causing an RSP_ADDRESS_ERROR
                                response to be sent to an out-of-bound request instead
                                of having an exception thrown.  You cause non-normal
                                responses to be given by specifying either a single
                                address or an address range and the response that goes
                                with that address or address range and optional port
                                number and request type.  You can also specify the time
                                frame in which that response applies and how many times
                                that response is to be given.  Multiple single addresses
                                and multiple address ranges are supported; however, no
                                overlap is allowed.  Active lines in this file (that is
                                lines that aren't whitespace or comments) fall into one
                                of three categories.
                                  1. Response definition lines
                                  2. Debug/logging lines
                                  3. Timing control lines
                                When a script file is specified, xtsc_memory spawns a
                                SystemC thread to process it.  At the beginning of
                                simulation this thread starts processing each line in
                                the script file in sequence up to the first timing
                                control line (category 3).  When the first timing
                                control line is hit, the thread stops processing the
                                script file and waits for the time or event implied by
                                the timing control line.  When the time or event implied
                                by the first timing control line occurs, the thread
                                resumes processing the script file until the next timing
                                control line, and so on.  A few examples are shown
                                below.
                                The supported line formats organized by category are:

    Category 1: Response Definition Line Formats:
        lowAddr port type status [limit]
        lowAddr-highAddr port type status [limit]
        CLEAR lowAddr 
        CLEAR lowAddr-highAddr 
        CLEAR

    Category 2: Debug/Logging Line Formats:
        DUMP  log_level
        NOTE  message
        INFO  message
                                
    Category 3: Timing Control Line Formats:
        SYNC  time
        WAIT  duration
        WAIT  port type match [count]
        delay STOP

                                1.  Integers can appear in decimal or hexadecimal (using
                                    '0x' prefix) format.
                                2.  port can be any non-negative integer less then
                                    "num_ports" or it can be an asterisk to indicate any
                                    port (i.e. port number is a don't care).
                                3.  type can be read|block_read|rcw|write|block_write
                                    or it can be an asterisk to indicate any request
                                    type (i.e. request type is a don't care).
                                4.  status can be okay|nacc|address_error|data_error|
                                    address_data_error.  Normally, when a request is
                                    received that matches a response definition line the
                                    specified response status is returned for that
                                    request; however, a request that would have gotten 
                                    RSP_NACC even without a match occurring will always
                                    get an RSP_NACC (regardless of the response status
                                    specified in the response definition line).  A
                                    request that would have gotten one of the three
                                    possible error responses can be overridden by the
                                    script file to get one of the other two possible
                                    error responses or an RSP_NACC but it cannot be
                                    overridden by the script file to get an RSP_OK.  
                                5.  limit specifies how many times the specified special
                                    response status is to be given.  A limit of 0, the
                                    default, means there is no limit.
                                6.  match can be *|hit|miss.  hit means to count the
                                    request if its address is found in the list of
                                    single addresses or in the list of address ranges.
                                    miss means to count the request if its address is 
                                    not in either list.  An asterisk means to count the
                                    request regardless of whether or not its address is
                                    found in one of the lists (i.e. address matching is
                                    a don't care).
                                7.  count must be greater than 0 and defaults to 1.
                                8.  time, duration, and delay can be any non-negative
                                    integer or floating point number to mean that many
                                    clock cycles.
                                9.  You can use an asterisk in lieu of lowAddr-highAddr 
                                    to indicate all addresses in this memory's address
                                    space.
                               10.  The CLEAR commands cause the specified single address
                                    or address range to be removed from the list of
                                    single addresses or address ranges.  CLEAR by itself
                                    clears all single addresses and all address ranges.
                                    Although you can have multiple response definitions
                                    in effect at one time, given an address and a point
                                    in time in the simulation, at most one response
                                    definition can be in effect for that address.
                                    Because of this, the CLEAR command is often used
                                    after one of the timing control lines in order to
                                    remove a response definition for an address before
                                    defining a new response for that address.
                               11.  The "SYNC time" command can be used to cause a wait
                                    until the specified absolute simulation time.
                               12.  The "WAIT duration" command can be used to cause a 
                                    wait for duration clock cycles.
                               13.  The "WAIT port request match [count]" command can be
                                    used to cause a wait until count number of requests
                                    of the specified request type and with the specified
                                    match criteria have been received on the specified
                                    port.
                               14.  The "delay STOP" command will call sc_stop() after
                                    delaying delay cycles.
                               15.  The DUMP command will cause a list of all current
                                    single addresses and address ranges that have a
                                    response status defined to be dumped using the logging
                                    facility.  log_level can be INFO|NOTE.
                               16.  The NOTE and INFO commands can be used to cause
                                    the entire line to be logged at NOTE_LOG_LEVEL
                                    or INFO_LOG_LEVEL, respectively.
                               17.  Words are case insensitive.
                               18.  Comments, extra whitespace, blank lines, and lines
                                    between "#if 0" and "#endif" are ignored.  
                                    See xtsc_script_file.

                                Here are some example script file contents for specific
                                purposes (each example is assumed to be in its own
                                script file):

                // Example 1: RSP_NACC the first 3 READ requests to 0x60001708.
                0x60001708 *  read nacc 3

                // Example 2: Accept the first 2 BLOCK_READ requests in the range of
                //            0x60001a00-0x60001aff, then RSP_NACC the next 3 WRITE
                //            requests to any address.
                0x60001a00-0x60001aff * block_read okay
                wait * * hit 2
                clear
                * * write nacc 3

                // Example 3: RSP_NACC the second transfer of the first BLOCK_WRITE 2
                //            times.
                * *  block_write okay 
                wait * * hit 1
                clear
                * *  block_write nacc 2

                // Example 4: Wait for 1000 requests then RSP_NACC the next 2
                wait * * * 1000
                note 1000 requests received; the next two will get RSP_NACC
                * * * nacc 2

   Note:  The implementation of the "script_file" facility assumes that at most one
   request can occur in a single delta cycle.  If this is not the case (e.g. when
   "num_ports" is greater then 1) the behavior of the "script_file" facility may be
   different then what you desire.

   "wraparound"         bool    Specifies what will happen when the end of file (EOF) is
                                reached on "script_file".  When EOF is reached and
                                "wraparound" is true, "script_file" will be reset to the
                                beginning of file and the script will be processed
                                again.  When EOF is reached and "wraparound" is false,
                                the xtsc_memory object will cease processing the script
                                file itself but response definitions may still remain in
                                effect.
                                Default = false.

   Note:  The following 4 parameters provide another method (besides "script_file") to
          cause the memory to generate special responses (RSP_NACC, RSP_ADDRESS_ERROR,
          RSP_DATA_ERROR, and RSP_ADDRESS_DATA_ERROR) in order to test the memory
          interface master module's handling of error and nacc responses.
          
   Note:  Combining the two methods for generating special responses is not supported.
          That is, it is not legal for both "script_file" and "fail_request_mask" to be 
          non-zero.

   "fail_status"          u32   This specifies which false error response status
                                (xtsc::xtsc_response::status_t) will be generated
                                (RSP_NACC, RSP_ADDRESS_ERROR, RSP_DATA_ERROR, or 
                                RSP_ADDRESS_DATA_ERROR).
                                Default = RSP_NACC.
                                
   "fail_request_mask"    u32   Each bit of this mask determines whether a false
                                error response may be generated for a particular
                                request type (see xtsc_memory::request_type_t).
                                If the bit is set (i.e. 1), then the corresponding
                                request type is a candidate for a false error
                                response.  If the bit is clear (i.e. 0), then
                                the corresponding request type will not be given
                                any false error responses.  If all bits are 0
                                (REQ_NONE) then no false error responses will
                                be generated.  If all bits are 1 (REQ_ALL), then
                                all requests are candidates for a false error
                                response.
                                Default = 0x00000000 (i.e. REQ_NONE).

   "fail_percentage"      u32   This parameter specifies the probability of
                                a false error response being generated when
                                a request is received whose corresponding
                                bit in "fail_request_mask" is set.  A value 
                                of 100 causes all requests whose corresponding
                                bit in "fail_request_mask" is set to receive
                                a false error response (i.e. 100 percent
                                receive a false error response).  A value of 
                                1 will result in a false error response being
                                sent approximately 1% of the time.  Valid 
                                values are 1 to 100.
                                Default = 100.

   "fail_seed"            u32   This parameter is used to seed the psuedo-
                                random number generator using the standard
                                library srand() method.

    \endverbatim
 *
 * @see xtsc_memory
 * @see xtsc::xtsc_parms
 */
class XTSC_COMP_API xtsc_memory_parms : public xtsc::xtsc_parms {
public:

  /**
   * Constructor for an xtsc_memory_parms object. After the object is constructed, the
   * data members can be directly written using the appropriate xtsc_parms::set() method
   * in cases where non-default values are desired.
   *
   * @param width8              Memory data interface width in bytes.
   *
   * @param delay               Default delay for read and write in terms of this
   *                            memory's clock period (see "clock_period").  Local
   *                            memory devices should use a delay of 0 for a 5-stage
   *                            pipeline and a delay of 1 for a 7-stage pipeline.  PIF
   *                            memory devices should use a delay of 1 or more.
   *
   * @param start_address8      The starting byte address of this memory.
   *
   * @param size8               The byte size of this memory.  0 means the memory
   *                            occupies all of the 4GB address space at and above
   *                            start_address8.
   *
   * @param num_ports           The number of ports this memory has.
   *
   */
  xtsc_memory_parms(xtsc::u32   width8            = 4,
                    xtsc::u32   delay             = 0,
                    xtsc::u32   start_address8    = 0,
                    xtsc::u32   size8             = 0,
                    xtsc::u32   num_ports         = 1)
  {
    init(width8, delay, start_address8, size8, num_ports);
  }


  /**
   * Constructor for an xtsc_memory_parms object based upon an xtsc_core object and a
   * named memory interface. 
   *
   * This constructor will determine width8, delay, start_address8, size8, and,
   * optionally, num_ports by querying the core object and then pass the values to the
   * init() method.  If port_name is a ROM interface, then "read_only" will be be set
   * to true.  In addition, the "clock_period" parameter will be set to match the
   * core's clock period.  For PIF memories, start_address8 and size8 will both be 0
   * indicating a memory which spans the entire 4 gigabyte address space and
   * "check_alignment" will be set to true.  If desired, after the xtsc_memory_parms
   * object is constructed, its data members can be changed using the appropriate
   * xtsc_parms::set() method before passing it to the xtsc_memory constructor.
   *
   * @param     core            A reference to the xtsc_core object upon which to base
   *                            the xtsc_memory_parms.
   *
   * @param     port_name       The memory port name (the name of the memory interface).
   *                            Note:  The core configuration must have the named memory
   *                            interface.
   *
   * @param     delay           Default delay for PIF read and write in terms of this
   *                            memory's clock period (see "clock_period").  PIF memory
   *                            devices should use a delay of 1 or more.  A value of
   *                            0xFFFFFFFF (the default) means to use a delay of 1 if
   *                            the core has a 5-stage pipeline and a delay of 2 if the
   *                            core has a 7-stage pipeline.  This parameter is ignored
   *                            except for PIF memory interfaces.
   *
   * @param     num_ports       The number of ports this memory has.  If 0, the default,
   *                            the number of ports (1 or 2) will be inferred thusly: If 
   *                            port_name is a LD/ST unit 0 port of a dual-ported core
   *                            interface, and the core is dual-ported and has no CBox,
   *                            and if the 2nd port of the core has not been bound, then
   *                            "num_ports" will be 2; otherwise, "num_ports" will be 1.
   *
   * @see xtsc::xtsc_core::How_to_do_memory_port_binding for a list of legal port_name
   *      values.
   */
  xtsc_memory_parms(const xtsc::xtsc_core& core, const char *port_name, xtsc::u32 delay = 0xFFFFFFFF, xtsc::u32 num_ports = 0);


  /**
   * Do initialization common to both constructors.
   */
  void init(xtsc::u32   width8            = 4,
            xtsc::u32   delay             = 0,
            xtsc::u32   start_address8    = 0,
            xtsc::u32   size8             = 0,
            xtsc::u32   num_ports         = 1)
  {
    std::vector<xtsc::u32> deny_fast_access;
    add("byte_width",           width8);
    add("start_byte_address",   start_address8);
    add("memory_byte_size",     size8);
    add("page_byte_size",       1024*16);
    add("initial_value_file",   (char*)NULL);
    add("memory_fill_byte",     0);
    add("num_ports",            num_ports);
    add("delay_from_receipt",   true);
    add("read_delay",           delay);
    add("block_read_delay",     delay);
    add("block_read_repeat",    1);
    add("burst_read_delay",     delay);
    add("burst_read_repeat",    1);
    add("rcw_repeat",           1);
    add("rcw_response",         delay);
    add("write_delay",          delay);
    add("block_write_delay",    delay);
    add("block_write_repeat",   delay);
    add("block_write_response", delay);
    add("burst_write_delay",    delay);
    add("burst_write_repeat",   delay);
    add("burst_write_response", delay);
    add("request_fifo_depth",   2);
    add("check_alignment",      false);
    add("read_only",            false);
    add("immediate_timing",     false);
    add("response_repeat",      1);
    add("recovery_time",        1);
    add("clock_period",         0xFFFFFFFF);
    add("use_fast_access",      true);
    add("deny_fast_access",     deny_fast_access);
    add("use_raw_access",       true);
    add("script_file",          (char*)NULL);
    add("wraparound",           false);
    add("fail_status",          xtsc::xtsc_response::RSP_NACC);
    add("fail_request_mask",    0x00000000);
    add("fail_percentage",      100);
    add("fail_seed",            1);
  }

  /// Return what kind of xtsc_parms this is (our C++ type)
  virtual const char* kind() const { return "xtsc_memory_parms"; }
};




/**
 * A PIF, XLMI, or local memory.
 *
 * Example XTSC module implementing a configurable memory.  
 *
 * On a given port, this memory model always processes transactions in the order they
 * were received.
 *
 * You may use this memory directly or just use the code as a starting place for
 * developing your own memory models.  In some cases, this class can be sub-classed for
 * special functionality.
 *
 * Here is a block diagram of an xtsc_memory as it is used in the hello_world example:
 * @image html  Example_hello_world.jpg
 * @image latex Example_hello_world.eps "hello_world Example" width=10cm
 *
 * @Note The xtsc_memory module does not ensure RCW transactions are atomic.  Ensuring
 * RCW transactions are atomic is the responsibility of upstream modules.
 *
 * @see xtsc_memory_parms
 * @see xtsc_memory_base
 * @see xtsc::xtsc_request_if
 * @see xtsc::xtsc_respond_if
 * @see xtsc::xtsc_core
 * @see xtsc::xtsc_cohctrl
 * @see xtsc_arbiter
 * @see xtsc_dma_engine
 * @see xtsc_router
 * @see xtsc_master
 */
class XTSC_COMP_API xtsc_memory : public sc_core::sc_module, public xtsc::xtsc_resettable {
public:


  /// From the memory interface masters (e.g xtsc_core, xtsc_router, etc) to us 
  sc_core::sc_export<xtsc::xtsc_request_if>   **m_request_exports;

  /// From us to the memory interface masters (e.g xtsc_core, xtsc_router, etc)
  sc_core::sc_port<xtsc::xtsc_respond_if>     **m_respond_ports;


  // SystemC needs this
  SC_HAS_PROCESS(xtsc_memory);


  /// Our C++ type (SystemC uses this)
  virtual const char* kind() const { return "xtsc_memory"; }


  /**
   * Constructor for an xtsc_memory.
   *
   * @param     module_name     Name of the xtsc_memory sc_module.
   *
   * @param     memory_parms    The remaining parameters for construction.
   *
   * @see xtsc_memory_parms
   */
  xtsc_memory(sc_core::sc_module_name module_name, const xtsc_memory_parms& memory_parms);


  /// The destructor.
  virtual ~xtsc_memory(void);


  /// Get the TextLogger for this component (e.g. to adjust its log level)
  log4xtensa::TextLogger& get_text_logger() { return m_text; }


  /// Get the BinaryLogger for this component (e.g. to adjust its log level)
  log4xtensa::BinaryLogger& get_binary_logger() { return m_binary; }


  /**
   * Non-hardware reads (for example, reads by the debugger).
   * @see xtsc::xtsc_request_if::nb_peek
   */
  void peek(xtsc::xtsc_address address8, xtsc::u32 size8, xtsc::u8 *buffer) { m_p_memory->peek(address8, size8, buffer); }


  /**
   * Non-hardware writes (for example, writes from the debugger).
   * @see xtsc::xtsc_request_if::nb_poke
   */
  void poke(xtsc::xtsc_address address8, xtsc::u32 size8, const xtsc::u8 *buffer) { m_p_memory->poke(address8, size8, buffer); }


  /**
   * This method dumps the specified number of bytes from the memory.  Each
   * line of output is divided into three columnar sections, each of which is
   * optional.  The first section contains an address.  The second section contains
   * a hex dump of some (possibly all) of the data (two hex nibbles and a space for
   * each byte from the memory).  The third section contains an ASCII dump of the 
   * same data.
   *
   * @param       address8                The starting byte address in memory.
   *                                      
   * @param       size8                   The number of bytes of data to dump.
   *
   * @param       os                      The ostream object to which the data is to be
   *                                      dumped.
   *
   * @param       left_to_right           If true, the data is dumped in the order:
   *                                      memory[0], memory[1], ..., memory[bytes_per_line-1].
   *                                      If false, the data is dumped in the order:
   *                                      memory[bytes_per_line-1], memory[bytes_per_line-2], ..., memory[0].
   *
   * @param       bytes_per_line          The number of bytes to dump on each line of output.
   *                                      If bytes_per_line is 0 then all size8 bytes are dumped 
   *                                      on a single line with no newline at the end.  If 
   *                                      bytes_per_line is non-zero, then all lines of output
   *                                      end in newline.
   *
   * @param       show_address            If true, the first columnar section contains an 
   *                                      address printed as an 8-hex-digit number with a 0x 
   *                                      prefix.  If false, the first columnar section is null
   *                                      and takes no space in the output.
   *
   * @param       show_hex_values         If true, the second (middle) columnar section of 
   *                                      hex data values is printed.  If false, the second
   *                                      columnar section is null and takes no space in the
   *                                      output.
   *
   * @param       do_column_heading       If true, print byte position column headings over 
   *                                      the hex values section.  If false, no column headings
   *                                      are printed.  If show_hex_values is false, then the
   *                                      do_column_heading value is ignored and no column
   *                                      headings are printed.
   *
   * @param       show_ascii_values       If true, the third (last) columnar section of ASCII
   *                                      data values is printed (if an ASCII value is a
   *                                      non-printable character a period is printed).  If 
   *                                      show_ascii_values is false, the third columnar
   *                                      section is null and takes no space in the output.
   *
   * @param       adjust_address          If adjust_address is true and address8 modulo 
   *                                      bytes_per_line is not 0, then offset the
   *                                      printed values on the first line of the hex and 
   *                                      ASCII columnar sections and adjust the printed 
   *                                      address so that the printed address modulo 
   *                                      bytes_per_line is always zero.  Otherwize, do not
   *                                      offset the first printed data values and do not
   *                                      adjust the printed address.
   */
  void byte_dump(xtsc::xtsc_address     address8,
                 xtsc::u32              size8,
                 std::ostream&          os                      = std::cout,
                 bool                   left_to_right           = true,
                 xtsc::u32              bytes_per_line          = 16,
                 bool                   show_address            = true,
                 bool                   show_hex_values         = true,
                 bool                   do_column_heading       = true,
                 bool                   show_ascii_values       = true,
                 bool                   adjust_address          = true)
  {
    m_p_memory->byte_dump(address8, size8, os, left_to_right, bytes_per_line, show_address, show_hex_values, do_column_heading,
                           show_ascii_values, adjust_address);
  }


  /**
   * Reset the memory.
   */
  void reset(bool hard_reset = false);


  /**
   * Connect an xtsc_arbiter with this xtsc_memory.
   *
   * This method connects the master port pair of the specified xtsc_arbiter with the
   * specified slave port pair of this xtsc_memory.
   *
   * @param     arbiter         The xtsc_arbiter to connect with this xtsc_memory.
   *
   * @param     mem_port        The slave port pair of this memory to connect the
   *                            xtsc_arbiter with.
   */
  void connect(xtsc_arbiter& arbiter, xtsc::u32 mem_port = 0);


  /**
   * Connect an xtsc_cohctrl with this xtsc_memory.
   *
   * This method connects the memory interface master port pair of the specified
   * xtsc_cohctrl with the specified slave port pair of this xtsc_memory.
   *
   * @param     cohctrl         The xtsc_cohctrl to connect with this xtsc_memory.
   *
   * @param     mem_port        The slave port pair of this memory to connect the
   *                            xtsc_cohctrl with.
   */
  void connect(xtsc::xtsc_cohctrl& cohctrl, xtsc::u32 mem_port = 0);


  /**
   * Connect an xtsc_core with this xtsc_memory.
   *
   * This method connects the specified memory interface master port pair of the
   * specified xtsc_core with the specified slave port pair of this xtsc_memory.
   *
   * @param     core                    The xtsc::xtsc_core to connect with this
   *                                    xtsc_memory.
   *
   * @param     memory_port_name        The name of the memory interface master port
   *                                    pair of the xtsc_core to connect with this
   *                                    xtsc_memory.
   *
   * @param     mem_port                The slave port pair of this xtsc_memory to
   *                                    connect the xtsc_core with.
   *
   * @param     single_connect          If true only one slave port pair of this memory
   *                                    will be connected.  If false, the default, and
   *                                    if memory_port_name names a LD/ST unit 0
   *                                    dual-ported interface of core and if mem_port+1
   *                                    exists and has not yet been connected, then both
   *                                    master port pairs of memory_port_name of core
   *                                    will be connected to the slave port pairs of
   *                                    this xtsc_memory numbered mem_port and
   *                                    mem_port+1.
   *
   * @returns number of ports that were connected by this call (1 or 2)
   */
  xtsc::u32 connect(xtsc::xtsc_core& core, const char *memory_port_name, xtsc::u32 mem_port = 0, bool single_connect = false);


  /**
   * Connect an xtsc_dma_engine with this xtsc_memory.
   *
   * This method connects the master port pair of the specified xtsc_dma_engine with the
   * specified slave port pair of this xtsc_memory.
   *
   * @param     dma             The xtsc_dma_engine to connect with this xtsc_memory.
   *
   * @param     mem_port        The slave port pair of this memory to connect the
   *                            xtsc_dma_engine with.
   */
  void connect(xtsc_dma_engine& dma, xtsc::u32 mem_port = 0);


  /**
   * Connect an xtsc_master with this xtsc_memory.
   *
   * This method connects the master port pair of the specified xtsc_master with the
   * specified slave port pair of this xtsc_memory.
   *
   * @param     master          The xtsc_master to connect with this xtsc_memory.
   *
   * @param     mem_port        The slave port pair of this memory to connect the
   *                            xtsc_master with.
   */
  void connect(xtsc_master& master, xtsc::u32 mem_port = 0);


  /**
   * Connect an xtsc_memory_trace with this xtsc_memory.
   *
   * This method connects the specified master port pair of the upstream
   * xtsc_memory_trace with the specified slave port pair of this xtsc_memory.
   *
   * @param     memory_trace    The xtsc_memory_trace to connect with this xtsc_memory.
   *
   * @param     trace_port      The master port pair of the xtsc_memory_trace to connect
   *                            with this xtsc_memory.
   *
   * @param     mem_port        The slave port pair of this memory to connect the
   *                            xtsc_memory_trace with.
   *
   * @param     single_connect  If true only one slave port pair of this memory will be
   *                            connected.  If false, the default, then all contiguous,
   *                            unconnected slave port pairs of this memory starting at
   *                            mem_port that have a corresponding existing master port
   *                            pair in memory_trace (starting at trace_port) will be
   *                            connected with that corresponding memory_trace master
   *                            port pair.
   *
   * @returns number of ports that were connected by this call (1 or more)
   */
  xtsc::u32 connect(xtsc_memory_trace& memory_trace, xtsc::u32 trace_port = 0, xtsc::u32 mem_port = 0, bool single_connect = false);


  /**
   * Connect an xtsc_pin2tlm_memory_transactor with this xtsc_memory.
   *
   * This method connects the specified TLM master port pair of the specified
   * xtsc_pin2tlm_memory_transactor with the specified slave port pair of this
   * xtsc_memory.
   *
   * @param     pin2tlm         The xtsc_pin2tlm_memory_transactor to connect with this
   *                            xtsc_memory.
   *
   * @param     tran_port       The xtsc_pin2tlm_memory_transactor TLM master port pair
   *                            to connect with this xtsc_memory.
   *
   * @param     mem_port        The slave port pair of this xtsc_memory to connect the
   *                            xtsc_pin2tlm_memory_transactor with.
   *
   * @param     single_connect  If true only one slave port pair of this xtsc_memory
   *                            will be connected.  If false, the default, then all
   *                            contiguous, unconnected slave port pairs of this
   *                            xtsc_memory starting at mem_port that have a
   *                            corresponding existing TLM master port pair in pin2tlm
   *                            (starting at tran_port) will be connected with that
   *                            corresponding pin2tlm master port pair.
   */
  xtsc::u32 connect(xtsc_pin2tlm_memory_transactor&     pin2tlm,
                    xtsc::u32                           tran_port       = 0,
                    xtsc::u32                           mem_port        = 0,
                    bool                                single_connect  = false);


  /**
   * Connect an xtsc_router with this xtsc_memory.
   *
   * This method connects the specified master port pair of the specified xtsc_router
   * with the specified slave port pair of this xtsc_memory.
   *
   * @param     router          The xtsc_router to connect with this xtsc_memory.
   *
   * @param     router_port     The xtsc_router master port pair to connect with this
   *                            xtsc_memory.
   *
   * @param     mem_port        The slave port pair of this xtsc_memory to connect the
   *                            xtsc_router with.
   */
  void connect(xtsc_router& router, xtsc::u32 router_port, xtsc::u32 mem_port = 0);


  // This enum gives the exact request type (especially to distinguish the different
  // block write requests and to distinguish the different rcw requests).
  typedef enum request_type_t {
    REQ_READ            = 1 << 0,         ///< 0x00000001 = Single read
    REQ_WRITE           = 1 << 1,         ///< 0x00000002 = Write
    REQ_BLOCK_READ      = 1 << 2,         ///< 0x00000004 = Block read
    REQ_RCW_1           = 1 << 3,         ///< 0x00000008 = Read-conditional-write request #1
    REQ_RCW_2           = 1 << 4,         ///< 0x00000010 = Read-conditional-write request #2
    REQ_BURST_READ      = 1 << 5,         ///< 0x00000020 = Burst read
    REQ_BURST_WRITE_1   = 1 << 8,         ///< 0x00000100 = Burst write request #1
    REQ_BURST_WRITE_2   = 1 << 9,         ///< 0x00000200 = Burst write request #2
    REQ_BURST_WRITE_3   = 1 << 10,        ///< 0x00000400 = Burst write request #3
    REQ_BURST_WRITE_4   = 1 << 11,        ///< 0x00000800 = Burst write request #4
    REQ_BURST_WRITE_5   = 1 << 12,        ///< 0x00001000 = Burst write request #5
    REQ_BURST_WRITE_6   = 1 << 13,        ///< 0x00002000 = Burst write request #6
    REQ_BURST_WRITE_7   = 1 << 14,        ///< 0x00004000 = Burst write request #7
    REQ_BURST_WRITE_8   = 1 << 15,        ///< 0x00008000 = Burst write request #8
    REQ_BLOCK_WRITE_1   = 1 << 16,        ///< 0x00010000 = Block write request #1
    REQ_BLOCK_WRITE_2   = 1 << 17,        ///< 0x00020000 = Block write request #2
    REQ_BLOCK_WRITE_3   = 1 << 18,        ///< 0x00040000 = Block write request #3
    REQ_BLOCK_WRITE_4   = 1 << 19,        ///< 0x00080000 = Block write request #4
    REQ_BLOCK_WRITE_5   = 1 << 20,        ///< 0x00100000 = Block write request #5
    REQ_BLOCK_WRITE_6   = 1 << 21,        ///< 0x00200000 = Block write request #6
    REQ_BLOCK_WRITE_7   = 1 << 22,        ///< 0x00400000 = Block write request #7
    REQ_BLOCK_WRITE_8   = 1 << 23,        ///< 0x00800000 = Block write request #8
    REQ_BLOCK_WRITE_9   = 1 << 24,        ///< 0x01000000 = Block write request #9
    REQ_BLOCK_WRITE_10  = 1 << 25,        ///< 0x02000000 = Block write request #10
    REQ_BLOCK_WRITE_11  = 1 << 26,        ///< 0x04000000 = Block write request #11
    REQ_BLOCK_WRITE_12  = 1 << 27,        ///< 0x08000000 = Block write request #12
    REQ_BLOCK_WRITE_13  = 1 << 28,        ///< 0x10000000 = Block write request #13
    REQ_BLOCK_WRITE_14  = 1 << 29,        ///< 0x20000000 = Block write request #14
    REQ_BLOCK_WRITE_15  = 1 << 30,        ///< 0x40000000 = Block write request #15
    REQ_BLOCK_WRITE_16  = 1 << 31,        ///< 0x80000000 = Block write request #16
    REQ_ALL                 = 0xFFFFFFFF, ///< 0xFFFFFFFF = All request types
    REQ_NONE                = 0           ///< 0x00000000 = No request types
  } request_type_t;


  /**
   * This method can be used to control the sending of false error responses (for example, to
   * test the upstream memory interface master device's handling of them).
   *
   * @param     status          see "fail_status" in xtsc_memory_parms.
   *
   * @param     request_mask    see "fail_request_mask" in xtsc_memory_parms.
   *
   * @param     fail_percentage see "fail_percentage" in xtsc_memory_parms.
   */
  void setup_false_error_responses(xtsc::xtsc_response::status_t status, xtsc::u32 request_mask, xtsc::u32 fail_percentage);


  /// Clear all addresses and address ranges that are to receive special responses. 
  void clear_addresses();


  /// Dump all addresses and address ranges that are to receive special responses. 
  void dump_addresses(std::ostream& os);


  /**
   * Determine if the specified request should get a special response and also compute type.
   *
   * @param     request         The xtsc_request object.
   *
   * @param     port_num        The slave port number the request came in on.
   *
   * @param     status          Reference in which to return the response status based
   *                            on the "script_file".  Even if this is RSP_OK, the
   *                            request might still get an error response due to a real
   *                            error.
   *
   * @param     type            Reference in which to return the request type:  
   *                            0=READ, 1=BLOCK_READ, 2=RCW, 3=WRITE, 4=BLOCK_WRITE
   *
   * @return true if this request matches up with one of the address lists.
   */
  bool compute_special_response(const xtsc::xtsc_request&       request,
                                xtsc::u32                       port_num,
                                xtsc::xtsc_response::status_t&  status,
                                xtsc::u32&                      type);


  /// Extract and return the request type code from the word at m_words[index]
  xtsc::u32 get_request_type_code(xtsc::u32 index);


  /// POD class to help keep track of information related to a special address or address range.
  class address_info {
  public:

    typedef xtsc::xtsc_response::status_t       status_t;
    typedef xtsc::xtsc_address                  xtsc_address;
    typedef xtsc::u32                           u32;

    address_info(xtsc_address   low_address,
                 xtsc_address   high_address,
                 bool           is_range,
                 u32            port_num,
                 u32            num_ports,
                 u32            type,
                 status_t       status,
                 u32            limit) :
      m_low_address     (low_address),
      m_high_address    (high_address),
      m_is_range        (is_range),
      m_port_num        (port_num),
      m_num_ports       (num_ports),
      m_type            (type),
      m_status          (status),
      m_limit           (limit),
      m_count           (0),
      m_finished        (false)
    { }

    void dump(std::ostream& os = std::cout) const;

    /// Increments m_count, adjusts m_finished if required, returns m_finished
    bool used();

    xtsc_address        m_low_address;  ///<  Single address or low address of address range.
    xtsc_address        m_high_address; ///<  High address of address range.
    bool                m_is_range;     ///<  True if this is an address range.
    u32                 m_port_num;     ///<  0-m_num_ports:  If m_port_num==m_num_ports it means port number is don't care.
    u32                 m_num_ports;    ///<  Number of ports that the memory has
    u32                 m_type;         ///<  0-6:  where 0=READ, 1=BLOCK_READ, 2=RCW, 3=WRITE, 4=BLOCK_WRITE, 5=don't care
    status_t            m_status;       ///<  The response status to be given.
    u32                 m_limit;        ///<  How many times this address/range should get the specified response (0=no limit).
    u32                 m_count;        ///<  How many times this address/range has gotten the specified response.
    bool                m_finished;     ///<  True when m_limit has been reached.
  };



protected:


  /// Implementation of xtsc_request_if.
  class xtsc_request_if_impl : public xtsc::xtsc_request_if, public sc_core::sc_object {
  public:

    /**
     * Constructor.
     * @param   memory      A reference to the owning xtsc_memory object.
     * @param   port_num    The slave port number that this object serves.
     */
    xtsc_request_if_impl(const char *object_name, xtsc_memory& memory, xtsc::u32 port_num) :
      sc_object         (object_name),
      m_memory          (memory),
      m_p_port          (0),
      m_port_num        (port_num)
    {}

    /// @see xtsc::xtsc_request_if
    virtual void nb_peek(xtsc::xtsc_address address8, xtsc::u32 size8, xtsc::u8 *buffer);

    /// @see xtsc::xtsc_request_if
    virtual void nb_poke(xtsc::xtsc_address address8, xtsc::u32 size8, const xtsc::u8 *buffer);

    /// @see xtsc::xtsc_request_if
    virtual bool nb_fast_access(xtsc::xtsc_fast_access_request  &request);

    /// @see xtsc::xtsc_request_if
    void nb_request(const xtsc::xtsc_request& request);

    /// @see xtsc::xtsc_request_if
    void nb_load_retired(xtsc::xtsc_address address8);

    /// @see xtsc::xtsc_request_if
    void nb_retire_flush();

    /// @see xtsc::xtsc_request_if
    void nb_lock(bool lock);

    /// Return true if a port has bound to this implementation
    bool is_connected() { return (m_p_port != 0); }


  protected:

    /// SystemC callback when something binds to us
    virtual void register_port(sc_core::sc_port_base& port, const char *if_typename);

    xtsc_memory&                m_memory;       ///< Our xtsc_memory object
    sc_core::sc_port_base      *m_p_port;       ///< Port that is bound to us
    xtsc::u32                   m_port_num;     ///< Our slave port pair number
  };



  /// Information about each request
  class request_info {
  public:
    /// Constructor
    request_info(const xtsc::xtsc_request& request, xtsc::xtsc_response::status_t status) {
      init(request, status);
    }
    void init(const xtsc::xtsc_request& request, xtsc::xtsc_response::status_t status) {
      m_request         = request;
      m_time_stamp      = sc_core::sc_time_stamp();
      m_status          = status;
    }
    xtsc::xtsc_request                  m_request;      ///< Our copy of the request
    sc_core::sc_time                    m_time_stamp;   ///< Timestamp when received
    xtsc::xtsc_response::status_t       m_status;       ///< Response status from "script_file"
  };


  /// Process optional "script_file"
  void script_thread();


  /// Translate fail_percentage into terms of RAND_MAX
  void compute_let_through();


  /// Return the exact request type of this request
  request_type_t get_request_type(const xtsc::xtsc_request& request, xtsc::u32 port_num);


  /// Return true if an RSP_NACC should be sent for testing purposes
  bool do_nacc_failure(const xtsc::xtsc_request& request, xtsc::u32 port_num);


  /// Return response status for testing purposes
  xtsc::xtsc_response::status_t get_status_for_testing_failures(request_info *p_request_info, xtsc::u32 port_num);


  /// Thread to handle transactions at the correct time.
  virtual void worker_thread(void);


  /// Method to handle the current (active) request
  virtual void do_active_request(xtsc::u32 port_num);


  /// Helper method to handle xtsc::xtsc_request::READ.
  virtual void do_read(xtsc::u32 port_num);


  /// Helper method to handle xtsc::xtsc_request::BLOCK_READ.
  virtual void do_block_read(xtsc::u32 port_num);


  /// Helper method to handle BURST_READ.
  void do_burst_read(xtsc::u32 port_num);


  /// Helper method to handle xtsc::xtsc_request::RCW.
  virtual void do_rcw(xtsc::u32 port_num);


  /// Helper method to handle xtsc::xtsc_request::WRITE.
  virtual void do_write(xtsc::u32 port_num);


  /// Helper method to handle xtsc::xtsc_request::BLOCK_WRITE.
  virtual void do_block_write(xtsc::u32 port_num);


  /// Helper method to handle BURST_WRITE.
  void do_burst_write(xtsc::u32 port_num);


  /// Helper method to binary log response and then send the response until it is accepted
  void send_response(xtsc::u32 port_num, bool log_data_binary);


  /// Get a new request_info (from the pool)
  request_info *new_request_info(const xtsc::xtsc_request& request, xtsc::xtsc_response::status_t status);


  /// Delete an request_info (return it to the pool)
  void delete_request_info(request_info*& p_request_info);


  /// Get the object to use for fast access implemented through CALLBACKS
  xtsc::xtsc_fast_access_if *get_fast_access_object() const;


  /// Get the next vector of words from the script file
  int get_words();


  /// Extract a u32 value (named argument_name) from the word at m_words[index]
  xtsc::u32 get_u32(xtsc::u32 index, const std::string& argument_name);


  /// Extract a double value (named argument_name) from the word at m_words[index]
  double get_double(xtsc::u32 index, const std::string& argument_name);


  /**
   * Method to convert an address or address range string into a pair of numeric
   * addresses.  An address range must be specified without any spaces.  For example,
   * 0x80000000-0x8FFFFFFF.
   *
   * @param index               The index of the string in m_words[].
   *
   * @param argument_name       The name of the argument being converted.  This name is
   *                            from the "script_file" line format documentation.
   *
   * @param low_address         The converted low address.
   *
   * @param high_address        If the string is an address range then this is the
   *                            converted high address.  Otherwise, this is equal to
   *                            low_address.
   *
   * @returns true if the string is an address range.
   */
  bool get_addresses(xtsc::u32           index,
                     const std::string&  argument_name,
                     xtsc::xtsc_address& low_address,
                     xtsc::xtsc_address& high_address);


  /// Helper function to load values from "initial_value_file"
  void load_initial_values() {
    m_p_memory->load_initial_values();
  }


  /// Get the page of memory containing address8 (allocate as needed). 
  xtsc::u32 get_page(xtsc::xtsc_address address8) {
    return m_p_memory->get_page(address8);
  }


  /// Get the page of storage corresponding to the specified address
  xtsc::u32 get_page_id(xtsc::xtsc_address address8) const {
    return m_p_memory->get_page_id(address8);
  }


  /// Get the offset into the page of storage corresponding to the specified address
  xtsc::u32 get_page_offset(xtsc::xtsc_address address8) const {
    return m_p_memory->get_page_offset(address8);
  }


  /// Helper method to read a u8 value (allocate as needed).
  virtual xtsc::u8 read_u8(xtsc::xtsc_address address8) {
    return m_p_memory->read_u8(address8);
  }


  /// Helper method to write a u8 value (allocate as needed).
  virtual void write_u8(xtsc::xtsc_address address8, xtsc::u8 value) {
    m_p_memory->write_u8(address8, value);
  }


  /// Helper method to read a u32 value (allocate as needed).
  virtual xtsc::u32 read_u32(xtsc::xtsc_address address8, bool big_endian = false) {
    return m_p_memory->read_u32(address8, big_endian);
  }


  /// Helper method to write a u32 value (allocate as needed).
  virtual void write_u32(xtsc::xtsc_address address8, xtsc::u32 value, bool big_endian = false) {
    m_p_memory->write_u32(address8, value, false);
  }



  xtsc::u32                             m_num_ports;                    ///< The number of ports this memory has
  xtsc::u32                             m_next_port_num;                ///< Used by worker_thread entry to get its slave port number

  xtsc_request_if_impl                **m_request_impl;                 ///< The m_request_export objects bind to these
  xtsc_memory_base                     *m_p_memory;                     ///< The memory itself

  // Buffer requests from the memory interface master
  sc_core::sc_fifo<request_info*>     **m_request_fifo;                 ///< The fifos for incoming requests on each port

  request_info                        **m_p_active_request_info;        ///< The active (current) request on each port
  xtsc::xtsc_response                 **m_p_active_response;            ///< The active (current) response on each port
  xtsc::u32                            *m_block_write_transfer_count;   ///< Keep track of block writes on each port
  xtsc::u32                            *m_burst_write_transfer_count;   ///< Keep track of burst writes on each port
  bool                                 *m_first_block_write;            ///< True if first block write request on each port
  bool                                 *m_first_burst_write;            ///< True if first burst write request on each port
  bool                                 *m_first_rcw;                    ///< True if first RCW request on each port
  sc_core::sc_time                     *m_last_action_time_stamp;       ///< Time of last action on each port: recovery time starts from here

  sc_core::sc_event                    *m_worker_thread_event;          ///< To notify worker_thread of a request on each port

  bool                                 *m_rcw_have_first_transfer;      ///< True if first RCW has been received but not second
  xtsc::u8                             *m_rcw_compare_data;             ///< Comparison data from RCW request

  sc_core::sc_time                      m_clock_period;                 ///< The clock period of this memory

  bool                                  m_immediate_timing;             ///< True if requests should be handled without any delay
  bool                                  m_delay_from_receipt;           ///< True if delay timing starts from receipt of request
  bool                                  m_check_alignment;              ///< If true, check that address is size aligned

  sc_core::sc_time                      m_recovery_time;                ///< See "recovery_time" parameter
  sc_core::sc_time                      m_read_delay;                   ///< See "read_delay" parameter
  sc_core::sc_time                      m_block_read_delay;             ///< See "block_read_delay" parameter
  sc_core::sc_time                      m_block_read_repeat;            ///< See "block_read_repeat" parameter
  sc_core::sc_time                      m_burst_read_delay;             ///< See "burst_read_delay" parameter
  sc_core::sc_time                      m_burst_read_repeat;            ///< See "burst_read_repeat" parameter
  sc_core::sc_time                      m_rcw_repeat;                   ///< See "rcw_repeat" parameter
  sc_core::sc_time                      m_rcw_response;                 ///< See "rcw_response" parameter
  sc_core::sc_time                      m_write_delay;                  ///< See "write_delay" parameter
  sc_core::sc_time                      m_block_write_delay;            ///< See "block_write_delay" parameter
  sc_core::sc_time                      m_block_write_repeat;           ///< See "block_write_repeat" parameter
  sc_core::sc_time                      m_block_write_response;         ///< See "block_write_response" parameter
  sc_core::sc_time                      m_burst_write_delay;            ///< See "burst_write_delay" parameter
  sc_core::sc_time                      m_burst_write_repeat;           ///< See "burst_write_repeat" parameter
  sc_core::sc_time                      m_burst_write_response;         ///< See "burst_write_response" parameter
  sc_core::sc_time                      m_response_repeat;              ///< See "response_repeat" parameter

  std::string                           m_script_file;                  ///< The name of the optional script file
  bool                                  m_wraparound;                   ///< Should script file wraparound at EOF

  sc_core::sc_event                     m_script_thread_event;          ///< To notify script_thread of a request 
  xtsc::xtsc_script_file               *m_p_script_stream;              ///< Pointer to the optional script file object
  std::string                           m_line;                         ///< The current script file line
  xtsc::u32                             m_line_count;                   ///< The current script file line number
  std::vector<std::string>              m_words;                        ///< Tokenized words from m_line

  xtsc::u32                             m_prev_type;                    ///< type code of most recent previous request
  bool                                  m_prev_hit;                     ///< true if request address was in one of the address lists
  xtsc::u32                             m_prev_port;                    ///< port number of most recent previous request

  xtsc::xtsc_response::status_t         m_fail_status;                  ///< See "fail_status" parameter
  xtsc::u32                             m_fail_request_mask;            ///< See "fail_request_mask" parameter
  xtsc::u32                             m_fail_percentage;              ///< See "fail_percentage" parameter
  xtsc::u32                             m_fail_seed;                    ///< See "fail_seed" parameter
  xtsc::i32                             m_let_through;                  ///< Gate when "fail_request_mask" is non-zero
  bool                                  m_read_only;                    ///< See "read_only" parameter

  std::vector<request_info*>            m_request_pool;                 ///< Pool of requests

  bool                                  m_use_fast_access;              ///< For turboxim
  std::vector<xtsc::u32>                m_deny_fast_access;             ///< For turboxim
  bool                                  m_use_raw_access;               ///< For turboxim
  xtsc::xtsc_fast_access_if            *m_fast_access_object;           ///< Object for fast access through CALLBACKS

  std::map<xtsc::xtsc_address, address_info*> m_address_map;            ///< Map of single addresses
  std::map<xtsc::xtsc_address, address_info*> m_address_range_map;      ///< Map of address ranges

  xtsc::xtsc_address                    m_start_address8;               ///< The starting byte address of this memory
  xtsc::u32                             m_size8;                        ///< The byte size of this memory
  xtsc::u32                             m_width8;                       ///< The byte width of this memories data interface
  xtsc::xtsc_address                    m_end_address8;                 ///< The ending byte address of this memory

  log4xtensa::TextLogger&               m_text;                         ///< Text logger
  log4xtensa::BinaryLogger&             m_binary;                       ///< Binary logger

};



XTSC_COMP_API std::ostream& operator<<(std::ostream& os, const xtsc_memory::address_info& info);



}  // namespace xtsc_component


#endif  // _XTSC_MEMORY_H_
