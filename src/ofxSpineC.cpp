#include "ofxSpineC.h"

#include <spine/extension.h>
#include "ofMain.h"

void _spAtlasPage_createTexture(spAtlasPage* self, const char* path) {
	
	ofTexture * texture = new ofTexture();

	// load image
	ofImage loader;
	loader.setUseTexture(false);
	loader.loadImage(path);

	// store width & height
	int imageWidth = loader.getWidth();
	int imageHeight = loader.getHeight();

	// allocate texture & copy in data
	texture->allocate(imageWidth, imageHeight, GL_RGBA);
	texture->loadData(loader.getPixels(), imageWidth, imageHeight, GL_RGBA);

	// clear temp image
	loader.clear();

	self->rendererObject = texture;
	self->width = imageWidth;
	self->height = imageHeight;
}

void _spAtlasPage_disposeTexture(spAtlasPage* self) {
	delete (ofTexture *)self->rendererObject;
}

char* _spUtil_readFile(const char* path, int* length) {
	return _readFile(ofToDataPath(path).c_str(), length);
}