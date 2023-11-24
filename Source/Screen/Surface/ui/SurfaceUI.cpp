/*
  ==============================================================================

	SurfaceUI.cpp
	Created: 8 Dec 2016 2:36:51pm
	Author:  Ben

  ==============================================================================
*/

#include "Screen/ScreenIncludes.h"

SurfaceUI::SurfaceUI(Surface* s) :
	BaseItemUI<Surface>(s)
{
	headerHeight = 20;

	//Surface->addSurfaceListener(this);

	lockUI.reset(item->isUILocked->createToggle(ImageCache::getFromMemory(OrganicUIBinaryData::padlock_png, OrganicUIBinaryData::padlock_pngSize)));
	addAndMakeVisible(lockUI.get());
}

SurfaceUI::~SurfaceUI()
{
	//item->removeSurfaceListener(this);

}

void SurfaceUI::resizedHeader(Rectangle<int>& r)
{
	r.removeFromRight(2);
	lockUI->setBounds(r.removeFromRight(r.getHeight()).reduced(2));

	//resizedInternalHeaderSurface(r);
}

