;#  This file gets processed by Xtensa/Software/src/system/Makefile.src,
;#  but is placed here for maintenance along with other OCD files.
<!-- topology.xml  -  JTAG chain topology description for the Xtensa OCD Daemon
; use Stash;
; stash_obj_isa($sys, "MultiCoreSystem") or die "system must be of type MultiCoreSystem";
; my @cores = @{$sys->cores};
; my $year = 1900 + (localtime)[5];
     Copyright (c) 2006-`$year` Tensilica Inc.

     NOTE: This file was automatically generated for subsystem "`$sys->name`" (` scalar @cores` cores).
  -->

<configuration>
  <controller id='Controller0' module='macraigor' cable='usb2demon' speed='4' port='0' />
  <!--
	Here are other example controller lines, that can replace the above ones
	when using different JTAG probes (scan controllers):

	Macraigor USB2Demon or USBwiggler:
	<controller id='Controller0' module='macraigor' cable='usb2demon' speed='4' port='0' />
	Macraigor Wiggler:
	<controller id='Controller0' module='macraigor' cable='wiggler' speed='1' port='0' />
;# Undocumented:
;#	<controller id='Controller0' module='parallel' cable='wiggler' speed='7' port='0' />

	ByteTools Catapult EJ-1 (Ethernet):
	<controller id='Controller0' module='catapult' speed='12500000' ipaddr='192.168.1.1' debug='0'/>
	ByteTools Catapult UJ-1 (USB) -- USB serial number is on back of probe:
	<controller id='Controller0' module='catapult' speed='12500000' usbser='332211' debug='0'/>
   -->
  <driver id='XtensaDriver0' module='xtensa' step-intr='mask,stepover,setps' />
;# Undocumented?:  can add this to the above driver line:  debug='verify'
  <driver id='TraxDriver0'   module='trax' />
  <chain controller='Controller0'> 
; foreach my $i (0 .. $#cores) {
    <tap id='TAP`$i`' irwidth='5' />   <!-- core `$cores[$i]->name` -->
; }
  </chain>
  <system module='jtag'>
; foreach my $i (0 .. $#cores) {
    <component id='Component`$i`' tap='TAP`$i`' config='trax' />
; }
  </system>
; foreach my $i (0 .. $#cores) {
  <device id='Xtensa`$i`' component='Component`$i`' driver='XtensaDriver0' />
; }
; # FIXME: support multiple TRAX? check config for presence?:
  <device id='Trax0'   component='Component0' driver='TraxDriver0' />
  <application id='GDBStub' module='gdbstub' port='20000'>
; foreach my $i (0 .. $#cores) {
    <target device='Xtensa`$i`' />
; }
  </application>
  <application id='TraxApp' module='traxapp' port='11444'>
; # FIXME: support multiple TRAX? check config for presence?:
    <target device='Trax0' />
  </application>
</configuration>

<!--
   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
  -->

