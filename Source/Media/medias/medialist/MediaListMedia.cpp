/*
  ==============================================================================

	MediaListMedia.cpp
	Created: 18 Sep 2025 12:43:29pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaListMedia::MediaListMedia(var params) :
	Media(getTypeString(), params),
	listCC("Media List")
{
	saveAndLoadRecursiveData = true;

	listCC.userCanAddControllables = true;
	listCC.customUserCreateControllableFunc = [](ControllableContainer* parent)
		{
			TargetParameter* tp = parent->addTargetParameter(parent->getUniqueNameInContainer("Media"), String("Media to use"), MediaManager::getInstance());
			tp->targetType = TargetParameter::CONTAINER;
			tp->maxDefaultSearchLevel = 0;
			return tp;
		};


	addChildControllableContainer(&listCC);

	index = addIntParameter("Index", "Index of the media to use", 1, 1);
	index->canBeDisabledByUser = true;
	currentMedia = nullptr;
}

MediaListMedia::~MediaListMedia()
{
}

void MediaListMedia::setCurrentMedia(Media* m)
{
	if (isCurrentlyLoadingData) return;

	GenericScopedLock<SpinLock> lock(mediaLock);
	if (currentMedia == m) return;

	if (currentMedia != nullptr)
	{
		currentMedia->unregisterTarget(this);
	}

	currentMedia = m;

	if (currentMedia != nullptr)
	{
		currentMedia->registerTarget(this);
	}
}

void MediaListMedia::setMediaFromIndex()
{
	Array<WeakReference<Controllable>> controllables = listCC.getAllControllables(true);
	int idx = jlimit(0, controllables.size() - 1, index->intValue() - 1);
	if (auto m = dynamic_cast<TargetParameter*>(controllables[idx].get()))
	{
		setCurrentMedia(m->getTargetAs<Media>());
	}
	else setCurrentMedia(nullptr);
}

void MediaListMedia::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == index)
	{
		setMediaFromIndex();
	}
}

void MediaListMedia::renderGLInternal()
{

}

Point<int> MediaListMedia::getMediaSize()
{
	return currentMediaSize;
}

void MediaListMedia::afterLoadJSONDataInternal()
{

	for (auto& c : listCC.controllables)
	{
		if (auto tp = dynamic_cast<TargetParameter*>(c))
		{
			tp->rootContainer = MediaManager::getInstance();
			tp->targetType = TargetParameter::CONTAINER;
			tp->maxDefaultSearchLevel = 0;
		}
	}

	setMediaFromIndex();
}
