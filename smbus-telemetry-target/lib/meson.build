smbus_slave_telemetry_includes = include_directories('include', '.')

subdir('include')

smbus_telemetry_target_gen = []

phosphor_logging = dependency('phosphor-logging')
smbus_telemetry_target_deps = [
    phosphor_logging
]

conf_data = configuration_data()
conf_data.set('libonly', get_option('libonly'))
conf_data.set_quoted('SMBUS_SYSFS_PATH', get_option('smbus-sysfs-path'))
conf_data.set_quoted('SMBUS_SLAVE_TELEMETRY_CONFIG_CSV', get_option('smbus-telemetry-config-csv'))
conf_data.set('SMBUS_DATA_RECORD_SIZE', get_option('smbus-data-record'))
conf_h_dep = declare_dependency(
    include_directories: include_directories('.'),
    sources: configure_file(
        output: 'config.h',
        configuration: conf_data
    )
)

libsmbus_telemetry = static_library(
    'smbus-telemetry',
    'smbus_telemetry_target_api.cpp',
    'smbus_telemetry_update.cpp',
    implicit_include_directories: false,
    include_directories: [smbus_slave_telemetry_includes],
    dependencies: [
        smbus_telemetry_target_deps,
        conf_h_dep,
    ],
    install: true,
)

smbus_telemetry_target_reqs = []
foreach dep : smbus_telemetry_target_deps
    if dep.type_name() == 'pkgconfig'
        smbus_telemetry_target_reqs += dep
    endif
endforeach

import('pkgconfig').generate(
  name: 'smbus-telemetry-target',
  description: 'SMBus device telemetry Library',
  version: meson.project_version(),
  requires: smbus_telemetry_target_reqs,
  libraries: libsmbus_telemetry)

smbus_slave_telemetry_dep = declare_dependency(
    include_directories: [smbus_slave_telemetry_includes],
    link_with: libsmbus_telemetry,
    sources: smbus_telemetry_target_gen,
    dependencies: smbus_telemetry_target_deps,
)

