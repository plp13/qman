# Documentation build file

# Documentation
if get_option('docs').enabled() or get_option('docs').auto()
  install_data(
    [
      'BUILDING.md',
      'CONTRIBUTING.md',
      'DEBUGGING.md',
      'OS_SPECIFIC.md',
      'PACKAGES.md',
      'TESTING.md',
      'TROUBLESHOOTING.md'
    ],
    install_dir: get_option('prefix') + '/' + get_option('docdir') + '/doc',
    install_tag: 'doc'
  )
endif
