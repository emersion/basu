project('basu', 'c',
  version: '239',
  license: 'LGPLv2+',
  default_options: [
    'c_std=gnu99',
    'prefix=/usr',
    'sysconfdir=/etc',
    'localstatedir=/var',
  ],
  meson_version: '>= 0.46',
)

libsystemd_version = '0.23.0'

# We need the same data in two different formats, ugh!
# Also, for hysterical reasons, we use different variable
# names, sometimes. Not all variables are included in every
# set. Ugh, ugh, ugh!
conf = configuration_data()
conf.set_quoted('PACKAGE_STRING',  meson.project_name() + ' ' + meson.project_version())
conf.set_quoted('PACKAGE_VERSION', meson.project_version())

substs = configuration_data()
substs.set('PACKAGE_URL',          'https://github.com/emersion/basu')
substs.set('PACKAGE_VERSION',      meson.project_version())

#####################################################################

if get_option('split-usr') == 'auto'
        split_usr = run_command('test', '-L', '/bin').returncode() != 0
else
        split_usr = get_option('split-usr') == 'true'
endif
conf.set10('HAVE_SPLIT_USR', split_usr,
           description : '/usr/bin and /bin directories are separate')

if get_option('split-bin') == 'auto'
        split_bin = run_command('test', '-L', '/usr/sbin').returncode() != 0
else
        split_bin = get_option('split-bin') == 'true'
endif
conf.set10('HAVE_SPLIT_BIN', split_bin,
           description : 'bin and sbin directories are separate')

rootprefixdir = get_option('rootprefix')
# Unusual rootprefixdir values are used by some distros
# (see https://github.com/systemd/systemd/pull/7461).
rootprefix_default = split_usr ? '/' : '/usr'
if rootprefixdir == ''
        rootprefixdir = rootprefix_default
endif

sysvinit_path = get_option('sysvinit-path')
sysvrcnd_path = get_option('sysvrcnd-path')
conf.set10('HAVE_SYSV_COMPAT', sysvinit_path != '' and sysvrcnd_path != '',
           description : 'SysV init scripts and rcN.d links are supported')

conf.set10('BUMP_PROC_SYS_FS_FILE_MAX', get_option('bump-proc-sys-fs-file-max'))
conf.set10('BUMP_PROC_SYS_FS_NR_OPEN',  get_option('bump-proc-sys-fs-nr-open'))
conf.set('HIGH_RLIMIT_NOFILE',          256*1024)

# join_paths ignore the preceding arguments if an absolute component is
# encountered, so this should canonicalize various paths when they are
# absolute or relative.
prefixdir = get_option('prefix')
if not prefixdir.startswith('/')
        error('Prefix is not absolute: "@0@"'.format(prefixdir))
endif
bindir = join_paths(prefixdir, get_option('bindir'))
libdir = join_paths(prefixdir, get_option('libdir'))
sysconfdir = join_paths(prefixdir, get_option('sysconfdir'))
includedir = join_paths(prefixdir, get_option('includedir'))
datadir = join_paths(prefixdir, get_option('datadir'))
localstatedir = join_paths('/', get_option('localstatedir'))

rootbindir = join_paths(rootprefixdir, 'bin')
rootsbindir = join_paths(rootprefixdir, split_bin ? 'sbin' : 'bin')
rootlibexecdir = join_paths(rootprefixdir, 'lib/systemd')

rootlibdir = get_option('rootlibdir')
if rootlibdir == ''
        rootlibdir = join_paths(rootprefixdir, libdir.split('/')[-1])
endif

# Dirs of external packages
pkgconfigdatadir = get_option('pkgconfigdatadir') == '' ? join_paths(datadir, 'pkgconfig') : get_option('pkgconfigdatadir')
pkgconfiglibdir = get_option('pkgconfiglibdir') == '' ? join_paths(libdir, 'pkgconfig') : get_option('pkgconfiglibdir')
polkitpolicydir = join_paths(datadir, 'polkit-1/actions')
polkitrulesdir = join_paths(datadir, 'polkit-1/rules.d')
polkitpkladir = join_paths(localstatedir, 'lib/polkit-1/localauthority/10-vendor.d')
varlogdir = join_paths(localstatedir, 'log')
xinitrcdir = join_paths(sysconfdir, 'X11/xinit/xinitrc.d')
rpmmacrosdir = get_option('rpmmacrosdir')
if rpmmacrosdir != 'no'
        rpmmacrosdir = join_paths(prefixdir, rpmmacrosdir)
endif
modprobedir = join_paths(rootprefixdir, 'lib/modprobe.d')

# Our own paths
pkgdatadir = join_paths(datadir, 'systemd')
environmentdir = join_paths(prefixdir, 'lib/environment.d')
pkgsysconfdir = join_paths(sysconfdir, 'systemd')
userunitdir = join_paths(prefixdir, 'lib/systemd/user')
userpresetdir = join_paths(prefixdir, 'lib/systemd/user-preset')
tmpfilesdir = join_paths(prefixdir, 'lib/tmpfiles.d')
sysusersdir = join_paths(prefixdir, 'lib/sysusers.d')
sysctldir = join_paths(prefixdir, 'lib/sysctl.d')
binfmtdir = join_paths(prefixdir, 'lib/binfmt.d')
modulesloaddir = join_paths(prefixdir, 'lib/modules-load.d')
networkdir = join_paths(rootprefixdir, 'lib/systemd/network')
pkgincludedir = join_paths(includedir, 'systemd')
systemgeneratordir = join_paths(rootlibexecdir, 'system-generators')
usergeneratordir = join_paths(prefixdir, 'lib/systemd/user-generators')
systemenvgeneratordir = join_paths(prefixdir, 'lib/systemd/system-environment-generators')
userenvgeneratordir = join_paths(prefixdir, 'lib/systemd/user-environment-generators')
systemshutdowndir = join_paths(rootlibexecdir, 'system-shutdown')
systemsleepdir = join_paths(rootlibexecdir, 'system-sleep')
systemunitdir = join_paths(rootprefixdir, 'lib/systemd/system')
systempresetdir = join_paths(rootprefixdir, 'lib/systemd/system-preset')
udevlibexecdir = join_paths(rootprefixdir, 'lib/udev')
udevhomedir = udevlibexecdir
udevrulesdir = join_paths(udevlibexecdir, 'rules.d')
udevhwdbdir = join_paths(udevlibexecdir, 'hwdb.d')
catalogdir = join_paths(prefixdir, 'lib/systemd/catalog')
kernelinstalldir = join_paths(prefixdir, 'lib/kernel/install.d')
factorydir = join_paths(datadir, 'factory')
bootlibdir = join_paths(prefixdir, 'lib/systemd/boot/efi')
testsdir = join_paths(prefixdir, 'lib/systemd/tests')
systemdstatedir = join_paths(localstatedir, 'lib/systemd')
catalogstatedir = join_paths(systemdstatedir, 'catalog')
randomseeddir = join_paths(localstatedir, 'lib/systemd')
profiledir = join_paths(rootlibexecdir, 'portable', 'profile')

