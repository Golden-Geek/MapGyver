/*
  ==============================================================================

	MediaIncludes.cpp
	Created: 16 Nov 2023 4:04:33pm
	Author:  bkupe

  ==============================================================================
*/

#include "MediaIncludes.h"

#include "Media.cpp"
#include "MediaManager.cpp"
#include "ui/MediaUI.cpp"
#include "ui/MediaManagerUI.cpp"

#include "medias/composition/CompositionLayer/CompositionLayer.cpp"
#include "medias/composition/CompositionLayer/CompositionLayerManager.cpp"
#include "medias/composition/CompositionMedia.cpp"

#include "medias/picture/PictureMedia.cpp"

#include "medias/ndi/NDIMedia.cpp"
#include "medias/sharedtexture/SharedTextureMedia.cpp"
#include "medias/shader/ShaderMedia.cpp"

#include "medias/color/ColorMedia.cpp"

#include "medias/video/VideoMedia.cpp"

#if !JUCE_LINUX
#include "medias/webcam/WebcamDevice.cpp"
#include "medias/webcam/WebcamManager.cpp"
#include "medias/webcam/WebcamDeviceParameter.cpp"
#include "medias/webcam/ui/WebcamDeviceChooser.cpp"
#include "medias/webcam/ui/WebcamDeviceParameterUI.cpp"
#include "medias/webcam/WebcamMedia.cpp"
#endif

#include "medias/node/NodeMedia.cpp"

#include "medias/canvas/CanvasMedia.cpp"
#include "medias/web/WebMedia.cpp"
#include "medias/grid/GridMedia.cpp"


#include "medias/interactiveapp/InteractiveAppMedia.cpp"

#include "medias/sequence/MediaClip.cpp"
#include "medias/sequence/ClipTransition.cpp"
#include "medias/sequence/MediaClipManager.cpp"
#include "medias/sequence/MediaLayer.cpp"
#include "medias/sequence/SequenceMedia.cpp"
#include "medias/sequence/ui/MediaClipUI.cpp"
#include "medias/sequence/ui/MediaClipManagerUI.cpp"
#include "medias/sequence/ui/MediaLayerTimeline.cpp"

#include "ui/MediaGridUI.cpp"
#include "ui/MediaManagerGridUI.cpp"
#include "ui/MediaPreviewPanel.cpp"
#include "ui/MediaEditor.cpp"
