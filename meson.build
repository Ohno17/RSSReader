project('rssreader', 'cpp', 
  version: 'v1.0.0', 
  default_options: ['cpp_std=c++17'], 
  meson_version: '>=1.1'
)

add_global_arguments([
  '-D_GNU_SOURCE'
], language: ['c', 'cpp'])

add_project_arguments('-Wno-deprecated-declarations', language: ['cpp', 'c'])

zlib_dep = dependency('zlib', 
  fallback: ['zlib', 'zlib_dep'],
  default_options: ['default_library=static']
)

libffi_sub = subproject('libffi', 
  default_options: [
    'default_library=static',
    'tests=false'  
  ]
)
libffi_dep = libffi_sub.get_variable('ffi_dep')

glib_dep = dependency('glib-2.0', 
  fallback: ['glib', 'libglib_dep'], 
  default_options: [
    'default_library=static',
    'selinux=disabled', 
    'libmount=disabled',
    'xattr=false',      
    'installed_tests=false'
  ]
)

# whyyyyy
libsoup_dep = dependency('libsoup-2.4', 
  fallback: ['libsoup', 'libsoup_dep'], 
  default_options: [
    'default_library=static', 
    'tests=false', 
    'introspection=disabled', 
    'vapi=disabled',
    'sysprof=disabled',
    'tls_check=false'
  ]
)

gtk_dep = dependency('gtk+-2.0') 
libxml_dep = dependency('libxml-2.0')

subdir('src')

executable(
  'rssreader',
  sources,
  include_directories: include_directories('src'),
  dependencies: [
    gtk_dep, 
    libsoup_dep, 
    libxml_dep, 
    glib_dep,
    libffi_dep,
    zlib_dep
  ],
  c_args: [
  ],
  cpp_args: [
    '-static-libstdc++', 
    '-static-libgcc'
  ],
  link_args: ['-static-libstdc++', '-static-libgcc']
)