docdir = get_option('docdir')
if docdir == ''
        docdir = join_paths(datadir, 'doc/systemd')
endif

dbuspolicydir = get_option('dbuspolicydir')
if dbuspolicydir == ''
        dbuspolicydir = join_paths(datadir, 'dbus-1/system.d')
endif

dbussessionservicedir = get_option('dbussessionservicedir')
if dbussessionservicedir == ''
        dbussessionservicedir = join_paths(datadir, 'dbus-1/services')
endif

dbussystemservicedir = get_option('dbussystemservicedir')
if dbussystemservicedir == ''
        dbussystemservicedir = join_paths(datadir, 'dbus-1/system-services')
endif

pamlibdir = get_option('pamlibdir')
if pamlibdir == ''
        pamlibdir = join_paths(rootlibdir, 'security')
endif

pamconfdir = get_option('pamconfdir')
if pamconfdir == ''
        pamconfdir = join_paths(sysconfdir, 'pam.d')
endif

memory_accounting_default = get_option('memory-accounting-default')

conf.set_quoted('PKGSYSCONFDIR',                              pkgsysconfdir)
conf.set_quoted('SYSTEM_CONFIG_UNIT_PATH',                    join_paths(pkgsysconfdir, 'system'))
conf.set_quoted('SYSTEM_DATA_UNIT_PATH',                      systemunitdir)
conf.set_quoted('SYSTEM_SYSVINIT_PATH',                       sysvinit_path)
conf.set_quoted('SYSTEM_SYSVRCND_PATH',                       sysvrcnd_path)
conf.set_quoted('RC_LOCAL_SCRIPT_PATH_START',                 get_option('rc-local'))
conf.set_quoted('RC_LOCAL_SCRIPT_PATH_STOP',                  get_option('halt-local'))

conf.set('ANSI_OK_COLOR',                                     'ANSI_' + get_option('ok-color').underscorify().to_upper())

conf.set_quoted('USER_CONFIG_UNIT_PATH',                      join_paths(pkgsysconfdir, 'user'))
conf.set_quoted('USER_DATA_UNIT_PATH',                        userunitdir)
conf.set_quoted('CERTIFICATE_ROOT',                           get_option('certificate-root'))
conf.set_quoted('CATALOG_DATABASE',                           join_paths(catalogstatedir, 'database'))
conf.set_quoted('SYSTEMD_CGROUP_AGENT_PATH',                  join_paths(rootlibexecdir, 'systemd-cgroups-agent'))
conf.set_quoted('SYSTEMD_BINARY_PATH',                        join_paths(rootlibexecdir, 'systemd'))
conf.set_quoted('SYSTEMD_FSCK_PATH',                          join_paths(rootlibexecdir, 'systemd-fsck'))
conf.set_quoted('SYSTEMD_MAKEFS_PATH',                        join_paths(rootlibexecdir, 'systemd-makefs'))
conf.set_quoted('SYSTEMD_GROWFS_PATH',                        join_paths(rootlibexecdir, 'systemd-growfs'))
conf.set_quoted('SYSTEMD_SHUTDOWN_BINARY_PATH',               join_paths(rootlibexecdir, 'systemd-shutdown'))
conf.set_quoted('SYSTEMD_SLEEP_BINARY_PATH',                  join_paths(rootlibexecdir, 'systemd-sleep'))
conf.set_quoted('SYSTEMCTL_BINARY_PATH',                      join_paths(rootbindir, 'systemctl'))
conf.set_quoted('SYSTEMD_TTY_ASK_PASSWORD_AGENT_BINARY_PATH', join_paths(rootbindir, 'systemd-tty-ask-password-agent'))
conf.set_quoted('SYSTEMD_STDIO_BRIDGE_BINARY_PATH',           join_paths(bindir, 'systemd-stdio-bridge'))
conf.set_quoted('ROOTPREFIX',                                 rootprefixdir)
conf.set_quoted('RANDOM_SEED_DIR',                            randomseeddir)
conf.set_quoted('RANDOM_SEED',                                join_paths(randomseeddir, 'random-seed'))
conf.set_quoted('SYSTEMD_CRYPTSETUP_PATH',                    join_paths(rootlibexecdir, 'systemd-cryptsetup'))
conf.set_quoted('SYSTEM_GENERATOR_PATH',                      systemgeneratordir)
conf.set_quoted('USER_GENERATOR_PATH',                        usergeneratordir)
conf.set_quoted('SYSTEM_ENV_GENERATOR_PATH',                  systemenvgeneratordir)
conf.set_quoted('USER_ENV_GENERATOR_PATH',                    userenvgeneratordir)
conf.set_quoted('SYSTEM_SHUTDOWN_PATH',                       systemshutdowndir)
conf.set_quoted('SYSTEM_SLEEP_PATH',                          systemsleepdir)
conf.set_quoted('SYSTEMD_KBD_MODEL_MAP',                      join_paths(pkgdatadir, 'kbd-model-map'))
conf.set_quoted('SYSTEMD_LANGUAGE_FALLBACK_MAP',              join_paths(pkgdatadir, 'language-fallback-map'))
conf.set_quoted('SYSTEMD_TEST_DATA',                          join_paths(testsdir, 'testdata'))
conf.set_quoted('SYSTEMD_CATALOG_DIR',                        catalogdir)
conf.set_quoted('UDEVLIBEXECDIR',                             udevlibexecdir)
conf.set_quoted('POLKIT_AGENT_BINARY_PATH',                   join_paths(bindir, 'pkttyagent'))
conf.set_quoted('LIBDIR',                                     libdir)
conf.set_quoted('ROOTLIBDIR',                                 rootlibdir)
conf.set_quoted('ROOTLIBEXECDIR',                             rootlibexecdir)
conf.set_quoted('BOOTLIBDIR',                                 bootlibdir)
conf.set_quoted('SYSTEMD_PULL_PATH',                          join_paths(rootlibexecdir, 'systemd-pull'))
conf.set_quoted('SYSTEMD_IMPORT_PATH',                        join_paths(rootlibexecdir, 'systemd-import'))
conf.set_quoted('SYSTEMD_EXPORT_PATH',                        join_paths(rootlibexecdir, 'systemd-export'))
conf.set_quoted('VENDOR_KEYRING_PATH',                        join_paths(rootlibexecdir, 'import-pubring.gpg'))
conf.set_quoted('USER_KEYRING_PATH',                          join_paths(pkgsysconfdir, 'import-pubring.gpg'))
conf.set_quoted('DOCUMENT_ROOT',                              join_paths(pkgdatadir, 'gatewayd'))
conf.set10('MEMORY_ACCOUNTING_DEFAULT',                       memory_accounting_default)
conf.set_quoted('MEMORY_ACCOUNTING_DEFAULT_YES_NO',           memory_accounting_default ? 'yes' : 'no')

