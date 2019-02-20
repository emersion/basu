# SPDX-License-Identifier: LGPL-2.1+

_systemd_headers = '''
        sd-bus.h
        sd-bus-protocol.h
        sd-bus-vtable.h
        sd-daemon.h
        sd-event.h
        sd-id128.h
        sd-messages.h
'''.split()

# https://github.com/mesonbuild/meson/issues/1633
systemd_headers = files(_systemd_headers)

install_headers(
        systemd_headers,
        '_sd-common.h',
        subdir : 'basu')


############################################################

opts = [['c'],
        ['c', '-ansi'],
        ['c', '-std=iso9899:1990'],
        ['c', '-std=iso9899:2011']]

if cc.has_argument('-std=iso9899:2017')
        opts += [['c', '-std=iso9899:2017']]
endif

cxx = find_program('c++', required : false)
if cxx.found()
        opts += [['c++'],
                 ['c++', '-std=c++98'],
                 ['c++', '-std=c++11']]
        if cc.has_argument('-std=c++14')
                opts += [['c++', '-std=c++14']]
        endif
        if cc.has_argument('-std=c++17')
                opts += [['c++', '-std=c++17']]
        endif
endif

foreach header : _systemd_headers + ['../libudev/libudev.h']
        foreach opt : opts
                name = ''.join(['cc-', header.split('/')[-1], ':'] + opt)
                if want_tests != 'false'
                        test(name,
                             check_compilation_sh,
                             args : cc.cmd_array() + ['-c', '-x'] + opt +
                                    ['-Werror', '-include',
                                     join_paths(meson.current_source_dir(), header)])
                endif
        endforeach
endforeach
