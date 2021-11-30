#ifndef FAULT_H
#define FAULT_H

#include "syscall.h"
#include "sched.h"
#include "put.h"
#include "stddef.h"
#include "slub.h"
#include "vm.h"
#include "buddy.h"

void do_page_fault();

#endif /* FAULT_H */