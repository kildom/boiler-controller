#ifndef _GLOBAL_HH_
#define _GLOBAL_HH_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "lowlevel.h"

static struct _GlobalInit {
	_GlobalInit() {
		global_init();
	}
} _global_init_call;

#endif

