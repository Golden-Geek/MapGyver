/*
  ==============================================================================

    MediaImage.h
    Created: 26 Sep 2020 1:51:42pm
    Author:  Mediaupe

  ==============================================================================
*/

#pragma once

class MediaImage :
    public Media
{
public:
    MediaImage(var params = var());
    ~MediaImage();

    FileParameter* filePath;

    void clearItem() override;
    void onContainerParameterChanged(Parameter* p) override;
    
    DECLARE_TYPE("Image")
};