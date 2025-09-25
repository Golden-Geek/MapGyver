/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "Common/CommonIncludes.h"
#include "Screen/ScreenIncludes.h"
#include "Media/MediaIncludes.h"
#include "Surface.h"

#define SURFACE_TARGET_MEDIA_ID 0
#define SURFACE_TARGET_MASK_ID 1
#define SURFACE_PATTERN_ID 2

Surface::Surface(var params) :
	BaseItem(params.getProperty("name", "Surface")),
	currentMedia(nullptr),
	positionningCC("Positionning"),
	bezierCC("Bezier"),
	adjustmentsCC("Adjustments"),
	formatCC("Aspect Ratio"),
	pinsCC("Pins"),
	objectType(params.getProperty("type", "Surface").toString()),
	objectData(params),
	previewMedia(nullptr),
	shouldUpdateVertices(true)

{
	saveAndLoadRecursiveData = true;
	canBeDisabled = true;

	isUILocked->hideInEditor = false;

	itemDataType = "Surface";

	mediaParam = addTargetParameter("Media", "Media to read on this surface", MediaManager::getInstance());
	mediaParam->maxDefaultSearchLevel = 0;
	mediaParam->targetType = TargetParameter::CONTAINER;
	mediaTextureName = addEnumParameter("Media Texture", "If the media is a shader or a composition, you can specify which texture to use here");


	topLeft = positionningCC.addPoint2DParameter("topLeft ", "");
	topRight = positionningCC.addPoint2DParameter("topRight ", "");
	bottomLeft = positionningCC.addPoint2DParameter("bottomLeft ", "");
	bottomRight = positionningCC.addPoint2DParameter("bottomRight ", "");

	resetBezierBtn = bezierCC.addTrigger("Reset bezier values", "");

	handleBezierTopLeft = bezierCC.addPoint2DParameter("Handle Top Left", "");
	handleBezierTopRight = bezierCC.addPoint2DParameter("Handle Top Right", "");
	handleBezierBottomLeft = bezierCC.addPoint2DParameter("Handle Bottom Left", "");
	handleBezierBottomRight = bezierCC.addPoint2DParameter("Handle Bottom Right", "");
	handleBezierLeftTop = bezierCC.addPoint2DParameter("Handle Left Top", "");
	handleBezierLeftBottom = bezierCC.addPoint2DParameter("Handle Left Bottom", "");
	handleBezierRightTop = bezierCC.addPoint2DParameter("Handle Right Top", "");
	handleBezierRightBottom = bezierCC.addPoint2DParameter("Handle Right Bottom", "");

	softEdgeTop = adjustmentsCC.addFloatParameter("Soft Edge Top", "", 0, 0, 1);
	softEdgeRight = adjustmentsCC.addFloatParameter("Soft Edge Right", "", 0, 0, 1);
	softEdgeBottom = adjustmentsCC.addFloatParameter("Soft Edge Bottom", "", 0, 0, 1);
	softEdgeLeft = adjustmentsCC.addFloatParameter("Soft Edge Left", "", 0, 0, 1);

	cropTop = adjustmentsCC.addFloatParameter("Source Top", "", 0, 0, 1);
	cropRight = adjustmentsCC.addFloatParameter("Source Right", "", 0, 0, 1);
	cropBottom = adjustmentsCC.addFloatParameter("Source Bottom", "", 0, 0, 1);
	cropLeft = adjustmentsCC.addFloatParameter("Source Left", "", 0, 0, 1);

	showTestPattern = adjustmentsCC.addBoolParameter("Show Test Pattern", "If checked this will not use the media but generate a TestPatterns", false);
	mask = adjustmentsCC.addTargetParameter("Mask", "Apply a mask to this surface", MediaManager::getInstance());
	invertMask = adjustmentsCC.addBoolParameter("Invert mask", "Invert mask", false);

	blendFunction = adjustmentsCC.addEnumParameter("Blend function", "");
	blendFunction
		->addOption("Standard Transparency", STANDARD)
		->addOption("Addition", ADDITION)
		->addOption("Multiplication", MULTIPLICATION)
		->addOption("Screen", SCREEN)
		->addOption("Darken", DARKEN)
		->addOption("Premultiplied Alpha", PREMULTALPHA)
		->addOption("Lighten", LIGHTEN)
		->addOption("Inversion", INVERT)
		->addOption("Color Addition", COLORADD)
		->addOption("Color Screen", COLORSCREEN)
		->addOption("Blur Effect", BLUR)
		->addOption("Inverse Color", INVERTCOLOR)
		->addOption("Subtraction", SUBSTRACT)
		->addOption("Color Difference", COLORDIFF)
		->addOption("Inverse Multiplication", INVERTMULT)
		->addOption("Custom", CUSTOM);

	blendFunctionSourceFactor = adjustmentsCC.addEnumParameter("Blend source factor", "");
	blendFunctionSourceFactor->addOption("GL_ZERO", (int)GL_ZERO)
		->addOption("GL_ONE", (int)GL_ONE)
		->addOption("GL_SRC_ALPHA", (int)GL_SRC_ALPHA)
		->addOption("GL_ONE_MINUS_SRC_ALPHA", (int)GL_ONE_MINUS_SRC_ALPHA)
		->addOption("GL_DST_ALPHA", (int)GL_DST_ALPHA)
		->addOption("GL_ONE_MINUS_DST_ALPHA", (int)GL_ONE_MINUS_DST_ALPHA)
		->addOption("GL_SRC_COLOR", (int)GL_SRC_COLOR)
		->addOption("GL_ONE_MINUS_SRC_COLOR", (int)GL_ONE_MINUS_SRC_COLOR)
		->addOption("GL_DST_COLOR", (int)GL_DST_COLOR)
		->addOption("GL_ONE_MINUS_DST_COLOR", (int)GL_ONE_MINUS_DST_COLOR);

	blendFunctionDestinationFactor = adjustmentsCC.addEnumParameter("Blend destination factor", "");
	blendFunctionDestinationFactor->addOption("GL_ZERO", (int)GL_ZERO)
		->addOption("GL_ONE", (int)GL_ONE)
		->addOption("GL_SRC_ALPHA", (int)GL_SRC_ALPHA)
		->addOption("GL_ONE_MINUS_SRC_ALPHA", (int)GL_ONE_MINUS_SRC_ALPHA)
		->addOption("GL_DST_ALPHA", (int)GL_DST_ALPHA)
		->addOption("GL_ONE_MINUS_DST_ALPHA", (int)GL_ONE_MINUS_DST_ALPHA)
		->addOption("GL_SRC_COLOR", (int)GL_SRC_COLOR)
		->addOption("GL_ONE_MINUS_SRC_COLOR", (int)GL_ONE_MINUS_SRC_COLOR)
		->addOption("GL_DST_COLOR", (int)GL_DST_COLOR)
		->addOption("GL_ONE_MINUS_DST_COLOR", (int)GL_ONE_MINUS_DST_COLOR);

	blendFunctionSourceFactor->setDefaultValue("GL_SRC_ALPHA");
	blendFunctionDestinationFactor->setDefaultValue("GL_ONE_MINUS_SRC_ALPHA");
	blendFunctionSourceFactor->setControllableFeedbackOnly(true);
	blendFunctionDestinationFactor->setControllableFeedbackOnly(true);

	tint = adjustmentsCC.addColorParameter("Tint", "", Colour::fromFloatRGBA(1, 1, 1, 1));
	boost = adjustmentsCC.addFloatParameter("Boost", "", 1, 1, 5);

	fillType = formatCC.addEnumParameter("Fill Type ", "");
	fillType->addOption("Stretch", STRETCH)->addOption("Fit", FIT)->addOption("Fill", FILL);
	ratioList = formatCC.addEnumParameter("Ratio", "");
	ratioList->addOption("16/9", R16_9)->addOption("16/10", R16_10)->addOption("4/3", R4_3)->addOption("square", R1)->addOption("Custom", RCUSTOM);
	ratioList->hideInEditor = true;
	ratio = formatCC.addFloatParameter("Ratio value", "", 16 / 9.0f, 0.0001);
	ratio->hideInEditor = true;
	//considerCrop = formatCC.addBoolParameter("Consider Crop", "", false);
	//considerCrop->hideInEditor = true;

	topLeft->setDefaultPoint(0, 1);
	topLeft->setBounds(-1, -1, 2, 2);
	topRight->setDefaultPoint(1, 1);
	topRight->setBounds(-1, -1, 2, 2);
	bottomLeft->setDefaultPoint(0, 0);
	bottomLeft->setBounds(-1, -1, 2, 2);
	bottomRight->setDefaultPoint(1, 0);
	bottomRight->setBounds(-1, -1, 2, 2);


	handleBezierTopLeft->setDefaultPoint(1 / 3.0f, 1);
	handleBezierTopLeft->setBounds(-1, -1, 2, 2);
	handleBezierTopRight->setDefaultPoint(2 / 3.0f, 1);
	handleBezierTopRight->setBounds(-1, -1, 2, 2);
	handleBezierBottomLeft->setDefaultPoint(1 / 3.0f, 0);
	handleBezierBottomLeft->setBounds(-1, -1, 2, 2);
	handleBezierBottomRight->setDefaultPoint(2 / 3.0f, 0);
	handleBezierBottomRight->setBounds(-1, -1, 2, 2);
	handleBezierLeftTop->setDefaultPoint(0, 2 / 3.0f);
	handleBezierLeftTop->setBounds(-1, -1, 2, 2);
	handleBezierLeftBottom->setDefaultPoint(0, 1 / 3.0f);
	handleBezierLeftBottom->setBounds(-1, -1, 2, 2);
	handleBezierRightTop->setDefaultPoint(1, 2 / 3.0f);
	handleBezierRightTop->setBounds(-1, -1, 2, 2);
	handleBezierRightBottom->setDefaultPoint(1, 1 / 3.0f);
	handleBezierRightBottom->setBounds(-1, -1, 2, 2);

	mask->maxDefaultSearchLevel = 0;
	mask->targetType = TargetParameter::CONTAINER;

	bezierCC.enabled->setDefaultValue(false);

	positionningCC.editorIsCollapsed = true;
	bezierCC.editorIsCollapsed = true;
	adjustmentsCC.editorIsCollapsed = true;
	pinsCC.selectItemWhenCreated = false;

	for (int i = 0; i < 4; i++) cornerPins.add(new Pin());

	positionningCC.saveAndLoadRecursiveData = true;
	positionningCC.addChildControllableContainer(&bezierCC);
	addChildControllableContainer(&positionningCC);
	addChildControllableContainer(&adjustmentsCC);
	addChildControllableContainer(&formatCC);
	addChildControllableContainer(&pinsCC);

	if (!Engine::mainEngine->isLoadingFile)
	{
		if (!MediaManager::getInstance()->items.isEmpty()) mediaParam->setValueFromTarget(MediaManager::getInstance()->items.getFirst());
	}


	updatePath();
}

