project('rss_reader', 'cpp', version: 'v1.0.0', default_options: ['cpp_std=c++17'], meson_version: '>=1.1')

# Define dependencies we want
gtk_dep = dependency('gtk+-2.0')
libsoup_dep = dependency('libsoup-2.4')
libxml_dep = dependency('libxml-2.0')

###
# Project definition
###
sources = files(
    './src/main.cpp',
    './src/save.cpp'
)

include_dirs = include_directories(
  './src/'
)

executable('rss_reader', sources, include_directories: include_dirs, dependencies: [gtk_dep, libsoup_dep, libxml_dep], cpp_args: '-static-libstdc++ -Wdeprecated-declarations', link_args: '-static-libstdc++')