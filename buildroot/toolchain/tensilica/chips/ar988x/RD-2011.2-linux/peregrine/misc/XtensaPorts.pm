# Customer ID=8327; Build=0x3b95c; Copyright (c) 2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of Tensilica Inc.
# They may not be modified, copied, reproduced, distributed, or disclosed to
# third parties in any manner, medium, or form, in whole or in part, without
# the prior written consent of Tensilica Inc.  

package XtensaPorts;

use strict;
use Exporter;
@XtensaPorts::ISA = qw(Exporter);
@XtensaPorts::EXPORT = qw();
@XtensaPorts::EXPORT_OK = ();
@XtensaPorts::EXPORT_TAGS = ();
use vars qw(%parameters %ports);


my %buildoptions = (
    'trax'              => 0,
    'trax_log_mem_size' => 0,
);

sub getBuildOptions {
   sort keys %buildoptions;
}

sub getBuildOption {
    my $option = shift;
    die "Error: option $option doesn't exist in XtensaPorts\n" unless(exists $buildoptions{$option});
    $buildoptions{$option};
}

%parameters = (
    'maxiwidth'     => 24,
    'width'         => 32,
    'iwidth'        => 24,
    'ridx'          => 4,
    'pcwidth'       => 32,
    'awidth'        => 32,
    'eawidth'       => 32,
    'pawidth'       => 32,
    'eintrwidth'    => 30,
    'bewidth'       => 4,
    'iccntsize'     => 0,
    'icwidth'       => 32,
    'iccwidth'      => 0,
    'itwidth'       => 0,
    'itcwidth'      => 0,
    'wordenwidth'   => 0,
    'iramwidth'     => 32,
    'iramwords'     => 1,
    'iramawidth'    => 17,
    'iram0width'    => 32,
    'iram0words'    => 1,
    'iram0awidth'   => 17,
    'iram0cwidth'   => 0,
    'iram1width'    => 32,
    'iram1words'    => 1,
    'iram1awidth'   => 17,
    'iram1cwidth'   => 0,
    'iromwidth'     => 0,
    'iromwords'     => 0,
    'asize'         => 5,
    'tpdwidth'      => 32,
    'tpswidth'      => 8,
    'dwaybawidth'   => 0,
    'dcwidth'       => 0,
    'nbytes'        => 0,
    'dcwidthoff'    => 0,
    'dclineoffset'  => 0,
    'edrdwidth'     => 32,
    'edwrwidth'     => 32,
    'edrdbytecnt'   => 4,
    'edrdlbw'       => 2,
    'jtiwidth'      => 5,
    'cpn'           => 0,
    'iPgLsb'        => 29,
    'dPgLsb'        => 29,
    'itawidth'      => 0,
    'icsetawidth'   => 0,
    'iromawidth'    => 0,
    'wbcountwidth'  => 3,
    'reqid'         => 6,
    'dreqid'        => 6,
    'ifwayselwidth' => 1,
   );
