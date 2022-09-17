/* Copyright (c) 2003-2009 by Tensilica Inc.  ALL RIGHTS RESERVED.
// These coded instructions, statements, and computer programs are the
// copyrighted works and confidential proprietary information of Tensilica Inc.
// They may not be modified, copied, reproduced, distributed, or disclosed to
// third parties in any manner, medium, or form, in whole or in part, without
// the prior written consent of Tensilica Inc.
*/

/* NOTE THIS FILE MUST BE COMPILABLE BY A C COMPILER AS WELL */

#ifndef LIBCAS_H
#define LIBCAS_H

#if defined __cplusplus
#  define EXTERN extern "C"
#else
#  define EXTERN extern
#endif

#ifndef XTCORE_DLLEXPORT
#define XTCORE_DLLEXPORT
#endif

#define LIBCAS_VERSION 79903

struct XTCORE;
typedef struct dll_data_prefix {
    struct XTCORE *core;
} dll_data_prefix_t;
#define GET_XTCORE(info) (((dll_data_prefix *)(info))->core)

typedef struct dll_data dll_data_t;
typedef dll_data_t *dll_data_ptr_t;

typedef unsigned (*dll_get_size_t)(void);
typedef unsigned (*dll_init_data_t)(dll_data_ptr_t, struct XTCORE *);
typedef void (*dll_enable_events_t)(dll_data_ptr_t, int);
typedef void (*dll_reset_states_t)(dll_data_ptr_t);
typedef void (*dll_advance_t)(dll_data_ptr_t);
typedef void (*dll_partial_advance_t)(dll_data_ptr_t, int);
typedef void (*dll_kill_t)(dll_data_ptr_t, int start_stage, int end_stage);
typedef int (*dll_instruction_get_length_t)(dll_data_ptr_t,unsigned char); 
typedef int (*dll_export_state_stall_t)(dll_data_ptr_t, int stage); 

typedef int (*dll_stall_fcn)(dll_data_ptr_t, xtensa_insnbuf insn);
typedef struct dll_stall_struct 
{
    char *opcode_name; 
    dll_stall_fcn stall_function;
} dll_stall_t;

typedef struct dll_slot_stall_struct 
{
    char *format_name;
    char *slot_name;
    int slot_position;
    const dll_stall_t *stall_functions;
} dll_slot_stall_t;
typedef const dll_slot_stall_t *(*dll_get_stall_t)(void);

typedef void (*dll_issue_fcn)(dll_data_ptr_t, xtensa_insnbuf insn);
typedef struct dll_issue_struct 
{
    char *opcode_name; 
    dll_issue_fcn issue_function;
} dll_issue_t;

typedef struct dll_slot_issue_struct 
{
    char *format_name;
    char *slot_name;
    int slot_position;
    const dll_issue_t *issue_functions;
} dll_slot_issue_t;
typedef const dll_slot_issue_t *(*dll_get_issue_t)(void);

typedef void (*dll_stage_fcn)(dll_data_ptr_t, xtensa_insnbuf insn, int gstall);
typedef struct dll_semantic_struct {
    char *opcode_name; 
    unsigned num_stages;
    const dll_stage_fcn *stage_functions;
} dll_semantic_t;

typedef struct dll_slot_semantic_struct {
    char *format_name;
    char *slot_name;
    int slot_position;
    const dll_semantic_t *semantic_functions;
} dll_slot_semantic_t;
typedef const dll_slot_semantic_t *(*dll_get_stage_t)(void);

typedef 
void (*dll_state_use_value_fcn)(dll_data_ptr_t, int stage, unsigned *buf);
typedef
int  (*dll_state_stage_value_fcn)(dll_data_ptr_t, int stage, unsigned *buf);
typedef
int  (*dll_state_set_stage_value_fcn)(dll_data_ptr_t, int stage, unsigned *buf);
typedef
void (*dll_state_commit_value_fcn)(dll_data_ptr_t, unsigned *buf);
typedef
void (*dll_state_set_commit_value_fcn)(dll_data_ptr_t, unsigned *buf);

