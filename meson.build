# Master build file

# Project declaration
project('qman', 'c',
  license: 'BSD-2-Clause',
  version: '1.5.0-49-g6cbf79e'
)

# Executable
subdir('src')

# Cofiguration
subdir('config')

# Manual page
subdir('man')

# Documentation
subdir('doc')

# README.md
if get_option('docs').enabled() or get_option('docs').auto()
  install_data(
    ['README.md'],
    install_dir: get_option('docdir'),
    install_tag: 'doc'
  )
endif
