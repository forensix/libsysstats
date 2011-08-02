/* -----------------------------------------------------------------------------
 *  sysstats.m
 *  sysstats
 *
 *  Created by Manuel Gebele on 02.08.11.
 *  Copyright 2011 Manuel Gebele. All rights reserved.
 *
 * -------------------------------------------------------------------------- */

#include "sysstats.h"
#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <mach/vm_map.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

#import "Math.h"

#ifdef __cplusplus
extern "C" {
#endif
    
/* -----------------------------------------------------------------------------
 * CPU
 * -------------------------------------------------------------------------- */

void
libsstats_get_cpu(libsstats_cpu *buf)
{
    processor_cpu_load_info_t  pinfo;
    mach_msg_type_number_t     icount;
    natural_t                  pcount; /* processor count */
    
    kern_return_t kret
    = host_processor_info(mach_host_self(),
    PROCESSOR_CPU_LOAD_INFO,
    &pcount,
    (processor_info_array_t*)&pinfo,
    &icount);
    
    if (kret) {
		return;
	}
    
    /* Loop through all processors an fill the user buf. */
    unsigned i;
    for (i = 0; i < pcount; i++) {
        buf->xcpu_user[i] = pinfo[i].cpu_ticks[CPU_STATE_USER];
		buf->xcpu_sys[i] = pinfo[i].cpu_ticks[CPU_STATE_SYSTEM];
		buf->xcpu_idle[i] = pinfo[i].cpu_ticks[CPU_STATE_IDLE];
		buf->xcpu_nice[i] = pinfo[i].cpu_ticks[CPU_STATE_NICE];
		buf->xcpu_total[i] = buf->xcpu_user[i] + buf->xcpu_sys[i] +
        buf->xcpu_idle[i] + buf->xcpu_nice[i];
        
		buf->user  += buf->xcpu_user[i];
		buf->sys   += buf->xcpu_sys[i];
		buf->idle  += buf->xcpu_idle[i];
		buf->nice  += buf->xcpu_nice[i];
		buf->total += buf->xcpu_total[i];
    }
    vm_deallocate (mach_task_self(), (vm_address_t)pinfo, icount);
    
	buf->frequency = 100;
}


void
libsstats_get_cpu_percentage(libsstats_cpu cpu, libsstats_cpu_percentage *buf,
                             unsigned cpu_idx)
{
    unsigned long uload, sload;
    unsigned long total;
    static unsigned long uold_load = 0, sold_load = 0;
    static unsigned long uold_total = 0, sold_total = 0;
    float upercent = .0f, spercent = .0f;
    
    uload = cpu.xcpu_user[cpu_idx] + cpu.xcpu_nice[cpu_idx];
    sload = cpu.xcpu_sys[cpu_idx] + cpu.xcpu_nice[cpu_idx];
    
    total = cpu.xcpu_total[cpu_idx];
    
    if (total != uold_total) {
        upercent = 100.0f * (uload - uold_load) / (total - uold_total);
    }
    
    if (total != sold_total) {
        spercent = 100.0f * (sload - sold_load) / (total - sold_total);
    }
    
    uold_load = uload;
    sold_load = sload;
    uold_total = total;
    sold_total = total;
    
    buf->user_cpu_percentage = upercent;
    buf->system_cpu_percentage = spercent;
}


#ifdef __cplusplus
}
#endif