Surface::~Surface()
{
}

void Surface::setupMedia()
{
	Media* newMedia = mediaParam->getTargetContainerAs<Media>();

	if (currentMedia != nullptr)
	{
		if (currentMedia == newMedia) return;

		unregisterUseMedia(SURFACE_TARGET_MEDIA_ID);
		currentMedia->removeAsyncMediaListener(this);
	}

	currentMedia = newMedia;

	if (currentMedia != nullptr)
	{
		registerUseMedia(SURFACE_TARGET_MEDIA_ID, currentMedia);
		currentMedia->addAsyncMediaListener(this);
	}


	shouldUpdateVertices = true;
	updateMediaTextureNames();

}

void Surface::updateMediaTextureNames()
{
	if (currentMedia != nullptr)
	{
		currentMedia->fillFrameBufferOptions(mediaTextureName);
		if (mediaTextureName->getValueKey().isEmpty()) mediaTextureName->setValueWithData("");
	}
	else mediaTextureName->clearOptions();
}

void Surface::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == mediaParam)
	{
		setupMedia();
	}
	if (p == isUILocked) {
		bool e = !isUILocked->boolValue();
		topLeft->setEnabled(e);
		topRight->setEnabled(e);
		bottomLeft->setEnabled(e);
		bottomRight->setEnabled(e);
		handleBezierTopLeft->setEnabled(e);
		handleBezierTopRight->setEnabled(e);
		handleBezierBottomLeft->setEnabled(e);
		handleBezierBottomRight->setEnabled(e);
		handleBezierLeftTop->setEnabled(e);
		handleBezierLeftBottom->setEnabled(e);
		handleBezierRightTop->setEnabled(e);
		handleBezierRightBottom->setEnabled(e);
		softEdgeTop->setEnabled(e);
		softEdgeRight->setEnabled(e);
		softEdgeBottom->setEnabled(e);
		softEdgeLeft->setEnabled(e);
		cropTop->setEnabled(e);
		cropRight->setEnabled(e);
		cropBottom->setEnabled(e);
		cropLeft->setEnabled(e);
	}
}

