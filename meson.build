# Master build file

# Project declaration
project('qman', 'c',
  license: 'BSD-2-Clause',
  version: '1.2.0'
)

subdir('src')
subdir('man')

# README.md
custom_target('readme_md',
  input: 'README.md',
  output: 'README.md',
  command: ['cp', '@INPUT@', '@OUTPUT@'],
  install: true,
  install_dir: 'share/doc/qman',
  install_tag: 'docs'
)

# TESTING.md
custom_target('testing_md',
  input: 'TESTING.md',
  output: 'TESTING.md',
  command: ['cp', '@INPUT@', '@OUTPUT@'],
  install: true,
  install_dir: 'share/doc/qman',
  install_tag: 'docs'
)