substs.set('prefix',                                          prefixdir)
substs.set('exec_prefix',                                     prefixdir)
substs.set('libdir',                                          libdir)
substs.set('rootlibdir',                                      rootlibdir)
substs.set('includedir',                                      includedir)
substs.set('pkgsysconfdir',                                   pkgsysconfdir)
substs.set('bindir',                                          bindir)
substs.set('rootbindir',                                      rootbindir)
substs.set('rootlibexecdir',                                  rootlibexecdir)
substs.set('systemunitdir',                                   systemunitdir)
substs.set('userunitdir',                                     userunitdir)
substs.set('systempresetdir',                                 systempresetdir)
substs.set('userpresetdir',                                   userpresetdir)
substs.set('udevhwdbdir',                                     udevhwdbdir)
substs.set('udevrulesdir',                                    udevrulesdir)
substs.set('udevlibexecdir',                                  udevlibexecdir)
substs.set('environmentdir',                                  environmentdir)
substs.set('catalogdir',                                      catalogdir)
substs.set('tmpfilesdir',                                     tmpfilesdir)
substs.set('sysusersdir',                                     sysusersdir)
substs.set('sysctldir',                                       sysctldir)
substs.set('binfmtdir',                                       binfmtdir)
substs.set('modulesloaddir',                                  modulesloaddir)
substs.set('modprobedir',                                     modprobedir)
substs.set('systemgeneratordir',                              systemgeneratordir)
substs.set('usergeneratordir',                                usergeneratordir)
substs.set('systemenvgeneratordir',                           systemenvgeneratordir)
substs.set('userenvgeneratordir',                             userenvgeneratordir)
substs.set('systemshutdowndir',                               systemshutdowndir)
substs.set('systemsleepdir',                                  systemsleepdir)
substs.set('VARLOGDIR',                                       varlogdir)
substs.set('CERTIFICATEROOT',                                 get_option('certificate-root'))
substs.set('SYSTEMCTL',                                       join_paths(rootbindir, 'systemctl'))
substs.set('RANDOM_SEED',                                     join_paths(randomseeddir, 'random-seed'))
substs.set('SYSTEM_SYSVINIT_PATH',                            sysvinit_path)
substs.set('SYSTEM_SYSVRCND_PATH',                            sysvrcnd_path)
substs.set('RC_LOCAL_SCRIPT_PATH_START',                      get_option('rc-local'))
substs.set('RC_LOCAL_SCRIPT_PATH_STOP',                       get_option('halt-local'))
substs.set('MEMORY_ACCOUNTING_DEFAULT',                       memory_accounting_default ? 'yes' : 'no')
substs.set('HIGH_RLIMIT_NOFILE',                              conf.get('HIGH_RLIMIT_NOFILE'))

#####################################################################

cc = meson.get_compiler('c')
pkgconfig = import('pkgconfig')

want_tests = get_option('tests')
slow_tests = want_tests != 'false' and get_option('slow-tests')
install_tests = get_option('install-tests')

cxx_cmd = ''
fuzzer_build = false

possible_cc_flags = [
        '-Wextra',
        '-Werror=undef',
        '-Wlogical-op',
        '-Wmissing-include-dirs',
        '-Wold-style-definition',
        '-Wpointer-arith',
        '-Winit-self',
        '-Wfloat-equal',
        '-Wsuggest-attribute=noreturn',
        '-Werror=missing-prototypes',
        '-Werror=implicit-function-declaration',
        '-Werror=missing-declarations',
        '-Werror=return-type',
        '-Werror=incompatible-pointer-types',
        '-Werror=format=2',
        '-Wstrict-prototypes',
        '-Wredundant-decls',
        '-Wmissing-noreturn',
        '-Wimplicit-fallthrough=5',
        '-Wshadow',
        '-Wendif-labels',
        '-Wstrict-aliasing=2',
        '-Wwrite-strings',
        '-Werror=overflow',
        '-Werror=shift-count-overflow',
        '-Werror=shift-overflow=2',
        '-Wdate-time',
        '-Wnested-externs',

        # negative arguments are correctly detected starting with meson 0.46.
        '-Wno-unused-parameter',
        '-Wno-missing-field-initializers',
        '-Wno-unused-result',
        '-Wno-format-signedness',

        # work-around for gcc 7.1 turning this on on its own.
        '-Wno-error=nonnull',

        # Disable -Wmaybe-uninitialized, since it's noisy on gcc 8 with
        # optimizations enabled, producing essentially false positives.
        '-Wno-maybe-uninitialized',

        '-ffast-math',
        '-fno-common',
        '-fdiagnostics-show-option',
        '-fno-strict-aliasing',
        '-fvisibility=hidden',
        '-fstack-protector',
        '-fstack-protector-strong',
        '--param=ssp-buffer-size=4',
]

# --as-needed and --no-undefined are provided by meson by default,
# run mesonconf to see what is enabled
possible_link_flags = [
        '-Wl,-z,relro',
        '-Wl,-z,now',
]

possible_cc_flags += '-fPIE'
possible_link_flags += '-pie'

if cc.get_id() == 'clang'
        possible_cc_flags += [
                '-Wno-typedef-redefinition',
                '-Wno-gnu-variable-sized-type-not-at-end',
        ]
endif

if get_option('buildtype') != 'debug'
        possible_cc_flags += [
                '-ffunction-sections',
                '-fdata-sections',
        ]

        possible_link_flags += '-Wl,--gc-sections'
endif

add_project_arguments(cc.get_supported_arguments(possible_cc_flags), language : 'c')
add_project_link_arguments(cc.get_supported_link_arguments(possible_link_flags), language : 'c')

if cc.compiles('''
   #include <time.h>
   #include <inttypes.h>
   typedef uint64_t usec_t;
   usec_t now(clockid_t clock);
   int main(void) {
           struct timespec now;
           return 0;
   }
''', name : '-Werror=shadow with local shadowing')
        add_project_arguments('-Werror=shadow', language : 'c')
endif

cpp = ' '.join(cc.cmd_array()) + ' -E'

#####################################################################
# compilation result tests

conf.set('_GNU_SOURCE', true)
conf.set('__SANE_USERSPACE_TYPES__', true)

conf.set('SIZEOF_PID_T', cc.sizeof('pid_t', prefix : '#include <sys/types.h>'))
conf.set('SIZEOF_UID_T', cc.sizeof('uid_t', prefix : '#include <sys/types.h>'))
conf.set('SIZEOF_GID_T', cc.sizeof('gid_t', prefix : '#include <sys/types.h>'))
conf.set('SIZEOF_DEV_T', cc.sizeof('dev_t', prefix : '#include <sys/types.h>'))
conf.set('SIZEOF_INO_T', cc.sizeof('ino_t', prefix : '#include <sys/types.h>'))
conf.set('SIZEOF_TIME_T', cc.sizeof('time_t', prefix : '#include <sys/time.h>'))
conf.set('SIZEOF_RLIM_T', cc.sizeof('rlim_t', prefix : '#include <sys/resource.h>'))

decl_headers = '''
#include <uchar.h>
#include <linux/ethtool.h>
#include <linux/fib_rules.h>
#include <sys/stat.h>
'''

