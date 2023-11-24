/*
  ==============================================================================

	Object.cpp
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#include "Screen/ScreenIncludes.h"
#include "Media/MediaIncludes.h"

#define SURFACE_TARGET_MEDIA_ID 0
#define SURFACE_TARGET_MASK_ID 1

Surface::Surface(var params) :
	BaseItem(params.getProperty("name", "Surface")),
	positionningCC("Positionning"),
	bezierCC("Bezier"),
	adjustmentsCC("Adjustments"),
	objectType(params.getProperty("type", "Surface").toString()),
	objectData(params),
	previewMedia(nullptr),
	shouldUpdateVertices(true)
{
	saveAndLoadRecursiveData = true;
	canBeDisabled = true;

	itemDataType = "Surface";

	media = addTargetParameter("Media", "Media to read on this surface", MediaManager::getInstance());
	media->maxDefaultSearchLevel = 0;
	media->targetType = TargetParameter::CONTAINER;

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

	cropTop = adjustmentsCC.addFloatParameter("Crop Top", "", 0, 0, 1);
	cropRight = adjustmentsCC.addFloatParameter("Crop Right", "", 0, 0, 1);
	cropBottom = adjustmentsCC.addFloatParameter("Crop Bottom", "", 0, 0, 1);
	cropLeft = adjustmentsCC.addFloatParameter("Crop Left", "", 0, 0, 1);

	showTestPattern = adjustmentsCC.addBoolParameter("Show Test Pattern", "If checked this will not use the media but generate a TestPatterns", false);
	mask = adjustmentsCC.addTargetParameter("Mask", "Apply a mask to this surface", MediaManager::getInstance());
	invertMask = adjustmentsCC.addBoolParameter("Invert mask", "Invert mask", false);

	topLeft->setDefaultPoint(0, 1);
	topLeft->setBounds(-1, -1, 2, 2);
	topRight->setDefaultPoint(1, 1);
	topRight->setBounds(-1, -1, 2, 2);
	bottomLeft->setDefaultPoint(0, 0);
	bottomLeft->setBounds(-1, -1, 2, 2);
	bottomRight->setDefaultPoint(1, 0);
	bottomRight->setBounds(-1, -1, 2, 2);


	handleBezierTopLeft->setDefaultPoint(1 / 3.0f, 1);
	handleBezierTopLeft->setBounds(0, 0, 1, 1);
	handleBezierTopRight->setDefaultPoint(2 / 3.0f, 1);
	handleBezierTopRight->setBounds(0, 0, 1, 1);
	handleBezierBottomLeft->setDefaultPoint(1 / 3.0f, 0);
	handleBezierBottomLeft->setBounds(0, 0, 1, 1);
	handleBezierBottomRight->setDefaultPoint(2 / 3.0f, 0);
	handleBezierBottomRight->setBounds(0, 0, 1, 1);
	handleBezierLeftTop->setDefaultPoint(0, 2 / 3.0f);
	handleBezierLeftTop->setBounds(0, 0, 1, 1);
	handleBezierLeftBottom->setDefaultPoint(0, 1 / 3.0f);
	handleBezierLeftBottom->setBounds(0, 0, 1, 1);
	handleBezierRightTop->setDefaultPoint(1, 2 / 3.0f);
	handleBezierRightTop->setBounds(0, 0, 1, 1);
	handleBezierRightBottom->setDefaultPoint(1, 1 / 3.0f);
	handleBezierRightBottom->setBounds(0, 0, 1, 1);

	mask->maxDefaultSearchLevel = 0;
	mask->targetType = TargetParameter::CONTAINER;

	bezierCC.enabled->setDefaultValue(false);

	positionningCC.editorIsCollapsed = true;
	bezierCC.editorIsCollapsed = true;
	adjustmentsCC.editorIsCollapsed = true;

	positionningCC.saveAndLoadRecursiveData = true;
	positionningCC.addChildControllableContainer(&bezierCC);
	addChildControllableContainer(&positionningCC);
	addChildControllableContainer(&adjustmentsCC);

	if (!Engine::mainEngine->isLoadingFile)
	{
		if (!MediaManager::getInstance()->items.isEmpty()) media->setValueFromTarget(MediaManager::getInstance()->items.getFirst());
	}
}

Surface::~Surface()
{
}

void Surface::onContainerParameterChangedInternal(Parameter* p)
{
	if (p == media)
	{
		if (Media* m = media->getTargetContainerAs<Media>()) registerUseMedia(SURFACE_TARGET_MEDIA_ID, m);
		else unregisterUseMedia(SURFACE_TARGET_MEDIA_ID);

		shouldUpdateVertices = true;
	}
}

void Surface::onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c)
{
	if (c == resetBezierBtn) {
		resetBezierPoints();
	}
	
	shouldUpdateVertices = true;
	
	if (c == topLeft || c == topRight || c == bottomLeft || c == bottomRight)
	{
		updatePath();
	}
	else if (c == mask)
	{
		if (Media* m = mask->getTargetContainerAs<Media>()) registerUseMedia(SURFACE_TARGET_MASK_ID, m);
		else unregisterUseMedia(SURFACE_TARGET_MASK_ID);
	}
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

Point<int> Surface::getMediaSize()
{
	if (Media* m = media->getTargetContainerAs<Media>())
		return m->getMediaSize();

	return Point<int>();
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
	return { topLeft, topRight, bottomLeft, bottomRight, handleBezierTopLeft, handleBezierTopRight, handleBezierBottomLeft, handleBezierBottomRight, handleBezierLeftTop, handleBezierLeftBottom, handleBezierRightTop, handleBezierRightBottom };
}

Array<Point2DParameter*> Surface::getBezierHandles(Point2DParameter* corner)
{
	if (corner != nullptr)
	{
		if(corner == topLeft) return { handleBezierTopLeft, handleBezierLeftTop };
		else if (corner == topRight) return { handleBezierTopRight, handleBezierRightTop };
		else if (corner == bottomLeft) return { handleBezierBottomLeft, handleBezierLeftBottom };
		else if (corner == bottomRight) return { handleBezierBottomRight, handleBezierRightBottom };
	}

	return { handleBezierTopLeft, handleBezierTopRight, handleBezierBottomLeft, handleBezierBottomRight, handleBezierLeftTop, handleBezierLeftBottom, handleBezierRightTop, handleBezierRightBottom };
}

void Surface::addToVertices(Point<float> posDisplay, Point<float> internalCoord, Vector3D<float> texCoord, Vector3D<float> maskCoord)
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
	verticesLock.enter();
	vertices.clear();
	verticesElements.clear();

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

	Media* med = media->getTargetContainerAs<Media>();

	if (med != nullptr && med->flipY) {
		tlTex.y = 1 - tlTex.y;
		trTex.y = 1 - trTex.y;
		blTex.y = 1 - blTex.y;
		brTex.y = 1 - brTex.y;
	}


	Vector3D<float> tlMask(0, 1, 1.0f);
	Vector3D<float> trMask(1, 1, 1.0f);
	Vector3D<float> blMask(0, 0, 1.0f);
	Vector3D<float> brMask(1, 0, 1.0f);

	Media* msk = mask->getTargetContainerAs<Media>();
	if (msk != nullptr && msk->flipY) {
		tlMask.y = 1 - tlMask.y;
		trMask.y = 1 - trMask.y;
		blMask.y = 1 - blMask.y;
		brMask.y = 1 - brMask.y;
	}

	float dtl = center.getDistanceFrom(tl);
	float dtr = center.getDistanceFrom(tr);
	float dbr = center.getDistanceFrom(br);
	float dbl = center.getDistanceFrom(bl);

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
				distBottom += grid[i][gridSize - 1].getDistanceFrom(grid[i-1][gridSize - 1]);
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

			Point<float> handleTop = deltaHandleLT +(deltaTop*ratioTop)+  grid[i][0];
			Point<float> handleBottom = deltaHandleLB+(deltaBottom*ratioBottom) + grid[i][gridSize - 1];

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
		if (med != nullptr && med->flipY) {
			fromY = 1-fromY;
			toY = 1-toY;
		}

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
				if (msk != nullptr && msk->flipY) {
					tlMask.y = 1- tlMask.y;
					trMask.y = 1 - trMask.y;
					blMask.y = 1 - blMask.y;
					brMask.y = 1 - brMask.y;
				}
				addToVertices(grid[i][j], Point<float>(((i) * 2 * ratio) - 1, -(((j) * 2 * ratio) - 1)), tlTex, tlMask);
				addToVertices(grid[i + 1][j], Point<float>(((i+1) * 2 * ratio) - 1, -(((j) * 2 * ratio) - 1)), trTex, trMask);
				addToVertices(grid[i][j + 1], Point<float>(((i) * 2 * ratio) - 1, -(((j+1) * 2 * ratio) - 1)), blTex, blMask);
				addToVertices(grid[i + 1][j + 1], Point<float>(((i+1) * 2 * ratio) - 1, -(((j+1) * 2 * ratio) - 1)), brTex, brMask);
				addLastFourAsQuad();
			}
		}


	}
	else {
		addToVertices(tl, Point<float>(-1, 1), tlTex, tlMask);
		addToVertices(tr, Point<float>(1, 1), trTex, trMask);
		addToVertices(bl, Point<float>(-1, -1), blTex, blMask);
		addToVertices(br, Point<float>(1, -1), brTex, brMask);
		addLastFourAsQuad();
	}

	verticesLock.exit();
}

void Surface::draw(GLuint shaderID)
{
	if (!enabled->boolValue()) return;

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

	Media* media = getMedia();

	GLuint textureLocation = glGetUniformLocation(shaderID, "tex");
	glUniform1i(textureLocation, 1);
	glActiveTexture(GL_TEXTURE1);
	glGetError();

	if (media == nullptr && !showTestPattern->boolValue())
	{
		return;
	}
	if (showTestPattern->boolValue()) {
		glBindTexture(GL_TEXTURE_2D, GlContextHolder::getInstance()->testPattern.getTextureID());
	}
	else {
		glBindTexture(GL_TEXTURE_2D, media->getTextureID());
	}

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

	glGetError();
}

Media* Surface::getMedia()
{
	return previewMedia != nullptr ? previewMedia : media->getTargetContainerAs<Media>();
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
