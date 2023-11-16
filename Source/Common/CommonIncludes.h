/*
  ==============================================================================

	CommonIncludes.h
	Created: 16 Nov 2023 4:04:45pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"

//NDI Lib
#include <Processing.NDI.Lib.h>
#ifdef _WIN32
#ifdef _WIN64
#pragma comment(lib, "Processing.NDI.Lib.x64.lib")
#else // _WIN64
#pragma comment(lib, "Processing.NDI.Lib.x86.lib")
#endif // _WIN64
#endif // _WIN32

#include "NDI/NDIDevice.h"
#include "NDI/NDIManager.h"
#include "NDI/NDIDeviceParameter.h"

#include "NDI/ui/NDIDeviceChooser.h"
#include "NDI/ui/NDIDeviceParameterUI.h"