foreach decl : ['char16_t',
                'char32_t',
                'struct ethtool_link_settings',
                'struct fib_rule_uid_range',
                'struct statx',
               ]

        # We get -1 if the size cannot be determined
        have = cc.sizeof(decl, prefix : decl_headers, args : '-D_GNU_SOURCE') > 0

        if decl == 'struct statx'
                if have
                        want_linux_stat_h = false
                else
                        have = cc.sizeof(decl,
                                         prefix : decl_headers + '#include <linux/stat.h>',
                                         args : '-D_GNU_SOURCE') > 0
                        want_linux_stat_h = have
                endif
        endif

        conf.set10('HAVE_' + decl.underscorify().to_upper(), have)
endforeach

conf.set10('WANT_LINUX_STAT_H', want_linux_stat_h)

foreach decl : [['IFLA_INET6_ADDR_GEN_MODE',         'linux/if_link.h'],
                ['IN6_ADDR_GEN_MODE_STABLE_PRIVACY', 'linux/if_link.h'],
                ['IFLA_VRF_TABLE',                   'linux/if_link.h'],
                ['IFLA_MACVLAN_FLAGS',               'linux/if_link.h'],
                ['IFLA_IPVLAN_FLAGS',                'linux/if_link.h'],
                ['IFLA_PHYS_PORT_ID',                'linux/if_link.h'],
                ['IFLA_BOND_AD_ACTOR_SYSTEM',        'linux/if_link.h'],
                ['IFLA_VLAN_PROTOCOL',               'linux/if_link.h'],
                ['IFLA_VXLAN_REMCSUM_NOPARTIAL',     'linux/if_link.h'],
                ['IFLA_VXLAN_GPE',                   'linux/if_link.h'],
                ['IFLA_GENEVE_LABEL',                'linux/if_link.h'],
                # if_tunnel.h is buggy and cannot be included on its own
                ['IFLA_VTI_REMOTE',                  'linux/if_tunnel.h', '#include <net/if.h>'],
                ['IFLA_IPTUN_ENCAP_DPORT',           'linux/if_tunnel.h', '#include <net/if.h>'],
                ['IFLA_GRE_ENCAP_DPORT',             'linux/if_tunnel.h', '#include <net/if.h>'],
                ['IFLA_BRIDGE_VLAN_INFO',            'linux/if_bridge.h'],
                ['IFLA_BRPORT_PROXYARP',             'linux/if_link.h'],
                ['IFLA_BRPORT_LEARNING_SYNC',        'linux/if_link.h'],
                ['IFLA_BR_VLAN_DEFAULT_PVID',        'linux/if_link.h'],
                ['IPVLAN_F_PRIVATE',                 'linux/if_link.h'],
                ['NDA_IFINDEX',                      'linux/neighbour.h'],
                ['IFA_FLAGS',                        'linux/if_addr.h'],
                ['FRA_UID_RANGE',                    'linux/fib_rules.h'],
                ['LO_FLAGS_PARTSCAN',                'linux/loop.h'],
                ['VXCAN_INFO_PEER',                  'linux/can/vxcan.h'],
                ['FOU_ATTR_REMCSUM_NOPARTIAL',       'linux/fou.h'],
                ['FOU_CMD_GET',                      'linux/fou.h'],
               ]
        prefix = decl.length() > 2 ? decl[2] : ''
        have = cc.has_header_symbol(decl[1], decl[0], prefix : prefix)
        conf.set10('HAVE_' + decl[0], have)
endforeach

foreach ident : ['secure_getenv', '__secure_getenv']
        conf.set10('HAVE_' + ident.to_upper(), cc.has_function(ident))
endforeach

foreach ident : [
        ['memfd_create',      '''#include <sys/mman.h>'''],
        ['gettid',            '''#include <sys/types.h>
                                 #include <unistd.h>'''],
        ['pivot_root',        '''#include <stdlib.h>
                                 #include <unistd.h>'''],     # no known header declares pivot_root
        ['name_to_handle_at', '''#include <sys/types.h>
                                 #include <sys/stat.h>
                                 #include <fcntl.h>'''],
        ['setns',             '''#include <sched.h>'''],
        ['renameat2',         '''#include <stdio.h>
                                 #include <fcntl.h>'''],
        ['kcmp',              '''#include <linux/kcmp.h>'''],
        ['keyctl',            '''#include <sys/types.h>
                                 #include <keyutils.h>'''],
        ['copy_file_range',   '''#include <sys/syscall.h>
                                 #include <unistd.h>'''],
        ['bpf',               '''#include <sys/syscall.h>
                                 #include <unistd.h>'''],
        ['statx',             '''#include <sys/types.h>
                                 #include <sys/stat.h>
                                 #include <unistd.h>'''],
        ['explicit_bzero' ,   '''#include <string.h>'''],
        ['reallocarray',      '''#include <malloc.h>'''],
]

        have = cc.has_function(ident[0], prefix : ident[1], args : '-D_GNU_SOURCE')
        conf.set10('HAVE_' + ident[0].to_upper(), have)
endforeach

if cc.has_function('getrandom', prefix : '''#include <sys/random.h>''', args : '-D_GNU_SOURCE')
        conf.set10('USE_SYS_RANDOM_H', true)
        conf.set10('HAVE_GETRANDOM', true)
else
        have = cc.has_function('getrandom', prefix : '''#include <linux/random.h>''')
        conf.set10('USE_SYS_RANDOM_H', false)
        conf.set10('HAVE_GETRANDOM', have)
endif

#####################################################################

sed = find_program('sed')
awk = find_program('awk')
m4 = find_program('m4')
stat = find_program('stat')
git = find_program('git', required : false)
env = find_program('env')
perl = find_program('perl', required : false)

mkdir_p = 'mkdir -p $DESTDIR/@0@'

# if -Dxxx-path option is found, use that. Otherwise, check in $PATH,
# /usr/sbin, /sbin, and fall back to the default from middle column.
progs = [['quotaon',    '/usr/sbin/quotaon'    ],
         ['quotacheck', '/usr/sbin/quotacheck' ],
         ['kmod',       '/usr/bin/kmod'        ],
         ['kexec',      '/usr/sbin/kexec'      ],
         ['sulogin',    '/usr/sbin/sulogin'    ],
         ['mount',      '/usr/bin/mount',      'MOUNT_PATH'],
         ['umount',     '/usr/bin/umount',     'UMOUNT_PATH'],
         ['loadkeys',   '/usr/bin/loadkeys',   'KBD_LOADKEYS'],
         ['setfont',    '/usr/bin/setfont',    'KBD_SETFONT'],
        ]
foreach prog : progs
        path = get_option(prog[0] + '-path')
        if path != ''
                message('Using @1@ for @0@'.format(prog[0], path))
        else
                exe = find_program(prog[0],
                                   '/usr/sbin/' + prog[0],
                                   '/sbin/' + prog[0],
                                   required: false)
                path = exe.found() ? exe.path() : prog[1]
        endif
        name = prog.length() > 2 ? prog[2] : prog[0].to_upper()
        conf.set_quoted(name, path)
        substs.set(name, path)
endforeach

