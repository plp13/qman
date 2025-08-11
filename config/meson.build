# Configuration build file

# Main configuration file
if get_option('config').enabled() or get_option('config').auto()
  install_data(
    ['qman.conf'],
    install_dir: get_option('configdir'),
    install_tag: 'config'
  )
endif

# Configuration files for themes
if get_option('config').enabled() or get_option('config').auto()
  install_data(
    [
      'themes/modernity.conf',
      'themes/modernity-light.conf',
      'themes/adwaita.conf',
      'themes/adwaita-light.conf',
      'themes/catppuccin_latte.conf'
    ],
    install_dir: get_option('configdir') + '/themes',
    install_tag: 'config'
  )
endif

# README.md
if get_option('docs').enabled() or get_option('docs').auto()
  install_data(
    ['README.md'],
    install_dir: get_option('docdir') + '/config',
    install_tag: 'doc'
  )
endif
