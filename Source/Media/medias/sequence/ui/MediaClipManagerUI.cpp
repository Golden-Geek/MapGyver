/*
  ==============================================================================

	MediaClipManagerUI.cpp
	Created: 21 Dec 2023 11:01:50am
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaClipManagerUI::MediaClipManagerUI(MediaLayerTimeline* timeline) :
	LayerBlockManagerUI(timeline, &timeline->mediaLayer->blockManager),
	mediaTimeline(timeline)
{
	addExistingItems();

	acceptedDropTypes.add("Media");
}

MediaClipManagerUI::~MediaClipManagerUI()
{
}


void MediaClipManagerUI::paintOverChildren(Graphics& g)
{
	ManagerUI::paintOverChildren(g);

	if (dropClipX >= 0)
	{
		g.setColour(BLUE_COLOR);
		g.drawLine(dropClipX, 0, dropClipX, getHeight());
	}
}

LayerBlockUI* MediaClipManagerUI::createUIForItem(LayerBlock* b)
{
	return new MediaClipUI((MediaClip*)b);
}

bool MediaClipManagerUI::isInterestedInDragSource(const SourceDetails& source)
{
	if (source.description.getProperty("type", "") == "OnlineContentItem") return true;
	return ManagerUI::isInterestedInDragSource(source);
}


void MediaClipManagerUI::itemDragEnter(const SourceDetails& source)
{
	dropClipX = source.localPosition.x;
	repaint();
}

void MediaClipManagerUI::itemDragExit(const SourceDetails& source)
{
	dropClipX = -1;
	repaint();
}

void MediaClipManagerUI::itemDragMove(const SourceDetails& source)
{
	dropClipX = source.localPosition.x;
	repaint();
}

void MediaClipManagerUI::itemDropped(const SourceDetails& source)
{
	ManagerUI::itemDropped(source);

	if (source.description.getProperty("type", "") == "OnlineContentItem")
	{
		OnlineContentItem* item = dynamic_cast<OnlineContentItem*>(source.sourceComponent.get());
		if (item != nullptr)
		{
			if (Media* m = item->createMedia())
			{
				OwnedMediaClip* clip = new OwnedMediaClip(m);
				mediaTimeline->mediaLayer->blockManager.addBlockAt(clip, timeline->getTimeForX(dropClipX));
			}
		}
	}
	else if (MediaUI* mui = dynamic_cast<MediaUI*>(source.sourceComponent.get()))
	{
		ReferenceMediaClip* clip = new ReferenceMediaClip();
		clip->mediaTarget->setValueFromTarget(mui->item);
		mediaTimeline->mediaLayer->blockManager.addBlockAt(clip, timeline->getTimeForX(dropClipX));
	}

	dropClipX = -1;
	repaint();

}

void MediaClipManagerUI::showMenuAndAddItem(bool fromAddButton, Point<int> mouseDownPos)
{
	mediaTimeline->mediaLayer->blockManager.managerFactory->showCreateMenu(manager, [this, mouseDownPos](LayerBlock* b)
		{
			float time = timeline->getTimeForX(mouseDownPos.x);
			if (ClipTransition* t = dynamic_cast<ClipTransition*>(b))
			{
				if (mediaTimeline->mediaLayer->blockManager.items.size() > 0)
				{
					MediaClip* inMedia = dynamic_cast<MediaClip*>(manager->getBlockAtTime(time,true));
					MediaClip* outMedia = dynamic_cast<MediaClip*>(manager->getNextBlockAtTime(time));
					if (inMedia == nullptr || outMedia == nullptr || inMedia == outMedia)
					{
						NLOGWARNING(manager->layer->niceName, "Cannot add transition at this time, no in or out media found");
						delete b;
						return;
					}
					else
					{
						manager->addBlockAt(b, time);
					}
				}
				else
				{
					NLOGWARNING(manager->layer->niceName, "Cannot add transition at this time, no in or out media found");
				}
			}
			else
			{
				manager->addBlockAt(b, time);
			}
		}
	);
}


void MediaClipManagerUI::mouseDoubleClick(const MouseEvent& e)
{
	//nothing, do not create on double click
}