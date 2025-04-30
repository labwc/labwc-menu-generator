project(
  'labwc-menu-generator',
  'c',
  version: '0.2.0',
  license: 'GPL-2.0-only',
  meson_version: '>=0.59.0',
  default_options: [
    'c_std=c11',
    'warning_level=2',
  ],
)

glib = dependency('glib-2.0')

executable(
  meson.project_name(),
  sources: files('main.c', 'desktop.c', 'ignore.c'),
  dependencies: [glib],
  install: true,
)

subdir('data')
subdir('t')
