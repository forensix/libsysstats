/* -----------------------------------------------------------------------------
 *  sysstats.m
 *  sysstats
 *
 *  Created by Manuel Gebele on 02.08.11.
 *  Copyright 2011 Manuel Gebele. All rights reserved.
 *
 * -------------------------------------------------------------------------- */

#include "sysstats.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <syslog.h>
#include <dlfcn.h>

#include <ifaddrs.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/route.h>

#include <sys/socket.h> /* Needed for net/if.h ! */
#include <sys/types.h>
#include <sys/sysctl.h>

#include <mach/mach_init.h>
#include <mach/mach_host.h>
#include <mach/host_info.h>
#include <mach/vm_map.h>

#define IPCONFIGURATION_BUNDLE_PATH \
"/System/Library/SystemConfiguration/IPConfiguration.bundle/IPConfiguration"

#define CORE_TELEPHONY_PATH \
"/System/Library/Frameworks/CoreTelephony.framework/CoreTelephony"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
#pragma mark CPU
// -----------------------------------------------------------------------------

void
libsstats_get_cpu(libsstats_cpu *buf)
{
    processor_cpu_load_info_t  pinfo;
    mach_msg_type_number_t icount;
    natural_t pcount; /* processor count */
    
    memset (buf, 0, sizeof (libsstats_cpu));
    
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
        buf->xcpu_user[i]   = pinfo[i].cpu_ticks[CPU_STATE_USER];
		buf->xcpu_sys[i]    = pinfo[i].cpu_ticks[CPU_STATE_SYSTEM];
		buf->xcpu_idle[i]   = pinfo[i].cpu_ticks[CPU_STATE_IDLE];
		buf->xcpu_nice[i]   = pinfo[i].cpu_ticks[CPU_STATE_NICE];
		buf->xcpu_total[i]  = buf->xcpu_user[i] + buf->xcpu_sys[i] +
        
        buf->xcpu_idle[i]   + buf->xcpu_nice[i];
        
		buf->user           += buf->xcpu_user[i];
		buf->sys            += buf->xcpu_sys[i];
		buf->idle           += buf->xcpu_idle[i];
		buf->nice           += buf->xcpu_nice[i];
		buf->total          += buf->xcpu_total[i];
    }
    
    vm_deallocate (mach_task_self(), (vm_address_t)pinfo, icount);
    
	buf->frequency = 100;
}

void
libsstats_get_cpu_percentage(libsstats_cpu cpu, libsstats_cpu_percentage *buf,
                             unsigned cpu_idx)
{
    unsigned long user_load;
    unsigned long system_load;
    unsigned long total_load;
    static unsigned long user_prev_load = 0;
    static unsigned long system_prev_load = 0;
    static unsigned long user_prev_total = 0;
    static unsigned long system_prev_total = 0;
    float user_percent = .0f;
    float system_percent = .0f;
    float idle_percent = 100.0f;
    float total_percent = .0f;
    
    memset (buf, 0, sizeof (libsstats_cpu_percentage));

    user_load = cpu.xcpu_user[cpu_idx] + cpu.xcpu_nice[cpu_idx];
    system_load = cpu.xcpu_sys[cpu_idx] + cpu.xcpu_nice[cpu_idx];
    
    total_load = cpu.xcpu_total[cpu_idx];
    
    if (total_load != user_prev_total) {
        user_percent = 100.0f * (user_load - user_prev_load)
                              / (total_load - user_prev_total);
    }
    
    if (total_load != system_prev_total) {
        system_percent = 100.0f * (system_load - system_prev_load)
                                / (total_load - system_prev_total);
    }
    
    total_percent = user_percent + system_percent;
    idle_percent = idle_percent - total_percent;
    
    /* Store for further calculation. */
    user_prev_load = user_load;
    system_prev_load = system_load;
    user_prev_total = total_load;
    system_prev_total = total_load;
    
    buf->user_cpu_percentage = user_percent;
    buf->system_cpu_percentage = system_percent;
    buf->idle_cpu_percentage = idle_percent;
}

void
libsstats_get_loadavg(libsstats_loadavg *buf)
{
    double ldavg[3];
    int i;
    
    memset (buf, 0, sizeof (libsstats_loadavg));
    
    if (getloadavg (ldavg, 3) != 3) {
        return;
    }
    
    for (i = 0; i < 3; i++) {
        buf->loadavg[i] = ldavg[i];
    }    
}

// -----------------------------------------------------------------------------
#pragma mark Net
// -----------------------------------------------------------------------------

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
            if (strcmp(intf, sdl->sdl_data) == 0) // TODO
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
	buf->mtu            = ifm->ifm_data.ifi_mtu;
	buf->subnet         = 0; /* FIXME */
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

