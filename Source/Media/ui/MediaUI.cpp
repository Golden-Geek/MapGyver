/*
  ==============================================================================

    MediaUI.cpp
    Created: 22 Nov 2023 3:38:52pm
    Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaUI::MediaUI(Media* item) :
    BaseItemUI(item)
{
}

MediaUI::~MediaUI()
{
}

void MediaUI::mouseDoubleClick(const MouseEvent& e)
{
    if (SequenceMedia* seq = dynamic_cast<SequenceMedia*>(item))
    {
        if (TimeMachineView* v = ShapeShifterManager::getInstance()->getContentForType<TimeMachineView>())
        {
		   v->setSequence(&seq->sequence);
	   }
	}
}
