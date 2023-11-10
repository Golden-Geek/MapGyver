/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "JuceHeader.h"
#include "Surface.h"
#include "SurfaceManager.h"

Surface::Surface(var params) :
	BaseItem(params.getProperty("name", "Surface")),
	objectType(params.getProperty("type", "Surface").toString()),
	objectData(params)
{
	saveAndLoadRecursiveData = true;
	nameCanBeChangedByUser = false;
	canBeDisabled = true;

	itemDataType = "Surface";

	topLeft = addPoint2DParameter("topLeft ", "");
	topRight = addPoint2DParameter("topRight ", "");
	bottomLeft = addPoint2DParameter("bottomLeft ", "");
	bottomRight = addPoint2DParameter("bottomRight ", "");

	topLeft->setBounds(-1.0f, -1.0f, 1.0f,1.0f);
	topRight->setBounds(-1.0f, -1.0f, 1.0f,1.0f);
	bottomLeft->setBounds(-1.0f, -1.0f, 1.0f,1.0f);
	bottomRight->setBounds(-1.0f, -1.0f, 1.0f,1.0f);

	var def; def.append(0); def.append(0);
	def[0] = -1.0f; def[1] = 1.0f;
	topLeft->setDefaultValue(def);
	def[0] = 1.0f; def[1] = 1.0f;
	topRight->setDefaultValue(def);
	def[0] = -1.0f; def[1] = -1.0f;
	bottomLeft->setDefaultValue(def);
	def[0] = 1.0f; def[1] = -1.0f;
	bottomRight->setDefaultValue(def);

	softEdgeTop = addFloatParameter("Soft Edge Top", "", 0, 0, 1);
	softEdgeRight = addFloatParameter("Soft Edge Right", "", 0, 0, 1);
	softEdgeBottom = addFloatParameter("Soft Edge Bottom", "", 0, 0, 1);
	softEdgeLeft = addFloatParameter("Soft Edge Left", "", 0, 0, 1);

}

Surface::~Surface()
{
}

void Surface::onContainerParameterChangedInternal(Parameter* p) {
}

