/*
  ==============================================================================

	ClipTransition.h
	Created: 27 Apr 2024 12:48:07pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class ClipTransition : public MediaClip
{
public:
	ClipTransition(var params = var());
	~ClipTransition();

	MediaClip* inMedia;
	MediaClip* outMedia;

	ShaderMedia shaderMedia;

	FloatParameter* progressParam;
	TargetParameter* mediaInParam;
	TargetParameter* mediaOutParam;

	void setInOutMedia(MediaClip* in, MediaClip* out);
	virtual void setTime(double time, bool seekMode) override;

	void onControllableFeedbackUpdateInternal(ControllableContainer*cc, Controllable* c) override;

	void loadJSONDataItemInternal(var data) override;

	DECLARE_TYPE("Transition")
};