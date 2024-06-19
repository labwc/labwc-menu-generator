test_lib = static_library(
  'test_lib',
  sources: files(
    'tap.c',
    'test-lib.c',
  ),
  dependencies: [dependency('glib-2.0')],
)

tests = [
  't1000.t.c',
  't1001.t.c',
]

foreach t : tests
  testname = t.split('.')[0].underscorify()
  exe = executable(
    testname,
    sources: [t],
    link_with: [test_lib],
  )
  test(
    testname,
    exe,
    protocol: 'tap',
    is_parallel: false,
  )
endforeach

