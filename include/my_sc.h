#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/cred.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include "const.h"

int tag_get(int key, int command, int permission);