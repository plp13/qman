# Project declaration
project('qman', 'c',
  license: 'BSD-2-Clause',
  version: 'nightly'
)

# Configuration subsystem sources (generated from config_def.py using cog)
cog = find_program('cogapp', 'cog')
cog_cmd = [cog, '-d', '-o', '@OUTPUT@', '@INPUT@']
config_h = custom_target('config.h',
  input: 'config.h.cog',
  output: 'config.h',
  command: cog_cmd
)
config_c = custom_target('config.c',
  input: 'config.c.cog',
  output: 'config.c',
  command: cog_cmd
)

# Executable
src = [
  config_h,
  config_c,
  'util.c',
  'program.c',
  'cli.c',
  'tui.c',
  'qman.c'
]
deps = [
  dependency('ncursesw'),
  dependency('inih')
]
executable('qman',
  sources: src,
  dependencies: deps,
  install: true,
  install_dir: 'bin'
)

# Manual page
pandoc = find_program('pandoc')
pandoc_cmd = [pandoc, '@INPUT@', '-s', '-t', 'man', '-o', '@OUTPUT@']
custom_target('manpage',
  input: 'qman.1.md',
  output: 'qman.1',
  command: pandoc_cmd,
  install: true,
  install_dir: 'man/man1'
)
