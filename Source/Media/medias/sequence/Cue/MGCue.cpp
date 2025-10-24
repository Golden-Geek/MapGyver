/*
  ==============================================================================

	MGCue.cpp
	Created: 5 Nov 2022 6:47:36pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "MGCue.h"

MGCue::MGCue(float time, TimeCueManager* m) :
	TimeCue(time, m),
	linkedBlock(nullptr)
{
	linkedBlockTarget = addTargetParameter("Linked Block", "Linked Block");
	linkedBlockTarget->targetType = TargetParameter::CONTAINER;
	linkedBlockTarget->defaultContainerTypeCheckFunc = [](ControllableContainer* cc)
		{
			return dynamic_cast<LayerBlock*>(cc) != nullptr;
		};

	blockOffset = addFloatParameter("Block Offset", "Block Offset", 0.0f);
	blockOffset->defaultUI = FloatParameter::TIME;

	offsetFromEnd = addBoolParameter("Offset From End", "If checked, the offset will be calculated from the end of the block.", false);


}

MGCue::~MGCue()
{
	setLinkedBlock(nullptr);
}

void MGCue::setParentContainer(ControllableContainer* newParent)
{
	TimeCue::setParentContainer(newParent);
	linkedBlockTarget->setRootContainer(ControllableUtil::findParentAs<Sequence>(this));
}

void MGCue::setLinkedBlock(LayerBlock* block)
{
	if (block == linkedBlock) return;

	if (linkedBlock != nullptr)
	{
		unregisterLinkedInspectable(linkedBlock);
		linkedBlock->time->removeParameterListener(this);
		linkedBlock->coreLength->removeParameterListener(this);
		linkedBlock->loopLength->removeParameterListener(this);

	}

	linkedBlock = block;

	if (linkedBlock != nullptr)
	{
		registerLinkedInspectable(linkedBlock);
		linkedBlock->time->addParameterListener(this);
		linkedBlock->coreLength->addParameterListener(this);
		linkedBlock->loopLength->addParameterListener(this);
	}

	updateTimeFromLinkedBlock();
}

void MGCue::onContainerParameterChangedInternal(Parameter* p)
{
	TimeCue::onContainerParameterChangedInternal(p);

	if (p == linkedBlockTarget)
	{
		setLinkedBlock(linkedBlockTarget->getTargetContainerAs<LayerBlock>());
	}

	if(p == blockOffset || p == offsetFromEnd)
	{
		updateTimeFromLinkedBlock();
	}

}

void MGCue::onExternalParameterValueChanged(Parameter* p)
{
	if (linkedBlock == nullptr) return;

	if (p == linkedBlock->time || p == linkedBlock->coreLength || p == linkedBlock->loopLength)
	{
		updateTimeFromLinkedBlock();
	}
}

void MGCue::updateTimeFromLinkedBlock()
{
	if (linkedBlock == nullptr) return;

	float offset = blockOffset->floatValue();
	float targetTime = linkedBlock->time->floatValue() + offset;
	if (offsetFromEnd->boolValue()) targetTime =  linkedBlock->getEndTime() - offset;
	time->setValue(targetTime);
}
