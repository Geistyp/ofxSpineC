#include "ofxSkeletonAnimation.h"

#include <spine/extension.h>
#include <algorithm>

void animationCallback (spAnimationState* state, int trackIndex, spEventType type, spEvent* event, int loopCount) {
	((ofxSkeletonAnimation*)state->rendererObject)->onAnimationStateEvent(trackIndex, type, event, loopCount);
}

void trackEntryCallback (spAnimationState* state, int trackIndex, spEventType type, spEvent* event, int loopCount) {
	((ofxSkeletonAnimation*)state->rendererObject)->onTrackEntryEvent(trackIndex, type, event, loopCount);
}

typedef struct _TrackEntryListeners {
	spine::StartListener startListener;
	spine::EndListener endListener;
	spine::CompleteListener completeListener;
	spine::EventListener eventListener;
} _TrackEntryListeners;

static _TrackEntryListeners* getListeners (spTrackEntry* entry) {
	if (!entry->rendererObject) {
		entry->rendererObject = new _TrackEntryListeners;
		entry->listener = trackEntryCallback;
	}
	return (_TrackEntryListeners*)entry->rendererObject;
}

void disposeTrackEntry (spTrackEntry* entry) {
	if (entry->rendererObject) delete entry->rendererObject;
	_spTrackEntry_dispose(entry);
}

//

shared_ptr<ofxSkeletonAnimation> ofxSkeletonAnimation::createWithData (spSkeletonData* skeletonData) {
	shared_ptr<ofxSkeletonAnimation> node = make_shared<ofxSkeletonAnimation>(skeletonData);
	return node;
}

shared_ptr<ofxSkeletonAnimation> ofxSkeletonAnimation::createWithFile (const char* skeletonDataFile, spAtlas* atlas, float scale) {
	shared_ptr<ofxSkeletonAnimation> node = make_shared<ofxSkeletonAnimation>(skeletonDataFile, atlas, scale);
	return node;
}

shared_ptr<ofxSkeletonAnimation> ofxSkeletonAnimation::createWithFile (const char* skeletonDataFile, const char* atlasFile, float scale) {
	shared_ptr<ofxSkeletonAnimation> node = make_shared<ofxSkeletonAnimation>(skeletonDataFile, atlasFile, scale);
	return node;
}

void ofxSkeletonAnimation::initialize () {
	ownsAnimationStateData = true;
	state = spAnimationState_create(spAnimationStateData_create(skeleton->data));
	state->rendererObject = this;
	state->listener = animationCallback;

	_spAnimationState* stateInternal = (_spAnimationState*)state;
	stateInternal->disposeTrackEntry = disposeTrackEntry;
}

ofxSkeletonAnimation::ofxSkeletonAnimation(spSkeletonData *skeletonData)
		: ofxSkeletonRenderer(skeletonData) {
	initialize();
}

ofxSkeletonAnimation::ofxSkeletonAnimation(const char* skeletonDataFile, spAtlas* atlas, float scale)
		: ofxSkeletonRenderer(skeletonDataFile, atlas, scale) {
	initialize();
}

ofxSkeletonAnimation::ofxSkeletonAnimation(const char* skeletonDataFile, const char* atlasFile, float scale)
		: ofxSkeletonRenderer(skeletonDataFile, atlasFile, scale) {
	initialize();
}

ofxSkeletonAnimation::~ofxSkeletonAnimation() {
	if (ownsAnimationStateData) spAnimationStateData_dispose(state->data);
	spAnimationState_dispose(state);
}

void ofxSkeletonAnimation::update (float deltaTime) {
	super::update(deltaTime);

	deltaTime *= timeScale;
	spAnimationState_update(state, deltaTime);
	spAnimationState_apply(state, skeleton);
	spSkeleton_updateWorldTransform(skeleton);
}

void ofxSkeletonAnimation::setAnimationStateData (spAnimationStateData* stateData) {
	//(stateData, "stateData cannot be null.");

	if (ownsAnimationStateData) spAnimationStateData_dispose(state->data);
	spAnimationState_dispose(state);

	ownsAnimationStateData = false;
	state = spAnimationState_create(stateData);
	state->rendererObject = this;
	state->listener = animationCallback;
}

void ofxSkeletonAnimation::setMix (const char* fromAnimation, const char* toAnimation, float duration) {
	spAnimationStateData_setMixByName(state->data, fromAnimation, toAnimation, duration);
}

spTrackEntry* ofxSkeletonAnimation::setAnimation (int trackIndex, const char* name, bool loop) {
	spAnimation* animation = spSkeletonData_findAnimation(skeleton->data, name);
	if (!animation) {
		ofLogError("Spine: Animation not found: %s", name);
		return 0;
	}
	return spAnimationState_setAnimation(state, trackIndex, animation, loop);
}

spTrackEntry* ofxSkeletonAnimation::addAnimation (int trackIndex, const char* name, bool loop, float delay) {
	spAnimation* animation = spSkeletonData_findAnimation(skeleton->data, name);
	if (!animation) {
		ofLogError("Spine: Animation not found: %s", name);
		return 0;
	}
	return spAnimationState_addAnimation(state, trackIndex, animation, loop, delay);
}

spTrackEntry* ofxSkeletonAnimation::getCurrent (int trackIndex) {
	return spAnimationState_getCurrent(state, trackIndex);
}

void ofxSkeletonAnimation::clearTracks () {
	spAnimationState_clearTracks(state);
}

void ofxSkeletonAnimation::clearTrack (int trackIndex) {
	spAnimationState_clearTrack(state, trackIndex);
}

void ofxSkeletonAnimation::onAnimationStateEvent (int trackIndex, spEventType type, spEvent* event, int loopCount) {
	switch (type) {
	case SP_ANIMATION_START:
		if (startListener) startListener(trackIndex);
		break;
	case SP_ANIMATION_END:
		if (endListener) endListener(trackIndex);
		break;
	case SP_ANIMATION_COMPLETE:
		if (completeListener) completeListener(trackIndex, loopCount);
		break;
	case SP_ANIMATION_EVENT:
		if (eventListener) eventListener(trackIndex, event);
		break;
	}
}

void ofxSkeletonAnimation::onTrackEntryEvent (int trackIndex, spEventType type, spEvent* event, int loopCount) {
	spTrackEntry* entry = spAnimationState_getCurrent(state, trackIndex);
	if (!entry->rendererObject) return;
	_TrackEntryListeners* listeners = (_TrackEntryListeners*)entry->rendererObject;
	switch (type) {
	case SP_ANIMATION_START:
		if (listeners->startListener) listeners->startListener(trackIndex);
		break;
	case SP_ANIMATION_END:
		if (listeners->endListener) listeners->endListener(trackIndex);
		break;
	case SP_ANIMATION_COMPLETE:
		if (listeners->completeListener) listeners->completeListener(trackIndex, loopCount);
		break;
	case SP_ANIMATION_EVENT:
		if (listeners->eventListener) listeners->eventListener(trackIndex, event);
		break;
	}
}

void ofxSkeletonAnimation::setStartListener (spTrackEntry* entry, spine::StartListener listener) {
	getListeners(entry)->startListener = listener;
}

void ofxSkeletonAnimation::setEndListener (spTrackEntry* entry, spine::EndListener listener) {
	getListeners(entry)->endListener = listener;
}

void ofxSkeletonAnimation::setCompleteListener (spTrackEntry* entry, spine::CompleteListener listener) {
	getListeners(entry)->completeListener = listener;
}

void ofxSkeletonAnimation::setEventListener (spTrackEntry* entry, spine::EventListener listener) {
	getListeners(entry)->eventListener = listener;
}

