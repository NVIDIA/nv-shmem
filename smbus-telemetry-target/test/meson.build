gtest_dep = dependency('gtest', main: true, disabler: true, required: false)
gmock_dep = dependency('gmock', disabler: true, required: false)
if not gtest_dep.found() or not gmock_dep.found()
    gtest_proj = import('cmake').subproject('googletest', required: false)
    if gtest_proj.found()
        gtest_dep = declare_dependency(
            dependencies: [
                dependency('threads'),
                gtest_proj.dependency('gtest'),
                gtest_proj.dependency('gtest_main'),
            ]
        )
        gmock_dep = gtest_proj.dependency('gmock')
  else
        assert(
            not get_option('tests').enabled(),
            'Googletest is required if tests are enabled'
        )
  endif
endif


tests = [
    'smbus_telemetry_target_utest',
]
foreach t : tests
    test(
        'test_' + t.underscorify(),
        executable(
            'test-' + t.underscorify(),
            t + '.cpp',
            dependencies: [
                gmock_dep,
                gtest_dep,
                conf_h_dep,
                smbus_slave_telemetry_dep,
            ],
            include_directories: include_directories('..'),            
        )
    )
endforeach
