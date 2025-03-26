# Master build file

# Project declaration
project('qman', 'c',
  license: 'BSD-2-Clause',
  version: '1.3.1-31-g378cb13'
)

# Executable
subdir('src')

# Cofiguration
subdir('config')

# Manual page
subdir('man')

# Additional documentation
if get_option('docs').enabled() or get_option('docs').auto()
  install_data(
    ['README.md', 'PACKAGING.md', 'TESTING.md'],
    install_dir: get_option('docdir'),
    install_tag: 'doc'
  )
endif
