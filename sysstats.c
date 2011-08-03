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
#include <sys/socket.h> /* Needed for net/if.h ! */
#include <net/if.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/route.h>
#include <stdlib.h> 

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
    
    memset (buf, 0, sizeof (libsstats_cpu));

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
    float upercent = .0f, spercent = .0f, ipercent = 100.0f, tpercent = .0f;
    
    memset (buf, 0, sizeof (libsstats_cpu_percentage));

    uload = cpu.xcpu_user[cpu_idx] + cpu.xcpu_nice[cpu_idx];
    sload = cpu.xcpu_sys[cpu_idx] + cpu.xcpu_nice[cpu_idx];
    
    total = cpu.xcpu_total[cpu_idx];
    
    if (total != uold_total) {
        upercent = 100.0f * (uload - uold_load) / (total - uold_total);
    }
    
    if (total != sold_total) {
        spercent = 100.0f * (sload - sold_load) / (total - sold_total);
    }
    
    tpercent = upercent + spercent;
    ipercent = ipercent - tpercent;
    
    uold_load = uload;
    sold_load = sload;
    uold_total = total;
    sold_total = total;
    
    buf->user_cpu_percentage = upercent;
    buf->system_cpu_percentage = spercent;
    buf->idle_cpu_percentage = ipercent;
}

/* -----------------------------------------------------------------------------
 * NET
 * -------------------------------------------------------------------------- */
    
char **
libsstats_get_netlist(libsstats_netlist *buf)
{
    struct if_nameindex *ifstart, *ifs;
	static char *devices[LIBSSTATS_MAX_NETDEVICES];
    
    memset (buf, 0, sizeof (libsstats_netlist));

    ifs = ifstart = if_nameindex();
    while(ifs && ifs->if_name) {
        devices[buf->number] = ifs->if_name;
        buf->number++;
        ifs++;
    }
    
    return (char **)devices;
}


void libsstats_get_netload(libsstats_netload *buf, const char *intf)
{
	int mib[] = { CTL_NET, PF_ROUTE, 0, 0, NET_RT_IFLIST, 0 };
    size_t bufsize;
	char *rtbuf, *ptr, *eob;
	struct if_msghdr *ifm;
        
	memset(buf, 0, sizeof (libsstats_netload));
	if (sysctl(mib, 6, NULL, &bufsize, NULL, 0) < 0)
		return;
    
	rtbuf = (char *)malloc(bufsize);
	if (rtbuf == NULL)
		return;
    
	if (sysctl(mib, 6, rtbuf, &bufsize, NULL, 0) < 0) {
		free(rtbuf);
		return;
	}
    
	eob = rtbuf + bufsize;
	ptr = rtbuf;
	while (ptr < eob) {
		struct sockaddr_dl *sdl;
        
		ifm = (struct if_msghdr *)ptr;
        
		if (ifm->ifm_type != RTM_IFINFO)
			break;
		ptr += ifm->ifm_msglen;
        
		while (ptr < eob) {
			struct if_msghdr *nextifm = (struct if_msghdr *)ptr;
            
			if (nextifm->ifm_type != RTM_NEWADDR)
				break;
			ptr += nextifm->ifm_msglen;
		}
        
		sdl = (struct sockaddr_dl *)(ifm + 1);
        if (sdl->sdl_family != AF_LINK)
			continue;
		
        if (strlen(intf) != sdl->sdl_nlen)
			continue;

        if (strncmp(intf, "en", 2) == 0)
        {
            if (strncmp(intf, sdl->sdl_data, 3) == 0)
                goto FOUND;
        }
        else
        {
            if (strcmp(intf, sdl->sdl_data) == 0)
			goto FOUND;
        }
	}
	free (rtbuf);
	return;
    
FOUND:
	if (ifm->ifm_flags & IFF_UP)
		buf->if_flags |= LIBSSTATS_IF_FLAGS_UP;
	if (ifm->ifm_flags & IFF_BROADCAST)
		buf->if_flags |= LIBSSTATS_IF_FLAGS_BROADCAST;
	if (ifm->ifm_flags & IFF_DEBUG)
		buf->if_flags |= LIBSSTATS_IF_FLAGS_DEBUG;
	if (ifm->ifm_flags & IFF_LOOPBACK)
		buf->if_flags |= LIBSSTATS_IF_FLAGS_LOOPBACK;
	if (ifm->ifm_flags & IFF_POINTOPOINT)
		buf->if_flags |= LIBSSTATS_IF_FLAGS_POINTOPOINT;
	if (ifm->ifm_flags & IFF_RUNNING)
		buf->if_flags |= LIBSSTATS_IF_FLAGS_RUNNING;
	if (ifm->ifm_flags & IFF_NOARP)
		buf->if_flags |= LIBSSTATS_IF_FLAGS_NOARP;
	if (ifm->ifm_flags & IFF_NOARP)
		buf->if_flags |= LIBSSTATS_IF_FLAGS_PROMISC;
	if (ifm->ifm_flags & IFF_ALLMULTI)
		buf->if_flags |= LIBSSTATS_IF_FLAGS_ALLMULTI;
	if (ifm->ifm_flags & IFF_OACTIVE)
		buf->if_flags |= LIBSSTATS_IF_FLAGS_OACTIVE;
	if (ifm->ifm_flags & IFF_SIMPLEX)
		buf->if_flags |= LIBSSTATS_IF_FLAGS_SIMPLEX;
	if (ifm->ifm_flags & IFF_LINK0)
		buf->if_flags |= LIBSSTATS_IF_FLAGS_LINK0;
	if (ifm->ifm_flags & IFF_LINK1)
		buf->if_flags |= LIBSSTATS_IF_FLAGS_LINK1;
	if (ifm->ifm_flags & IFF_LINK2)
		buf->if_flags |= LIBSSTATS_IF_FLAGS_LINK2;
	if (ifm->ifm_flags & IFF_ALTPHYS)
		buf->if_flags |= LIBSSTATS_IF_FLAGS_ALTPHYS;
	if (ifm->ifm_flags & IFF_MULTICAST)
		buf->if_flags |= LIBSSTATS_IF_FLAGS_MULTICAST;
	buf->mtu		= ifm->ifm_data.ifi_mtu;
	buf->subnet		= 0; /* FIXME */
	buf->address		= 0; /* FIXME */
	buf->packets_in		= ifm->ifm_data.ifi_ipackets;
	buf->packets_out	= ifm->ifm_data.ifi_opackets;
	buf->packets_total	= buf->packets_in + buf->packets_out;
	buf->bytes_in		= ifm->ifm_data.ifi_ibytes;
	buf->bytes_out		= ifm->ifm_data.ifi_obytes;
	buf->bytes_total	= buf->bytes_in + buf->bytes_out;
	buf->errors_in		= ifm->ifm_data.ifi_ierrors;
	buf->errors_out		= ifm->ifm_data.ifi_oerrors;
	buf->errors_total	= buf->errors_in + buf->errors_out;
	buf->collisions		= ifm->ifm_data.ifi_collisions;
}

    
#ifdef __cplusplus
}
#endif


























