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


## Downloads

- Windows : https://benjamin.kuperberg.fr/mapgyver/download/app/MapGyver-win-x64-bleedingedge.exe
- MacOS Intel : https://benjamin.kuperberg.fr/mapgyver/download/app/MapGyver-osx-intel-bleedingedge.pkg
- MacOS Silicon : https://benjamin.kuperberg.fr/mapgyver/download/app/MapGyver-osx-silicon-bleedingedge.pkg
- Linux x64 : Not ready yet (you can contribute by compiling and testing it yourself with the instructions below)
- Raspberry 32 bit : Not ready yet (you can contribute by compiling and testing it yourself)
- Raspberry 64 bit : Not ready yet (you can contribute by compiling and testing it yourself)

## Linux specific instructions

- You need to install the [NDI SDK](https://ndi.video/for-developers/ndi-sdk/) (you can use the `ndi-sdk` AUR package on Arch)
- `libvlc` **4.0.x** (for 64bit x86 only) and `Servus` pre-built libraries are included but you might need to recompile them for your needs
- After compiling, run MapGyver with :
```bash
LD_LIBRARY_PATH=../../External/servus/lib/linux:../../External/vlc/lib/linux:$LD_LIBRARY_PATH build/MapGyver
```
