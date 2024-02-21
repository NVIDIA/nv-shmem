project('nvshmem', 'cpp',
    meson_version: '>= 0.62.2',
    default_options: [
      'buildtype=debugoptimized',
      'cpp_std=c++20',
      'warning_level=3',
      'werror=true',
      'libonly=' + (meson.is_subproject() ? 'true' : 'false'),
      'tests=' + (meson.is_subproject() ? 'disabled' : 'auto'),
    ],
    version: '1.0',
)

add_project_arguments('-Wno-psabi', language: 'cpp')
package_datadir = join_paths(get_option('prefix'), get_option('datadir'), meson.project_name())

if not get_option('nvidia-shmem').disabled()
  subdir('lib')
  subdir('tools')
  install_data('configurations/shm_mapping.json', install_dir: package_datadir)
  install_data('configurations/shm_namespace_config.json', install_dir: package_datadir)
endif

if get_option('libonly')
  subdir_done()
endif

if not get_option('tests').disabled()
    subdir('test')
endif

if not get_option('smbus-telemetry-target').disabled()
  subdir('smbus-telemetry-target')
  if not get_option('libsmbus-test').disabled()
    subdir('smbus-telemetry-target/test')
  endif
endif