typedef struct dll_state_struct {
    char *name;
    int width;
    dll_state_use_value_fcn        use_value;
    dll_state_stage_value_fcn      stage_value;
    dll_state_commit_value_fcn     commit_value;
    dll_state_set_stage_value_fcn  set_stage_value;
    dll_state_set_commit_value_fcn set_commit_value;
} dll_state_t;
typedef const dll_state_t *(*dll_get_state_table_t)(void);

typedef
int  (*dll_regfile_stage_value_fcn)(dll_data_ptr_t, int stage, 
                                    int index, unsigned *buf);
typedef
int  (*dll_regfile_set_stage_value_fcn)(dll_data_ptr_t, int stage, 
                                        int index, unsigned *buf);
typedef
void (*dll_regfile_commit_value_fcn)(dll_data_ptr_t, 
                                     int index, unsigned *buf);
typedef
void (*dll_regfile_set_commit_value_fcn)(dll_data_ptr_t, 
                                         int index, unsigned *buf);
typedef struct dll_regfile_struct {
    char *name;
    char *shortname;
    int width;
    int depth;
    dll_regfile_stage_value_fcn      stage_value;
    dll_regfile_commit_value_fcn     commit_value;
    dll_regfile_set_stage_value_fcn  set_stage_value;
    dll_regfile_set_commit_value_fcn set_commit_value;
} dll_regfile_t;
typedef const dll_regfile_t *(*dll_get_regfile_table_t)(void);

typedef void (*dll_exception_handler_fcn)(dll_data_ptr_t);
typedef struct dll_exception_struct {
    char *name;
    dll_exception_handler_fcn handler;
} dll_exception_t;
typedef const dll_exception_t *(*dll_get_exceptions_t)(void);

#define CORE_SIGNAL_INT unsigned int
typedef CORE_SIGNAL_INT (*dll_core_signal_handler_fcn)(struct XTCORE *);
typedef struct dll_core_signal_struct {
    char *name;
    dll_core_signal_handler_fcn handler;
} dll_core_signal_t;
typedef const dll_core_signal_t *(*dll_get_core_signals_t)(void);

typedef void
(*dll_interface_function)(void *link, void *core, void *device, unsigned *data);

typedef struct dll_interface_table_entry {
    char *name;
    unsigned stage;
} dll_interface_table_entry_t;

#define TIE_PORT_GROUP_ExportState   0x00
#define TIE_PORT_GROUP_ImportWire    0x01
#define TIE_PORT_GROUP_InputQueue    0x02 
#define TIE_PORT_GROUP_OutputQueue   0x03
#define TIE_PORT_GROUP_Lookup        0x04
#define TIE_PORT_GROUP_LookupMemory 0x104

typedef int (*dll_tie_port_group_internal_pop)(dll_data_ptr_t, unsigned *dst);
typedef int (*dll_tie_port_group_internal_push)(dll_data_ptr_t, const unsigned *src);
typedef unsigned (*dll_tie_port_group_internal_maxcount)(dll_data_ptr_t);

typedef const char **(*dll_get_shared_functions_t)(void);

EXTERN XTCORE_DLLEXPORT void 
iss4_raise_exception(struct XTCORE *xtc, unsigned stage, unsigned exc_code);

EXTERN XTCORE_DLLEXPORT void
iss4_link_state(struct XTCORE *xtc, const char *name, void *info);

EXTERN XTCORE_DLLEXPORT void
iss4_link_regfile(struct XTCORE *xtc, const char *name, void *info);

EXTERN XTCORE_DLLEXPORT void
iss4_link_shared_function(struct XTCORE *xtc, const char *name, void *info);

EXTERN XTCORE_DLLEXPORT void
iss4_link_symbol(struct XTCORE *xtc, const char *name, void *sym, void *info);

EXTERN XTCORE_DLLEXPORT void
iss4_link_interface(struct XTCORE *xtc, const char *name, int in, int width,
                    dll_interface_function *func,
                    void **link, void **core, void **device);