conf.set_quoted('TELINIT', get_option('telinit-path'))

if run_command('ln', '--relative', '--help').returncode() != 0
        error('ln does not support --relative (added in coreutils 8.16)')
endif

############################################################

gperf = find_program('gperf')

gperf_test_format = '''
#include <string.h>
const char * in_word_set(const char *, @0@);
@1@
'''
gperf_snippet_format = 'echo foo,bar | @0@ -L ANSI-C'
gperf_snippet = run_command('sh', '-c', gperf_snippet_format.format(gperf.path()))
gperf_test = gperf_test_format.format('size_t', gperf_snippet.stdout())
if cc.compiles(gperf_test)
        gperf_len_type = 'size_t'
else
        gperf_test = gperf_test_format.format('unsigned', gperf_snippet.stdout())
        if cc.compiles(gperf_test)
                gperf_len_type = 'unsigned'
        else
                error('unable to determine gperf len type')
        endif
endif
message('gperf len type is @0@'.format(gperf_len_type))
conf.set('GPERF_LEN_TYPE', gperf_len_type,
         description : 'The type of gperf "len" parameter')

############################################################

if not cc.has_header('sys/capability.h')
        error('POSIX caps headers not found')
endif
foreach header : ['crypt.h',
                  'linux/btrfs.h',
                  'linux/fou.h',
                  'linux/memfd.h',
                  'linux/vm_sockets.h',
                  'sys/auxv.h',
                  'valgrind/memcheck.h',
                  'valgrind/valgrind.h',
                 ]

        conf.set10('HAVE_' + header.underscorify().to_upper(),
                   cc.has_header(header))
endforeach

############################################################

conf.set_quoted('FALLBACK_HOSTNAME', get_option('fallback-hostname'))
conf.set10('ENABLE_COMPAT_GATEWAY_HOSTNAME', get_option('compat-gateway-hostname'))
gateway_hostnames = ['_gateway'] + (conf.get('ENABLE_COMPAT_GATEWAY_HOSTNAME') == 1 ? ['gateway'] : [])

default_hierarchy = get_option('default-hierarchy')
conf.set_quoted('DEFAULT_HIERARCHY_NAME', default_hierarchy,
                description : 'default cgroup hierarchy as string')
if default_hierarchy == 'legacy'
        conf.set('DEFAULT_HIERARCHY', 'CGROUP_UNIFIED_NONE')
elif default_hierarchy == 'hybrid'
        conf.set('DEFAULT_HIERARCHY', 'CGROUP_UNIFIED_SYSTEMD')
else
        conf.set('DEFAULT_HIERARCHY', 'CGROUP_UNIFIED_ALL')
endif

time_epoch = get_option('time-epoch')
if time_epoch == -1
        NEWS = files('README.md')
        time_epoch = run_command(stat, '-c', '%Y', NEWS).stdout().to_int()
endif
conf.set('TIME_EPOCH', time_epoch)

system_uid_max = get_option('system-uid-max')
if system_uid_max == -1
        system_uid_max = run_command(
                awk,
                '/^\s*SYS_UID_MAX\s+/ { uid=$2 } END { print uid }',
                '/etc/login.defs').stdout().strip()
        if system_uid_max == ''
                system_uid_max = 999
        else
                system_uid_max = system_uid_max.to_int()
        endif
endif
conf.set('SYSTEM_UID_MAX', system_uid_max)
substs.set('systemuidmax', system_uid_max)

system_gid_max = get_option('system-gid-max')
if system_gid_max == -1
        system_gid_max = run_command(
                awk,
                '/^\s*SYS_GID_MAX\s+/ { gid=$2 } END { print gid }',
                '/etc/login.defs').stdout().strip()
        if system_gid_max == ''
                system_gid_max = 999
        else
                system_gid_max = system_gid_max.to_int()
        endif
endif
conf.set('SYSTEM_GID_MAX', system_gid_max)
substs.set('systemgidmax', system_gid_max)

dynamic_uid_min = get_option('dynamic-uid-min')
dynamic_uid_max = get_option('dynamic-uid-max')
conf.set('DYNAMIC_UID_MIN', dynamic_uid_min)
conf.set('DYNAMIC_UID_MAX', dynamic_uid_max)
substs.set('dynamicuidmin', dynamic_uid_min)
substs.set('dynamicuidmax', dynamic_uid_max)

container_uid_base_min = get_option('container-uid-base-min')
container_uid_base_max = get_option('container-uid-base-max')
conf.set('CONTAINER_UID_BASE_MIN', container_uid_base_min)
conf.set('CONTAINER_UID_BASE_MAX', container_uid_base_max)
substs.set('containeruidbasemin', container_uid_base_min)
substs.set('containeruidbasemax', container_uid_base_max)

nobody_user = get_option('nobody-user')
nobody_group = get_option('nobody-group')

if not meson.is_cross_build()
        getent_result = run_command('getent', 'passwd', '65534')
        if getent_result.returncode() == 0
                name = getent_result.stdout().split(':')[0]
                if name != nobody_user
                        warning('\n' +
                                'The local user with the UID 65534 does not match the configured user name "@0@" of the nobody user (its name is @1@).\n'.format(nobody_user, name) +
                                'Your build will result in an user table setup that is incompatible with the local system.')
                endif
        endif
        id_result = run_command('id', '-u', nobody_user)
        if id_result.returncode() == 0
                id = id_result.stdout().to_int()
                if id != 65534
                        warning('\n' +
                                'The local user with the configured user name "@0@" of the nobody user does not have UID 65534 (it has @1@).\n'.format(nobody_user, id) +
                                'Your build will result in an user table setup that is incompatible with the local system.')
                endif
        endif

        getent_result = run_command('getent', 'group', '65534')
        if getent_result.returncode() == 0
                name = getent_result.stdout().split(':')[0]
                if name != nobody_group
                        warning('\n' +
                                'The local group with the GID 65534 does not match the configured group name "@0@" of the nobody group (its name is @1@).\n'.format(nobody_group, name) +
                                'Your build will result in an group table setup that is incompatible with the local system.')
                endif
        endif
        id_result = run_command('id', '-g', nobody_group)
        if id_result.returncode() == 0
                id = id_result.stdout().to_int()
                if id != 65534
                        warning('\n' +
                                'The local group with the configured group name "@0@" of the nobody group does not have UID 65534 (it has @1@).\n'.format(nobody_group, id) +
                                'Your build will result in an group table setup that is incompatible with the local system.')
                endif
        endif
endif
if nobody_user != nobody_group and not (nobody_user == 'nobody' and nobody_group == 'nogroup')
        warning('\n' +
                'The configured user name "@0@" and group name "@0@" of the nobody user/group are not equivalent.\n'.format(nobody_user, nobody_group) +
                'Please re-check that both "nobody-user" and "nobody-group" options are correctly set.')
endif

conf.set_quoted('NOBODY_USER_NAME', nobody_user)
conf.set_quoted('NOBODY_GROUP_NAME', nobody_group)
substs.set('NOBODY_USER_NAME', nobody_user)
substs.set('NOBODY_GROUP_NAME', nobody_group)

