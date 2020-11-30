/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

#define DEFAULT_TIMEOUT_USEC (90*USEC_PER_SEC)

/* Note that we use the new /run prefix here (instead of /var/run) since we require them to be aliases and that way we
 * become independent of /var being mounted */
#define DEFAULT_SYSTEM_BUS_ADDRESS "unix:path=/run/dbus/system_bus_socket"
#define DEFAULT_USER_BUS_ADDRESS_FMT "unix:path=%s/bus"

#define LONG_LINE_MAX (1U*1024U*1024U)
