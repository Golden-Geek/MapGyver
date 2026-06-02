/*
  ==============================================================================

	MediaListCustomOrderList.cpp
	Created: 2025
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"

MediaListCustomOrderList::MediaListCustomOrderList(const String& name, var params) :
	BaseItem(name)
{
	saveAndLoadRecursiveData = true;
	editorIsCollapsed = false;
	userCanAddControllables = true;
	userAddControllablesFilters.add(IntParameter::getTypeStringStatic());
}

MediaListCustomOrderList::~MediaListCustomOrderList()
{
}

void MediaListCustomOrderList::addIndexSlot(int oneBasedIndex)
{
	int slotNum = getAllParameters(false).size() + 1;
	IntParameter* p = addIntParameter("Index " + String(slotNum), "1-based index into the media list", oneBasedIndex, 1);
	p->isRemovableByUser = true;
	p->isCustomizableByUser = true;
	p->userCanChangeName = true;
}

Array<int> MediaListCustomOrderList::getOrderedIndices()
{
	Array<int> result;
	for (auto& weakParam : getAllParameters(false))
	{
		if (IntParameter* ip = dynamic_cast<IntParameter*>(weakParam.get()))
		{
			result.add(ip->intValue() - 1); // convert to 0-based
		}
	}
	return result;
}
