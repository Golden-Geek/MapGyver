/*
  ==============================================================================

	ObjectManager.cpp
	Created: 26 Sep 2020 10:02:28am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "MediaManager.h"

juce_ImplementSingleton(MediaManager);

MediaManager::MediaManager() :
	BaseManager("Media"),
	editingSequenceMedia(nullptr)
{
	managerFactory = &factory;

	factory.defs.add(Factory<Media>::Definition::createDef<ColorMedia>(""));
	factory.defs.add(Factory<Media>::Definition::createDef<PictureMedia>(""));
	factory.defs.add(Factory<Media>::Definition::createDef<VideoMedia>(""));
	factory.defs.add(Factory<Media>::Definition::createDef<WebcamMedia>(""));
	factory.defs.add(Factory<Media>::Definition::createDef<NDIMedia>(""));
	factory.defs.add(Factory<Media>::Definition::createDef<SharedTextureMedia>(""));
	factory.defs.add(Factory<Media>::Definition::createDef<ShaderMedia>(""));
	factory.defs.add(Factory<Media>::Definition::createDef<CompositionMedia>(""));
	factory.defs.add(Factory<Media>::Definition::createDef<NodeMedia>(""));
	factory.defs.add(Factory<Media>::Definition::createDef<SequenceMedia>(""));

	itemDataType = "Media";
	selectItemWhenCreated = true;

	gridThumbSize = addIntParameter("gridThumbSize", "Grid Thumb Size", 100, 50, 200);

	InspectableSelectionManager::mainSelectionManager->addSelectionListener(this);
}

MediaManager::~MediaManager()
{
	InspectableSelectionManager::mainSelectionManager->removeSelectionListener(this);
}


void MediaManager::addItemInternal(Media* o, var data)
{
	reorderItems();
}


void MediaManager::setEditingSequenceMedia(SequenceMedia* sm)
{
	if (editingSequenceMedia == sm) return;

	if (editingSequenceMedia != nullptr)
	{
		editingSequenceMedia->setIsEditing(false);
		editingSequenceMedia->removeInspectableListener(this);
	}

	editingSequenceMedia = sm;

	if (editingSequenceMedia != nullptr)
	{
		editingSequenceMedia->setIsEditing(true);
		editingSequenceMedia->addInspectableListener(this);
		TimeMachineView* v = ShapeShifterManager::getInstance()->getContentForType<TimeMachineView>();
		if (v == nullptr) v = dynamic_cast<TimeMachineView*>(ShapeShifterManager::getInstance()->showContent("Sequence Editor"));
		if (v != nullptr) v->setSequence(&sm->sequence);
	}
}

void MediaManager::inspectablesSelectionChanged()
{
	if (SequenceMedia* sm = InspectableSelectionManager::mainSelectionManager->getInspectableAs<SequenceMedia>())
	{
		setEditingSequenceMedia(sm);
	}
}

void MediaManager::inspectableDestroyed(Inspectable* i)
{
	if (editingSequenceMedia == i)
	{
		setEditingSequenceMedia(nullptr);
	}
}

var MediaManager::getJSONData(bool includeNonOverriden)
{
	var data = BaseManager::getJSONData(includeNonOverriden);
	if (editingSequenceMedia != nullptr)
	{
		data.getDynamicObject()->setProperty("editingSequenceMedia", editingSequenceMedia->getControlAddress());
	}
	return data;
}

void MediaManager::loadJSONDataManagerInternal(var data)
{
	BaseManager::loadJSONDataManagerInternal(data);
	if (data.hasProperty("editingSequenceMedia"))
	{
		if (SequenceMedia* sm = dynamic_cast<SequenceMedia*>(Engine::mainEngine->getControllableContainerForAddress(data.getProperty("editingSequenceMedia", var()))))
		{

			setEditingSequenceMedia(sm);
		}
	}
}

