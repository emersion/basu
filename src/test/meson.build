# SPDX-License-Identifier: LGPL-2.1-or-later

############################################################

tests += [
        [['src/libsystemd/sd-bus/test-bus-signature.c'],
         [],
         [threads]],

        [['src/libsystemd/sd-bus/test-bus-chat.c'],
         [],
         [threads]],

        [['src/libsystemd/sd-bus/test-bus-cleanup.c'],
         [],
         [threads]],

        [['src/libsystemd/sd-bus/test-bus-error.c'],
         [libshared_static,
          libsystemd_static],
         []],

        [['src/libsystemd/sd-bus/test-bus-server.c'],
         [],
         [threads]],

        [['src/libsystemd/sd-bus/test-bus-objects.c'],
         [],
         [threads]],

        [['src/libsystemd/sd-bus/test-bus-creds.c'],
         [],
         []],

        [['src/libsystemd/sd-bus/test-bus-match.c'],
         [],
         []],

        [['src/libsystemd/sd-bus/test-bus-benchmark.c'],
         [],
         [threads],
         '', 'manual'],
]

if cxx_cmd != ''
        tests += [
                [['src/libsystemd/sd-bus/test-bus-vtable-cc.cc'],
                 [],
                 []]
        ]
endif

