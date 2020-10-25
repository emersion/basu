/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

typedef enum CopyFlags {
        COPY_REFLINK    = 1 << 0, /* Try to reflink */
        COPY_MERGE      = 1 << 1, /* Merge existing trees with our new one to copy */
        COPY_REPLACE    = 1 << 2, /* Replace an existing file if there's one */
        COPY_SAME_MOUNT = 1 << 3, /* Don't descend recursively into other file systems, across mount point boundaries */
} CopyFlags;

int copy_bytes_full(int fdf, int fdt, uint64_t max_bytes, CopyFlags copy_flags, void **ret_remains, size_t *ret_remains_size);
static inline int copy_bytes(int fdf, int fdt, uint64_t max_bytes, CopyFlags copy_flags) {
        return copy_bytes_full(fdf, fdt, max_bytes, copy_flags, NULL, NULL);
}
