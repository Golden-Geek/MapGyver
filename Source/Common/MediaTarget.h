/*
  ==============================================================================

	MediaTarget.h
	Created: 23 Nov 2023 11:45:20pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class Media;

class MediaTarget
{
public:
	MediaTarget() {}
	virtual ~MediaTarget();

	void clearTarget();

	HashMap<int, WeakReference<Media>, DefaultHashFunctions, CriticalSection> usedMedias;

	virtual bool isUsingMedia(Media* m);

	void registerUseMedia(int id, Media* m);
	void unregisterUseMedia(int id);


};