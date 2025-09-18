/*
  ==============================================================================

	Object.h
	Created: 26 Sep 2020 10:02:32am
	Author:  bkupe

  ==============================================================================
*/

#pragma once


class Media;

class Surface :
	public BaseItem,
	public MediaTarget,
	public Media::AsyncListener
{
public:
	Surface(var params = var());
	virtual ~Surface();

	String objectType;
	var objectData;

	bool shouldUpdateVertices;


	TargetParameter* mediaParam;
	EnumParameter* mediaTextureName;
	Media* currentMedia;

	std::unique_ptr<Media> patternMedia;
	SpinLock patternMediaLock;

	ControllableContainer positionningCC;
	Point2DParameter* topLeft;
	Point2DParameter* topRight;
	Point2DParameter* bottomLeft;
	Point2DParameter* bottomRight;

	EnablingControllableContainer bezierCC;
	Point2DParameter* handleBezierTopLeft;
	Point2DParameter* handleBezierTopRight;
	Point2DParameter* handleBezierBottomLeft;
	Point2DParameter* handleBezierBottomRight;
	Point2DParameter* handleBezierLeftTop;
	Point2DParameter* handleBezierLeftBottom;
	Point2DParameter* handleBezierRightTop;
	Point2DParameter* handleBezierRightBottom;

	Manager<Pin> pinsCC;
	OwnedArray<Pin> cornerPins; // 0: topLeft, 1: topRight, 2: bottomRight, 3: bottomLeft

	ControllableContainer adjustmentsCC;
	EnumParameter* blendFunction;
	enum BlendPreset {
		STANDARD, ADDITION, MULTIPLICATION, SCREEN, DARKEN, PREMULTALPHA, LIGHTEN, INVERT, COLORADD, COLORSCREEN, BLUR, INVERTCOLOR, SUBSTRACT, COLORDIFF, INVERTMULT, CUSTOM
	};
	EnumParameter* blendFunctionSourceFactor;
	EnumParameter* blendFunctionDestinationFactor;

	ColorParameter* tint;
	FloatParameter* boost;

	BoolParameter* showTestPattern;
	TargetParameter* mask;
	BoolParameter* invertMask;

	ControllableContainer formatCC;
	enum FillType { STRETCH, FIT, FILL };
	EnumParameter* fillType;
	enum Ratio { R4_3, R16_9, R16_10, R1, RCUSTOM };
	EnumParameter* ratioList;
	FloatParameter* ratio;
	BoolParameter* considerCrop;

	FloatParameter* softEdgeTop;
	FloatParameter* softEdgeRight;
	FloatParameter* softEdgeBottom;
	FloatParameter* softEdgeLeft;

	FloatParameter* cropTop;
	FloatParameter* cropRight;
	FloatParameter* cropBottom;
	FloatParameter* cropLeft;

	Media* previewMedia;
	Path quadPath;

	// openGL variables
	GLuint vbo;
	GLint posAttrib;
	GLint surfacePosAttrib;
	GLint texAttrib;
	GLint maskAttrib;
	GLuint borderSoftLocation;
	GLuint invertMaskLocation;
	GLuint ratioLocation;
	GLuint tintLocation;
	GLuint ebo;

	void setupMedia();
	void updateMediaTextureNames();

	void onContainerParameterChangedInternal(Parameter* p);
	void onControllableFeedbackUpdateInternal(ControllableContainer* cc, Controllable* c) override;

	void updatePath();

	bool isPointInside(Point<float> pos);

	void resetBezierPoints();
	Trigger* resetBezierBtn;

	unsigned int verticesVersion;
	Array<GLfloat> vertices;
	Array<GLuint> verticesElements;
	CriticalSection verticesLock;

	int addToVertices(Point<float> posDisplay, Point<float>itnernalCoord, Vector3D<float> texCoord, Vector3D<float> maskCoord);
	void addLastFourAsQuad();
	void updateVertices();
	void draw(GLuint shaderID);

	Media* getMedia();

	void newMessage(const Media::MediaEvent& e) override;

	bool isUsingMedia(Media* m) override;

	Array<Point2DParameter*> getCornerHandles();
	Array<Point2DParameter*> getAllHandles();
	Array<Point2DParameter*> getBezierHandles(Point2DParameter* corner = nullptr);

	String getTypeString() const override { return objectType; }
	static Surface* create(var params) { return new Surface(params); }

	static Point<float> getBeziers(Point<float>a, Point<float>b, Point<float>c, Point<float>d, float r);
	static bool intersection(Point<float> p1, Point<float> p2, Point<float> p3, Point<float> p4, Point<float>* intersect); // should be in another objet
	static Point<float> openGLPoint(Point2DParameter* p);
	static bool isPointInsideTriangle(Point<float> point, Point<float> vertex1, Point<float> vertex2, Point<float> vertex3);
	static bool isPointInsideCircumcircle(Point<float> point, Point<float> vertex1, Point<float> vertex2, Point<float> vertex3);
};


