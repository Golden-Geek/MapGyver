/*
  ==============================================================================

	GridClipManager.cpp
	Created: 11 Jun 2025 4:39:38pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

GridClipFactory::GridClipFactory(GridMedia* gridMedia)
{
	defs.add(new GridClipDefinition("", ReferenceGridClip::getTypeStringStatic(), gridMedia, &ReferenceGridClip::create, var()));
	for (auto& md : MediaManager::getInstance()->factory.defs)
	{
		var params(new DynamicObject());
		params.getDynamicObject()->setProperty("mediaType", md->type);
		defs.add(new GridClipDefinition("Owned", md->type, gridMedia, &OwnedGridClip::create, params));
	}
}

GridClipFactory::GridClipDefinition::GridClipDefinition(juce::StringRef menuPath, juce::StringRef type, GridMedia* gridMedia, CreateClipFunc createFunc, var params)
	: FactoryDefinition(menuPath, type, createFunc),
	gridMedia(gridMedia),
	params(params)
{
}



GridClipManager::GridClipManager(GridMedia* gridMedia) :
	Manager<GridClip>("Clips"),
	gridMedia(gridMedia),
	factory(gridMedia)
{
	managerFactory = &factory;
}

GridClip* GridClipFactory::GridClipDefinition::create()
{
	return createFunc(gridMedia, params);
}
