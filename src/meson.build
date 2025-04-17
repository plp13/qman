# Executable build file

# Compiler
cc = meson.get_compiler('c')
add_global_arguments(['-Wall', '-Wextra', '-Wno-sign-compare'], language: 'c')
# add_global_arguments('-O2', '-D_FORTIFY_SOURCE=2', language: 'c')

# Configuration subsystem sources (generated from config_def.py using cog)
cog = find_program('cogapp', 'cog', 'cog.py')
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

# Non-main sources
src = [
  config_h,
  config_c,
  'util.c',
  'eini.c',
  'base64.c',
  'program.c',
  'cli.c',
  'tui.c'
]

# Dependencies
deps = [
  dependency('ncursesw', required: true),
]
if get_option('gzip').enabled() or get_option('gzip').auto()
  gzip = dependency('zlib', required: true)
  deps += [gzip]
  add_global_arguments('-DQMAN_GZIP=true', language: 'c')
endif
if get_option('bzip2').enabled() or get_option('bzip2').auto()
  bzip2 = dependency('bzip2', required: false)
  if not bzip2.found()
    bzip2 = cc.find_library('bz2', required: true)
  endif
  deps += [bzip2]
  add_global_arguments('-DQMAN_BZIP2=true', language: 'c')
endif
if get_option('lzma').enabled() or get_option('lzma').auto()
  xz = dependency('liblzma', required: false)
  if not xz.found()
    xz = cc.find_library('lzma', required: true)
  endif
  deps += [xz]
  add_global_arguments('-DQMAN_LZMA=true', language: 'c')
endif
if get_option('tests').enabled() or get_option('tests').auto()
  cunit = dependency('cunit', required: true)
  deps += [cunit]
endif

# Qman executable
executable('qman',
  sources: src + ['qman.c'],
  dependencies: deps,
  install: true,
  install_dir: 'bin',
  install_tag: 'program'
)

# Unit testing (executable and tests)
if get_option('tests').enabled()
  t_exe = executable('qman_tests',
    sources: src + ['qman_tests.c'],
    dependencies: deps + [cunit],
    install: false
  )
  t_all = run_command(find_program('qman_tests_list.sh'), check: true).stdout().split('\n')
  foreach t: t_all
    if t != ''
      test(t, t_exe, args: [t])
    endif
  endforeach
endif