void Surface::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	if (c == resetBezierBtn) {
		resetBezierPoints();
	}

	if (c == topLeft || c == topRight || c == bottomLeft || c == bottomRight)
	{
		updatePath();
	}
	else if (c == mask)
	{
		if (Media* m = mask->getTargetContainerAs<Media>()) registerUseMedia(SURFACE_TARGET_MASK_ID, m);
		else unregisterUseMedia(SURFACE_TARGET_MASK_ID);
	}
	else if (c == fillType || c == ratioList) {

		FillType t = fillType->getValueDataAsEnum<FillType>();
		Ratio r = ratioList->getValueDataAsEnum<Ratio>();

		switch (r)
		{
		case R16_9: ratio->setValue(16 / 9.0f); break;
		case R16_10: ratio->setValue(16 / 10.0f); break;
		case R4_3: ratio->setValue(4 / 3.0f); break;
		case R1: ratio->setValue(1.0f); break;
		}

		ratioList->hideInEditor = t == STRETCH;
		ratio->hideInEditor = t == STRETCH || r != RCUSTOM;
		queuedNotifier.addMessage(new ContainerAsyncEvent(ContainerAsyncEvent::ControllableContainerNeedsRebuild, this));
	}
	else if (c == showTestPattern)
	{
		GenericScopedLock lock(patternMediaLock);
		ShaderMedia* sm = nullptr;

		unregisterUseMedia(SURFACE_PATTERN_ID);

		if (showTestPattern->boolValue())
		{
			sm = new ShaderMedia();
			sm->keepOfflineCache->setValue(true);
			sm->shaderType->setValueWithData(ShaderMedia::ShaderToyFile);
			sm->shaderOfflineData = String(BinaryData::fragmentShaderTestGrid_glsl, BinaryData::fragmentShaderTestGrid_glslSize);
			sm->shouldReloadShader = true;
			registerUseMedia(SURFACE_PATTERN_ID, sm);
		}

		patternMedia.reset(sm);
	}
	else if (c == blendFunction) {
		BlendPreset preset = blendFunction->getValueDataAsEnum<BlendPreset>();
		if (preset == CUSTOM) {
			blendFunctionSourceFactor->setControllableFeedbackOnly(false);
			blendFunctionDestinationFactor->setControllableFeedbackOnly(false);
		}
		else
		{
			blendFunctionSourceFactor->setControllableFeedbackOnly(true);
			blendFunctionDestinationFactor->setControllableFeedbackOnly(true);
			switch (preset)
			{
			case STANDARD:
				blendFunctionSourceFactor->setValueWithData((int)GL_SRC_ALPHA);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE_MINUS_SRC_ALPHA);
				break;
			case ADDITION:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case MULTIPLICATION:
				blendFunctionSourceFactor->setValueWithData((int)GL_DST_COLOR);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ZERO);
				break;
			case SCREEN:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE_MINUS_SRC_COLOR);
				break;
			case DARKEN:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_DST_ALPHA);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case PREMULTALPHA:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE_MINUS_SRC_ALPHA);
				break;
			case LIGHTEN:
				blendFunctionSourceFactor->setValueWithData((int)GL_SRC_ALPHA);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case INVERT:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_DST_COLOR);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE_MINUS_SRC_COLOR);
				break;
			case COLORADD:
				blendFunctionSourceFactor->setValueWithData((int)GL_SRC_COLOR);
				blendFunctionDestinationFactor->setValueWithData((int)GL_DST_COLOR);
				break;
			case COLORSCREEN:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_DST_COLOR);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case BLUR:
				blendFunctionSourceFactor->setValueWithData((int)GL_SRC_ALPHA);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case INVERTCOLOR:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_SRC_COLOR);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE);
				break;
			case SUBSTRACT:
				blendFunctionSourceFactor->setValueWithData((int)GL_ZERO);
				blendFunctionDestinationFactor->setValueWithData((int)GL_ONE_MINUS_SRC_COLOR);
				break;
			case COLORDIFF:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_DST_COLOR);
				blendFunctionDestinationFactor->setValueWithData((int)GL_SRC_COLOR);
				break;
			case INVERTMULT:
				blendFunctionSourceFactor->setValueWithData((int)GL_ONE_MINUS_SRC_COLOR);
				blendFunctionDestinationFactor->setValueWithData((int)GL_SRC_COLOR);
				break;

			}
		}
	}

	shouldUpdateVertices = true;
}

