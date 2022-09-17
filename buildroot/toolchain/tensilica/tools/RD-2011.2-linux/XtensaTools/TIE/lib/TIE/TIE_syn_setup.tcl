################################################################################
# Copyright (c) 2008 by Tensilica Inc.  ALL RIGHTS RESERVED.                   #
# These coded instructions, statements, and computer programs are the          #
# copyrighted works and confidential proprietary information of Tensilica Inc. #
# They may not be modified, copied, reproduced, distributed, or disclosed to   #
# third parties in any manner, medium, or form, in whole or in part, without   #
# the prior written consent of Tensilica Inc.                                  #
################################################################################

# This file is automatically filled in by TC to specifiy parameters necessary
# to synthesize the TIE file.

set XTENSA_CORE_PATH @SWCONFIG@
set TIE_FILE_NAME @ROOT@.v
set TIE_DESIGN_PREFIX @DESIGN_PREFIX@
set TIE_DESIGN_NAME @DESIGN_PREFIX@TIE
@RETIME@
