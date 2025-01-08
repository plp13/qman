# Executable build file

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
  'base64.c',
  'program.c',
  'cli.c',
  'tui.c',
  'qman.c'
]
deps = [
  dependency('zlib'),
  dependency('ncursesw'),
  dependency('inih')
]
executable('qman',
  sources: src,
  dependencies: deps,
  install: true,
  install_dir: 'bin'
)
