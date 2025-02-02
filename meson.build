# Master build file

# Project declaration
project('qman', 'c',
  license: 'BSD-2-Clause',
  version: '1.2.1'
)

# Executable
subdir('src')

# Manual page
subdir('man')

# Additional documentation
if get_option('docs').enabled()
  install_data(
    ['README.md', 'TESTING.md'],
    install_dir: get_option('docdir'),
    install_tag: 'doc'
  )
endif
