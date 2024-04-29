/*
  ==============================================================================

	Object.h
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#pragma once


class CompositionLayer :
	public BaseItem,
	public MediaTarget
{
public:
	CompositionLayer(const String& name = "Layer", var params = var());
	virtual ~CompositionLayer();

	Media* media;

	Point2DParameter* position;
	Point2DParameter* size;
	FloatParameter* alpha;
	FloatParameter* rotation;

	enum blendPreset {
		STANDARD, ADDITION, MULTIPLICATION, SCREEN, DARKEN, PREMULTALPHA, LIGHTEN, INVERT, COLORADD, COLORSCREEN, BLUR, INVERTCOLOR, SUBSTRACT, COLORDIFF, INVERTMULT, CUSTOM
	};
	enum blendOption { ZERO, ONE, SRC_ALPHA, ONE_MINUS_SRC_ALPHA, DST_ALPHA, ONE_MINUS_DST_ALPHA, SRC_COLOR, ONE_MINUS_SRC_COLOR, DST_COLOR, ONE_MINUS_DST_COLOR };

	EnumParameter* blendFunction;
	EnumParameter* blendFunctionSourceFactor;
	EnumParameter* blendFunctionDestinationFactor;

	virtual void onContainerParameterChangedInternal(Parameter* p);
	virtual void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

	virtual void setMedia(Media* m);

	bool isUsingMedia(Media* m) override;
};

class ReferenceCompositionLayer : public CompositionLayer
{
public:
	ReferenceCompositionLayer(var params = var());
	virtual ~ReferenceCompositionLayer();

	TargetParameter* targetMedia;

	void onContainerParameterChangedInternal(Parameter* p) override;

	DECLARE_TYPE("Reference Layer");
};

class OwnedCompositionLayer : public CompositionLayer
{
public:
	OwnedCompositionLayer(var params = var());
	virtual ~OwnedCompositionLayer();

	void setMedia(Media* m) override;

	std::unique_ptr<Media> ownedMedia;

	String getTypeString() const override { return ownedMedia != nullptr ? ownedMedia->getTypeString() : ""; }
	static OwnedCompositionLayer* create(var params) { return new OwnedCompositionLayer(params); }
};
