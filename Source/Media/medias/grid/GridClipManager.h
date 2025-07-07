/*
  ==============================================================================

	GridClipManager.h
	Created: 11 Jun 2025 4:39:38pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class GridMedia;

class GridClipFactory :
	public Factory<GridClip>
{
public:
	GridClipFactory(GridMedia* gridMedia);
	~GridClipFactory() {}

	typedef std::function<GridClip* (GridMedia*, var)> CreateClipFunc;
	class GridClipDefinition : public FactoryDefinition<GridClip, CreateClipFunc>
	{
	public:
		GridMedia* gridMedia;
		var params;

		GridClipDefinition(juce::StringRef menuPath, juce::StringRef type, GridMedia* gridMedia, CreateClipFunc createFunc, var params = var());

		GridClip* create() override;
	};
};

class GridClipManager : public Manager<GridClip>
{
public:
	GridClipManager(GridMedia* gridMedia);
	~GridClipManager();

	GridClipFactory factory;
	GridMedia* gridMedia;

	FloatParameter* slotSize;
};