void
libsstats_get_mac(const char *intf, libsstats_mac *buf)
{
    int success;
    struct ifaddrs *addrs;
    struct ifaddrs *cursor;
    const struct sockaddr_dl *sock_addr;
    const unsigned char *base;
    int i;
    
    memset (buf, 0, sizeof (libsstats_mac));
    
    success = (getifaddrs(&addrs) == 0);
    if (!success) {
        return;
    }
    
    cursor = addrs;
    while (cursor != 0) {
        if ((cursor->ifa_addr->sa_family == AF_LINK)
            && (((const struct sockaddr_dl *)cursor->ifa_addr)->sdl_type == IFT_ETHER)
            && strcmp(intf,  cursor->ifa_name) == 0) {
            sock_addr = (const struct sockaddr_dl *)cursor->ifa_addr;
            base = (const unsigned char *)&sock_addr->sdl_data[sock_addr->sdl_nlen];
            strcpy(buf->macaddress, ""); 
            for (i = 0; i < sock_addr->sdl_alen; i++) {
                if (i != 0) {
                    strcat(buf->macaddress, ":");
                }
                char partialAddr[3];
                sprintf(partialAddr, "%02x", base[i]);  // TODO
                strcat(buf->macaddress, partialAddr);   // TODO
            }
        }
        cursor = cursor->ifa_next;
    }
    
    freeifaddrs(addrs);
}

void
libsstats_get_ip(const char *intf, libsstats_ip *buf)
{
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = 0;
    
    memset (buf, 0, sizeof (libsstats_ip));

    strncpy(buf->ipaddress, "N/A", 4);
    
    success = getifaddrs(&interfaces);
    if (success == 0)
    {
        temp_addr = interfaces;
        while(temp_addr != NULL)
        {
            if(temp_addr->ifa_addr->sa_family == AF_INET)
            {
                if (strcmp(temp_addr->ifa_name, intf) == 0) // TODO
                {
                    strcpy(buf->ipaddress,
                           inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr)); // TODO
                }
            }
            
            temp_addr = temp_addr->ifa_next;
        }
    }
    
    freeifaddrs(interfaces);
}

// -----------------------------------------------------------------------------
#pragma mark Memory
// -----------------------------------------------------------------------------

void
libsstats_get_mem(libsstats_mem *buf)
{
	vm_statistics_data_t vm_info;
	mach_msg_type_number_t info_count;
        
	memset (buf, 0, sizeof (libsstats_mem));
    
	info_count = HOST_VM_INFO_COUNT;
	if (host_statistics(mach_host_self(), HOST_VM_INFO,
                         (host_info_t)&vm_info, &info_count)) {
		return;
	}
    
    float active_count   = vm_info.active_count   * vm_page_size / 1048576.0;
    float inactive_count = vm_info.inactive_count * vm_page_size / 1048576.0;
    float wire_count     = vm_info.wire_count     * vm_page_size / 1048576.0;
    float total_count    = (vm_info.active_count
                         +  vm_info.inactive_count
                         +  vm_info.free_count
                         +  vm_info.wire_count)    * vm_page_size / 1048576.0;
    float free_count     = vm_info.free_count      * vm_page_size / 1048576.0;
    float used_count     = total_count - free_count               / 1048576.0;
    buf->active             = active_count;
    buf->inactive           = inactive_count;
    buf->wired              = wire_count;
    buf->total              = total_count;
    buf->free               = free_count;
    buf->used               = used_count;
}


// -----------------------------------------------------------------------------
#pragma mark Wireless
// -----------------------------------------------------------------------------

static int curr_scanning = 0;

void
libsstats_get_wireless(libsstats_wireless *buf)
{
    if (curr_scanning)
    {
        return;
    }
    curr_scanning = 1;

    memset (buf, 0, sizeof (libsstats_wireless));
   
    typedef int (*open_h)(void *);
    typedef int (*close_h)(void *);
    typedef int (*bind_h)(void *, CFStringRef);
    typedef int (*scan_h)(void *, CFArrayRef *, void *);

    open_h o;
    close_h c;
    bind_h b;
    scan_h s;
    
    void *handle = dlopen(IPCONFIGURATION_BUNDLE_PATH, RTLD_LAZY);
    if (!handle) goto DONE;
    
    *(void **)(&o) = dlsym(handle, "Apple80211Open");
    if (!o) goto DONE;
    
    *(void **)(&c) = dlsym(handle, "Apple80211Close");
    if (!c) goto DONE;
    
    *(void **)(&b) = dlsym(handle, "Apple80211BindToInterface");
    if (!b) goto DONE;
    
    *(void **)(&s) = dlsym(handle, "Apple80211Scan");
    if (!s) goto DONE;
    
    CFDictionaryRef parameters
    = CFDictionaryCreateMutable(
        NULL,
        0,
        &kCFTypeDictionaryKeyCallBacks,
        &kCFTypeDictionaryValueCallBacks
    );
    
    int retval;
    
    // Open
    retval = o(&handle);
    if (retval) goto DONE;
    
    // Bind
    retval = b(handle, CFSTR("en0"));
    if (retval) goto DONE;
    
    CFArrayRef networks;

    // Scan
    retval = s(handle, &networks, (void *)parameters);
    if (retval) goto DONE;
    
    // Info
    int i;
    for (i = 0; i < CFArrayGetCount(networks) && i < 256; i++) {
        CFDictionaryRef network
        = (CFDictionaryRef)CFArrayGetValueAtIndex(networks, i);
        buf->networks[i] = network;
        buf->number++;
    }
    
    // Close
    c(handle);
    dlclose(handle);    
    
DONE:
    curr_scanning = 0;
}

