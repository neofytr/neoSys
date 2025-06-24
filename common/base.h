#ifndef BASE_H
#define BASE_H

#define NULL ((void *)0)

#define public         // these are functions/variables that are exported by the kernel to the user (syscall API)
#define private static // static means only for that translation unit; these are functions/variable used internally in the kernel

#endif