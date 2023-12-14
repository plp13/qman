project('qman', 'c', license: 'BSD-2-Clause', version: 'nightly')
src = ['util.c', 'program.c', 'cli.c', 'tui.c', 'qman.c']
deps = [dependency('ncursesw'), dependency('inih')]
executable('qman', sources: src, dependencies: deps)