void Surface::updatePath()
{
	quadPath.clear();
	quadPath.startNewSubPath(topLeft->getPoint());
	quadPath.lineTo(topRight->getPoint());
	quadPath.lineTo(bottomRight->getPoint());
	quadPath.lineTo(bottomLeft->getPoint());
	quadPath.closeSubPath();
}

bool Surface::isPointInside(Point<float> pos)
{
	return quadPath.contains(pos);
}

void Surface::resetBezierPoints()
{
	Point<float> tl(topLeft->x, topLeft->y);
	Point<float> tr(topRight->x, topRight->y);
	Point<float> bl(bottomLeft->x, bottomLeft->y);
	Point<float> br(bottomRight->x, bottomRight->y);

	Point<float> temp;
	var v; v.append(0); v.append(0);

	temp = tl + ((tr - tl) / 3);			v[0] = temp.x; v[1] = temp.y;		handleBezierTopLeft->setValue(v);
	temp = tl + ((tr - tl) * 2 / 3);		v[0] = temp.x; v[1] = temp.y;		handleBezierTopRight->setValue(v);
	temp = bl + ((br - bl) / 3);			v[0] = temp.x; v[1] = temp.y;		handleBezierBottomLeft->setValue(v);
	temp = bl + ((br - bl) * 2 / 3);		v[0] = temp.x; v[1] = temp.y;		handleBezierBottomRight->setValue(v);
	temp = tl + ((bl - tl) / 3);			v[0] = temp.x; v[1] = temp.y;		handleBezierLeftTop->setValue(v);
	temp = tl + ((bl - tl) * 2 / 3);		v[0] = temp.x; v[1] = temp.y;		handleBezierLeftBottom->setValue(v);
	temp = tr + ((br - tr) / 3);			v[0] = temp.x; v[1] = temp.y;		handleBezierRightTop->setValue(v);
	temp = tr + ((br - tr) * 2 / 3);		v[0] = temp.x; v[1] = temp.y;		handleBezierRightBottom->setValue(v);

}

void Surface::newMessage(const Media::MediaEvent& e)
{
	if (e.type == Media::MediaEvent::SUB_FRAMEBUFFERS_CHANGED)
	{
		updateMediaTextureNames();
	}
}

bool Surface::isUsingMedia(Media* m)
{
	if (!enabled->boolValue()) return false;
	return MediaTarget::isUsingMedia(m);
}

Array<Point2DParameter*> Surface::getCornerHandles()
{
	return { topLeft, topRight, bottomLeft, bottomRight };
}

Array<Point2DParameter*> Surface::getAllHandles()
{
	Array<Point2DParameter*> ret = { topLeft, topRight, bottomLeft, bottomRight, handleBezierTopLeft, handleBezierTopRight, handleBezierBottomLeft, handleBezierBottomRight, handleBezierLeftTop, handleBezierLeftBottom, handleBezierRightTop, handleBezierRightBottom };
	for (int i = 0; i < pinsCC.items.size(); i++)
	{
		ret.add(pinsCC.items[i]->position);
	}
	return ret;
}

Array<Point2DParameter*> Surface::getBezierHandles(Point2DParameter* corner)
{
	if (corner != nullptr)
	{
		if (corner == topLeft) return { handleBezierTopLeft, handleBezierLeftTop };
		else if (corner == topRight) return { handleBezierTopRight, handleBezierRightTop };
		else if (corner == bottomLeft) return { handleBezierBottomLeft, handleBezierLeftBottom };
		else if (corner == bottomRight) return { handleBezierBottomRight, handleBezierRightBottom };
	}

	return { handleBezierTopLeft, handleBezierTopRight, handleBezierBottomLeft, handleBezierBottomRight, handleBezierLeftTop, handleBezierLeftBottom, handleBezierRightTop, handleBezierRightBottom };
}

int Surface::addToVertices(Point<float> posDisplay, Point<float> internalCoord, Vector3D<float> texCoord, Vector3D<float> maskCoord)
{
	vertices.add(posDisplay.x);
	vertices.add(posDisplay.y);
	vertices.add(internalCoord.x);
	vertices.add(internalCoord.y);
	vertices.add(texCoord.x);
	vertices.add(texCoord.y);
	vertices.add(texCoord.z);
	vertices.add(maskCoord.x);
	vertices.add(maskCoord.y);
	vertices.add(maskCoord.z);
	int nVertices = vertices.size() / 10;
	return nVertices - 1;
}

void Surface::addLastFourAsQuad()
{
	int nVertices = vertices.size() / 10;
	if (nVertices >= 4) {
		verticesElements.add(nVertices - 4);
		verticesElements.add(nVertices - 3);
		verticesElements.add(nVertices - 2);
		verticesElements.add(nVertices - 3);
		verticesElements.add(nVertices - 2);
		verticesElements.add(nVertices - 1);
	}
}

