/*
  ==============================================================================

	fListMedia.cpp
	Created: 18 Sep 2025 12:43:29pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "MediaListMedia.h"

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
			tp->saveValueOnly = false;
			return tp;
		};


	addChildControllableContainer(&listCC);

	index = addIntParameter("Index", "Index of the media to use", 1, 1);
	currentMedia = nullptr;

	alwaysRedraw = true;
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
		unregisterUseMedia(0);
	}

	currentMedia = m;
	currentMediaSize = Point<int>(0, 0);

	if (currentMedia != nullptr)
	{
		registerUseMedia(0, currentMedia);
		currentMediaSize = currentMedia->getMediaSize();

	}
}

void MediaListMedia::setMediaFromIndex()
{
	Array<WeakReference<Controllable>> controllables = listCC.getAllControllables(true);
	int idx = jlimit(0, controllables.size() - 1, index->intValue() - 1);
	if (auto m = dynamic_cast<TargetParameter*>(controllables[idx].get()))
	{
		setCurrentMedia(m->getTargetContainerAs<Media>());
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

void MediaListMedia::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	if(cc == &listCC)
	{
		if (auto tp = dynamic_cast<TargetParameter*>(c))
		{
			setMediaFromIndex();
		}
	}
}

void MediaListMedia::renderGLInternal()
{
	GenericScopedLock<SpinLock> lock(mediaLock);
	if (currentMedia != nullptr)
	{
		//draw current media framebuffer
		if (currentMedia->getFrameBuffer() != nullptr)
		{
			glColor4f(1, 1, 1, 1);
			glBindTexture(GL_TEXTURE_2D, currentMedia->getTextureID());
			Draw2DTexRect(0, 0, frameBuffer.getWidth(), frameBuffer.getHeight());
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
	else
	{
		glColor4f(0, 0, 0, 1);
		Draw2DRect(0, 0, frameBuffer.getWidth(), frameBuffer.getHeight());
	}
}

Point<int> MediaListMedia::getDefaultMediaSize()
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
			tp->saveValueOnly = false;
		}
	}

	setMediaFromIndex();
}
