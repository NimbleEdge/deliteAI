/*
 * SPDX-FileCopyrightText: (C) 2025 DeliteAI Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <sys/sysctl.h>
#include <errno.h>
#include <stdio.h>
#import "CrashReporterUtil.h"


@implementation CrashReporterUtil : NSObject
+ (bool)is_debugger_attached {
#if !TARGET_OS_IPHONE
    return false;
#endif

    struct kinfo_proc info;
    size_t info_size = sizeof(info);
    int name[4];
    
    name[0] = CTL_KERN;
    name[1] = KERN_PROC;
    name[2] = KERN_PROC_PID;
    name[3] = getpid();
    
    if (sysctl(name, 4, &info, &info_size, NULL, 0) == -1) {
        printf("sysctl() failed: %s\n", strerror(errno));
        return false;
    }

    if ((info.kp_proc.p_flag & P_TRACED) != 0)
        return true;
    
    return false;
}


@end

