# Manual page build file

pandoc = find_program('pandoc', required: get_option('man-pages'))
if pandoc.found()
  output = 'qman.1'
  manpage = configure_file(
    input: ['qman.1.md'],
    output: output,
    command: [
      pandoc,
      '-s',
      '-t', 'man',
      '@INPUT@',
      '-o', '@OUTPUT@'
    ]
  )

  install_man(manpage)
endif
