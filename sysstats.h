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

#define LIBSSTATS_NETLIST_NUMBER        0
#define LIBSSTATS_MAX_NETLIST           1
#define LIBSSTATS_MAX_NETDEVICES        256

#define LIBSSTATS_NETLOAD_IF_FLAGS      0
#define LIBSSTATS_NETLOAD_MTU           1
#define LIBSSTATS_NETLOAD_SUBNET        2
#define LIBSSTATS_NETLOAD_ADDRESS       3
#define LIBSSTATS_NETLOAD_PACKETS_IN    4
#define LIBSSTATS_NETLOAD_PACKETS_OUT   5
#define LIBSSTATS_NETLOAD_PACKETS_TOTAL 6
#define LIBSSTATS_NETLOAD_BYTES_IN      7
#define LIBSSTATS_NETLOAD_BYTES_OUT     8
#define LIBSSTATS_NETLOAD_BYTES_TOTAL   9
#define LIBSSTATS_NETLOAD_ERRORS_IN     10
#define LIBSSTATS_NETLOAD_ERRORS_OUT    11
#define LIBSSTATS_NETLOAD_ERRORS_TOTAL  12
#define LIBSSTATS_NETLOAD_COLLISIONS    13
#define LIBSSTATS_NETLOAD_ADDRESS6      14
#define LIBSSTATS_NETLOAD_PREFIX6       15
#define LIBSSTATS_NETLOAD_SCOPE6        16
#define LIBSSTATS_NETLOAD_HWADDRESS     17
#define LIBSSTATS_MAX_NETLOAD           18

enum {
	LIBSSTATS_IF_FLAGS_UP = 1,
	LIBSSTATS_IF_FLAGS_BROADCAST,
	LIBSSTATS_IF_FLAGS_DEBUG,
	LIBSSTATS_IF_FLAGS_LOOPBACK,
	LIBSSTATS_IF_FLAGS_POINTOPOINT,
	LIBSSTATS_IF_FLAGS_RUNNING,
	LIBSSTATS_IF_FLAGS_NOARP,
	LIBSSTATS_IF_FLAGS_PROMISC,
	LIBSSTATS_IF_FLAGS_ALLMULTI,
	LIBSSTATS_IF_FLAGS_OACTIVE,
	LIBSSTATS_IF_FLAGS_SIMPLEX,
	LIBSSTATS_IF_FLAGS_LINK0,
	LIBSSTATS_IF_FLAGS_LINK1,
	LIBSSTATS_IF_FLAGS_LINK2,
	LIBSSTATS_IF_FLAGS_ALTPHYS,
	LIBSSTATS_IF_FLAGS_MULTICAST,
	LIBSSTATS_IF_FLAGS_WIRELESS
};

enum LIBSSTATS_IF_IN6_SCOPE
{
	LIBSSTATS_IF_IN6_SCOPE_UNKNOWN = 0,
	LIBSSTATS_IF_IN6_SCOPE_LINK    = 1,
	LIBSSTATS_IF_IN6_SCOPE_SITE    = 2,
	LIBSSTATS_IF_IN6_SCOPE_GLOBAL  = 4,
	LIBSSTATS_IF_IN6_SCOPE_HOST    = 8
};

typedef struct {
	uint32_t number;
} libsstats_netlist;
    
typedef struct {
	uint64_t flags;
	uint64_t if_flags;      /* LIBSSTATS_NETLOAD_IF_FLAGS         */
    
	uint32_t mtu;           /* LIBSSTATS_NETLOAD_MTU              */
	uint32_t subnet;        /* LIBSSTATS_NETLOAD_SUBNET           */
	uint32_t address;       /* LIBSSTATS_NETLOAD_ADDRESS          */

	uint64_t packets_in;    /* LIBSSTATS_NETLOAD_PACKETS_IN       */
	uint64_t packets_out;   /* LIBSSTATS_NETLOAD_PACKETS_OUT      */
	uint64_t packets_total; /* LIBSSTATS_NETLOAD_PACKETS_TOTAL    */
	uint64_t bytes_in;      /* LIBSSTATS_NETLOAD_BYTES_IN         */
	uint64_t bytes_out;     /* LIBSSTATS_NETLOAD_BYTES_OUT        */
	uint64_t bytes_total;   /* LIBSSTATS_NETLOAD_BYTES_TOTAL      */
	uint64_t errors_in;		/* LIBSSTATS_NETLOAD_ERRORS_IN        */
	uint64_t errors_out;    /* LIBSSTATS_NETLOAD_ERRORS_OUT       */
	uint64_t errors_total;  /* LIBSSTATS_NETLOAD_ERRORS_TOTAL     */
	uint64_t collisions;    /* LIBSSTATS_NETLOAD_COLLISIONS       */
    
	uint8_t address6[16];   /* LIBSSTATS_NETLOAD_ADDRESS6         */
	uint8_t prefix6[16];    /* LIBSSTATS_NETLOAD_PREXIF6          */
	uint8_t scope6;         /* LIBSSTATS_NETLOAD_SCOPE6           */
    
	uint8_t hwaddress[8];   /* LIBSSTATS_NETLOAD_HWADDRESS        */
} libsstats_netload;

char **libsstats_get_netlist(libsstats_netlist *buf);
void libsstats_get_netload(libsstats_netload *buf, const char *intf);
    
    
    
    
    
    
    
    
    
    
    
    
    
#ifdef __cplusplus
}
#endif