tty_gid = get_option('tty-gid')
conf.set('TTY_GID', tty_gid)
substs.set('TTY_GID', tty_gid)

# Ensure provided GID argument is numeric, otherwise fallback to default assignment
users_gid = get_option('users-gid')
substs.set('USERS_GID', users_gid < 0 ? '-' : users_gid)

conf.set10('ENABLE_ADM_GROUP', get_option('adm-group'))
conf.set10('ENABLE_WHEEL_GROUP', get_option('wheel-group'))

dev_kvm_mode = get_option('dev-kvm-mode')
substs.set('DEV_KVM_MODE', dev_kvm_mode)
conf.set10('DEV_KVM_UACCESS', dev_kvm_mode != '0666')
substs.set('GROUP_RENDER_MODE', get_option('group-render-mode'))

kill_user_processes = get_option('default-kill-user-processes')
conf.set10('KILL_USER_PROCESSES', kill_user_processes)
conf.set_quoted('KILL_USER_PROCESSES_YES_NO', kill_user_processes ? 'yes' : 'no')
substs.set('KILL_USER_PROCESSES', kill_user_processes ? 'yes' : 'no')

dns_servers = get_option('dns-servers')
conf.set_quoted('DNS_SERVERS', dns_servers)
substs.set('DNS_SERVERS', dns_servers)

ntp_servers = get_option('ntp-servers')
conf.set_quoted('NTP_SERVERS', ntp_servers)
substs.set('NTP_SERVERS', ntp_servers)

conf.set_quoted('GETTEXT_PACKAGE', meson.project_name())

substs.set('SUSHELL', get_option('debug-shell'))
substs.set('DEBUGTTY', get_option('debug-tty'))

enable_debug_hashmap = false
enable_debug_mmap_cache = false
foreach name : get_option('debug-extra')
        if name == 'hashmap'
                enable_debug_hashmap = true
        elif name == 'mmap-cache'
                enable_debug_mmap_cache = true
        else
                message('unknown debug option "@0@", ignoring'.format(name))
        endif
endforeach
conf.set10('ENABLE_DEBUG_HASHMAP', enable_debug_hashmap)
conf.set10('ENABLE_DEBUG_MMAP_CACHE', enable_debug_mmap_cache)

conf.set10('VALGRIND', get_option('valgrind'))
conf.set10('LOG_TRACE', get_option('log-trace'))

#####################################################################

threads = dependency('threads')
librt = cc.find_library('rt')
libm = cc.find_library('m')
libdl = cc.find_library('dl')
libcrypt = cc.find_library('crypt')

libcap = dependency('libcap', required : false)
if not libcap.found()
        # Compat with Ubuntu 14.04 which ships libcap w/o .pc file
        libcap = cc.find_library('cap')
endif

libmount = dependency('mount',
                      version : fuzzer_build ? '>= 0' : '>= 2.30')

want_seccomp = get_option('seccomp')
if want_seccomp != 'false' and not fuzzer_build
        libseccomp = dependency('libseccomp',
                                version : '>= 2.3.1',
                                required : want_seccomp == 'true')
        have = libseccomp.found()
else
        have = false
        libseccomp = []
endif
conf.set10('HAVE_SECCOMP', have)

want_selinux = get_option('selinux')
if want_selinux != 'false' and not fuzzer_build
        libselinux = dependency('libselinux',
                                version : '>= 2.1.9',
                                required : want_selinux == 'true')
        have = libselinux.found()
else
        have = false
        libselinux = []
endif
conf.set10('HAVE_SELINUX', have)

want_apparmor = get_option('apparmor')
if want_apparmor != 'false' and not fuzzer_build
        libapparmor = dependency('libapparmor',
                                 required : want_apparmor == 'true')
        have = libapparmor.found()
else
        have = false
        libapparmor = []
endif
conf.set10('HAVE_APPARMOR', have)

smack_run_label = get_option('smack-run-label')
if smack_run_label != ''
        conf.set_quoted('SMACK_RUN_LABEL', smack_run_label)
endif

want_polkit = get_option('polkit')
install_polkit = false
install_polkit_pkla = false
if want_polkit != 'false' and not fuzzer_build
        install_polkit = true

        libpolkit = dependency('polkit-gobject-1',
                               required : false)
        if libpolkit.found() and libpolkit.version().version_compare('< 0.106')
                message('Old polkit detected, will install pkla files')
                install_polkit_pkla = true
        endif
endif
conf.set10('ENABLE_POLKIT', install_polkit)

want_acl = get_option('acl')
if want_acl != 'false' and not fuzzer_build
        libacl = cc.find_library('acl', required : want_acl == 'true')
        have = libacl.found()
else
        have = false
        libacl = []
endif
conf.set10('HAVE_ACL', have)

want_audit = get_option('audit')
if want_audit != 'false' and not fuzzer_build
        libaudit = dependency('audit', required : want_audit == 'true')
        have = libaudit.found()
else
        have = false
        libaudit = []
endif
conf.set10('HAVE_AUDIT', have)

want_blkid = get_option('blkid')
if want_blkid != 'false' and not fuzzer_build
        libblkid = dependency('blkid', required : want_blkid == 'true')
        have = libblkid.found()
else
        have = false
        libblkid = []
endif
conf.set10('HAVE_BLKID', have)

want_kmod = get_option('kmod')
if want_kmod != 'false' and not fuzzer_build
        libkmod = dependency('libkmod',
                             version : '>= 15',
                             required : want_kmod == 'true')
        have = libkmod.found()
else
        have = false
        libkmod = []
endif
conf.set10('HAVE_KMOD', have)

want_pam = get_option('pam')
if want_pam != 'false' and not fuzzer_build
        libpam = cc.find_library('pam', required : want_pam == 'true')
        libpam_misc = cc.find_library('pam_misc', required : want_pam == 'true')
        have = libpam.found() and libpam_misc.found()
else
        have = false
        libpam = []
        libpam_misc = []
endif
conf.set10('HAVE_PAM', have)

want_microhttpd = get_option('microhttpd')
if want_microhttpd != 'false' and not fuzzer_build
        libmicrohttpd = dependency('libmicrohttpd',
                                   version : '>= 0.9.33',
                                   required : want_microhttpd == 'true')
        have = libmicrohttpd.found()
else
        have = false
        libmicrohttpd = []
endif
conf.set10('HAVE_MICROHTTPD', have)

want_libcryptsetup = get_option('libcryptsetup')
if want_libcryptsetup != 'false' and not fuzzer_build
        libcryptsetup = dependency('libcryptsetup',
                                   version : '>= 1.6.0',
                                   required : want_libcryptsetup == 'true')
        have = libcryptsetup.found()
        have_sector = cc.has_member(
                    'struct crypt_params_plain',
                    'sector_size',
                    prefix : '#include <libcryptsetup.h>')
else
        have = false
        have_sector = false
        libcryptsetup = []
endif
conf.set10('HAVE_LIBCRYPTSETUP', have)
conf.set10('HAVE_LIBCRYPTSETUP_SECTOR_SIZE', have_sector)

