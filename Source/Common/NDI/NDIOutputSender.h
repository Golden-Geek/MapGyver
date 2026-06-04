/*
  ==============================================================================

    NDIOutputSender.h
    Created: 03 Jun 2026
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class NDIOutputSender
{
public:
    NDIOutputSender(const String& name, int width, int height);
    ~NDIOutputSender();

    void setName(const String& name);
    void setSize(int width, int height);
    void setEnabled(bool enabled);

    void sendFrame(juce::OpenGLFrameBuffer& frameBuffer);

private:
    String senderName;
    int width, height;
    bool isEnabled;

    NDIlib_send_instance_t pNDI_send;
    std::vector<uint8_t> pixelBuffer;

    void recreateSender();
    void destroySender();
};
