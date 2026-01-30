project('rssreader', 'cpp', version: 'v1.0.0', default_options: ['cpp_std=c++17'], meson_version: '>=1.1')

add_project_arguments('-Wno-deprecated-declarations', language: 'cpp')

# Define dependencies we want
gtk_dep = dependency('gtk+-2.0')
libsoup_dep = dependency('libsoup-2.4')
libxml_dep = dependency('libxml-2.0')

subdir('src')

includes = include_directories(
  './src/'
)

executable(
  'rssreader',
  sources,
  include_directories: includes,
  dependencies: [gtk_dep, libsoup_dep, libxml_dep],
  cpp_args: ['-static-libstdc++'], # add '-DKINDLE' when cross compiling
  link_args: ['-static-libstdc++']
)