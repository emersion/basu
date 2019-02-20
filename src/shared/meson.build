# SPDX-License-Identifier: LGPL-2.1+

shared_sources = files('''
        apparmor-util.c
        apparmor-util.h
        bus-util.c
        bus-util.h
        condition.c
        condition.h
        efivars.c
        efivars.h
        enable-mempool.c
        fdset.c
        fdset.h
        ima-util.c
        ima-util.h
        nsflags.c
        nsflags.h
        serialize.c
        serialize.h
        tests.c
        tests.h
        tomoyo-util.c
        tomoyo-util.h
'''.split())

if get_option('tests') != 'false'
        shared_sources += files('tests.c', 'tests.h')
endif

libshared_name = 'systemd-shared-@0@'.format(meson.project_version())

libshared_deps = [threads,
                  librt,
                  libcap,
                  libacl,
                  libcryptsetup,
                  libgcrypt,
                  libiptc,
                  libkmod,
                  libseccomp,
                  libselinux,
                  libidn,
                  libxz,
                  liblz4,
                  libblkid]

libshared_sym_path = '@0@/libshared.sym'.format(meson.current_source_dir())

libshared_static = static_library(
        libshared_name,
        shared_sources,
        include_directories : includes,
        dependencies : libshared_deps,
        c_args : ['-fvisibility=default'])
