/*
  ==============================================================================

    NDIOutputSender.cpp
    Created: 03 Jun 2026
    Author:  bkupe

  ==============================================================================
*/

#include "Common/CommonIncludes.h"

using namespace juce::gl;

NDIOutputSender::NDIOutputSender(const String& name, int width, int height) :
    senderName(name),
    width(width),
    height(height),
    isEnabled(true),
    pNDI_send(nullptr)
{
    pixelBuffer.resize(width * height * 4);
    recreateSender();
}

NDIOutputSender::~NDIOutputSender()
{
    destroySender();
}

void NDIOutputSender::setName(const String& name)
{
    if (senderName == name) return;
    senderName = name;
    recreateSender();
}

void NDIOutputSender::setSize(int w, int h)
{
    width = w;
    height = h;
    pixelBuffer.resize(w * h * 4);
}

void NDIOutputSender::setEnabled(bool enabled)
{
    isEnabled = enabled;
}

void NDIOutputSender::sendFrame(juce::OpenGLFrameBuffer& frameBuffer)
{
    if (!isEnabled || pNDI_send == nullptr) return;

    int w = frameBuffer.getWidth();
    int h = frameBuffer.getHeight();

    if (w == 0 || h == 0) return;

    if (w != width || h != height)
        setSize(w, h);

    // Read pixels from the FBO — OpenGL stores rows bottom-up
    glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer.getFrameBufferID());
    glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixelBuffer.data());
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    // Flip vertically so NDI receives top-down row order
    const int stride = w * 4;
    for (int y = 0; y < h / 2; y++)
        std::swap_ranges(
            pixelBuffer.data() + y * stride,
            pixelBuffer.data() + y * stride + stride,
            pixelBuffer.data() + (h - 1 - y) * stride);

    NDIlib_video_frame_v2_t frame;
    frame.xres = w;
    frame.yres = h;
    frame.FourCC = NDIlib_FourCC_video_type_RGBA;
    frame.frame_rate_N = 60000;
    frame.frame_rate_D = 1000;
    frame.picture_aspect_ratio = (float)w / (float)h;
    frame.frame_format_type = NDIlib_frame_format_type_progressive;
    frame.timecode = NDIlib_send_timecode_synthesize;
    frame.p_data = pixelBuffer.data();
    frame.line_stride_in_bytes = stride;
    frame.p_metadata = nullptr;
    frame.timestamp = 0;

    NDIlib_send_send_video_v2(pNDI_send, &frame);
}

void NDIOutputSender::recreateSender()
{
    destroySender();

    NDIlib_send_create_t sendSettings;
    sendSettings.p_ndi_name = senderName.toRawUTF8();
    sendSettings.p_groups = nullptr;
    sendSettings.clock_video = false;
    sendSettings.clock_audio = false;

    pNDI_send = NDIlib_send_create(&sendSettings);
}

void NDIOutputSender::destroySender()
{
    if (pNDI_send != nullptr)
    {
        NDIlib_send_destroy(pNDI_send);
        pNDI_send = nullptr;
    }
}
