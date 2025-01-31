# Master build file

# Project declaration
project('qman', 'c',
  license: 'BSD-2-Clause',
  version: '1.2.0'
)

subdir('src')
subdir('man')

# Documentation files
install_data(
  ['README.md', 'TESTING.md'],
  install_dir: get_option('docdir'),
  install_tag: 'doc'
)
