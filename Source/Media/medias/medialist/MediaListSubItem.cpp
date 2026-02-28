/*
  ==============================================================================

	MediaListSubItem.cpp
	Created: 28 Feb 2026 9:58:37am
	Author:  bkupe

  ==============================================================================
*/
#include "Media/MediaIncludes.h"
#include "MediaListSubItem.h"

#define MEDIALISTITEM_MEDIA_ID 0
#define MEDIALISTITEM_TRANSITION_ID 1

MediaListSubItem::MediaListSubItem(const String& name, var params) :
	ControllableContainer(name),
	media(nullptr),
	listSubItemNotifier(10),
	reference(nullptr),
	forceRenderShader(false)
{
	type = addEnumParameter("Type", "Type of the media layer");
	type->addOption("Empty", "empty")->addOption("Reference", "reference")->addOption("Sub Texture", "sub_texture");
	for (auto& m : MediaManager::getInstance()->factory.defs)
	{
		type->addOption(m->type, m->type);
	}

	textureName = addEnumParameter("Texture name", "Name of the texture to use from the media");

	var manualRenderParams(new DynamicObject());
	manualRenderParams.getDynamicObject()->setProperty("manualRender", true);
	shaderMedia.reset(new ShaderMedia(manualRenderParams));


	shaderMedia->enabled->setValue(false);
	shaderMedia->addAsyncInspectableListener(this);
	shaderMedia->setNiceName("Transition Shader");
	transitionProgression = shaderMedia->mediaParams.addFloatParameter("progression", "Progression", 0, 0, 1);
	transitionProgression->setControllableFeedbackOnly(true);
	transitionSourceMedia = shaderMedia->sourceMedias.addTargetParameter("Source", "Media");
	transitionSourceMedia->targetType = TargetParameter::CONTAINER;
	transitionSourceMedia->defaultContainerTypeCheckFunc = [](ControllableContainer* cc) { return dynamic_cast<Media*>(cc) != nullptr; };
	transitionTargetMedia = shaderMedia->sourceMedias.addTargetParameter("Target", "Media");
	transitionTargetMedia->targetType = TargetParameter::CONTAINER;
	transitionTargetMedia->defaultContainerTypeCheckFunc = [](ControllableContainer* cc) { return dynamic_cast<Media*>(cc) != nullptr; };


	addChildControllableContainer(shaderMedia.get());
	shaderMedia->editorIsCollapsed = true;
}

MediaListSubItem::~MediaListSubItem()
{
}


void MediaListSubItem::clear()
{
	if (ownedMedia != nullptr)
	{
		ownedMedia->clearItem();
		removeChildControllableContainer(ownedMedia.get());
		ownedMedia.reset();
	}
	setMedia(nullptr);
	shaderMedia->removeAsyncInspectableListener(this);
	shaderMedia->clearItem();
	MediaTarget::clearTarget();
	ControllableContainer::clear();
}


void MediaListSubItem::onContainerParameterChanged(Parameter* p)
{
	ControllableContainer::onContainerParameterChanged(p);
	if (p == type)
	{
		updateCurrentMedia();
	}
	else if (p == reference)
	{
		updateCurrentMedia();
	}
	else if (p == textureName)
	{
		updateCurrentMedia();
	}
}

void MediaListSubItem::onControllableFeedbackUpdate(ControllableContainer* cc, Controllable* c)
{
	ControllableContainer::onControllableFeedbackUpdate(cc, c);

	if (media != nullptr && (c == media->width || c == media->height))
	{
		//size->setPoint(media->getMediaSize().toFloat());
	}

	if (c == shaderMedia->enabled)
	{
		if (shaderMedia->enabled->boolValue())
		{
			transitionTargetMedia->setValueFromTarget(media);
		}
		else
		{
		}
	}
}


void MediaListSubItem::updateCurrentMedia(bool force)
{
	if (isCurrentlyLoadingData && !force) return;

	GenericScopedLock lock(GlContextHolder::getInstance()->renderLock);

	String mType = type->getValueData().toString();


	bool isOwned = false;

	if (mType == "reference")
	{
		if (reference == nullptr)
		{
			reference = addTargetParameter("Reference", "Media list item to reference");
		}
	}
	else
	{
		if (reference != nullptr)
		{
			removeControllable(reference);
			reference = nullptr;
		}
	}

	if (mType == "empty")
	{
		setMedia(nullptr);
	}
	else if (mType == "reference")
	{

		if (reference != nullptr)
		{
			if (Media* refMedia = reference->getTargetContainerAs<Media>())
			{
				setMedia(refMedia);
			}
			else
			{
				setMedia(nullptr);
			}
		}
		else
		{
			setMedia(nullptr);
		}
	}
	else if (mType == "sub_texture")
	{
		if (textureName->getValue().toString().isNotEmpty())
		{
			setMedia(nullptr);
		}
	}
	else
	{
		if (auto* m = MediaManager::getInstance()->factory.create(mType))
		{
			setMedia(m);
			isOwned = true;
		}
		else
		{
			setMedia(nullptr);
		}
	}

	if (ownedMedia != nullptr)
	{
		ownedMedia->clearItem();
		removeChildControllableContainer(ownedMedia.get());
		ownedMedia.reset();
	}

	if (isOwned)
	{
		ownedMedia.reset(media);
		if (media != nullptr) addChildControllableContainer(media);
	}
}

