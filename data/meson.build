scdoc = find_program('scdoc', required: false)

if scdoc.found()
  markdown = 'labwc-menu-generator.1.scd'
  manpage = 'labwc-menu-generator.1'
  custom_target(
    manpage,
    output: manpage,
    input: markdown,
    command: scdoc,
    feed: true,
    capture: true,
    install: true,
    install_dir : join_paths(get_option('datadir'), 'man', 'man1')
  )
endif
