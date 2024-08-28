/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   composition_png;
    const int            composition_pngSize = 665;

    extern const char*   ndi_png;
    const int            ndi_pngSize = 847;

    extern const char*   node_png;
    const int            node_pngSize = 936;

    extern const char*   picture_png;
    const int            picture_pngSize = 728;

    extern const char*   sequence_png;
    const int            sequence_pngSize = 1994;

    extern const char*   shader_png;
    const int            shader_pngSize = 952;

    extern const char*   sharedTexture_png;
    const int            sharedTexture_pngSize = 2769;

    extern const char*   solidColor_png;
    const int            solidColor_pngSize = 1002;

    extern const char*   video_png;
    const int            video_pngSize = 656;

    extern const char*   webcam_png;
    const int            webcam_pngSize = 1196;

    extern const char*   default_rmplayout;
    const int            default_rmplayoutSize = 2452;

    extern const char*   fragmentShaderMainSurface_glsl;
    const int            fragmentShaderMainSurface_glslSize = 1301;

    extern const char*   fragmentShaderTestGrid_glsl;
    const int            fragmentShaderTestGrid_glslSize = 9164;

    extern const char*   icon_png;
    const int            icon_pngSize = 52976;

    extern const char*   testPattern_png;
    const int            testPattern_pngSize = 85942;

    extern const char*   VertexShaderMainSurface_glsl;
    const int            VertexShaderMainSurface_glslSize = 381;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 16;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
