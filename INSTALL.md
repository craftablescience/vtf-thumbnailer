# Installing VTF Thumbnailer

This document covers the different ways you can install VTF Thumbnailer on its supported platforms:

- Windows 7/8/8.1/10/11 (64-bit)
- Linux (64-bit)

## Windows

1. You will need to install the VS2015-2022 runtime located at https://aka.ms/vs/17/release/vc_redist.x64.exe.
2. When that is installed, download the installer from [the latest
   GitHub release](https://github.com/craftablescience/vtf-thumbnailer/releases/latest), under the `Assets` dropdown.
3. Run the installer. When running the application Windows will give you a safety warning, ignore it and hit
   `More Info` → `Run Anyway`.

## Linux

Installation on Linux will vary depending on your distro. If your distro is not listed, you will probably have to
compile from source.

#### Debian-based:

###### Automatic:

1. Visit https://craftablescience.info/ppa/ and follow the instructions.
2. VTF Thumbnailer should now be installable and upgradable from `apt` (the package name being `vtf-thumbnailer`, or
   `vtf-thumbnailer-kde5` on KDE 5).

###### Manual:

1. Download the installer from the GitHub releases section, and extract the `.deb` file from inside.
2. Run `sudo apt install ./<name of deb file>.deb` in the directory you extracted it to.
