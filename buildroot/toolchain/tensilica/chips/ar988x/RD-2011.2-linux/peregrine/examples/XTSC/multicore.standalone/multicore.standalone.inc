// Customer ID=8327; Build=0x3b95c; Copyright (c) 2006-2007 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of
// Tensilica Inc.  They may be adapted and modified by bona fide
// purchasers for internal use, but neither the original nor any adapted
// or modified version may be disclosed or distributed to third parties
// in any manner, medium, or form, in whole or in part, without the prior
// written consent of Tensilica Inc.

-logging=../TextLogger.txt

// Edit the xtensa_system, xtensa_core, and xtensa_params assignments as required for the first core
-xtensa_system=/home/spatel/depot/private/11ac/buildroot/toolchain/tensilica/chips/ar988x/RD-2011.2-linux/peregrine/config
-xtensa_core=peregrine
-xtensa_params=
-core_program=target/core0.out
-create_core=core0

// Edit the xtensa_system, xtensa_core, and xtensa_params assignments as required for the second core
-xtensa_system=/home/spatel/depot/private/11ac/buildroot/toolchain/tensilica/chips/ar988x/RD-2011.2-linux/peregrine/config
-xtensa_core=peregrine
-xtensa_params=
-core_program=target/core1.out
-create_core=core1

