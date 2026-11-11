# MapGyver

Cross-platform lightweight video mapping software

Features :
- Spout / Syphon texture sharing (source / output)
- NDI texture sharing (source / output)
- 4-corner perspective warping + bezier warping
- Video playback of almost any format through VLC lib
- Live Shader Manipulation (GLSL / ShaderToy) as sources and effects
- Online Explorer (ShaderToy / ISF / Pexels Photo / Pexels Video) with drag'n'drop mechanism
- Node Engine
- Compositions : Manage multiple medias, compose, blend and transform
- OrganicUI framework : fully controllable from external software like Chataigne
- Timeline : Time based animations, blending, etc.

## Interactive App textures

`Texture Selection Mode` defaults to `Automatic`. Discovery checks for new outputs regularly and keeps the selected texture name while its sender is unavailable. `Refresh Textures` rescans outputs and recreates their receivers to recover a stalled connection.

Choose `Exact Name` and enter the full sender name in `Texture Name` to connect without OSCQuery discovery or choosing from the texture list. Choose `Name Suffix` to match the end of the sender name across computers (for example, `/Main` matches both `Computer-A/Main` and `Computer-B/Main`). Suffix matching reconnects automatically when a matching sender appears; if several senders match, use a longer suffix or an exact name.


## Downloads

- Windows : https://www.goldengeek.org/mapgyver/download/app/MapGyver-win-x64-bleedingedge.exe
- MacOS Intel :Not ready yet (you can contribute by compiling and testing it yourself)
- MacOS Silicon : https://www.goldengeek.org/mapgyver/download/app/MapGyver-osx-silicon-bleedingedge.pkg
- Linux x64 : AppImage built by CI on tags, available on the [releases page](https://github.com/Golden-Geek/MapGyver/releases) (you can also compile it yourself with the instructions below)
- Raspberry 32 bit : Not ready yet (you can contribute by compiling and testing it yourself)
- Raspberry 64 bit : Not ready yet (you can contribute by compiling and testing it yourself)

## Linux specific instructions

### Dependencies
- [NDI SDK](https://ndi.video/for-developers/ndi-sdk/) (proprietary, required) :
  - Arch Linux : install the `ndi-sdk` AUR package.
  - Debian/Ubuntu : download the SDK, extract it, then copy `include/*` to `/usr/local/include` and `bin/x86_64/lib*` to `/usr/local/lib`.
- Build dependencies :
  - Arch Linux : `sudo pacman -S --needed base-devel git freetype2 libx11 libxinerama libxrandr libxcursor libxcomposite mesa alsa-lib gtk3 webkit2gtk-4.1 curl openssl mpv`
  - Debian/Ubuntu : run `./install_linux_deps.sh`
- Optional : enable the Avahi daemon (`sudo systemctl enable --now avahi-daemon`) for zeroconf network discovery (NDI announcement, OSCQuery senders).

### Building
The easiest way is the provided build script, which handles submodules, JUCE and the runtime files staging:
```bash
git clone --recursive https://github.com/Golden-Geek/MapGyver.git
cd MapGyver
./linux_build.sh
```

Or manually :
```bash
# Clone benkuper's JUCE fork (branch juce8_local) into ./JUCE
git clone --branch juce8_local http://github.com/benkuper/JUCE JUCE

# Build
make -C Builds/LinuxMakefile -j$(nproc) CONFIG=Release

# Stage the bundled runtime libs and Ultralight resources next to the binary
mkdir -p Builds/LinuxMakefile/build/lib Builds/LinuxMakefile/build/resources
cp External/servus/lib/linux/libServus.so.6 External/ultralight/lib/linux/*.so Builds/LinuxMakefile/build/lib/
cp -r Resources/ultralight/resources/. Builds/LinuxMakefile/build/resources/
```

If you re-save the project in Projucer, the Makefile regenerates with the correct settings : the Linux exporter already declares all the needed external libs (Servus, NDI, mpv, Ultralight), search paths and the `$ORIGIN` rpath.

### Running
After compiling, run MapGyver with :
```bash
./Builds/LinuxMakefile/build/MapGyver
```
The binary uses an `$ORIGIN` rpath, so it finds Servus and Ultralight libs in `build/lib` and the Ultralight resources in `build/resources` automatically.
