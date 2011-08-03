/* -----------------------------------------------------------------------------
 *  sysstats.h
 *  sysstats
 *
 *  Created by Manuel Gebele on 02.08.11.
 *  Copyright 2011 Manuel Gebele. All rights reserved.
 *
 * -------------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdint.h>

/* -----------------------------------------------------------------------------
 * CPU
 * -------------------------------------------------------------------------- */

#define LIBSSTATS_CPU_TOTAL     0
#define LIBSSTATS_CPU_USER      1
#define LIBSSTATS_CPU_NICE      2
#define LIBSSTATS_CPU_SYS       3
#define LIBSSTATS_CPU_IDLE      4
#define LIBSSTATS_CPU_FREQUENCY 5
#define LIBSSTATS_XCPU_TOTAL    6
#define LIBSSTATS_XCPU_USER     7
#define LIBSSTATS_XCPU_NICE     8
#define LIBSSTATS_XCPU_SYS      9
#define LIBSSTATS_XCPU_IDLE     10
#define LIBSSTATS_XCPU_FLAGS    11
#define LIBSSTATS_CPU_IOWAIT    12
#define LIBSSTATS_CPU_IRQ       13
#define LIBSSTATS_CPU_SOFTIRQ   14
#define LIBSSTATS_XCPU_IOWAIT   15
#define LIBSSTATS_XCPU_IRQ      16
#define LIBSSTATS_XCPU_SOFTIRQ  17
#define LIBSSTATS_MAX_CPU       18
#define LIBSSTATS_NCPU          32

typedef struct {
	uint64_t flags;
	uint64_t total;                         /* LIBSSTATS_CPU_TOTAL		*/
	uint64_t user;                          /* LIBSSTATS_CPU_USER		*/
	uint64_t nice;                          /* LIBSSTATS_CPU_NICE		*/
	uint64_t sys;                           /* LIBSSTATS_CPU_SYS		*/
	uint64_t idle;                          /* LIBSSTATS_CPU_IDLE		*/
	uint64_t iowait;                        /* LIBSSTATS_CPU_IOWAIT		*/
	uint64_t irq;                           /* LIBSSTATS_CPU_IRQ		*/
	uint64_t softirq;                       /* LIBSSTATS_CPU_SOFTIRQ    */
	uint64_t frequency;                     /* LIBSSTATS_CPU_FREQUENCY  */
	uint64_t xcpu_total[LIBSSTATS_NCPU];    /* LIBSSTATS_XCPU_TOTAL		*/
	uint64_t xcpu_user[LIBSSTATS_NCPU];     /* LIBSSTATS_XCPU_USER		*/
	uint64_t xcpu_nice[LIBSSTATS_NCPU];     /* LIBSSTATS_XCPU_NICE		*/
	uint64_t xcpu_sys[LIBSSTATS_NCPU];      /* LIBSSTATS_XCPU_SYS		*/
	uint64_t xcpu_idle[LIBSSTATS_NCPU];     /* LIBSSTATS_XCPU_IDLE		*/
	uint64_t xcpu_iowait[LIBSSTATS_NCPU];   /* LIBSSTATS_XCPU_IOWAIT    */
	uint64_t xcpu_irq[LIBSSTATS_NCPU];      /* LIBSSTATS_XCPU_IRQ		*/
	uint64_t xcpu_softirq[LIBSSTATS_NCPU];  /* LIBSSTATS_XCPU_SOFTIRQ   */
	uint64_t xcpu_flags;	                /* LIBSSTATS_XCPU_IDLE		*/
} libsstats_cpu;


typedef struct {
    float user_cpu_percentage;
    float system_cpu_percentage;
    float idle_cpu_percentage;
} libsstats_cpu_percentage;


void libsstats_get_cpu(libsstats_cpu *buf);
void libsstats_get_cpu_percentage(libsstats_cpu cpu, libsstats_cpu_percentage *buf, unsigned cpu_idx);


/* -----------------------------------------------------------------------------
 * NET
 * -------------------------------------------------------------------------- */

#define LIBSSTATS_NETLIST_NUMBER 0
#define LIBSSTATS_MAX_NETLIST    1
#define LIBSSTATS_MAX_NETDEVICES 256

typedef struct {
	uint32_t number;
} libsstats_netlist;
    

char **libsstats_get_netlist(libsstats_netlist *buf);
    
    
    
    
    
    
    
    
    
    
    
    
    
    
#ifdef __cplusplus
}
#endif