want_libcurl = get_option('libcurl')
if want_libcurl != 'false' and not fuzzer_build
        libcurl = dependency('libcurl',
                             version : '>= 7.32.0',
                             required : want_libcurl == 'true')
        have = libcurl.found()
else
        have = false
        libcurl = []
endif
conf.set10('HAVE_LIBCURL', have)

want_libidn = get_option('libidn')
want_libidn2 = get_option('libidn2')
if want_libidn == 'true' and want_libidn2 == 'true'
        error('libidn and libidn2 cannot be requested simultaneously')
endif

if want_libidn != 'false' and want_libidn2 != 'true' and not fuzzer_build
        libidn = dependency('libidn',
                            required : want_libidn == 'true')
        have = libidn.found()
else
        have = false
        libidn = []
endif
conf.set10('HAVE_LIBIDN', have)
if not have and want_libidn2 != 'false' and not fuzzer_build
        # libidn is used for both libidn and libidn2 objects
        libidn = dependency('libidn2',
                            required : want_libidn2 == 'true')
        have = libidn.found()
else
        have = false
endif
conf.set10('HAVE_LIBIDN2', have)

want_libiptc = get_option('libiptc')
if want_libiptc != 'false' and not fuzzer_build
        libiptc = dependency('libiptc',
                             required : want_libiptc == 'true')
        have = libiptc.found()
else
        have = false
        libiptc = []
endif
conf.set10('HAVE_LIBIPTC', have)

want_qrencode = get_option('qrencode')
if want_qrencode != 'false' and not fuzzer_build
        libqrencode = dependency('libqrencode',
                                 required : want_qrencode == 'true')
        have = libqrencode.found()
else
        have = false
        libqrencode = []
endif
conf.set10('HAVE_QRENCODE', have)

want_gcrypt = get_option('gcrypt')
if want_gcrypt != 'false' and not fuzzer_build
        libgcrypt = cc.find_library('gcrypt', required : want_gcrypt == 'true')
        libgpg_error = cc.find_library('gpg-error', required : want_gcrypt == 'true')
        have = libgcrypt.found() and libgpg_error.found()
else
        have = false
endif
if not have
        # link to neither of the libs if one is not found
        libgcrypt = []
        libgpg_error = []
endif
conf.set10('HAVE_GCRYPT', have)

want_gnutls = get_option('gnutls')
if want_gnutls != 'false' and not fuzzer_build
        libgnutls = dependency('gnutls',
                               version : '>= 3.1.4',
                               required : want_gnutls == 'true')
        have = libgnutls.found()
else
        have = false
        libgnutls = []
endif
conf.set10('HAVE_GNUTLS', have)

want_openssl = get_option('openssl')
if want_openssl != 'false' and not fuzzer_build
        libopenssl = dependency('openssl',
                                version : '>= 1.1.0',
                                required : want_openssl == 'true')
        have = libopenssl.found()
else
        have = false
        libopenssl = []
endif
conf.set10('HAVE_OPENSSL', have)

want_elfutils = get_option('elfutils')
if want_elfutils != 'false' and not fuzzer_build
        libdw = dependency('libdw',
                           required : want_elfutils == 'true')
        have = libdw.found()
else
        have = false
        libdw = []
endif
conf.set10('HAVE_ELFUTILS', have)

want_zlib = get_option('zlib')
if want_zlib != 'false' and not fuzzer_build
        libz = dependency('zlib',
                          required : want_zlib == 'true')
        have = libz.found()
else
        have = false
        libz = []
endif
conf.set10('HAVE_ZLIB', have)

want_bzip2 = get_option('bzip2')
if want_bzip2 != 'false' and not fuzzer_build
        libbzip2 = cc.find_library('bz2',
                                   required : want_bzip2 == 'true')
        have = libbzip2.found()
else
        have = false
        libbzip2 = []
endif
conf.set10('HAVE_BZIP2', have)

want_xz = get_option('xz')
if want_xz != 'false' and not fuzzer_build
        libxz = dependency('liblzma',
                           required : want_xz == 'true')
        have = libxz.found()
else
        have = false
        libxz = []
endif
conf.set10('HAVE_XZ', have)

want_lz4 = get_option('lz4')
if want_lz4 != 'false' and not fuzzer_build
        liblz4 = dependency('liblz4',
                            version : '>= 1.3.0',
                            required : want_lz4 == 'true')
        have = liblz4.found()
else
        have = false
        liblz4 = []
endif
conf.set10('HAVE_LZ4', have)

want_xkbcommon = get_option('xkbcommon')
if want_xkbcommon != 'false' and not fuzzer_build
        libxkbcommon = dependency('xkbcommon',
                                  version : '>= 0.3.0',
                                  required : want_xkbcommon == 'true')
        have = libxkbcommon.found()
else
        have = false
        libxkbcommon = []
endif
conf.set10('HAVE_XKBCOMMON', have)

want_pcre2 = get_option('pcre2')
if want_pcre2 != 'false'
        libpcre2 = dependency('libpcre2-8',
                              required : want_pcre2 == 'true')
        have = libpcre2.found()
else
        have = false
        libpcre2 = []
endif
conf.set10('HAVE_PCRE2', have)

want_glib = get_option('glib')
if want_glib != 'false' and not fuzzer_build
        libglib =    dependency('glib-2.0',
                                version : '>= 2.22.0',
                                required : want_glib == 'true')
        libgobject = dependency('gobject-2.0',
                                version : '>= 2.22.0',
                                required : want_glib == 'true')
        libgio =     dependency('gio-2.0',
                                required : want_glib == 'true')
        have = libglib.found() and libgobject.found() and libgio.found()
else
        have = false
        libglib = []
        libgobject = []
        libgio = []
endif
conf.set10('HAVE_GLIB', have)

want_dbus = get_option('dbus')
if want_dbus != 'false' and not fuzzer_build
        libdbus = dependency('dbus-1',
                             version : '>= 1.3.2',
                             required : want_dbus == 'true')
        have = libdbus.found()
else
        have = false
        libdbus = []
endif
conf.set10('HAVE_DBUS', have)

default_dnssec = get_option('default-dnssec')
if fuzzer_build
        default_dnssec = 'no'
endif
if default_dnssec != 'no' and conf.get('HAVE_GCRYPT') == 0
        message('default-dnssec cannot be set to yes or allow-downgrade when gcrypt is disabled. Setting default-dnssec to no.')
        default_dnssec = 'no'
endif
conf.set('DEFAULT_DNSSEC_MODE',
         'DNSSEC_' + default_dnssec.underscorify().to_upper())
substs.set('DEFAULT_DNSSEC_MODE', default_dnssec)

