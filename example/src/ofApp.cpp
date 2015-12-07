#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
	//ofDisableArbTex();
	skel_render = ofxSkeletonAnimation::createWithFile("spineboy.json", "spineboy.atlas", 0.6);		// scale = -1 to oF coordinate
	//skel_render = ofxSkeletonAnimation::createWithFile("goblins/goblins.json", "goblins/goblins.atlas", 0.6);		// scale = -1 to oF coordinate

	skel_render->startListener = [this](int trackIndex) {
		spTrackEntry* entry = spAnimationState_getCurrent(skel_render->state, trackIndex);
		const char* animationName = (entry && entry->animation) ? entry->animation->name : 0;
		printf("%d start: %s\n", trackIndex, animationName);
	};
	//skel_render->endListener = endTracker; //void endTracker(int trackIndex)
	//skel_render->endListener = std::bind(&ofApp::endTracker, this, placeholders::_1);
	skel_render->endListener = [](int trackIndex) {
		printf("%d end\n", trackIndex);
	};
	skel_render->completeListener = [](int trackIndex, int loopCount) {
		printf("%d complete: %d\n", trackIndex, loopCount);
	};
	skel_render->eventListener = [](int trackIndex, spEvent* event) {
		printf("%d event: %s, %d, %f, %s\n", trackIndex, event->data->name, event->intValue, event->floatValue, event->stringValue);
	};

	//skel_render->setSkin("goblingirl");

	skel_render->setMix("walk", "jump", 0.2f);
	skel_render->setMix("jump", "run", 0.2f);
	skel_render->setMix("run", "death", 0.2f);
	skel_render->setAnimation(0, "walk", true);
	spTrackEntry* jumpEntry = skel_render->addAnimation(0, "jump", false, 3);
	skel_render->addAnimation(0, "run", true);

	if (jumpEntry)
	{
		skel_render->setStartListener(jumpEntry, [](int trackIndex) {
			printf("jumped!\n");
		});
	}
	
	//skel_render->setAnimation(0, "walk", true);

	skel_render->debugBones = true;
	skel_render->debugSlots = true;


}

//--------------------------------------------------------------
void ofApp::update(){
	skel_render->update(ofGetLastFrameTime());
}

//--------------------------------------------------------------
void ofApp::draw(){
	ofPushMatrix();
	ofTranslate(512, 376);
	skel_render->draw();
	ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	if (key == ' ')
	{
		skel_render->addAnimation(0, "jump", false, 2.3);
		skel_render->addAnimation(0, "run", true);
	}
	if (key == 'd')
	{
		skel_render->addAnimation(0, "death", false);
	}
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
