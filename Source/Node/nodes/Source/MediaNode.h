/*
  ==============================================================================

    MediaNode.h
    Created: 2 Dec 2022 10:23:46am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaNode :
    public Node
{
public:
    MediaNode(var params = var());
    ~MediaNode();

    TargetParameter* source;

    NodeConnectionSlot* outBuffer;

    bool initInternal() override;
    void processInternal() override;

    DECLARE_TYPE("Media")
};
