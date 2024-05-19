/*
  ==============================================================================

	MediaUI.cpp
	Created: 22 Nov 2023 3:38:52pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "MediaUI.h"

MediaUI::MediaUI(Media* item) :
	BaseItemUI(item)
{
	item->addAsyncMediaListener(this);
	updateUI();

	int imgDataSize = 0;
	const char* imgData = BinaryData::getNamedResource((StringUtil::toShortName(item->getTypeString(), false) + "_png").toRawUTF8(), imgDataSize);
	icon = ImageCache::getFromMemory(imgData, imgDataSize);

}

MediaUI::~MediaUI()
{
	if (!inspectable.wasObjectDeleted())	item->removeAsyncMediaListener(this);
}

void MediaUI::paint(Graphics& g)
{
	BaseItemUI::paint(g);

	if (icon.isValid())
	{
		g.drawImage(icon, iconBounds.toFloat());
	}
}

void MediaUI::resizedHeader(Rectangle<int>& r)
{
	if (icon.isValid())
	{
		iconBounds = r.removeFromLeft(r.getHeight()).reduced(2);
	}

	BaseItemUI::resizedHeader(r);
}

void MediaUI::updateUI()
{
	bgColor = item->itemColor != nullptr ? item->itemColor->getColor() : BG_COLOR.brighter(.1f);
	if (item->isEditing) bgColor = BLUE_COLOR.darker();
	else if (item->isBeingUsed()) bgColor = bgColor.brighter();
	repaint();
}

void MediaUI::newMessage(const Media::MediaEvent& e)
{
	switch (e.type)
	{
	case Media::MediaEvent::EDITING_CHANGED:
		updateUI();
		break;
	}
}