sub getParameters {
   \%parameters;
}
# Port info structure 
# signal_name -> [ direction, bit_index, group, interface_type, protocol, delay ] 
%ports = (
    'CLK'                => [ 'input',         '', 'Global', 'xtintfGlobal', 'CLK', '' ],
    'BReset'             => [ 'input',         '', 'Global', 'xtintfGlobal', 'Reset', 'C*0.25' ],
    'IRam0Data'          => [ 'input',   '[31:0]', 'IRam0', 'xtintfInstRam', 'Data', '1.95' ],
    'IRam1Data'          => [ 'input',   '[31:0]', 'IRam1', 'xtintfInstRam', 'Data', '1.95' ],
    'DRam0Data0'         => [ 'input',   '[31:0]', 'DRam0', 'xtintfDataRam', 'Data', '1.872' ],
    'DRam1Data0'         => [ 'input',   '[31:0]', 'DRam1', 'xtintfDataRam', 'Data', '1.872' ],
    'PIReqRdy'           => [ 'input',         '', 'PIFM', 'xtintfPIF', 'ReqRdy', 'C*0.60' ],
    'PIRespData'         => [ 'input',   '[31:0]', 'PIFM', 'xtintfPIF', 'RespData', 'C*0.70' ],
    'PIRespCntl'         => [ 'input',    '[7:0]', 'PIFM', 'xtintfPIF', 'RespCntl', 'C*0.70' ],
    'PIRespValid'        => [ 'input',         '', 'PIFM', 'xtintfPIF', 'RespValid', 'C*0.70' ],
    'PIRespPriority'     => [ 'input',    '[1:0]', 'PIFM', 'xtintfPIF', 'RespPriority', 'C*0.70' ],
    'PIRespId'           => [ 'input',    '[5:0]', 'PIFM', 'xtintfPIF', 'RespId', 'C*0.70' ],
    'PIReqValid'         => [ 'input',         '', 'PIFS', 'xtintfPIF', 'ReqValid', 'C*0.70' ],
    'PIReqCntl'          => [ 'input',    '[7:0]', 'PIFS', 'xtintfPIF', 'ReqCntl', 'C*0.70' ],
    'PIReqAdrs'          => [ 'input',   '[31:0]', 'PIFS', 'xtintfPIF', 'ReqAdrs', 'C*0.70' ],
    'PIReqData'          => [ 'input',   '[31:0]', 'PIFS', 'xtintfPIF', 'ReqData', 'C*0.70' ],
    'PIReqDataBE'        => [ 'input',    '[3:0]', 'PIFS', 'xtintfPIF', 'ReqDataBE', 'C*0.70' ],
    'PIReqPriority'      => [ 'input',    '[1:0]', 'PIFS', 'xtintfPIF', 'ReqPriority', 'C*0.70' ],
    'PIReqId'            => [ 'input',    '[5:0]', 'PIFS', 'xtintfPIF', 'ReqId', 'C*0.70' ],
    'PIRespRdy'          => [ 'input',         '', 'PIFS', 'xtintfPIF', 'RespRdy', 'C*0.70' ],
    'BInterrupt'         => [ 'input',   '[29:0]', 'Interrupt', 'xtintfWire', 'Data', 'C*0.40' ],
    'RunStall'           => [ 'input',         '', 'RunStall', 'xtintfWire', 'Data', 'C*0.80' ],
    'TMode'              => [ 'input',         '', 'TMode', 'xtintfWire', 'Data', 'C*0.10' ],
    'TResetB'            => [ 'input',         '', 'TAP', 'xtintfTAP', 'TResetB', 'C*0.15' ],
    'TClockDR'           => [ 'input',         '', 'TAP', 'xtintfTAP', 'TClockDR', 'C*0.15' ],
    'TUpdateDR'          => [ 'input',         '', 'TAP', 'xtintfTAP', 'TUpdateDR', 'C*0.15' ],
    'TShiftDR'           => [ 'input',         '', 'TAP', 'xtintfTAP', 'TShiftDR', 'C*0.15' ],
    'JTDI'               => [ 'input',         '', 'TAP', 'xtintfTAP', 'JTDI', 'C*0.15' ],
    'JTCK'               => [ 'input',         '', 'TAP', 'xtintfTAP', 'JTCK', '' ],
    'TInst'              => [ 'input',    '[4:0]', 'TAP', 'xtintfTAP', 'TInst', 'C*0.15' ],
    'TUpdateIR'          => [ 'input',         '', 'TAP', 'xtintfTAP', 'TUpdateIR', 'C*0.15' ],
    'OCDHaltOnReset'     => [ 'input',         '', 'OCDHaltOnReset', 'xtintfWire', 'Data', 'C*0.25' ],
    'TDebugInterrupt'    => [ 'input',         '', 'BreakIn', 'xtintfWire', 'Data', 'C*0.7' ],
    'IRam0Addr'          => [ 'output',   '[18:2]', 'IRam0', 'xtintfInstRam', 'Addr', '0.6625' ],
    'IRam0En'            => [ 'output',         '', 'IRam0', 'xtintfInstRam', 'En', '1.025' ],
    'IRam0Wr'            => [ 'output',         '', 'IRam0', 'xtintfInstRam', 'Wr', '0.5875' ],
    'IRam0WrData'        => [ 'output',   '[31:0]', 'IRam0', 'xtintfInstRam', 'WrData', '0.7125' ],
    'IRam0LoadStore'     => [ 'output',         '', 'IRam0', 'xtintfInstRam', 'LoadStore', 'C*0.30' ],
    'IRam1Addr'          => [ 'output',   '[18:2]', 'IRam1', 'xtintfInstRam', 'Addr', '0.6625' ],
    'IRam1En'            => [ 'output',         '', 'IRam1', 'xtintfInstRam', 'En', '1.025' ],
    'IRam1Wr'            => [ 'output',         '', 'IRam1', 'xtintfInstRam', 'Wr', '0.5875' ],
    'IRam1WrData'        => [ 'output',   '[31:0]', 'IRam1', 'xtintfInstRam', 'WrData', '0.7125' ],
    'IRam1LoadStore'     => [ 'output',         '', 'IRam1', 'xtintfInstRam', 'LoadStore', 'C*0.30' ],
    'DRam0Addr0'         => [ 'output',   '[17:2]', 'DRam0', 'xtintfDataRam', 'Addr', '0.636' ],
    'DRam0En0'           => [ 'output',         '', 'DRam0', 'xtintfDataRam', 'En', '0.984' ],
    'DRam0ByteEn0'       => [ 'output',    '[3:0]', 'DRam0', 'xtintfDataRam', 'ByteEn', '0.624' ],
    'DRam1Addr0'         => [ 'output',   '[17:2]', 'DRam1', 'xtintfDataRam', 'Addr', '0.636' ],
    'DRam1En0'           => [ 'output',         '', 'DRam1', 'xtintfDataRam', 'En', '0.984' ],
    'DRam1ByteEn0'       => [ 'output',    '[3:0]', 'DRam1', 'xtintfDataRam', 'ByteEn', '0.624' ],
    'DRam0Wr0'           => [ 'output',         '', 'DRam0', 'xtintfDataRam', 'Wr', '0.564' ],
    'DRam0WrData0'       => [ 'output',   '[31:0]', 'DRam0', 'xtintfDataRam', 'WrData', '0.684' ],
    'DRam1Wr0'           => [ 'output',         '', 'DRam1', 'xtintfDataRam', 'Wr', '0.564' ],
    'DRam1WrData0'       => [ 'output',   '[31:0]', 'DRam1', 'xtintfDataRam', 'WrData', '0.684' ],
    'POReqValid'         => [ 'output',         '', 'PIFM', 'xtintfPIF', 'ReqValid', 'C*0.80' ],
    'POReqCntl'          => [ 'output',    '[7:0]', 'PIFM', 'xtintfPIF', 'ReqCntl', 'C*0.80' ],
    'POReqAdrs'          => [ 'output',   '[31:0]', 'PIFM', 'xtintfPIF', 'ReqAdrs', 'C*0.80' ],
    'POReqData'          => [ 'output',   '[31:0]', 'PIFM', 'xtintfPIF', 'ReqData', 'C*0.80' ],
    'POReqDataBE'        => [ 'output',    '[3:0]', 'PIFM', 'xtintfPIF', 'ReqDataBE', 'C*0.80' ],
    'POReqPriority'      => [ 'output',    '[1:0]', 'PIFM', 'xtintfPIF', 'ReqPriority', 'C*0.80' ],
    'POReqId'            => [ 'output',    '[5:0]', 'PIFM', 'xtintfPIF', 'ReqId', 'C*0.80' ],
    'PORespRdy'          => [ 'output',         '', 'PIFM', 'xtintfPIF', 'RespRdy', 'C*0.80' ],
    'POReqRdy'           => [ 'output',         '', 'PIFS', 'xtintfPIF', 'ReqRdy', 'C*0.80' ],
    'PORespData'         => [ 'output',   '[31:0]', 'PIFS', 'xtintfPIF', 'RespData', 'C*0.80' ],
    'PORespCntl'         => [ 'output',    '[7:0]', 'PIFS', 'xtintfPIF', 'RespCntl', 'C*0.80' ],
    'PORespValid'        => [ 'output',         '', 'PIFS', 'xtintfPIF', 'RespValid', 'C*0.80' ],
    'PORespPriority'     => [ 'output',    '[1:0]', 'PIFS', 'xtintfPIF', 'RespPriority', 'C*0.80' ],
    'PORespId'           => [ 'output',    '[5:0]', 'PIFS', 'xtintfPIF', 'RespId', 'C*0.80' ],
    'PWaitMode'          => [ 'output',         '', 'Interrupt', '', 'WaitMode', 'C*0.85' ],
    'XTDO'               => [ 'output',         '', 'TAP', 'xtintfTAP', 'XTDO', 'C*0.15' ],
    'XTDV'               => [ 'output',         '', 'TAP', 'xtintfTAP', 'XTDV', 'C*0.15' ],
    'XOCDMode'           => [ 'output',         '', 'XOCDMode', 'xtintfWire', 'XOCDMode', 'C*0.7' ],
    'XOCDModePulse'      => [ 'output',         '', 'BreakOut', 'xtintfWire', 'Data', 'C*0.7' ],
   );

