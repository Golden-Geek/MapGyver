/*
  ==============================================================================

    ViewStatsTimer.h
    Created: 4 Aug 2022 11:09:34am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class ViewStatsTimer : public Timer
{
public:
	juce_DeclareSingleton(ViewStatsTimer, false);

	ViewStatsTimer() { startTimerHz(10); }

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void refreshStats() {}
	};

	ListenerList<Listener> listeners;
	void addListener(Listener* newListener) { listeners.add(newListener); }
	void removeListener(Listener* listener) { listeners.remove(listener); }


	void timerCallback() { listeners.call(&Listener::refreshStats); }
};
