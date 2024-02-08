nv_shmem_includes = include_directories('include', 'utils', 'impl', '.')

subdir('include')

nv_shmem_gen = []

phosphor_logging = dependency('phosphor-logging')
nv_shmem_deps = [
    phosphor_logging
]

conf_data = configuration_data()
conf_data.set('libonly', get_option('libonly'))
conf_data.set_quoted('PLATFORMSYSTEMID', get_option('platform-system-id'))
conf_data.set_quoted('PLATFORMDEVICEPREFIX', get_option('platform-device-prefix'))
conf_data.set_quoted('SHM_NAMESPACE_CFG_JSON', join_paths(package_datadir, 'shm_namespace_config.json'))
conf_data.set_quoted('SHM_MAPPING_JSON', join_paths(package_datadir, 'shm_mapping.json'))
if get_option('enable-shm-debug').enabled()
    add_project_arguments('-DENABLE_SHM_DEBUG', language : 'cpp')
endif
conf_h_dep = declare_dependency(
    include_directories: include_directories('.'),
    sources: configure_file(
        output: 'config.h',
        configuration: conf_data
    )
)

libnvshmem = static_library(
    'nvshmem',
    'managed_shmem.cpp',
    'shmem_map.cpp',
    'config_json_reader.cpp',
    'telemetry_mrd_producer.cpp',
    'shm_sensor_aggregator.cpp',
    'telemetry_mrd_client.cpp',
    implicit_include_directories: false,
    include_directories: [nv_shmem_includes],
    dependencies: [
        nv_shmem_deps,
    ],
    install: true,
)

nv_shmem_reqs = []
foreach dep : nv_shmem_deps
    if dep.type_name() == 'pkgconfig'
        nv_shmem_reqs += dep
    endif
endforeach

import('pkgconfig').generate(
  name: 'nvidia-shmem',
  description: 'Nvidia Shared Memory Library',
  version: meson.project_version(),
  requires: nv_shmem_reqs,
  libraries: libnvshmem)

nv_shmem_dep = declare_dependency(
    include_directories: [nv_shmem_includes],
    link_with: libnvshmem,
    sources: nv_shmem_gen,
    dependencies: nv_shmem_deps,
)

