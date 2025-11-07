#include "Common/CommonIncludes.h"
#include "Media/MediaIncludes.h"

MediaTarget::~MediaTarget()
{
	clearTarget();
}

void MediaTarget::clearTarget()
{
	if (Engine::mainEngine->isClearing) return;
	Array<int> ids;
	HashMap<int, WeakReference<Media>, DefaultHashFunctions, CriticalSection>::Iterator it(usedMedias);
	while (it.next()) ids.add(it.getKey());

	for (auto& id : ids) unregisterUseMedia(id);
}

bool MediaTarget::isUsingMedia(Media* m)
{
	HashMap<int, WeakReference<Media>, DefaultHashFunctions, CriticalSection>::Iterator it(usedMedias);
	while (it.next()) if (it.getValue() == m) return true;
	return false;
}

void MediaTarget::updateUsedMedias()
{
	HashMap<int, WeakReference<Media>, DefaultHashFunctions, CriticalSection>::Iterator it(usedMedias);
	while (it.next())
	{
		if (Media* m = it.getValue())
			m->updateBeingUsed();
	}
}

void MediaTarget::registerUseMedia(int id, Media* m)
{
	if (m == nullptr)
	{
		unregisterUseMedia(id);
		return;
	}

	if (usedMedias.contains(id))
	{
		if (m == usedMedias[id]) return;
		unregisterUseMedia(id);
	}

	usedMedias.set(id, WeakReference<Media>(m));
	m->registerTarget(this);
}

void MediaTarget::unregisterUseMedia(int id)
{
	if (!usedMedias.contains(id)) return;
	if (!usedMedias[id].wasObjectDeleted()) usedMedias[id]->unregisterTarget(this);
	usedMedias.remove(id);
}
