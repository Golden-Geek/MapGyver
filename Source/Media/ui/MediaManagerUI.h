/*
  ==============================================================================

    CVMediaManagerUI.h
    Created: 22 Feb 2018 3:42:48pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class MediaManagerUI :
	public BaseManagerShapeShifterUI<MediaManager, Media, MediaUI>
{
public:
	MediaManagerUI(const String &contentName);
	~MediaManagerUI();

	static MediaManagerUI * create(const String &name) { return new MediaManagerUI(name); }

	bool isInterestedInDragSource(const SourceDetails& source) override;
	void itemDropped(const SourceDetails& source) override;
};

