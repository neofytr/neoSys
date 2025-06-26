#ifndef BASE_H
#define BASE_H

// Define NULL
#define NULL ((void *)0)

/*
 * ----------------------------------------------------------------------------
 * Symbol Visibility Reference (for shared libraries / kernel modularity)
 *
 * 1. static
 *    - Restricts symbol to the current translation unit (.c file)
 *    - Used for internal linkage; not visible to the linker across files
 *
 * 2. __attribute__((visibility("hidden")))
 *    - Symbol is not exported from the shared object
 *    - Usable within the shared object or kernel, but invisible to external users
 *    - Still allows use across multiple .c files of the same build
 *
 * 3. __attribute__((visibility("default")))
 *    - Symbol is exported and available outside (e.g., to user programs or other shared objects)
 *    - Used for defining kernel APIs (e.g., syscalls) visible to userland
 *
 * Usage of macros:
 *   - Use `public` for kernel APIs exposed to userland
 *   - Use `private` for internal-only functions/variables
 * ----------------------------------------------------------------------------
 */

#define public __attribute__((visibility("default")))  // functions/variables exported to the user (shell process) by the kernel
#define private static                                 // functions/variables private to a translation unit
#define internal __attribute__((visibility("hidden"))) // functions/variables internal to the kernel shared library object file

#endif // BASE_H
