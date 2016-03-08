//- GeistYp
#pragma once

#include <spine/spine.h>
#include "ofMain.h"
#include "ofxPolygonBatch.h"

/** Draws a skeleton. */
class ofxSkeletonRenderer
{
public:

	struct SkelBlendFunc
	{
		GLuint src;
		GLuint dst;
	};

	spSkeleton* skeleton;
	spBone* rootBone;
	float timeScale;
	bool debugSlots;
	bool debugBones;
	bool premultipliedAlpha;

	static shared_ptr<ofxSkeletonRenderer> createWithData (spSkeletonData* skeletonData, bool ownsSkeletonData = false);
	static shared_ptr<ofxSkeletonRenderer> createWithFile (const char* skeletonDataFile, spAtlas* atlas, float scale = 0);
	static shared_ptr<ofxSkeletonRenderer> createWithFile (const char* skeletonDataFile, const char* atlasFile, float scale = 0);

	ofxSkeletonRenderer(spSkeletonData* skeletonData, bool ownsSkeletonData = false);
	ofxSkeletonRenderer(const char* skeletonDataFile, spAtlas* atlas, float scale = 0);
	ofxSkeletonRenderer(const char* skeletonDataFile, const char* atlasFile, float scale = 0);

	virtual ~ofxSkeletonRenderer();

	virtual void update (float deltaTime);
	virtual void draw ();
	virtual ofRectangle boundingBox();

	// --- Convenience methods for common Skeleton_* functions.
	void updateWorldTransform ();

	void setToSetupPose ();
	void setBonesToSetupPose ();
	void setSlotsToSetupPose ();

	/* Returns 0 if the bone was not found. */
	spBone* findBone (const char* boneName) const;
	/* Returns 0 if the slot was not found. */
	spSlot* findSlot (const char* slotName) const;
	
	/* Sets the skin used to look up attachments not found in the SkeletonData defaultSkin. Attachments from the new skin are
	 * attached if the corresponding attachment from the old skin was attached. If there was no old skin, each slot's setup mode
	 * attachment is attached from the new skin. Returns false if the skin was not found.
	 * @param skin May be 0.*/
	bool setSkin (const char* skinName);

	/* Returns 0 if the slot or attachment was not found. */
	spAttachment* getAttachment (const char* slotName, const char* attachmentName) const;
	/* Returns false if the slot or attachment was not found. */
	bool setAttachment (const char* slotName, const char* attachmentName);

	// --- BlendProtocol
	SkelBlendFunc blendFunc;
	void setBlendFunc(GLuint src, GLuint dst);
	virtual void setOpacityModifyRGB (bool value);
	virtual bool isOpacityModifyRGB ();

	ofVec2f getPosition() {
		return ofVec2f(skeleton->x, skeleton->y);
	}

	ofVec2f getWorldPosition() {
		return ofVec2f(rootBone->worldX, rootBone->worldY);
	}

	void setPoision(ofVec2f v) {
		position = v; 
		skeleton->x = position.x;
		skeleton->y = position.y;
		updateWorldTransform();
	}
	void setColor(ofColor c) { color = c; }
	//void setScale(ofVec2f s) { scale = s; }

protected:
	ofxSkeletonRenderer();
	void setSkeletonData (spSkeletonData* skeletonData, bool ownsSkeletonData);

	virtual ofTexture* getTexture (spRegionAttachment* attachment) const;
	virtual ofTexture* getTexture (spMeshAttachment* attachment) const;
	virtual ofTexture* getTexture (spWeightedMeshAttachment* attachment) const;

private:
	bool ownsSkeletonData;
	spAtlas* atlas;
	float* worldVertices;
	void initialize ();

	shared_ptr<ofxPolygonBatch> batch;
	ofVec2f position;
	ofColor color;
	ofVec2f scale;
};

