/*
 * Tensilica's interface to FlexLM licensing
 */

/*
 * Copyright 2000 Tensilica Inc.
 * These coded instructions, statements, and computer programs are
 * Confidential Proprietary Information of Tensilica Inc. and may not be
 * disclosed to third parties or copied in any form, in whole or in part,
 * without the prior written consent of Tensilica Inc.
 */

#if !defined(TENLP_INCLUDE)
#define TENLP_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Exported datatypes
 */
typedef void * TENLP_HANDLE;

/*
 * Exported API for all platforms
 */
extern int    tenlp_init(void);
extern int    tenlp_checkout(int policy, char *feat, char *vers, 
			     int count, char *path, TENLP_HANDLE **h);
extern void   tenlp_checkin(TENLP_HANDLE *h);
extern int    tenlp_heartbeat(TENLP_HANDLE *h, int *nr, int nm);
extern char * tenlp_errstring(TENLP_HANDLE *h);
extern char * tenlp_warning(TENLP_HANDLE *h);
extern void   tenlp_perror(TENLP_HANDLE *h, char *s);
extern void   tenlp_pwarn(TENLP_HANDLE *h, char *s);

/*
 * Main policies first byte -- mask: 0xff
 * one of the following:
 */
#define LM_RESTRICTIVE		0x1
#define LM_QUEUE		0x2
#define LM_FAILSAFE		0x3
#define LM_LENIENT		0x4

/*
 * Modifiers -- next 3 bytes
 */

#define LM_MANUAL_HEARTBEAT 		0x100
#define LM_RETRY_RESTRICTIVE 		0x200
#define LM_ALLOW_FLEXLMD		0x400
#define LM_CHECK_BADDATE		0x800
#define LM_FLEXLOCK     	       0x1000

/*
 * Our error codes
 */
#define TENLP_BADAUTH (-799401)
#define TENLP_BADAUTH_STR "Tensilica Licensing authentication error"

#define TENLP_NODLL (-799402)
#define TENLP_NODLL_STR "Cannot load Tensilica Licensing library"

#ifdef __cplusplus
}
#endif

#endif /* TENLP_INCLUDE */

