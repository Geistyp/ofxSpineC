//- GeistYp
#pragma once

#include <spine/spine.h>
#include "ofMain.h"
#include "ofxSkeletonRenderer.h"

namespace spine {
	typedef std::function<void(int trackIndex)> StartListener;
	typedef std::function<void(int trackIndex)> EndListener;
	typedef std::function<void(int trackIndex, int loopCount)> CompleteListener;
	typedef std::function<void(int trackIndex, spEvent* event)> EventListener;
}


/** Draws an animated skeleton, providing an AnimationState for applying one or more animations and queuing animations to be
  * played later. */
class ofxSkeletonAnimation: public ofxSkeletonRenderer {
public:
	spAnimationState* state;

	static shared_ptr<ofxSkeletonAnimation> createWithData (spSkeletonData* skeletonData);
	static shared_ptr<ofxSkeletonAnimation> createWithFile (const char* skeletonDataFile, spAtlas* atlas, float scale = 0);
	static shared_ptr<ofxSkeletonAnimation> createWithFile (const char* skeletonDataFile, const char* atlasFile, float scale = 0);

	ofxSkeletonAnimation(spSkeletonData* skeletonData);
	ofxSkeletonAnimation(const char* skeletonDataFile, spAtlas* atlas, float scale = 0);
	ofxSkeletonAnimation(const char* skeletonDataFile, const char* atlasFile, float scale = 0);

	virtual ~ofxSkeletonAnimation();

	virtual void update (float deltaTime);

	void setAnimationStateData (spAnimationStateData* stateData);
	void setMix (const char* fromAnimation, const char* toAnimation, float duration);

	spTrackEntry* setAnimation (int trackIndex, const char* name, bool loop);
	spTrackEntry* addAnimation (int trackIndex, const char* name, bool loop, float delay = 0);
	spTrackEntry* getCurrent (int trackIndex = 0);
	void clearTracks ();
	void clearTrack (int trackIndex = 0);

	spine::StartListener startListener;
	spine::EndListener endListener;
	spine::CompleteListener completeListener;
	spine::EventListener eventListener;
	void setStartListener (spTrackEntry* entry, spine::StartListener listener);
	void setEndListener (spTrackEntry* entry, spine::EndListener listener);
	void setCompleteListener (spTrackEntry* entry, spine::CompleteListener listener);
	void setEventListener (spTrackEntry* entry, spine::EventListener listener);

	virtual void onAnimationStateEvent (int trackIndex, spEventType type, spEvent* event, int loopCount);
	virtual void onTrackEntryEvent (int trackIndex, spEventType type, spEvent* event, int loopCount);

protected:
	ofxSkeletonAnimation() {}

private:
	typedef ofxSkeletonRenderer super;
	bool ownsAnimationStateData;

	void initialize ();
};