dns_over_tls = get_option('dns-over-tls')
if dns_over_tls != 'false'
        if dns_over_tls == 'openssl'
                have_gnutls = false
        else
                have_gnutls = (conf.get('HAVE_GNUTLS') == 1 and libgnutls.version().version_compare('>= 3.5.3'))
                if dns_over_tls == 'gnutls' and not have_gnutls
                        error('DNS-over-TLS support was requested with gnutls, but dependencies are not available')
                endif
        endif
        if dns_over_tls == 'gnutls' or have_gnutls
                have_openssl = false
        else
                have_openssl = conf.get('HAVE_OPENSSL') == 1
                if dns_over_tls != 'auto' and not have_openssl
                        str = dns_over_tls == 'openssl' ? ' with openssl' : ''
                        error('DNS-over-TLS support was requested$0$, but dependencies are not available'.format(str))
                endif
        endif
        have = have_gnutls or have_openssl
else
        have = have_gnutls = have_openssl = false
endif
conf.set10('ENABLE_DNS_OVER_TLS', have)
conf.set10('DNS_OVER_TLS_USE_GNUTLS', have_gnutls)
conf.set10('DNS_OVER_TLS_USE_OPENSSL', have_openssl)

default_dns_over_tls = get_option('default-dns-over-tls')
if fuzzer_build
        default_dns_over_tls = 'no'
endif
if default_dns_over_tls != 'no' and conf.get('ENABLE_DNS_OVER_TLS') == 0
        message('default-dns-over-tls cannot be set to opportunistic when DNS-over-TLS support is disabled. Setting default-dns-over-tls to no.')
        default_dns_over_tls = 'no'
endif
conf.set('DEFAULT_DNS_OVER_TLS_MODE',
         'DNS_OVER_TLS_' + default_dns_over_tls.underscorify().to_upper())
substs.set('DEFAULT_DNS_OVER_TLS_MODE', default_dns_over_tls)

want_importd = get_option('importd')
if want_importd != 'false'
        have = (conf.get('HAVE_LIBCURL') == 1 and
                conf.get('HAVE_ZLIB') == 1 and
                conf.get('HAVE_XZ') == 1 and
                conf.get('HAVE_GCRYPT') == 1)
        if want_importd == 'true' and not have
                error('importd support was requested, but dependencies are not available')
        endif
else
        have = false
endif
conf.set10('ENABLE_IMPORTD', have)

want_remote = get_option('remote')
if want_remote != 'false'
        have_deps = [conf.get('HAVE_MICROHTTPD') == 1,
                     conf.get('HAVE_LIBCURL') == 1]
        # sd-j-remote requires µhttpd, and sd-j-upload requires libcurl, so
        # it's possible to build one without the other. Complain only if
        # support was explictly requested. The auxiliary files like sysusers
        # config should be installed when any of the programs are built.
        if want_remote == 'true' and not (have_deps[0] and have_deps[1])
                error('remote support was requested, but dependencies are not available')
        endif
        have = have_deps[0] or have_deps[1]
else
        have = false
endif
conf.set10('ENABLE_REMOTE', have)

foreach term : ['utmp',
                'hibernate',
                'environment-d',
                'binfmt',
                'coredump',
                'resolve',
                'logind',
                'hostnamed',
                'localed',
                'machined',
                'portabled',
                'networkd',
                'timedated',
                'timesyncd',
                'firstboot',
                'randomseed',
                'backlight',
                'vconsole',
                'quotacheck',
                'sysusers',
                'tmpfiles',
                'hwdb',
                'rfkill',
                'ldconfig',
                'efi',
                'tpm',
                'ima',
                'smack',
                'gshadow',
                'idn',
                'nss-myhostname',
                'nss-systemd']
        have = get_option(term)
        name = 'ENABLE_' + term.underscorify().to_upper()
        conf.set10(name, have)
endforeach

foreach tuple : [['nss-mymachines', 'machined'],
                 ['nss-resolve',    'resolve']]
        want = get_option(tuple[0])
        if want != 'false'
                have = get_option(tuple[1])
                if want == 'true' and not have
                        error('@0@ is requested but @1@ is disabled'.format(tuple[0], tuple[1]))
                endif
        else
                have = false
        endif
        name = 'ENABLE_' + tuple[0].underscorify().to_upper()
        conf.set10(name, have)
endforeach

enable_nss = false
foreach term : ['ENABLE_NSS_MYHOSTNAME',
                'ENABLE_NSS_MYMACHINES',
                'ENABLE_NSS_RESOLVE',
                'ENABLE_NSS_SYSTEMD']
        if conf.get(term) == 1
                enable_nss = true
        endif
endforeach
conf.set10('ENABLE_NSS', enable_nss)

conf.set10('ENABLE_TIMEDATECTL', get_option('timedated') or get_option('timesyncd'))

tests = []
fuzzers = []

conf.set10('SYSTEMD_SLOW_TESTS_DEFAULT', slow_tests)

#####################################################################

if get_option('efi')
        efi_arch = host_machine.cpu_family()

        if efi_arch == 'x86'
                EFI_MACHINE_TYPE_NAME = 'ia32'
                gnu_efi_arch = 'ia32'
        elif efi_arch == 'x86_64'
                EFI_MACHINE_TYPE_NAME = 'x64'
                gnu_efi_arch = 'x86_64'
        elif efi_arch == 'arm'
                EFI_MACHINE_TYPE_NAME = 'arm'
                gnu_efi_arch = 'arm'
        elif efi_arch == 'aarch64'
                EFI_MACHINE_TYPE_NAME = 'aa64'
                gnu_efi_arch = 'aarch64'
        else
                EFI_MACHINE_TYPE_NAME = ''
                gnu_efi_arch = ''
        endif

        have = true
        conf.set_quoted('EFI_MACHINE_TYPE_NAME', EFI_MACHINE_TYPE_NAME)

        conf.set('SD_TPM_PCR', get_option('tpm-pcrindex'))
else
        have = false
endif
conf.set10('ENABLE_EFI', have)

config_h = configure_file(
  output : 'config.h',
  configuration : conf,
)

cpp = ' '.join(cc.cmd_array()) + ' -E'


includes = include_directories(
  'src/basic',
  'src/shared',
  'src/systemd',
  'src/libsystemd/sd-bus',
  'src/libsystemd/sd-id128',
)

add_project_arguments('-include', 'config.h', language : 'c')

subdir('src/basic')
subdir('src/shared')
subdir('src/libsystemd')

libsystemd_sym_path = '@0@/@1@'.format(meson.current_source_dir(), libsystemd_sym)
libsystemd = shared_library(
        'basu',
        disable_mempool_c,
        version : libsystemd_version,
        include_directories : includes,
        link_args : ['-shared',
                     '-Wl,--version-script=' + libsystemd_sym_path],
        link_with : libbasic,
        link_whole : libsystemd_static,
        dependencies : [threads,
                        librt,
                        libxz,
                        liblz4],
        link_depends : libsystemd_sym,
        install : true,
        install_dir : rootlibdir)

exe = executable('busctl',
                 'src/busctl/busctl.c',
                 'src/busctl/busctl-introspect.c',
                 'src/busctl/busctl-introspect.h',
                 include_directories : includes,
                 link_with : [libshared_static,
                              libbasic,
                              libsystemd_static],
                 install_rpath : rootlibexecdir,
                 install : true)
