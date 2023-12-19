project('qman', 'c', license: 'BSD-2-Clause', version: 'nightly')

src = ['util.c', 'program.c', 'cli.c', 'tui.c', 'qman.c']
deps = [dependency('ncursesw'), dependency('inih')]
executable('qman',
  sources: src,
  dependencies: deps,
  install: true,
  install_dir: 'bin'
)

pandoc = find_program('pandoc')
pandoc_cmd = [pandoc, '@INPUT@', '-s', '-t', 'man', '-o', '@OUTPUT@']
custom_target('manpage',
  input: 'qman.1.md',
  output: 'qman.1',
  command: pandoc_cmd,
  install: true,
  install_dir: 'man/man1'
)