void MediaListSubItem::setMedia(Media* m)
{
	if (media == m) return;

	if (media != nullptr)
	{
		media->removeAsyncInspectableListener(this);
		media->removeAsyncMediaListener(this);
		unregisterUseMedia(MEDIALISTITEM_MEDIA_ID);
	}

	media = m;


	if (media != nullptr)
	{
		Point<int> mediaSize = media->getMediaSize();
		if (mediaSize.x > 0 && mediaSize.y > 0)
		{
			shaderMedia->width->setValue(mediaSize.getX());
			shaderMedia->height->setValue(mediaSize.getY());
		}
		media->addAsyncMediaListener(this);
		media->addAsyncInspectableListener(this);
	}

	updateTextureNameOptions();

	transitionTargetMedia->setValueFromTarget(media);
}

void MediaListSubItem::setupTransition(Media* source)
{
	if (shaderMedia->enabled->boolValue())
	{
		transitionProgression->setValue(0.f);
		transitionSourceMedia->setValueFromTarget(source);
		transitionTargetMedia->setValueFromTarget(media);
		forceRenderShader = true;
	}
}

void MediaListSubItem::renderShaderIfNecessary()
{
	if (forceRenderShader)
	{
		NLOG(niceName, "Force render shader");
		shaderMedia->renderOpenGLMedia(true);
		forceRenderShader = false;
	}
}

void MediaListSubItem::updateTextureNameOptions()
{
	if (media == nullptr) return;
	StringArray textureNames = media->getFrameBufferNames();

	Array<EnumParameter::EnumValue> options;
	for (auto& tn : textureNames)
		options.add({ tn, tn });
	textureName->setOptions(options);
}

void MediaListSubItem::render(bool isLoading)
{
	if (media != nullptr) media->renderOpenGLMedia();
	bool useShader = isLoading && shaderMedia != nullptr && shaderMedia->enabled->boolValue() && shaderMedia->shaderLoaded->boolValue();
	if (useShader) shaderMedia->renderOpenGLMedia();
}



void MediaListSubItem::newMessage(const Media::MediaEvent& event)
{
	if (event.type == Media::MediaEvent::MEDIA_FINISHED)
	{
		listSubItemNotifier.addMessage(new MediaListSubItemEvent(MediaListSubItemEvent::SUBMEDIA_FINISHED, this));
	}
	else if (event.type == Media::MediaEvent::MEDIA_CONTENT_CHANGED)
	{
		if (media != nullptr && shaderMedia != nullptr)
		{
			Point<int> mediaSize = media->getMediaSize();
			if (mediaSize.x > 0 && mediaSize.y > 0)
			{
				shaderMedia->width->setValue(mediaSize.getX());
				shaderMedia->height->setValue(mediaSize.getY());
			}
		}
	}
}

void MediaListSubItem::newMessage(const Inspectable::InspectableEvent& event)
{
	if (event.type == Inspectable::InspectableEvent::SELECTION_CHANGED)
	{
		listSubItemNotifier.addMessage(new MediaListSubItemEvent(MediaListSubItemEvent::SELECTION_CHANGED, this));
	}
}

OpenGLFrameBuffer* MediaListSubItem::getFrameBuffer()
{
	if (media != nullptr)
	{
		return media->getFrameBuffer(textureName->getValue().toString());
	}
	return nullptr;
}

GLuint MediaListSubItem::getTextureID()
{
	if (media != nullptr)
	{
		return media->getTextureID(textureName->getValue().toString());
	}
	return GLuint();
}

var MediaListSubItem::getJSONData(bool includeNonOverriden)
{
	var data = ControllableContainer::getJSONData(includeNonOverriden);
	data.getDynamicObject()->setProperty("shaderMedia", shaderMedia->getJSONData(includeNonOverriden));
	if (ownedMedia != nullptr) data.getDynamicObject()->setProperty("ownedMedia", ownedMedia->getJSONData(includeNonOverriden));
	return data;
}

void MediaListSubItem::loadJSONDataInternal(var data)
{
	ControllableContainer::loadJSONDataInternal(data);
	if (data.hasProperty("shaderMedia"))
	{
		shaderMedia->loadJSONData(data["shaderMedia"]);
	}

	updateCurrentMedia(true);

	if (data.hasProperty("ownedMedia") && ownedMedia != nullptr)
	{
		ownedMedia->loadJSONData(data["ownedMedia"]);
	}
}
