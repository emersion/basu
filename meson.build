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

time_epoch = get_option('time-epoch')
if time_epoch == -1
        NEWS = files('README.md')
        time_epoch = run_command(stat, '-c', '%Y', NEWS).stdout().to_int()
endif
conf.set('TIME_EPOCH', time_epoch)

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

conf.set_quoted('GETTEXT_PACKAGE', meson.project_name())

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

tests = []
fuzzers = []

conf.set10('SYSTEMD_SLOW_TESTS_DEFAULT', slow_tests)

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
                        librt],
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
