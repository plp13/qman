# Manual page build file

# Manual page
custom_target('manpage',
  input: 'qman.1',
  output: 'qman.1',
  command: ['cp', '@INPUT@', '@OUTPUT@'],
  install: true,
  install_dir: 'man/man1',
  install_tag: 'doc'
)
