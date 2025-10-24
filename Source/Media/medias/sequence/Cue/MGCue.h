#pragma once

#include "JuceHeader.h"

class MGCue :
	public TimeCue
{
public:
	MGCue(float time = 0, TimeCueManager* m = nullptr);
	~MGCue();

	TargetParameter* linkedBlockTarget;
	FloatParameter* blockOffset;
	BoolParameter* offsetFromEnd;

	LayerBlock* linkedBlock;

	void setParentContainer(ControllableContainer* newParent) override;

	void setLinkedBlock(LayerBlock* block);
	virtual void onContainerParameterChangedInternal(Parameter* p) override;
	virtual void onExternalParameterValueChanged(Parameter* p) override;

	virtual void updateTimeFromLinkedBlock();
};