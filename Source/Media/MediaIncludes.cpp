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
#include "medias/medialist/MediaListItem.cpp"
#include "medias/medialist/MediaListItemManager.cpp"
#include "medias/medialist/MediaListMedia.cpp"

#include "medias/ndi/NDIMedia.cpp"
#include "medias/sharedtexture/SharedTextureMedia.cpp"
#include "medias/shader/ShaderMedia.cpp"

#include "medias/color/ColorMedia.cpp"

#include "medias/video/VideoAudioProcessor.cpp"
#include "medias/video/MPVPlayer.cpp"
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

#if JUCE_WINDOWS
#include "medias/web/WebViewMedia.cpp"
#endif

#include "medias/web/WebMedia.cpp"

#include "medias/grid/GridMedia.cpp"

#include "medias/grid/GridClip.cpp"
#include "medias/grid/GridClipManager.cpp"
#include "medias/grid/GridLayer.cpp"
#include "medias/grid/GridLayerGroup.cpp"
#include "medias/grid/GridLayerManager.cpp"
#include "medias/grid/GridLayerGroupManager.cpp"
#include "medias/grid/GridColumn.cpp"
#include "medias/grid/GridColumnManager.cpp"

#include "medias/grid/ui/GridClipUI.cpp"
#include "medias/grid/ui/GridEmptySlotUI.cpp"
#include "medias/grid/ui/GridClipManagerUI.cpp"
#include "medias/grid/ui/GridColumnUI.cpp"
#include "medias/grid/ui/GridColumnManagerUI.cpp"
#include "medias/grid/ui/GridBoard.cpp"
#include "medias/grid/ui/GridLayerPanel.cpp"
#include "medias/grid/ui/GridLayerManagerUI.cpp"
#include "medias/grid/ui/GridLayerGroupPanel.cpp"
#include "medias/grid/ui/GridLayerGroupManagerUI.cpp"

#include "medias/grid/ui/GridMediaPanel.cpp"

#include "medias/interactiveapp/InteractiveAppMedia.cpp"

#include "medias/sequence/MediaClip.cpp"
#include "medias/sequence/ClipTransition.cpp"
#include "medias/sequence/MediaClipManager.cpp"
#include "medias/sequence/MediaLayer.cpp"
#include "medias/sequence/MGAudioLayer.cpp"
#include "medias/sequence/Cue/MGCue.cpp"
#include "medias/sequence/SequenceMedia.cpp"
#include "medias/sequence/ui/MediaClipUI.cpp"
#include "medias/sequence/ui/MediaClipManagerUI.cpp"
#include "medias/sequence/ui/MediaLayerTimeline.cpp"

#include "ui/MediaGridUI.cpp"
#include "ui/MediaManagerGridUI.cpp"
#include "ui/MediaPreviewPanel.cpp"
#include "ui/MediaEditor.cpp"
