/*
  ==============================================================================

    SurfaceUI.h
    Created: 8 Dec 2016 2:36:51pm
    Author:  Ben

  ==============================================================================
*/

#pragma once

class SurfaceUI :
	public ItemUI<Surface>
	//public Surface::SurfaceListener
{
public:
	SurfaceUI(Surface * Surface);
	virtual ~SurfaceUI();

	std::unique_ptr<BoolToggleUI> lockUI;

	virtual void resizedHeader(Rectangle<int> &r) override;
	//virtual void resizedInternalHeaderSurface(Rectangle<int>& r) {}

	//std::unique_ptr<TriggerImageUI> inActivityUI;
	//std::unique_ptr<TriggerImageUI> outActivityUI;
	//ImageComponent iconUI;

	//std::unique_ptr<BoolToggleUI> connectionFeedbackUI;

	//void mouseDown(const MouseEvent &e) override;
	//void paintOverChildren(Graphics& g) override;
	//void SurfaceIOConfigurationChanged() override;
	//void updateConnectionFeedbackRef();
	//void controllableFeedbackUpdateInternal(Controllable* c) override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SurfaceUI)
};