/*
  ==============================================================================

	SharedTextureMedia.h
	Created: 22 Nov 2023 9:45:13pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class SharedTextureMedia :
	public Media,
	public SharedTextureManager::Listener,
	public SharedTextureReceiver::Listener
{
public:
	SharedTextureMedia(var params = var());
	~SharedTextureMedia();
	
	bool frameUpdated;

	StringParameter* sharingName;
	SharedTextureReceiver* receiver;

	void setupReceiver();

	void onContainerParameterChangedInternal(Parameter* p) override;
	void textureUpdated(SharedTextureReceiver* receiver) override;

	void renderGLInternal();

	Point<int> getMediaSize();

	void afterLoadJSONDataInternal() override;

	DECLARE_TYPE("Shared Texture")
};