void Surface::updateVertices()
{
	ScopedLock l(verticesLock);

	vertices.clear();
	verticesElements.clear();

	Media* med = mediaParam->getTargetContainerAs<Media>();

	Point<float>tl = openGLPoint(topLeft);
	Point<float>tr = openGLPoint(topRight);
	Point<float>bl = openGLPoint(bottomLeft);
	Point<float>br = openGLPoint(bottomRight);

	Point<float> center(0, 0);
	intersection(tl, br, bl, tr, &center);

	Vector3D<float> tlTex(cropLeft->floatValue(), 1 - cropTop->floatValue(), 1.0f);
	Vector3D<float> trTex(1 - cropRight->floatValue(), 1 - cropTop->floatValue(), 1.0f);
	Vector3D<float> blTex(cropLeft->floatValue(), cropBottom->floatValue(), 1.0f);
	Vector3D<float> brTex(1 - cropRight->floatValue(), cropBottom->floatValue(), 1.0f);

	float hTex = tlTex.y - blTex.y;
	float wTex = trTex.x - tlTex.x;
	float texMidX = blTex.x + (wTex / 2.0f);
	float texMidY = blTex.y + (hTex / 2.0f);

	FillType t = fillType->getValueDataAsEnum<FillType>();

	if (t != STRETCH) {
		float outputRatio = ratio->floatValue();

		if (hTex == 0) hTex = 0.0000001;

		if (med != nullptr) {
			Point<int> mediaSize = med->getMediaSize(mediaTextureName->stringValue());
			float mediaRatio = abs((wTex * mediaSize.x) / (hTex * (float)mediaSize.y));
			if (mediaRatio != outputRatio) {
				if (t == FIT) {
					float transformRatio = mediaRatio / outputRatio;

					if (transformRatio > 1) {
						float newH = 1 / transformRatio;
						float allTextHeight = hTex / newH;
						float midTextHeight = allTextHeight / 2.0f;

						tlTex.y = texMidY - midTextHeight;
						trTex.y = texMidY - midTextHeight;
						blTex.y = texMidY + midTextHeight;
						brTex.y = texMidY + midTextHeight;
					}
					else {
						float newW = transformRatio;
						float allTextWidth = wTex / newW;
						float midTextWidth = allTextWidth / 2.0f;

						tlTex.x = texMidX - midTextWidth;
						blTex.x = texMidX - midTextWidth;
						trTex.x = texMidX + midTextWidth;
						brTex.x = texMidX + midTextWidth;
					}

				}
				else if (t == FILL) {
					float transformRatio = mediaRatio / outputRatio;

					if (transformRatio > 1) {
						float newW = transformRatio;
						float allTextWidth = hTex / newW;
						float midTextWidth = allTextWidth / 2.0f;

						tlTex.x = texMidX - midTextWidth;
						blTex.x = texMidX - midTextWidth;
						trTex.x = texMidX + midTextWidth;
						brTex.x = texMidX + midTextWidth;
					}
					else {
						float newH = 1 / transformRatio;
						float allTextHeight = wTex / newH;
						float midTextHeight = allTextHeight / 2.0f;

						tlTex.y = texMidY - midTextHeight;
						trTex.y = texMidY - midTextHeight;
						blTex.y = texMidY + midTextHeight;
						brTex.y = texMidY + midTextHeight;
					}

				}

			}

		}

	}

	Vector3D<float> tlMask(0, 1, 1.0f);
	Vector3D<float> trMask(1, 1, 1.0f);
	Vector3D<float> blMask(0, 0, 1.0f);
	Vector3D<float> brMask(1, 0, 1.0f);

	float dtl = center.getDistanceFrom(tl);
	float dtr = center.getDistanceFrom(tr);
	float dbr = center.getDistanceFrom(br);
	float dbl = center.getDistanceFrom(bl);

	if (bezierCC.enabled->boolValue()) {
		const int gridSize = 40;
		Point<float> grid[gridSize][gridSize];

		for (int i = 0; i < gridSize; i++) {
			for (int j = 0; j < gridSize; j++) {
				grid[i][j] = Point<float>();
			}
		}

		float distTop = 0;
		float distBottom = 0;
		for (int i = 0; i < gridSize; i++) {
			float ratio = i / (float)(gridSize - 1);
			grid[i][0] = getBeziers(openGLPoint(topLeft), openGLPoint(handleBezierTopLeft), openGLPoint(handleBezierTopRight), openGLPoint(topRight), ratio);
			grid[i][gridSize - 1] = getBeziers(openGLPoint(bottomLeft), openGLPoint(handleBezierBottomLeft), openGLPoint(handleBezierBottomRight), openGLPoint(bottomRight), ratio);

			if (i > 0) {
				distTop += grid[i][0].getDistanceFrom(grid[i - 1][0]);
				distBottom += grid[i][gridSize - 1].getDistanceFrom(grid[i - 1][gridSize - 1]);
			}
		}

		Point<float>deltaHandleLT = openGLPoint(handleBezierLeftTop) - openGLPoint(topLeft);
		Point<float>deltaHandleRT = openGLPoint(handleBezierRightTop) - openGLPoint(topRight);
		Point<float>deltaHandleLB = openGLPoint(handleBezierLeftBottom) - openGLPoint(bottomLeft);
		Point<float>deltaHandleRB = openGLPoint(handleBezierRightBottom) - openGLPoint(bottomRight);

		Point<float> deltaTop = deltaHandleRT - deltaHandleLT;
		Point<float> deltaBottom = deltaHandleRB - deltaHandleLB;

		float currentDistTop = 0;
		float currentDistBottom = 0;
		for (int i = 0; i < gridSize; i++) {
			if (i > 0) {
				currentDistTop += grid[i][0].getDistanceFrom(grid[i - 1][0]);
				currentDistBottom += grid[i][gridSize - 1].getDistanceFrom(grid[i - 1][gridSize - 1]);
			}

			float ratioTop = currentDistTop / distTop;
			float ratioBottom = currentDistBottom / distBottom;

			Point<float> handleTop = deltaHandleLT + (deltaTop * ratioTop) + grid[i][0];
			Point<float> handleBottom = deltaHandleLB + (deltaBottom * ratioBottom) + grid[i][gridSize - 1];

			for (int j = 1; j < gridSize - 1; j++) {
				float ratio2 = j / (float)(gridSize - 1);
				grid[i][j] = getBeziers(grid[i][0], handleTop, handleBottom, grid[i][gridSize - 1], ratio2);
			}

		}

		float ratio = 1.0f / (float)(gridSize - 1);

		float fromX = cropLeft->floatValue();
		float toX = 1 - cropRight->floatValue();
		float fromY = cropBottom->floatValue();
		float toY = 1 - cropTop->floatValue();


		for (int i = 0; i < gridSize - 1; i++) {
			for (int j = 0; j < gridSize - 1; j++) {
				tlTex = Vector3D<float>(jmap(i * ratio, 0.0f, 1.0f, fromX, toX), jmap(1 - (j * ratio), 0.0f, 1.0f, fromY, toY), 1);
				trTex = Vector3D<float>(jmap((i + 1) * ratio, 0.0f, 1.0f, fromX, toX), jmap(1 - (j * ratio), 0.0f, 1.0f, fromY, toY), 1);
				blTex = Vector3D<float>(jmap(i * ratio, 0.0f, 1.0f, fromX, toX), jmap(1 - ((j + 1) * ratio), 0.0f, 1.0f, fromY, toY), 1);
				brTex = Vector3D<float>(jmap((i + 1) * ratio, 0.0f, 1.0f, fromX, toX), jmap(1 - ((j + 1) * ratio), 0.0f, 1.0f, fromY, toY), 1);
				tlMask = Vector3D<float>(i * ratio, 1 - (j * ratio), 1);
				trMask = Vector3D<float>((i + 1) * ratio, 1 - (j * ratio), 1);
				blMask = Vector3D<float>(i * ratio, 1 - ((j + 1) * ratio), 1);
				brMask = Vector3D<float>((i + 1) * ratio, 1 - ((j + 1) * ratio), 1);


				addToVertices(grid[i][j], Point<float>(((i) * 2 * ratio) - 1, -(((j) * 2 * ratio) - 1)), tlTex, tlMask);
				addToVertices(grid[i + 1][j], Point<float>(((i + 1) * 2 * ratio) - 1, -(((j) * 2 * ratio) - 1)), trTex, trMask);
				addToVertices(grid[i][j + 1], Point<float>(((i) * 2 * ratio) - 1, -(((j + 1) * 2 * ratio) - 1)), blTex, blMask);
				addToVertices(grid[i + 1][j + 1], Point<float>(((i + 1) * 2 * ratio) - 1, -(((j + 1) * 2 * ratio) - 1)), brTex, brMask);
				addLastFourAsQuad();
			}
		}


	}
	else
	{
		Array<Pin*> pins;
		if (pinsCC.items.size() > 0) {
			for (int i = 0; i < pinsCC.items.size(); i++) {
				if (pinsCC.items[i]->enabled->boolValue()) {
					pins.add(pinsCC.items[i]);
				}
			}
		}

		if (pins.size() > 0)
		{
			cornerPins[0]->position->x = topLeft->x; cornerPins[0]->position->y = topLeft->y;
			cornerPins[0]->mediaPos->x = tlTex.x; cornerPins[0]->mediaPos->y = tlTex.y;
			cornerPins[1]->position->x = topRight->x; cornerPins[1]->position->y = topRight->y;
			cornerPins[1]->mediaPos->x = trTex.x; cornerPins[1]->mediaPos->y = trTex.y;
			cornerPins[2]->position->x = bottomLeft->x; cornerPins[2]->position->y = bottomLeft->y;
			cornerPins[2]->mediaPos->x = blTex.x; cornerPins[2]->mediaPos->y = blTex.y;
			cornerPins[3]->position->x = bottomRight->x; cornerPins[3]->position->y = bottomRight->y;
			cornerPins[3]->mediaPos->x = brTex.x; cornerPins[3]->mediaPos->y = brTex.y;

			pins.addArray(cornerPins);

			Array<int> verticeId;

			for (int i = 0; i < pins.size(); i++)
			{
				Point<float> pinPos = openGLPoint(pins[i]->position);
				Point<float> pinMediaCoord = pins[i]->mediaPos->getPoint();
				Vector3D<float> pinTex = Vector3D<float>(pinMediaCoord.x, pinMediaCoord.y, 1);
				pinTex *= pins[i]->ponderation->floatValue();
				Vector3D<float> pinMask = Vector3D<float>(pinMediaCoord.x, pinMediaCoord.y, 1);
				pinMask *= pins[i]->ponderation->floatValue();
				verticeId.add(addToVertices(pinPos, pinMediaCoord, pinTex, pinMask));
			}

			for (int iA = 0; iA < pins.size(); iA++)
			{
				Pin* pinA = pins[iA];
				Point<float>a = pinA->position->getPoint();
				for (int iB = iA + 1; iB < pins.size(); iB++)
				{
					Pin* pinB = pins[iB];
					Point<float>b = pinB->position->getPoint();
					for (int iC = iB + 1; iC < pins.size(); iC++)
					{
						Pin* pinC = pins[iC];
						Point<float>c = pinC->position->getPoint();
						if (pinA != pinB && pinB != pinC && pinA != pinC)
						{
							bool triangleIsValid = true;
							for (int iD = 0; iD < pins.size() && triangleIsValid; iD++)
							{
								Pin* pinD = pins[iD];
								Point<float>d = pinD->position->getPoint();
								if (pinD != pinA && pinD != pinB && pinD != pinC)
								{
									if (isPointInsideCircumcircle(d, a, b, c))
									{
										triangleIsValid = false;
									}
								}
							}
							if (triangleIsValid) {
								verticesElements.add(verticeId[iA]);
								verticesElements.add(verticeId[iB]);
								verticesElements.add(verticeId[iC]);
							}
						}
					}
				}
			}

		}
		else
		{
			float ztl = ((dtl + dbr) / dbr);
			float ztr = ((dtr + dbl) / dbl);
			float zbr = ((dbr + dtl) / dtl);
			float zbl = ((dbl + dtr) / dtr);

			tlTex *= ztl;
			trTex *= ztr;
			blTex *= zbl;
			brTex *= zbr;

			tlMask *= ztl;
			trMask *= ztr;
			blMask *= zbl;
			brMask *= zbr;

			addToVertices(tl, Point<float>(-1, 1), tlTex, tlMask);
			addToVertices(tr, Point<float>(1, 1), trTex, trMask);
			addToVertices(bl, Point<float>(-1, -1), blTex, blMask);
			addToVertices(br, Point<float>(1, -1), brTex, brMask);
			addLastFourAsQuad();
		}
	}
}

