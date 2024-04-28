/*
  ==============================================================================

	ObjectManager.cpp
	Created: 26 Sep 2020 10:02:28am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

CompositionLayerManager::CompositionLayerManager() :
	BaseManager("CompositionLayer")
{
	itemDataType = "CompositionLayer";
	selectItemWhenCreated = false;
	managerFactory = CompositionLayerFactory::getInstance();
}

CompositionLayerManager::~CompositionLayerManager()
{
	// stopThread(1000);
}


void CompositionLayerManager::addItemInternal(CompositionLayer* o, var data)
{
}

void CompositionLayerManager::removeItemInternal(CompositionLayer* o)
{

}

void CompositionLayerManager::onContainerParameterChanged(Parameter* p)
{
	BaseManager::onContainerParameterChanged(p);
}


juce_ImplementSingleton(CompositionLayerFactory)

CompositionLayerFactory::CompositionLayerFactory()
{
	defs.add(Definition::createDef<ReferenceCompositionLayer>(""));
	for (auto& md : MediaManager::getInstance()->factory.defs)
	{
		defs.add(Definition::createDef("Owned", md->type, &OwnedCompositionLayer::create)->addParam("mediaType", md->type));
	}
}
