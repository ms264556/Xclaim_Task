# Customer ID=8327; Build=0x3b95c; Copyright (c) 2007-2011 by Tensilica Inc.  ALL RIGHTS RESERVED.
# These coded instructions, statements, and computer programs are the
# copyrighted works and confidential proprietary information of
# Tensilica Inc.  They may be adapted and modified by bona fide
# purchasers for internal use, but neither the original nor any adapted
# or modified version may be disclosed or distributed to third parties
# in any manner, medium, or form, in whole or in part, without the prior
# written consent of Tensilica Inc.


::pct::open_library $env(XTENSA_SW_TOOLS)/misc/xtsc_vp/xtsc_vp.xml

::scsh::open-project
::scsh::cosim::enable_hdl_sdi
::scsh::build-options -skip-elab off
::scsh::build-options -cache-objects off

::scsh::build
::scsh::elab sim



