/*
  ==============================================================================

	MediaListCustomOrderList.h
	Created: 2025
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class MediaListCustomOrderList :
	public BaseItem
{
public:
	MediaListCustomOrderList(const String& name = "Custom Order List", var params = var());
	~MediaListCustomOrderList();

	// Each child IntParameter in this container represents one slot in the play order.
	// The int value is a 1-based index into the official MediaListItemManager item list.

	void addIndexSlot(int oneBasedIndex = 1);

	// Returns an ordered array of 0-based item indices for navigation
	Array<int> getOrderedIndices();

	DECLARE_TYPE("MediaListCustomOrderList")
};
