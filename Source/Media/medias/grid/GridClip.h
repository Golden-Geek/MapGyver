/*
  ==============================================================================

	GridClip.h
	Created: 11 Jun 2025 4:39:14pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class GridMedia;

class GridClip :
	public BaseItem,
	public MediaTarget,
	public Media::AsyncListener
{
public:
	GridClip(GridMedia* gridMedia = nullptr, const String& name = "", var params = var());
	~GridClip() override;

	Media* gridMedia;

	Media* media;
	WeakReference<Inspectable> mediaRef;

	TargetParameter* layerTarget;
	TargetParameter* columnTarget;

	FloatParameter* fadeIn;
	FloatParameter* fadeOut;
	Automation fadeCurve;

	virtual void clearItem()override;
	virtual void setMedia(Media* m);

	void newMessage(const Media::MediaEvent& e) override;

	class  GridClipListener
	{
	public:
		/** Destructor. */
		virtual ~GridClipListener() {}
		virtual void mediaChanged(GridClip*) {}
		virtual void mediaClipFadesChanged(GridClip*) {}
	};

	DECLARE_INSPECTACLE_SAFE_LISTENER(GridClip, gridClip);

	DECLARE_ASYNC_EVENT(GridClip, GridClip, gridClip, ENUM_LIST(MEDIA_CHANGED, FADES_CHANGED, PREVIEW_CHANGED), EVENT_ITEM_CHECK);


	DECLARE_TYPE("Grid Clip");

};

class ReferenceGridClip :
	public GridClip
{
public:
	ReferenceGridClip(GridMedia* gridMedia, var params = var());
	virtual ~ReferenceGridClip();

	TargetParameter* mediaTarget;
	void onContainerParameterChangedInternal(Parameter* p) override;

	static String getTypeStringStatic() { return "Reference Grid Clip"; }
	String getTypeString() const override { return getTypeStringStatic(); }
	static ReferenceGridClip* create(GridMedia* gridMedia, var params = var()) { return new ReferenceGridClip(gridMedia, params); }

};

class OwnedGridClip :
	public GridClip
{
public:
	OwnedGridClip(GridMedia* gridMedia = nullptr, var params = var());
	virtual ~OwnedGridClip();

	std::unique_ptr<Media> ownedMedia;

	void setMedia(Media* m) override;

	String getTypeString() const override { return ownedMedia != nullptr ? ownedMedia->getTypeString() : ""; }

	static OwnedGridClip* create(GridMedia* gridMedia, var params = var()) { return new OwnedGridClip(gridMedia, params); }
};