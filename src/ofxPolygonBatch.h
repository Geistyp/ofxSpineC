//- GeistYp
#pragma once

#include "ofMain.h"

class ofxPolygonBatch
{
public:

	struct PolygonVertex
	{
		ofVec2f vertex;
		ofColor color;
		ofVec2f texCoord;
	};

	static shared_ptr<ofxPolygonBatch> createWithCapacity (int capacity);

	ofxPolygonBatch();

	virtual ~ofxPolygonBatch();

	bool initWithCapacity (int capacity);
	void add (ofTexture* texture,
		const float* vertices, const float* uvs, int verticesCount,
		const int* triangles, int trianglesCount,
		ofColor color);
	void draw ();

private:
	int capacity;
	PolygonVertex* vertices;
	int verticesCount;
	GLushort* triangles;
	int trianglesCount;
	ofTexture* texture;
};