void Surface::draw(GLuint shaderID)
{
	if (!enabled->boolValue()) return;


	Media* media = getMedia();
	String texName = media == currentMedia ? mediaTextureName->stringValue() : String();

	{
		GenericScopedLock lock(patternMediaLock);
		if (patternMedia != nullptr)
		{
			Point<int> ms = media != nullptr ? media->getMediaSize(texName) : Point<int>(512, 512);
			patternMedia->width->setValue(ms.x);
			patternMedia->height->setValue(ms.y);
			media = patternMedia.get();
		}
	}

	if (media == nullptr) return;

	Point<float> tl, tr, bl, br;

	Media* maskMedia = mask->getTargetContainerAs<Media>();// dynamic_cast<Media*>(mask->targetContainer.get());
	std::shared_ptr<OpenGLTexture> texMask = nullptr;

	GLuint maskLocation = glGetUniformLocation(shaderID, "mask");
	glUniform1i(maskLocation, 0);
	glActiveTexture(GL_TEXTURE0);

	if (maskMedia != nullptr && !showTestPattern->boolValue())
	{
		glBindTexture(GL_TEXTURE_2D, maskMedia->getTextureID());
	}
	else
	{
		juce::Image whiteImage(juce::Image::PixelFormat::ARGB, 1, 1, true);
		whiteImage.setPixelAt(0, 0, Colours::white);
		texMask = std::make_shared<OpenGLTexture>();
		texMask->loadImage(whiteImage);
		texMask->bind();
	}
	glGetError();



	GLuint textureLocation = glGetUniformLocation(shaderID, "tex");
	glUniform1i(textureLocation, 1);
	glActiveTexture(GL_TEXTURE1);
	glGetError();

	if (media == nullptr) return;

	glBindTexture(GL_TEXTURE_2D, media->getTextureID(texName));

	Colour tintColor = tint->getColor();


	// vertices start
	if (shouldUpdateVertices) {
		shouldUpdateVertices = false;
		updateVertices();

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		posAttrib = glGetAttribLocation(shaderID, "position");
		glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), 0);

		surfacePosAttrib = glGetAttribLocation(shaderID, "surfacePosition");
		glVertexAttribPointer(surfacePosAttrib, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (void*)(2 * sizeof(float)));

		texAttrib = glGetAttribLocation(shaderID, "texcoord");
		glVertexAttribPointer(texAttrib, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(4 * sizeof(float)));

		maskAttrib = glGetAttribLocation(shaderID, "maskcoord");
		glVertexAttribPointer(maskAttrib, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(7 * sizeof(float)));

		borderSoftLocation = glGetUniformLocation(shaderID, "borderSoft");
		glUniform4f(borderSoftLocation, softEdgeTop->floatValue(), softEdgeRight->floatValue(), softEdgeBottom->floatValue(), softEdgeLeft->floatValue());

		invertMaskLocation = glGetUniformLocation(shaderID, "invertMask");
		glUniform1i(invertMaskLocation, invertMask->boolValue() ? 1 : 0);

		ratioLocation = glGetUniformLocation(shaderID, "ratio");
		glUniform1f(ratioLocation, ratio->floatValue());

		tintLocation = glGetUniformLocation(shaderID, "tint");

		glGenBuffers(1, &ebo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	}

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), 0);
	glVertexAttribPointer(surfacePosAttrib, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(GLfloat), (void*)(2 * sizeof(float)));
	glVertexAttribPointer(texAttrib, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(4 * sizeof(float)));
	glVertexAttribPointer(maskAttrib, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (void*)(7 * sizeof(float)));
	glUniform4f(borderSoftLocation, softEdgeTop->floatValue(), softEdgeRight->floatValue(), softEdgeBottom->floatValue(), softEdgeLeft->floatValue());
	glUniform1i(invertMaskLocation, invertMask->boolValue() ? 1 : 0);
	glUniform1i(ratioLocation, ratio->floatValue());

	float boostValue = boost->floatValue();
	glUniform4f(tintLocation, tintColor.getFloatRed() * boostValue, tintColor.getFloatGreen() * boostValue, tintColor.getFloatBlue() * boostValue, tintColor.getFloatAlpha());

	glEnableVertexAttribArray(posAttrib);
	glGetError();

	glEnableVertexAttribArray(surfacePosAttrib);
	glGetError();

	glEnableVertexAttribArray(texAttrib);
	glGetError();

	glEnableVertexAttribArray(maskAttrib);
	glGetError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glGetError();

	verticesLock.enter();
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), vertices.getRawDataPointer(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, verticesElements.size() * sizeof(GLuint), verticesElements.getRawDataPointer(), GL_STATIC_DRAW);
	verticesLock.exit();

	glBlendFunc((GLenum)(int)blendFunctionSourceFactor->getValueData(), (GLenum)(int)blendFunctionDestinationFactor->getValueData());

	//glDrawElements(GL_LINES, verticesElements.size(), GL_UNSIGNED_INT, 0);
	glDrawElements(GL_TRIANGLES, verticesElements.size(), GL_UNSIGNED_INT, 0);
	glGetError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	//glDeleteBuffers(1, &ebo);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glDeleteBuffers(1, &vbo);

	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);

	glActiveTexture(GL_TEXTURE0);
	glGetError();
}

