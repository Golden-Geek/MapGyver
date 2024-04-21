#include "Common/CommonIncludes.h"
#include "Media/MediaIncludes.h"

MediaTarget::~MediaTarget()
{
	Array<int> ids;
	HashMap<int, Media*>::Iterator it(usedMedias);
	while (it.next()) ids.add(it.getKey());

	for (auto& id : ids) unregisterUseMedia(id);
}

bool MediaTarget::isUsingMedia(Media* m)
{
	HashMap<int, Media*>::Iterator it(usedMedias);
	while (it.next()) if(it.getValue() == m) return true;
	return false;
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

	usedMedias.set(id, m);
	m->registerTarget(this);
}

void MediaTarget::unregisterUseMedia(int id)
{
	if (!usedMedias.contains(id)) return;
	usedMedias[id]->unregisterTarget(this);
	usedMedias.remove(id);
}
