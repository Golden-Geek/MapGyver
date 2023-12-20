/*
  ==============================================================================

	CropboxNode.h
	Created: 5 Apr 2022 10:45:43am
	Author:  bkupe

  ==============================================================================
*/

#pragma once
class ShapeNode :
	public Node
{
public:
	ShapeNode(var params = var());
	~ShapeNode();

	NodeConnectionSlot* out;

	void processInternal() override;

	DECLARE_TYPE("Crop")
};