Media* Surface::getMedia()
{
	return previewMedia != nullptr ? previewMedia : currentMedia;
}


Point<float> Surface::getBeziers(Point<float>a, Point<float>b, Point<float>c, Point<float>d, float r) {

	float x = (pow(1 - r, 3) * a.x) + (3 * (pow(1 - r, 2)) * r * b.x) + (3 * (pow(1 - r, 1)) * r * r * c.x) + (r * r * r * d.x);
	float y = (pow(1 - r, 3) * a.y) + (3 * (pow(1 - r, 2)) * r * b.y) + (3 * (pow(1 - r, 1)) * r * r * c.y) + (r * r * r * d.y);
	return Point<float>(x, y);
}

bool Surface::intersection(Point<float> p1, Point<float> p2, Point<float> p3, Point<float> p4, Point<float>* intersect)
{
	// Store the values for fast access and easy
	// equations-to-code conversion
	float x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
	float y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;

	float d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	// If d is zero, there is no intersection
	if (d == 0) return false;

	// Get the x and y
	float pre = (x1 * y2 - y1 * x2), post = (x3 * y4 - y3 * x4);
	float x = (pre * (x3 - x4) - (x1 - x2) * post) / d;
	float y = (pre * (y3 - y4) - (y1 - y2) * post) / d;

	// Check if the x and y coordinates are within both lines
	if (x < jmin(x1, x2) || x > jmax(x1, x2) ||
		x < jmin(x3, x4) || x > jmax(x3, x4)) return false;
	if (y < jmin(y1, y2) || y > jmax(y1, y2) ||
		y < jmin(y3, y4) || y > jmax(y3, y4)) return false;

	// Return the point of intersection
	intersect->setXY(x, y);
	return true;
}