EXTERN XTCORE_DLLEXPORT void
iss4_add_tie_port_group_fn(struct XTCORE *xtc, const char *name,
			   unsigned kind, unsigned latency,
			   dll_data_ptr_t data,
			   dll_tie_port_group_internal_maxcount internal_max_fn,
			   dll_tie_port_group_internal_pop internal_pop_fn,
			   dll_tie_port_group_internal_push internal_push_fn);

EXTERN XTCORE_DLLEXPORT void
iss4_add_wire(struct XTCORE *xtc, int slot, const char *block_name, 
              char *wire_name, int width, unsigned *data);

EXTERN XTCORE_DLLEXPORT void
iss4_update_wire(struct XTCORE *xtc, unsigned *data, unsigned words);

EXTERN XTCORE_DLLEXPORT void
iss4_schedule_for_end_of_stall_cycle(struct XTCORE *xtc, void *f, void *i);

EXTERN XTCORE_DLLEXPORT void
iss4_schedule_for_start_of_free_cycle(struct XTCORE *xtc, void *f, void *i);

EXTERN XTCORE_DLLEXPORT void
iss4_schedule_for_end_of_free_cycle(struct XTCORE *xtc, void *f, void *i);

EXTERN XTCORE_DLLEXPORT void
iss4_schedule_for_start_of_global_cycle(struct XTCORE *xtc, void *f, void *i);

EXTERN XTCORE_DLLEXPORT void
iss4_schedule_for_end_of_global_cycle(struct XTCORE *xtc, void *f, void *i);

EXTERN XTCORE_DLLEXPORT int
iss4_iterative_stall_count(unsigned opctl, unsigned opnd1, unsigned opnd2);

EXTERN XTCORE_DLLEXPORT int
iss4_set_iterative_stall(struct XTCORE *xtc, int stage);

EXTERN XTCORE_DLLEXPORT unsigned
iss4_iterative_insn(struct XTCORE *xtc, int stage,
                    unsigned opctl, unsigned opnd1, unsigned opnd2);

EXTERN XTCORE_DLLEXPORT void
iss4_schedule_tie_port_func(struct XTCORE *xtc, void *f, void *i,
                            int stage, int end_of_cycle);

EXTERN XTCORE_DLLEXPORT int
iss4_stage_killed(struct XTCORE *xtc, int stage);

EXTERN XTCORE_DLLEXPORT int
iss4_killpipe_w(struct XTCORE *xtc);

EXTERN XTCORE_DLLEXPORT int
iss4_interrupt_stall_m(struct XTCORE *xtc);

EXTERN XTCORE_DLLEXPORT int
iss4_global_stall(struct XTCORE *xtc);

EXTERN XTCORE_DLLEXPORT void
iss4_process_stage(struct XTCORE *xtc, int stage);

EXTERN XTCORE_DLLEXPORT void
iss4_set_tie_stall_eval(struct XTCORE *xtc, int value);

EXTERN XTCORE_DLLEXPORT int
iss4_export_state_stall(struct XTCORE *xtc, int stage);

EXTERN XTCORE_DLLEXPORT void
iss4_state_event(struct XTCORE *xtc, int stage, const char *name,
                 unsigned width, const unsigned *value, const unsigned *old);

EXTERN XTCORE_DLLEXPORT void
iss4_regfile_event(struct XTCORE *xtc, int stage, const char *name, int idx,
                   unsigned width, const unsigned *value, const unsigned *old);

EXTERN XTCORE_DLLEXPORT void
iss4_tie_print_event(struct XTCORE *core, int stage, unsigned slot_number,
		     unsigned order, const char *tie_printf_output );

EXTERN XTCORE_DLLEXPORT void
iss4_tieport_event(struct XTCORE *core, int stage, const char *name, const char *type,
		     unsigned width, const unsigned *value);

#endif

// Emacs formatting variables.
/*
 * Local Variables:
 * mode:c++
 * c-basic-offset:4
 * End:
 */
