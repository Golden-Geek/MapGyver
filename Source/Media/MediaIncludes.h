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

#include "medias/medialist/MediaListItem.h"
#include "medias/medialist/MediaListItemManager.h"
#include "medias/medialist/MediaListMedia.h"

#include "medias/ndi/NDIMedia.h"
#include "medias/sharedtexture/SharedTextureMedia.h"

#include "medias/canvas/CanvasMedia.h"

#if JUCE_WINDOWS
#include "medias/web/WebViewMedia.h"
#endif

#include "medias/web/WebMedia.h"

#include "medias/grid/GridClip.h"
#include "medias/grid/GridClipManager.h"
#include "medias/grid/GridLayer.h"
#include "medias/grid/GridLayerManager.h"
#include "medias/grid/GridLayerGroup.h"
#include "medias/grid/GridLayerGroupManager.h"
#include "medias/grid/GridColumn.h"
#include "medias/grid/GridColumnManager.h"

#include "medias/grid/GridMedia.h"


#include "medias/grid/ui/GridClipUI.h"
#include "medias/grid/ui/GridEmptySlotUI.h"
#include "medias/grid/ui/GridClipManagerUI.h"
#include "medias/grid/ui/GridColumnUI.h"
#include "medias/grid/ui/GridColumnManagerUI.h"
#include "medias/grid/ui/GridBoard.h"
#include "medias/grid/ui/GridLayerPanel.h"
#include "medias/grid/ui/GridLayerManagerUI.h"
#include "medias/grid/ui/GridLayerGroupPanel.h"
#include "medias/grid/ui/GridLayerGroupManagerUI.h"
#include "medias/grid/ui/GridMediaPanel.h"

#include "medias/interactiveapp/InteractiveAppMedia.h"

#include "medias/shader/ShaderMedia.h"

#include "medias/color/ColorMedia.h"

#include "mpv/client.h"
#include "mpv/render_gl.h"

#include "medias/video/VideoAudioProcessor.h"
#include "medias/video/MPVPlayer.h"
#include "medias/video/VideoMedia.h"

#if !JUCE_LINUX
#include "medias/webcam/WebcamDevice.h"
#include "medias/webcam/WebcamManager.h"
#include "medias/webcam/WebcamDeviceParameter.h"
#include "medias/webcam/ui/WebcamDeviceChooser.h"
#include "medias/webcam/ui/WebcamDeviceParameterUI.h"
#include "medias/webcam/WebcamMedia.h"
#endif

#include "medias/node/NodeMedia.h"

#include "medias/sequence/MediaClip.h"
#include "medias/sequence/ClipTransition.h"
#include "medias/sequence/MediaClipManager.h"
#include "medias/sequence/MediaLayer.h"
#include "medias/sequence/Cue/MGCue.h"
#include "medias/sequence/MGAudioLayer.h"
#include "medias/sequence/SequenceMedia.h"
#include "medias/sequence/ui/MediaClipUI.h"
#include "medias/sequence/ui/MediaClipManagerUI.h"
#include "medias/sequence/ui/MediaLayerTimeline.h"

#include "ui/MediaPreviewPanel.h"
#include "ui/MediaGridUI.h"
#include "ui/MediaManagerGridUI.h"
#include "ui/MediaEditor.h"