Point<float> Surface::openGLPoint(Point2DParameter* p)
{
	Point<float> r = p->getPoint();
	r.x = (r.x * 2) - 1;
	r.y = (r.y * 2) - 1;
	return r;
}


bool Surface::isPointInsideTriangle(Point<float> point, Point<float> vertex1, Point<float> vertex2, Point<float> vertex3)
{
	juce::Point<float> AB(vertex2.x - vertex1.x, vertex2.y - vertex1.y);
	juce::Point<float> BC(vertex3.x - vertex2.x, vertex3.y - vertex2.y);
	juce::Point<float> CA(vertex1.x - vertex3.x, vertex1.y - vertex3.y);

	juce::Point<float> AP(point.x - vertex1.x, point.y - vertex1.y);
	juce::Point<float> BP(point.x - vertex2.x, point.y - vertex2.y);
	juce::Point<float> CP(point.x - vertex3.x, point.y - vertex3.y);

	float crossAB = AB.x * AP.y - AB.y * AP.x;
	float crossBC = BC.x * BP.y - BC.y * BP.x;
	float crossCA = CA.x * CP.y - CA.y * CP.x;

	return (crossAB > 0 && crossBC > 0 && crossCA > 0) || (crossAB < 0 && crossBC < 0 && crossCA < 0);
}

bool Surface::isPointInsideCircumcircle(juce::Point<float> point, juce::Point<float> vertex1, juce::Point<float> vertex2, juce::Point<float> vertex3)
{
	float x1 = vertex1.x, y1 = vertex1.y;
	float x2 = vertex2.x, y2 = vertex2.y;
	float x3 = vertex3.x, y3 = vertex3.y;

	float D = 2 * (x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2));

	float Ux = ((x1 * x1 + y1 * y1) * (y2 - y3) + (x2 * x2 + y2 * y2) * (y3 - y1) + (x3 * x3 + y3 * y3) * (y1 - y2)) / D;
	float Uy = ((x1 * x1 + y1 * y1) * (x3 - x2) + (x2 * x2 + y2 * y2) * (x1 - x3) + (x3 * x3 + y3 * y3) * (x2 - x1)) / D;

	Point<float> center = Point<float>(Ux, Uy);

	// Comparaison des distances au carré
	return center.getDistanceFrom(point) < center.getDistanceFrom(vertex1);
}