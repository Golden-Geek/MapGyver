/*
  ==============================================================================

	NodeFactory.cpp
	Created: 15 Nov 2020 8:56:42am
	Author:  bkupe

  ==============================================================================
*/

#include "NodeIncludes.h"

juce_ImplementSingleton(NodeFactory)

NodeFactory::NodeFactory()
{
	//defs.add(Definition::createDef<ContainerNode>("", ContainerNode::getTypeStringStatic()));

	defs.add(Definition::createDef<MediaNode>("Source"));
	defs.add(Definition::createDef<ShapeNode>("Drawing"));

}
