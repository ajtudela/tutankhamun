#!/usr/bin/env sh
# The macOS workspace: not available yet. The platform layer of the Linux build (src/platform/platform_posix.c,
# src/platform/platform_file.c, src/app/port_posix.c) is POSIX and SDL, but the program's own files
# (src/app/app_files.c: /proc/self/exe) and the build have not been made and tried on macOS. The script stops instead
# of making a build that may not run.
echo "update-workspace-mac.sh: the macOS build is not available yet." >&2
echo "tutankhamun builds on Windows (update-workspace.bat, build.bat) and Linux (update-workspace-linux.sh)." >&2
exit 1
