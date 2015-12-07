/******************************************************************************
 * Spine Runtimes Software License
 * Version 2.3
 * 
 * Copyright (c) 2013-2015, Esoteric Software
 * All rights reserved.
 * 
 * You are granted a perpetual, non-exclusive, non-sublicensable and
 * non-transferable license to use, install, execute and perform the Spine
 * Runtimes Software (the "Software") and derivative works solely for personal
 * or internal use. Without the written permission of Esoteric Software (see
 * Section 2 of the Spine Software License Agreement), you may not (a) modify,
 * translate, adapt or otherwise create derivative works, improvements of the
 * Software or develop new applications using the Software or (b) remove,
 * delete, alter or obscure any trademarks or any copyright, trademark, patent
 * or other intellectual property or proprietary rights notices on or in the
 * Software, including any copy thereof. Redistributions in binary or source
 * form must include this license and terms.
 * 
 * THIS SOFTWARE IS PROVIDED BY ESOTERIC SOFTWARE "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL ESOTERIC SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#include "ofxSkeletonRenderer.h"
#include <spine/extension.h>
#include <algorithm>

static const int quadTriangles[6] = {0, 1, 2, 2, 3, 0};

shared_ptr<ofxSkeletonRenderer> ofxSkeletonRenderer::createWithData (spSkeletonData* skeletonData, bool ownsSkeletonData) {
	auto node = make_shared<ofxSkeletonRenderer>(skeletonData, ownsSkeletonData);
	return node;
}

shared_ptr<ofxSkeletonRenderer> ofxSkeletonRenderer::createWithFile (const char* skeletonDataFile, spAtlas* atlas, float scale) {
	auto node = make_shared<ofxSkeletonRenderer>(skeletonDataFile, atlas, scale);
	return node;
}

shared_ptr<ofxSkeletonRenderer> ofxSkeletonRenderer::createWithFile (const char* skeletonDataFile, const char* atlasFile, float scale) {
	auto node = make_shared<ofxSkeletonRenderer>(skeletonDataFile, atlasFile, scale);
	return node;
}

void ofxSkeletonRenderer::initialize () {
	worldVertices = MALLOC(float, 1000); // Max number of vertices per mesh.

	batch = ofxPolygonBatch::createWithCapacity(2000); // Max number of vertices and triangles per batch.

	blendFunc.src = GL_SRC_ALPHA;
	blendFunc.dst = GL_ONE_MINUS_SRC_ALPHA;
	setOpacityModifyRGB(true);
}

void ofxSkeletonRenderer::setSkeletonData (spSkeletonData *skeletonData, bool ownsSkeletonData) {
	skeleton = spSkeleton_create(skeletonData);
	skeleton->flipY = true;	// of Flip needed?
	rootBone = skeleton->bones[0];
	this->ownsSkeletonData = ownsSkeletonData;
}

ofxSkeletonRenderer::ofxSkeletonRenderer()
	: atlas(0), debugSlots(false), debugBones(false), timeScale(1) {
	initialize();
}

ofxSkeletonRenderer::ofxSkeletonRenderer(spSkeletonData *skeletonData, bool ownsSkeletonData)
	: atlas(0), debugSlots(false), debugBones(false), timeScale(1) {
	initialize();

	setSkeletonData(skeletonData, ownsSkeletonData);
}

ofxSkeletonRenderer::ofxSkeletonRenderer(const char* skeletonDataFile, spAtlas* atlas, float scale)
	: atlas(0), debugSlots(false), debugBones(false), timeScale(1) {
	initialize();

	spSkeletonJson* json = spSkeletonJson_create(atlas);
	json->scale = scale;
	spSkeletonData* skeletonData = spSkeletonJson_readSkeletonDataFile(json, skeletonDataFile);
	//Assert(skeletonData, json->error ? json->error : "Error reading skeleton data.");
	spSkeletonJson_dispose(json);

	setSkeletonData(skeletonData, true);

	//- TODO
	this->scale = ofVec2f(scale, scale);
}

ofxSkeletonRenderer::ofxSkeletonRenderer(const char* skeletonDataFile, const char* atlasFile, float scale)
	: atlas(0), debugSlots(false), debugBones(false), timeScale(1) {
	initialize();

	atlas = spAtlas_createFromFile(atlasFile, 0);
	//Assert(atlas, "Error reading atlas file.");

	spSkeletonJson* json = spSkeletonJson_create(atlas);
	json->scale = scale;
	spSkeletonData* skeletonData = spSkeletonJson_readSkeletonDataFile(json, skeletonDataFile);
	//Assert(skeletonData, json->error ? json->error : "Error reading skeleton data file.");
	spSkeletonJson_dispose(json);

	setSkeletonData(skeletonData, true);

	int i;
	// Add skins
	for (i = 0; i < skeletonData->skinsCount; ++i) {
		if (strcmp("default", skeletonData->skins[i]->name) != 0) {
			//skins.push_back(skeletonData->skins[i]->name);
			printf("+ Skin %s\n", skeletonData->skins[i]->name);
		}
	}

	// Auto-skin , ignore default [0]
	if (skeletonData->skinsCount > 0) 
		setSkin(skeletonData->skins[1]->name);

	for (i = 0; i < skeletonData->animationsCount; ++i) {
		//animations.push_back(skeletonData->animations[i]->name);
		//durations.push_back(skeletonData->animations[i]->duration);
		printf("+ Animation %s : %f\n", skeletonData->animations[i]->name, skeletonData->animations[i]->duration);
	}
}

ofxSkeletonRenderer::~ofxSkeletonRenderer() {
	if (ownsSkeletonData) spSkeletonData_dispose(skeleton->data);
	if (atlas) spAtlas_dispose(atlas);
	spSkeleton_dispose(skeleton);
	FREE(worldVertices);
	batch.reset();
}

void ofxSkeletonRenderer::update (float deltaTime) {
	spSkeleton_update(skeleton, deltaTime * timeScale);
}

void ofxSkeletonRenderer::draw () {
	//CC_NODE_DRAW_SETUP();
	//ccGLBindVAO(0);

	skeleton->r = color.r / (float)255;
	skeleton->g = color.g / (float)255;
	skeleton->b = color.b / (float)255;
	skeleton->a = color.a / (float)255;

	int blendMode = -1;
	ofColor color;
	const float* uvs = nullptr;
	int verticesCount = 0;
	const int* triangles = nullptr;
	int trianglesCount = 0;
	float r = 0, g = 0, b = 0, a = 0;
	for (int i = 0, n = skeleton->slotsCount; i < n; i++) {
		spSlot* slot = skeleton->drawOrder[i];
		if (!slot->attachment) continue;
		ofTexture *texture = nullptr;
		switch (slot->attachment->type) {
		case SP_ATTACHMENT_REGION: {
			spRegionAttachment* attachment = (spRegionAttachment*)slot->attachment;
			spRegionAttachment_computeWorldVertices(attachment, slot->bone, worldVertices);
			texture = getTexture(attachment);
			uvs = attachment->uvs;
			verticesCount = 8;
			triangles = quadTriangles;
			trianglesCount = 6;
			r = attachment->r;
			g = attachment->g;
			b = attachment->b;
			a = attachment->a;
			break;
		}
		case SP_ATTACHMENT_MESH: {
			spMeshAttachment* attachment = (spMeshAttachment*)slot->attachment;
			spMeshAttachment_computeWorldVertices(attachment, slot, worldVertices);
			texture = getTexture(attachment);
			uvs = attachment->uvs;
			verticesCount = attachment->verticesCount;
			triangles = attachment->triangles;
			trianglesCount = attachment->trianglesCount;
			r = attachment->r;
			g = attachment->g;
			b = attachment->b;
			a = attachment->a;
			break;
		}
		case SP_ATTACHMENT_SKINNED_MESH: {
			spSkinnedMeshAttachment* attachment = (spSkinnedMeshAttachment*)slot->attachment;
			spSkinnedMeshAttachment_computeWorldVertices(attachment, slot, worldVertices);
			texture = getTexture(attachment);
			uvs = attachment->uvs;
			verticesCount = attachment->uvsCount;
			triangles = attachment->triangles;
			trianglesCount = attachment->trianglesCount;
			r = attachment->r;
			g = attachment->g;
			b = attachment->b;
			a = attachment->a;
			break;
		}
		}
		if (texture) {
			if (slot->data->blendMode != blendMode) {
				//batch->flush();
				blendMode = slot->data->blendMode;
				//glEnable(GL_BLEND);
				switch (slot->data->blendMode) {
				case SP_BLEND_MODE_ADDITIVE:
					glBlendFunc(premultipliedAlpha ? GL_ONE : GL_SRC_ALPHA, GL_ONE);
					break;
				case SP_BLEND_MODE_MULTIPLY:
					glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
					break;
				case SP_BLEND_MODE_SCREEN:
					glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);
					break;
				default:
					glBlendFunc(blendFunc.src, blendFunc.dst);
				}
			}
			color.a = skeleton->a * slot->a * a * 255;
			float multiplier = premultipliedAlpha ? color.a : 255;
			color.r = skeleton->r * slot->r * r * multiplier;
			color.g = skeleton->g * slot->g * g * multiplier;
			color.b = skeleton->b * slot->b * b * multiplier;
			batch->add(texture, worldVertices, uvs, verticesCount, triangles, trianglesCount, color);
		}
	}
	batch->draw();

	if (debugSlots) {
		// Slots.
		ofSetColor(0, 0, 255, 255);
		ofSetLineWidth(1);
		ofPolyline poly;
		for (int i = 0, n = skeleton->slotsCount; i < n; i++) {
			spSlot* slot = skeleton->drawOrder[i];
			if (!slot->attachment || slot->attachment->type != SP_ATTACHMENT_REGION) continue;
			spRegionAttachment* attachment = (spRegionAttachment*)slot->attachment;
			spRegionAttachment_computeWorldVertices(attachment, slot->bone, worldVertices);
			poly.addVertex(ofPoint(worldVertices[0], worldVertices[1]));
			poly.addVertex(ofPoint(worldVertices[2], worldVertices[3]));
			poly.addVertex(ofPoint(worldVertices[4], worldVertices[5]));
			poly.addVertex(ofPoint(worldVertices[6], worldVertices[7]));
			poly.draw();
		}
	}
	if (debugBones) {
		// Bone lengths.
		ofSetLineWidth(2);
		ofSetColor(255, 0, 0, 255);
		for (int i = 0, n = skeleton->bonesCount; i < n; i++) {
			spBone *bone = skeleton->bones[i];
			float x = bone->data->length * bone->m00 + bone->worldX;
			float y = bone->data->length * bone->m10 + bone->worldY;
			ofDrawLine(ofVec2f(bone->worldX, bone->worldY), ofVec2f(x, y));
		}
		// Bone origins.
		ofSetColor(0, 0, 255, 255); // Root bone is blue.
		for (int i = 0, n = skeleton->bonesCount; i < n; i++) {
			spBone *bone = skeleton->bones[i];
			ofDrawCircle(ofVec2f(bone->worldX, bone->worldY), 4);
			if (i == 0) ofSetColor(0, 255, 0, 255);
		}
	}
	ofSetColor(255);
}

ofTexture* ofxSkeletonRenderer::getTexture (spRegionAttachment* attachment) const {
	return (ofTexture*)((spAtlasRegion*)attachment->rendererObject)->page->rendererObject;
}

ofTexture* ofxSkeletonRenderer::getTexture (spMeshAttachment* attachment) const {
	return (ofTexture*)((spAtlasRegion*)attachment->rendererObject)->page->rendererObject;
}

ofTexture* ofxSkeletonRenderer::getTexture (spSkinnedMeshAttachment* attachment) const {
	return (ofTexture*)((spAtlasRegion*)attachment->rendererObject)->page->rendererObject;
}

ofRectangle ofxSkeletonRenderer::boundingBox () {
	float minX = FLT_MAX, minY = FLT_MAX, maxX = FLT_MIN, maxY = FLT_MIN;
	float scaleX = scale.x, scaleY = scale.y;
	for (int i = 0; i < skeleton->slotsCount; ++i) {
		spSlot* slot = skeleton->slots[i];
		if (!slot->attachment) continue;
		int verticesCount;
		if (slot->attachment->type == SP_ATTACHMENT_REGION) {
			spRegionAttachment* attachment = (spRegionAttachment*)slot->attachment;
			spRegionAttachment_computeWorldVertices(attachment, slot->bone, worldVertices);
			verticesCount = 8;
		} else if (slot->attachment->type == SP_ATTACHMENT_MESH) {
			spMeshAttachment* mesh = (spMeshAttachment*)slot->attachment;
			spMeshAttachment_computeWorldVertices(mesh, slot, worldVertices);
			verticesCount = mesh->verticesCount;
		} else if (slot->attachment->type == SP_ATTACHMENT_SKINNED_MESH) {
			spSkinnedMeshAttachment* mesh = (spSkinnedMeshAttachment*)slot->attachment;
			spSkinnedMeshAttachment_computeWorldVertices(mesh, slot, worldVertices);
			verticesCount = mesh->uvsCount;
		} else
			continue;
		for (int ii = 0; ii < verticesCount; ii += 2) {
			float x = worldVertices[ii] * scaleX, y = worldVertices[ii + 1] * scaleY;
			minX = min(minX, x);
			minY = min(minY, y);
			maxX = max(maxX, x);
			maxY = max(maxY, y);
		}
	}
	return ofRectangle(position.x + minX, position.y + minY, maxX - minX, maxY - minY);
}

// --- Convenience methods for Skeleton_* functions.

void ofxSkeletonRenderer::updateWorldTransform () {
	spSkeleton_updateWorldTransform(skeleton);
}

void ofxSkeletonRenderer::setToSetupPose () {
	spSkeleton_setToSetupPose(skeleton);
}
void ofxSkeletonRenderer::setBonesToSetupPose () {
	spSkeleton_setBonesToSetupPose(skeleton);
}
void ofxSkeletonRenderer::setSlotsToSetupPose () {
	spSkeleton_setSlotsToSetupPose(skeleton);
}

spBone* ofxSkeletonRenderer::findBone (const char* boneName) const {
	return spSkeleton_findBone(skeleton, boneName);
}

spSlot* ofxSkeletonRenderer::findSlot (const char* slotName) const {
	return spSkeleton_findSlot(skeleton, slotName);
}

bool ofxSkeletonRenderer::setSkin (const char* skinName) {
	return spSkeleton_setSkinByName(skeleton, skinName) ? true : false;
}

spAttachment* ofxSkeletonRenderer::getAttachment (const char* slotName, const char* attachmentName) const {
	return spSkeleton_getAttachmentForSlotName(skeleton, slotName, attachmentName);
}
bool ofxSkeletonRenderer::setAttachment (const char* slotName, const char* attachmentName) {
	return spSkeleton_setAttachment(skeleton, slotName, attachmentName) ? true : false;
}

// --- CCBlendProtocol

void ofxSkeletonRenderer::setBlendFunc(GLuint src, GLuint dst) 
{
	blendFunc.src = src;
	blendFunc.dst = dst;
}

void ofxSkeletonRenderer::setOpacityModifyRGB (bool value) {
	premultipliedAlpha = value;
}

bool ofxSkeletonRenderer::isOpacityModifyRGB () {
	return premultipliedAlpha;
}
