/*
  ==============================================================================

	NodeIncludes.h
	Created: 5 Apr 2022 10:36:31am
	Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
using namespace juce;
//external libraries
// 
//pcl
#include "Common/CommonIncludes.h"

// classes
#include "Connection/NodeConnectionSlot.h"
#include "Connection/NodeConnection.h"
#include "Node.h"

#include "Connection/NodeConnectionManager.h"
#include "NodeFactory.h"
#include "NodeManager.h"



#include "nodes/Output/OutputNode.h"
#include "nodes/Source/MediaNode.h"
#include "nodes/Drawing/shape/ShapeNode.h"

#include "ui/ViewStatsTimer.h"

#include "Connection/ui/NodeConnector.h"
#include "Connection/ui/NodeConnectionViewUI.h"
#include "Connection/ui/NodeConnectionManagerViewUI.h"

#include "ui/NodeUI.h"
#include "ui/NodeManagerUI.h"

#include "ui/NodeViewUI.h"
#include "ui/NodeManagerViewUI.h"