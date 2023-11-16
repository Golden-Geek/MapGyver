

// https://github.com/COx2/juce_meets_ndi/blob/master/NdiReceiver/Source/NdiWrapper.h

class NdiVideoHelper
{
public:
    static void YUVfromRGB(double& Y, double& U, double& V, const double R, const double G, const double B)
    {
        Y = 0.257 * R + 0.504 * G + 0.098 * B + 16;
        U = -0.148 * R - 0.291 * G + 0.439 * B + 128;
        V = 0.439 * R - 0.368 * G - 0.071 * B + 128;
    }

    static void RGBfromYUV(double& R, double& G, double& B, double Y, double U, double V)
    {
        Y -= 16;
        U -= 128;
        V -= 128;
        R = 1.164 * Y + 1.596 * V;
        G = 1.164 * Y - 0.392 * U - 0.813 * V;
        B = 1.164 * Y + 2.017 * U;
    }

    static juce::Colour GetColourFromYCbCr(int y, int cb, int cr, int a)
    {
        double Y = (double)y;
        double Cb = (double)cb;
        double Cr = (double)cr;

        int r = (int)(Y + 1.40200 * (Cr - 0x80));
        int g = (int)(Y - 0.34414 * (Cb - 0x80) - 0.71414 * (Cr - 0x80));
        int b = (int)(Y + 1.77200 * (Cb - 0x80));

        r = juce::jmax(0, juce::jmin(255, r));
        g = juce::jmax(0, juce::jmin(255, g));
        b = juce::jmax(0, juce::jmin(255, b));

        return juce::Colour::fromRGBA(r, g, b, a);
    }

    static juce::Colour GetColourFromYUV(int y, int u, int v, int a)
    {
        double Y = (double)y;
        double U = (double)u;
        double V = (double)v;

        int r = (int)(1.164 * (Y - 16) + 1.596 * (V - 128));
        int g = (int)(1.164 * (Y - 16) - 0.813 * (V - 128) - 0.391 * (U - 128));
        int b = (int)(1.164 * (Y - 16) + 2.018 * (U - 128));

        r = juce::jmax(0, juce::jmin(255, r));
        g = juce::jmax(0, juce::jmin(255, g));
        b = juce::jmax(0, juce::jmin(255, b));

        return juce::Colour::fromRGBA(r, g, b, a);
    }

    static void convertVideoFrame(NDIlib_video_frame_v2_t* srcFrame, Image& image)
    {
    if (image.getWidth() != srcFrame->xres || image.getHeight() != srcFrame->yres) {return;}

        switch (srcFrame->FourCC)
        {
        case NDIlib_FourCC_video_type_e::NDIlib_FourCC_type_RGBA:
        {
            for (int y_idx = 0; y_idx < srcFrame->yres; ++y_idx)
            {
                for (int x_idx = 0; x_idx < srcFrame->xres; ++x_idx)
                {
                    const int pix_idx = x_idx + y_idx * srcFrame->xres;
                    const int fourcc_idx = pix_idx * 4;
                    juce::Colour col = juce::Colour::fromRGBA(srcFrame->p_data[fourcc_idx + 0], srcFrame->p_data[fourcc_idx + 1], srcFrame->p_data[fourcc_idx + 2], srcFrame->p_data[fourcc_idx + 3]);
                    image.setPixelAt(x_idx, y_idx, col);
                }
            }
        }
        break;
        case NDIlib_FourCC_video_type_e::NDIlib_FourCC_type_RGBX:
        {
            for (int y_idx = 0; y_idx < srcFrame->yres; ++y_idx)
            {
                for (int x_idx = 0; x_idx < srcFrame->xres; ++x_idx)
                {
                    const int pix_idx = x_idx + y_idx * srcFrame->xres;
                    const int fourcc_idx = pix_idx * 3;
                    juce::Colour col = juce::Colour::fromRGBA(srcFrame->p_data[fourcc_idx + 0], srcFrame->p_data[fourcc_idx + 1], srcFrame->p_data[fourcc_idx + 2], 255);
                    image.setPixelAt(x_idx, y_idx, col);
                }
            }
        }
        break;
        case NDIlib_FourCC_video_type_e::NDIlib_FourCC_type_BGRA:
        {
            for (int y_idx = 0; y_idx < srcFrame->yres; ++y_idx)
            {
                for (int x_idx = 0; x_idx < srcFrame->xres; ++x_idx)
                {
                    const int pix_idx = x_idx + y_idx * srcFrame->xres;
                    const int fourcc_idx = pix_idx * 4;
                    juce::Colour col = juce::Colour::fromRGBA(srcFrame->p_data[fourcc_idx + 2], srcFrame->p_data[fourcc_idx + 1], srcFrame->p_data[fourcc_idx + 0], srcFrame->p_data[fourcc_idx + 3]);
                    image.setPixelAt(x_idx, y_idx, col);
                }
            }
        }
        break;
        case NDIlib_FourCC_video_type_e::NDIlib_FourCC_type_BGRX:
        {
            for (int y_idx = 0; y_idx < srcFrame->yres; ++y_idx)
            {
                for (int x_idx = 0; x_idx < srcFrame->xres; ++x_idx)
                {
                    const int pix_idx = x_idx + y_idx * srcFrame->xres;
                    const int fourcc_idx = pix_idx * 3;
                    juce::Colour col = juce::Colour::fromRGBA(srcFrame->p_data[fourcc_idx + 2], srcFrame->p_data[fourcc_idx + 1], srcFrame->p_data[fourcc_idx + 0], 255);
                    image.setPixelAt(x_idx, y_idx, col);
                }
            }
        }
        break;
        case NDIlib_FourCC_video_type_e::NDIlib_FourCC_type_UYVY:
        {
            for (int y_idx = 0; y_idx < srcFrame->yres; ++y_idx)
            {
                for (int x_idx = 0; x_idx < srcFrame->xres; x_idx += 2)
                {
                    const int pix_idx = x_idx + y_idx * srcFrame->xres;
                    const int fourcc_idx = pix_idx * 2;
                    const int u0 = srcFrame->p_data[fourcc_idx + 0];
                    const int y0 = srcFrame->p_data[fourcc_idx + 1];
                    const int v0 = srcFrame->p_data[fourcc_idx + 2];
                    const int y1 = srcFrame->p_data[fourcc_idx + 3];
                    juce::Colour col0 = GetColourFromYUV(y0, u0, v0, 255);
                    image.setPixelAt(x_idx, y_idx, col0);

                    juce::Colour col1 = GetColourFromYUV(y1, u0, v0, 255);
                    image.setPixelAt(x_idx + 1, y_idx, col1);
                }
            }
        }
        break;
        case NDIlib_FourCC_video_type_e::NDIlib_FourCC_video_type_UYVA:
        {
            for (int y_idx = 0; y_idx < srcFrame->yres; ++y_idx)
            {
                for (int x_idx = 0; x_idx < srcFrame->xres; ++x_idx)
                {
                    const int pix_idx = x_idx + y_idx * srcFrame->xres;
                    const int fourcc_idx = pix_idx * 3;
                    const int y = srcFrame->p_data[fourcc_idx + 0];
                    const int cb = srcFrame->p_data[fourcc_idx + 1] & 0xf0 >> 4;
                    const int cr = srcFrame->p_data[fourcc_idx + 1] & 0x0f;
                    const int a = srcFrame->p_data[fourcc_idx + 2];
                    juce::Colour col = GetColourFromYCbCr(y, cb, cr, a);
                    image.setPixelAt(x_idx, y_idx, col);
                }
            }
        }
        break;
        default:
            break;
        }

    }

};