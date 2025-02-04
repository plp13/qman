# Master build file

# Project declaration
project('qman', 'c',
  license: 'BSD-2-Clause',
  version: run_command('git', 'describe', '--tags', '--abbrev', check: true).stdout().strip()
)

# Executable
subdir('src')

# Manual page
subdir('man')

# Additional documentation
if get_option('docs').enabled() or get_option('docs').auto()
  install_data(
    ['README.md', 'TESTING.md'],
    install_dir: get_option('docdir'),
    install_tag: 'doc'
  )
endif
