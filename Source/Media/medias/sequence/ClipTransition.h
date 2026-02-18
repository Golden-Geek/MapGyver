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

	MediaClip* inClip;
	MediaClip* outClip;

	std::unique_ptr<ShaderMedia> shaderMedia;

	FloatParameter* progressParam;
	TargetParameter* clipInParam;
	TargetParameter* clipOutParam;
	TargetParameter* mediaInParam;
	TargetParameter* mediaOutParam;

	void clearItem() override;

	void setInOutClips(MediaClip* in, MediaClip* out);
	virtual void setTime(double time, bool seekMode) override;

	void setInClip(MediaClip* in);
	void setOutClip(MediaClip* out);

	void computeTimes(MediaClip* origin);

	void onContainerParameterChangedInternal(Parameter* p) override;
	void onExternalParameterValueChanged(Parameter* p) override;

	void loadJSONDataItemInternal(var data) override;

	DECLARE_TYPE("Transition")
};