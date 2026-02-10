# RSSReader
This is a simple RSS reader that has been cross-compiled for kindle targets.

## Compilation
Binaries for x86-64 linux and kindlepw2 are included in releases. Otherwise, use the following steps:

Compile for regular targets
```bash
meson setup build
meson compile -C build
```
Compile for kindlepw2
```bash
meson setup builddir_kindlepw2   --cross-file ./x-tools/arm-kindlepw2-linux-gnueabi/meson-crosscompile.txt   -Dglib:selinux=false   -Dglib:libmount=false  -Dlibsoup:introspection=disabled   -Dlibsoup:vapi=disabled --wrap-mode=forcefallback --reconfigure -Dlibsoup:brotli=disabled
meson compile -C builddir_kindlepw2
```
