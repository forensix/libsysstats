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
#include <CoreFoundation/CoreFoundation.h> // CFDictionaryRef

#define LIBSSTATS_NCPU              32

#define LIBSSTATS_MAX_NETDEVICES    256
#define LIBSSTATS_MAX_HOST          1025
#define LIBSSTATS_NUMERICHOST       2

#define LIBSSTATS_MAX_PROCESSES     512
#define LIBSSTATS_MAX_PROCESSES     512

#define LIBSSTATS_MAX_NAMELEN       256

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

enum LIBSSTATS_IF_IN6_SCOPE {
	LIBSSTATS_IF_IN6_SCOPE_UNKNOWN = 0,
	LIBSSTATS_IF_IN6_SCOPE_LINK    = 1,
	LIBSSTATS_IF_IN6_SCOPE_SITE    = 2,
	LIBSSTATS_IF_IN6_SCOPE_GLOBAL  = 4,
	LIBSSTATS_IF_IN6_SCOPE_HOST    = 8
};

typedef struct {
	uint64_t flags;
	uint64_t total;
	uint64_t user;
	uint64_t nice;
	uint64_t sys;
	uint64_t idle;
	uint64_t iowait;
	uint64_t irq;
	uint64_t softirq;
	uint64_t frequency;
	uint64_t xcpu_total[LIBSSTATS_NCPU];
	uint64_t xcpu_user[LIBSSTATS_NCPU];
	uint64_t xcpu_nice[LIBSSTATS_NCPU];
	uint64_t xcpu_sys[LIBSSTATS_NCPU];
	uint64_t xcpu_idle[LIBSSTATS_NCPU];
	uint64_t xcpu_iowait[LIBSSTATS_NCPU];
	uint64_t xcpu_irq[LIBSSTATS_NCPU];
	uint64_t xcpu_softirq[LIBSSTATS_NCPU];
	uint64_t xcpu_flags;
} libsstats_cpu;

typedef struct {
    float user_cpu_percentage;
    float system_cpu_percentage;
    float idle_cpu_percentage;
} libsstats_cpu_percentage;

typedef struct {
	double   loadavg [3];
	uint64_t nr_running;
	uint64_t nr_tasks;
	uint64_t last_pid;
} libsstats_loadavg;

typedef struct {
	uint32_t number;
} libsstats_netlist;
    
typedef struct {
	uint64_t flags;
	uint64_t if_flags;
    
	uint32_t mtu;
	uint32_t subnet;
	uint32_t address;

	uint64_t packets_in;
	uint64_t packets_out;
	uint64_t packets_total;
	uint64_t bytes_in;
	uint64_t bytes_out;
	uint64_t bytes_total;
	uint64_t errors_in;
	uint64_t errors_out;
	uint64_t errors_total;
	uint64_t collisions;
    
	uint8_t address6[16];
	uint8_t prefix6[16];
	uint8_t scope6;
    
	uint8_t hwaddress[8];
} libsstats_netload;

typedef struct {
    char macaddress[18];
} libsstats_mac;

typedef struct {
    char ipaddress[LIBSSTATS_MAX_HOST];
} libsstats_ip;

typedef struct {
	float total;
	float used;
	float free;
    float active;
    float inactive;
    float wired;
} libsstats_mem;

typedef struct {
    uint32_t number;
    CFDictionaryRef networks[256];
} libsstats_wireless;

typedef struct {
    uint32_t rssi;
} libsstats_cellular;

typedef struct {
    char name[LIBSSTATS_MAX_NAMELEN];
    uint32_t  pid;
} libsstats_process;

typedef struct {
    int number;
    libsstats_process processes[LIBSSTATS_MAX_PROCESSES];
    
} libsstats_processinfo;

typedef union  {
	libsstats_cpu               cpu;
	libsstats_cpu_percentage	cpu_percentage;
    libsstats_loadavg           loadavg;
    libsstats_netlist           netlist;
    libsstats_netload           netload;
    libsstats_mac               mac;
    libsstats_ip                ip;
    libsstats_mem               mem;
    libsstats_wireless          wireless;
    libsstats_cellular          cellular;;
    libsstats_process           process;
    libsstats_processinfo       processinfo;
} libsstats_union;

void libsstats_get_cpu(libsstats_cpu *buf);
void libsstats_get_cpu_percentage(libsstats_cpu cpu, libsstats_cpu_percentage *buf, unsigned cpu_idx);
void libsstats_get_loadavg(libsstats_loadavg *buf);
char **libsstats_get_netlist(libsstats_netlist *buf);
void libsstats_get_netload(libsstats_netload *buf, const char *intf);
void libsstats_get_mac(const char *intf, libsstats_mac *buf);
void libsstats_get_ip(const char *intf, libsstats_ip *buf);     
void libsstats_get_mem(libsstats_mem *buf);
void libsstats_get_wireless(libsstats_wireless *buf);
void libsstats_get_cellular(libsstats_cellular *buf);
void libsstats_get_processinfo(libsstats_processinfo *buf);

#ifdef __cplusplus
}
#endif
