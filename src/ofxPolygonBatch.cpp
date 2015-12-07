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

#include "ofxPolygonBatch.h"


shared_ptr<ofxPolygonBatch> ofxPolygonBatch::createWithCapacity (int capacity) {
	auto batch = make_shared<ofxPolygonBatch>();
	batch->initWithCapacity(capacity);
	return batch;
}

ofxPolygonBatch::ofxPolygonBatch() :
	capacity(0), 
	vertices(nullptr), verticesCount(0),
	triangles(nullptr), trianglesCount(0),
	texture(nullptr)
{}

bool ofxPolygonBatch::initWithCapacity (int capacity) {
	// 32767 is max index, so 32767 / 3 - (32767 / 3 % 3) = 10920.
	//Assert(capacity <= 10920, "capacity cannot be > 10920");
	//Assert(capacity >= 0, "capacity cannot be < 0");
	this->capacity = capacity;
	vertices = new PolygonVertex[capacity];
	triangles = new GLushort[capacity * 3];
	return true;
}

ofxPolygonBatch::~ofxPolygonBatch() {
	delete vertices;
	delete triangles;
}

void ofxPolygonBatch::add (ofTexture* addTexture,
		const float* addVertices, const float* uvs, int addVerticesCount,
		const int* addTriangles, int addTrianglesCount,
		ofColor color) 
{

	if (
		addTexture != texture
		|| verticesCount + (addVerticesCount >> 1) > capacity
		|| trianglesCount + addTrianglesCount > capacity * 3) {
		//this->flush();
		texture = addTexture;
	}
	
	for (int i = 0; i < addTrianglesCount; ++i, ++trianglesCount)
		triangles[trianglesCount] = addTriangles[i] + verticesCount;

	for (int i = 0; i < addVerticesCount; i += 2, ++verticesCount) {
		PolygonVertex* vertex = vertices + verticesCount;
		vertex->vertex.x = addVertices[i];
		vertex->vertex.y = addVertices[i + 1];
		vertex->color = color;
		// add texture type
		vertex->texCoord.x = uvs[i] * (texture->texData.textureTarget == GL_TEXTURE_2D? 1.0 : texture->texData.width);
		vertex->texCoord.y = uvs[i + 1] * (texture->texData.textureTarget == GL_TEXTURE_2D ? 1.0 : texture->texData.height);
	}
}

void ofxPolygonBatch::draw () {
	if (!verticesCount) return;

	texture->bind();
	/*
	glEnableVertexAttribArray(kCCVertexAttrib_Position);
	glEnableVertexAttribArray(kCCVertexAttrib_Color);
	glEnableVertexAttribArray(kCCVertexAttrib_TexCoords);
	glVertexAttribPointer(kCCVertexAttrib_Position, 2, GL_FLOAT, GL_FALSE, sizeof(ccV2F_C4B_T2F), &vertices[0].vertices);
	glVertexAttribPointer(kCCVertexAttrib_Color, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ccV2F_C4B_T2F), &vertices[0].colors);
	glVertexAttribPointer(kCCVertexAttrib_TexCoords, 2, GL_FLOAT, GL_FALSE, sizeof(ccV2F_C4B_T2F), &vertices[0].texCoords);
	*/
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(2, GL_FLOAT, sizeof(PolygonVertex), vertices[0].vertex.getPtr());
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(PolygonVertex), &vertices[0].color);
	glTexCoordPointer(2, GL_FLOAT, sizeof(PolygonVertex), vertices[0].texCoord.getPtr());
	glDrawElements(GL_TRIANGLES, trianglesCount, GL_UNSIGNED_SHORT, triangles);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	texture->unbind();

	verticesCount = 0;
	trianglesCount = 0;
}
