/*
  ==============================================================================

	MediaIncludes.h
	Created: 16 Nov 2023 4:04:33pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "Common/CommonIncludes.h"

#include "Media.h"
#include "MediaManager.h"
#include "ui/MediaUI.h"
#include "ui/MediaManagerUI.h"

#include "medias/composition/CompositionLayer/CompositionLayer.h"
#include "medias/composition/CompositionLayer/CompositionLayerManager.h"
#include "medias/composition/CompositionMedia.h"

#include "medias/picture/PictureMedia.h"

#include "medias/ndi/NDIMedia.h"
#include "medias/sharedtexture/SharedTextureMedia.h"

#include "medias/shader/ShaderMedia.h"

#include "medias/color/ColorMedia.h"

#include "vlcpp/vlc.hpp"
#include "medias/video/VideoMedia.h"

#include "medias/Webcam/WebcamDevice.h"
#include "medias/Webcam/WebcamManager.h"
#include "medias/Webcam/WebcamDeviceParameter.h"
#include "medias/Webcam/ui/WebcamDeviceChooser.h"
#include "medias/Webcam/ui/WebcamDeviceParameterUI.h"
#include "medias/Webcam/WebcamMedia.h"

#include "medias/node/NodeMedia.h"

#include "medias/sequence/MediaClip.h"
#include "medias/sequence/ClipTransition.h"
#include "medias/sequence/MediaClipManager.h"
#include "medias/sequence/MediaLayer.h"
#include "medias/sequence/SequenceMedia.h"
#include "medias/sequence/ui/MediaClipUI.h"
#include "medias/sequence/ui/MediaClipManagerUI.h"
#include "medias/sequence/ui/MediaLayerTimeline.h"

#include "ui/MediaPreviewPanel.h"
#include "ui/MediaGridUI.h"
#include "ui/MediaManagerGridUI.h"
#include "ui/MediaEditor.h"