/*
  ==============================================================================

	GridClip.cpp
	Created: 11 Jun 2025 4:39:14pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "GridClip.h"

GridClip::GridClip(GridMedia* gridMedia, const String& name, var params) :
	BaseItem(getTypeString(), false),
	gridMedia(gridMedia),
	media(nullptr),
	mediaRef(nullptr),
	gridClipNotifier(5)
{
	jassert(gridMedia != nullptr);

	saveAndLoadRecursiveData = true;

	layerTarget = addTargetParameter("Layer", "Layer to use for this clip", &gridMedia->layerGroupManager);
	layerTarget->targetType = TargetParameter::CONTAINER;
	layerTarget->typesFilter = { GridLayer::getTypeStringStatic() };

	columnTarget = addTargetParameter("Column", "Column to use for this clip", &gridMedia->columnManager);
	columnTarget->targetType = TargetParameter::CONTAINER;
	columnTarget->typesFilter = { GridColumn::getTypeStringStatic() };

	fadeIn = addFloatParameter("Fade In", "Fade in time", 0, 0, INT32_MAX, false);
	fadeIn->defaultUI = FloatParameter::TIME;
	fadeIn->canBeDisabledByUser = true;
	fadeOut = addFloatParameter("Fade Out", "Fade out time", 0, 0, INT32_MAX, false);
	fadeOut->defaultUI = FloatParameter::TIME;
	fadeOut->canBeDisabledByUser = true;

	fadeCurve.addKey(0, 0);
	fadeCurve.addKey(1, 1);
	addChildControllableContainer(&fadeCurve);

}

GridClip::~GridClip()
{
}


void GridClip::clearItem()
{
	clearTarget();
	BaseItem::clearItem();

	setMedia(nullptr);
	if (media != nullptr && !mediaRef.wasObjectDeleted()) unregisterUseMedia(CLIP_MEDIA_ID);
}

void GridClip::setMedia(Media* m)
{
	if (m == media) return;
	if (media != nullptr && !mediaRef.wasObjectDeleted() && !media->isClearing)
	{
		media->removeAsyncMediaListener(this);
		media->handleExit();
		unregisterUseMedia(CLIP_MEDIA_ID);
	}

	media = m;
	mediaRef = media;

	if (media != nullptr)
	{
		media->addAsyncMediaListener(this);
		registerUseMedia(CLIP_MEDIA_ID, media);
		media->handleEnter(0);
	}

	gridClipListeners.call(&GridClipListener::mediaChanged, this);
	gridClipNotifier.addMessage(new GridClipEvent(GridClipEvent::MEDIA_CHANGED, this));
}

void GridClip::newMessage(const Media::MediaEvent& e)
{
	switch (e.type)
	{
	case Media::MediaEvent::PREVIEW_CHANGED:
		gridClipNotifier.addMessage(new GridClipEvent(GridClipEvent::PREVIEW_CHANGED, this));
		break;

	default:
		break;
	}
}



ReferenceGridClip::ReferenceGridClip(GridMedia* gridMedia, var params) :
	GridClip(gridMedia, getTypeString(), params)
{

	mediaTarget = addTargetParameter("Media", "Media to use for this clip", MediaManager::getInstance());
	mediaTarget->targetType = TargetParameter::CONTAINER;
	mediaTarget->maxDefaultSearchLevel = 0;
}

ReferenceGridClip::~ReferenceGridClip()
{
}

void ReferenceGridClip::onContainerParameterChangedInternal(Parameter* p)
{
	GridClip::onContainerParameterChangedInternal(p);

	if (isClearing) return;
	if (p == mediaTarget) setMedia(mediaTarget->getTargetContainerAs<Media>());
}

OwnedGridClip::OwnedGridClip(GridMedia* gridMedia, var params) :
	GridClip(gridMedia, params.getProperty("mediaType", "[notype]").toString(), params),
	ownedMedia(nullptr)
{
	var extraParams(new DynamicObject());
	//extraParams.getDynamicObject()->setProperty("manualRender", true);
	Media* m = MediaManager::getInstance()->factory.createWithExtraParams(params.getProperty("mediaType", "").toString(), extraParams);
	setMedia(m);
}


OwnedGridClip::~OwnedGridClip()
{
}

void OwnedGridClip::setMedia(Media* m)
{
	if (m == media) return;

	if (ownedMedia != nullptr)
	{
		ownedMedia->removeAsyncMediaListener(this);
		removeChildControllableContainer(ownedMedia.get());
		ownedMedia.reset();
	}

	GridClip::setMedia(m);

	if (media != nullptr)
	{
		ownedMedia.reset(media);
		media->addAsyncMediaListener(this);
		addChildControllableContainer(ownedMedia.get());
	}
	else
	{
		ownedMedia.reset();
	}
}
