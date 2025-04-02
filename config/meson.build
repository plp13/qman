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
      'themes/catppuccin_latte.conf'
    ],
    install_dir: get_option('configdir') + '/themes',
    install_tag: 'config'
  )
endif
