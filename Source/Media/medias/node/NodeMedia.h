/*
  ==============================================================================

    NodeMedia.h
    Created: 28 Nov 2023 12:07:51pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class RootNodeManager;

class NodeMedia :
	public Media
{
public:
	NodeMedia(var params = var());
	~NodeMedia();

	std::unique_ptr<RootNodeManager> nodes;

	void renderGLInternal() override;
	Point<int> getDefaultMediaSize() override;

	DECLARE_TYPE("Node")
};