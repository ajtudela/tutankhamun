#!/usr/bin/env sh
# The macOS workspace: not available yet. tutankhamun is a 32-bit Windows program today (MSVC, Win32, the Windows
# API in its platform and start-up code); a macOS build needs a 64-bit build and a POSIX platform layer. The script
# stops instead of making a build that would not run.
echo "update-workspace-mac.sh: the macOS build is not available yet." >&2
echo "tutankhamun builds only as a 32-bit Windows program (MSVC, Win32) today. On Windows use update-workspace.bat or build.bat." >&2
exit 1
