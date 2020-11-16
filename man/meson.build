# SPDX-License-Identifier: LGPL-2.1+

# This is lame, I know, but meson has no other include mechanism
subdir('rules')

want_man = get_option('man')
xsltproc = find_program('xsltproc',
                        required : want_man == 'true')
want_man = want_man != 'false' and xsltproc.found()

xsltproc_flags = [
        '--nonet',
        '--xinclude',
        '--maxdepth', '9000',
        '--stringparam', 'man.output.quietly', '1',
        '--stringparam', 'funcsynopsis.style', 'ansi',
        '--stringparam', 'man.authors.section.enabled', '0',
        '--stringparam', 'man.copyright.section.enabled', '0',
        '--stringparam', 'systemd.version', '@0@'.format(meson.project_version()),
        '--path',
        '@0@:@1@'.format(meson.current_build_dir(), meson.current_source_dir())]

custom_man_xsl = files('custom-man.xsl')
xslt_cmd = [xsltproc, '-o', '@OUTPUT0@'] + xsltproc_flags

custom_entities_ent = configure_file(
        input : 'custom-entities.ent.in',
        output : 'custom-entities.ent',
        configuration : conf)

man_pages = []
source_xml_files = []
foreach tuple : xsltproc.found() ? manpages : []
        stem = tuple[0]
        section = tuple[1]
        aliases = tuple[2]
        condition = tuple[3]

        xml = stem + '.xml'
        man = stem + '.' + section

        manaliases = []
        foreach alias : aliases
                manaliases += alias + '.' + section
        endforeach

        mandirn = join_paths(get_option('mandir'), 'man' + section)

        if condition == '' or conf.get(condition) == 1
                p1 = custom_target(
                        man,
                        input : xml,
                        output : [man] + manaliases,
                        command : xslt_cmd + [custom_man_xsl, '@INPUT@'],
                        depend_files : custom_entities_ent,
                        install : want_man,
                        install_dir : mandirn)
                man_pages += p1

                file = files(tuple[0] + '.xml')
                source_xml_files += file
        else
                message('Skipping @0@.@1@ because @2@ is false'.format(stem, section, condition))
        endif
endforeach

############################################################

have_lxml = run_command(xml_helper_py).returncode() == 0
if not have_lxml
        message('python-lxml not available, not making man page indices')
endif

systemd_directives_xml = custom_target(
        'systemd.directives.xml',
        input : ['directives-template.xml', source_xml_files],
        output : 'systemd.directives.xml',
        command : [make_directive_index_py, '@OUTPUT@', '@INPUT@'])

nonindex_xml_files = source_xml_files + [systemd_directives_xml]
systemd_index_xml = custom_target(
        'systemd.index.xml',
        input : nonindex_xml_files,
        output : 'systemd.index.xml',
        command : [make_man_index_py, '@OUTPUT@'] + nonindex_xml_files)

foreach tuple : xsltproc.found() ? [['systemd.directives', '7', systemd_directives_xml],
                                    ['systemd.index',      '7', systemd_index_xml]] : []
        stem = tuple[0]
        section = tuple[1]
        xml = tuple[2]

        man = stem + '.' + section

        mandirn = join_paths(get_option('mandir'), 'man' + section)

        p1 = custom_target(
                man,
                input : xml,
                output : man,
                command : xslt_cmd + [custom_man_xsl, '@INPUT@'],
                install : want_man and have_lxml,
                install_dir : mandirn)
        man_pages += p1
endforeach

# Cannot use run_target because those targets are used in depends
# Also see https://github.com/mesonbuild/meson/issues/368.
man = custom_target(
        'man',
        output : 'man',
        depends : man_pages,
        command : ['echo'])

############################################################

configure_file(
        input : 'man.in',
        output : 'man',
        configuration : substs)