sub getGroups {
    my %groups;
    foreach (keys %ports) {
        next if($ports{$_}->[2] eq 'Global');
        $groups{$ports{$_}->[2]}++;
    }
    sort keys %groups;
}
sub getGroupsSignals {
    my ($group) = @_;
    my %signals;
    foreach (keys %ports) {
        $signals{$_}++ if($ports{$_}->[2] eq $group);
    }
    sort keys %signals;
}
sub getSignalsDirection {
    my ($signal) = @_;
    $ports{$signal}->[0];
}
sub getDir {
    my ($sig) = @_;
    $ports{$sig}->[0];
}
sub getSignalsBitIndex {
    my ($signal) = @_;
    $ports{$signal}->[1];
}
sub getSignalsGroup {
    my ($signal) = @_;
    $ports{$signal}->[2];
}
sub getPorts {
    my %remove_dir;
    foreach (keys %ports) {
       $remove_dir{$_} = $ports{$_}->[1];
    }
    \%remove_dir;
}
sub getSignalsInterfaceType {
    my ($signal) = @_;
    $ports{$signal}->[3];
}
sub getSignalsProtocol {
    my ($signal) = @_;
    $ports{$signal}->[4];
}
sub getSignalsDelay {
    my ($signal) = @_;
    $ports{$signal}->[5];
}
sub getIOC {
    foreach my $group ('Global', getGroups) {
        print "\n# Signal group: $group\n";
        foreach my $signal (getGroupsSignals($group)) {
            next if($signal =~ /CLK$/);
            next if($signal eq 'JTCK');
            next if($signal eq 'JTDI');
            (my $dir = getDir($signal)) =~ s/^([io]).+$/$1/;
            (my $timing = getSignalsDelay($signal)) =~ s/C\*/Target_ClockPeriod \* /;
            my $clock = ($group =~ /^PIF[MS]$/) ? 'BCLK' :
                        ($group eq 'TAP')       ? 'JTCK' :
                        ($group eq 'APB')       ? 'PBCLK' :
                                                  'CLK';
            printf "%-20s %-8s %-4s    %s\n", $signal, $dir, $clock, $timing;
        }
    }
}
1;