// -----------------------------------------------------------------------------
#pragma mark Cellular Data
// -----------------------------------------------------------------------------

void
libsstats_get_cellular(libsstats_cellular *buf)
{
    memset (buf, 0, sizeof(libsstats_cellular));
    void *libHandle = dlopen(CORE_TELEPHONY_PATH, RTLD_LAZY);
	
    int (*CTGetSignalStrength)();
	CTGetSignalStrength = dlsym(libHandle, "CTGetSignalStrength");
	if (CTGetSignalStrength == NULL) return;	
	
    buf->rssi = CTGetSignalStrength();
	
    dlclose(libHandle);	
}

void libsstats_get_processinfo(libsstats_processinfo *buf)
{
    int mib[5];
    struct kinfo_proc *procs = NULL, *newprocs;
    int i, st, nprocs;
    size_t miblen, size;
    
    memset (buf, 0, sizeof (libsstats_processinfo));

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_ALL;
    mib[3] = 0;
    miblen = 4;
    
    st = sysctl(mib, miblen, NULL, &size, NULL, 0);
    do {
        size += size / 10;
        newprocs = realloc(procs, size);
        if (!newprocs) {
            if (procs) {
                free(procs);
            }
            
            syslog(1, "libsysstats: Error: realloc failed.");
            return;
        }
        
        procs = newprocs;
        st = sysctl(mib, miblen, procs, &size, NULL, 0);
    } while (st == -1 && errno == ENOMEM);
    
    if (st != 0) {
        syslog(1, "libsysstats: Error: sysctl(KERN_PROC) failed.");
        return;
    }
    
    if (size % sizeof(struct kinfo_proc) != 0) {
        return;
    }
    nprocs = size / sizeof(struct kinfo_proc);
    
    if (!nprocs) {
        syslog(1, "libsysstats: !nprocs");
        return;
    }

    /* Get all processes. */
    for (i = nprocs - 1; i >= 0;  i--) {
        time_t now;
        libsstats_process proc;
        strncpy(proc.name, procs[i].kp_proc.p_comm, 256 - 1);
        proc.pid = (int)procs[i].kp_proc.p_pid;
        proc.priority = (u_char)(procs[i].kp_proc.p_priority);
        
        time (&now);

        proc.run_time = now - procs[i].kp_proc.p_starttime.tv_sec;
        
        switch (procs[i].kp_proc.p_stat) {
        case SIDL:
            proc.state = LIBSSTATS_PROC_IDLE;
            break;
        case SRUN:
            proc.state = LIBSSTATS_PROC_RUN;
            break;
        case SSLEEP:
            proc.state = LIBSSTATS_PROC_SLEEP;
            break;
        case SSTOP:
            proc.state = LIBSSTATS_PROC_STOP;
            break;
        case SZOMB:
            proc.state = LIBSSTATS_PROC_ZOMBIE;
            break;
        default:
            proc.state = LIBSSTATS_PROC_UNKNOWN;
            break;
        }
        
        buf->processes[buf->number] = proc;
        buf->number++;
    }
    
    free(procs);
}

// -----------------------------------------------------------------------------
#pragma mark Uptime
// -----------------------------------------------------------------------------

void libsstats_get_uptime(libsstats_uptime *buf)
{
    int mib[] = { CTL_KERN, KERN_BOOTTIME };
	struct timeval boottime;
	size_t size = sizeof (boottime);
	time_t now;
    	
	memset (buf, 0, sizeof (libsstats_uptime));
        
	if (sysctl (mib, 2, &boottime, &size, NULL, 0) == -1)
		return;
	time (&now);
    
    buf->boot_time = boottime.tv_sec;
	buf->uptime = now - boottime.tv_sec + 30;    
}

#ifdef __cplusplus
}
#endif
