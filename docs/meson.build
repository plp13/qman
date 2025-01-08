
pandoc = find_program('pandoc')
pandoc_cmd = [pandoc, '@INPUT@', '-s', '-t', 'man', '-o', '@OUTPUT@']
custom_target('manpage',
  input: 'qman.1.md',
  output: 'qman.1',
  command: pandoc_cmd,
  install: true,
  install_dir: 'man/man1'
)

