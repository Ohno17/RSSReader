project('libpsl', 'c', version: '0.21.0')

# Create an empty dependency that does nothing
libpsl_dep = declare_dependency(
  include_directories: include_directories('.')
)

# This allows libsoup to find it via subproject
meson.override_dependency('libpsl', libpsl_dep)