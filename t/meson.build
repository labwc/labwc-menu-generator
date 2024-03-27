test_env = environment()
test_env.set('SHARNESS_TEST_SRCDIR', meson.current_source_dir())
test_env.set('BUILDDIR', meson.current_build_dir())

tests = [
  't1000.t',
  't1001.t',
]

foreach t : tests
  test(
    t, files(t),
    env: test_env,
    args: [ '--verbose' ],
    protocol: 'tap',
    is_parallel: false,
  )
endforeach

