/*
  ==============================================================================

	SharedTextureMedia.h
	Created: 22 Nov 2023 9:45:13pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class BaseSharedTextureMedia :
	public Media,
	public SharedTextureManager::Listener,
	public SharedTextureReceiver::Listener
{
public:
	BaseSharedTextureMedia(const String& name = "BaseSharedTexture", var params = var());
	virtual ~BaseSharedTextureMedia();

	bool frameUpdated;

	StringParameter* sharingName;
	SharedTextureReceiver* receiver;

	void setupReceiver();

	void onContainerParameterChangedInternal(Parameter* p) override;
	void textureUpdated(SharedTextureReceiver* receiver) override;

	void renderGLInternal() override;

	Point<int> getDefaultMediaSize() override;

	void afterLoadJSONDataInternal() override;

};

class SharedTextureMedia :
	public BaseSharedTextureMedia
{
public:
	SharedTextureMedia(var params = var()) : BaseSharedTextureMedia(getTypeString(), params) {}
	~SharedTextureMedia() {}

	DECLARE_TYPE("Shared Texture")
};
