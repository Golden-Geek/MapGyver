/*
  ==============================================================================

    Object.h
    Created: 26 Sep 2020 10:02:32am
    Author:  bkupe

  ==============================================================================
*/

#pragma once

class Surface :
    public BaseItem
{
public:
    Surface(var params = var());
    virtual ~Surface();

    String objectType;
    var objectData;

    BoolParameter* showTestPattern;

    Point2DParameter* topLeft;
    Point2DParameter* topRight;
    Point2DParameter* bottomLeft;
    Point2DParameter* bottomRight;

    BoolParameter* isBezier;
    Point2DParameter* handleBezierTopLeft;
    Point2DParameter* handleBezierTopRight;
    Point2DParameter* handleBezierBottomLeft;
    Point2DParameter* handleBezierBottomRight;
    Point2DParameter* handleBezierLeftTop;
    Point2DParameter* handleBezierLeftBottom;
    Point2DParameter* handleBezierRightTop;
    Point2DParameter* handleBezierRightBottom;

    FloatParameter* softEdgeTop;
    FloatParameter* softEdgeRight;
    FloatParameter* softEdgeBottom;
    FloatParameter* softEdgeLeft;

    FloatParameter* cropTop;
    FloatParameter* cropRight;
    FloatParameter* cropBottom;
    FloatParameter* cropLeft;

    TargetParameter* media;
    TargetParameter* mask;
    BoolParameter* invertMask;

    Path quadPath;

    void onContainerParameterChangedInternal(Parameter* p);

    void updatePath();

    bool isPointInside(Point<float> pos);

    void resetBezierPoints();
    Trigger* resetBezierBtn;

    void triggerTriggered(Trigger* t) override;

    unsigned int verticesVersion;
    Array<GLfloat> vertices;
    Array<GLuint> verticesElements;
    CriticalSection verticesLock;

    void addToVertices(Point<float> posDisplay, Point<float>itnernalCoord, Vector3D<float> texCoord, Vector3D<float> maskCoord);
    void addLastFourAsQuad();
    void updateVertices();

    Point<int> getMediaSize();

    String getTypeString() const override { return objectType; }
    static Surface* create(var params) { return new Surface(params); }

    static Point<float> getBeziers(Point<float>a, Point<float>b, Point<float>c, Point<float>d, float r);
    static bool intersection(Point<float> p1, Point<float> p2, Point<float> p3, Point<float> p4, Point<float>* intersect); // should be in another objet
    static Point<float> openGLPoint(Point2DParameter* p);

};


