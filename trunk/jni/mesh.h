//
// ==[ XPGL: eXPerimental Graphics Library ]== 
//
// Copyright 2006 JeGX / oZone3D.Net
// http://www.oZone3D.Net - jegx@ozone3d.net
//
// This SOFTWARE is distributed in the hope that it will be useful.
// TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
// *AS IS* AND oZone3D.Net DISCLAIM ALL WARRANTIES, EITHER EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL oZone3D.Net 
// BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
// WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
// BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
// ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF oZone3D.Net HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES 
//


#ifndef _MESH_H
#define _MESH_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "vec4.h"


struct sFaceU16
{
	unsigned short a;
	unsigned short b;
	unsigned short c;
};

struct sFaceU32
{
	unsigned long a;
	unsigned long b;
	unsigned long c;
};



struct sRenderMeshParams
{
	int use_normals;
	int use_texture[8];
	int use_color;
	unsigned long vbo_mode;
	unsigned long ibo_mode;

};

#define VBO_BUFFER_OFFSET(i) ((char *)NULL + (i))



//===================================================================
/**
cMesh: a simple triangular mesh with the same material for all faces.
*/
//===================================================================
class cMesh
{

public:

	cMesh();
	~cMesh();


	void freeMeshData();
	void allocVertexData( unsigned long num_vertices );
	void allocFaceData( int num_faces );
	void ComputeTriangleData();
	void ComputeQuadData();

	void setVerticesColor();
	void computeVerticesNormals();

	//void buildTorus( float radius, float section_radius, int vertex_density );
	void buildPlane( float width, float height, int wsegs, int hsegs );
	//void buildBox( float w, float h, float d, int wsegs, int hsegs, int dsegs );


	//void renderVertexArray( sRenderMeshParams *p );
	//void renderVertexBuffer( sRenderMeshParams *p );



public:


	cVec4 *m_pVertexPosition;
	cVec4 *m_pTriangleList;
	cVec4 *m_pVertexNormal;
	cVec4 *m_pVertexColor;
	cVec4 *m_pVertexTexCoord0;
	unsigned long m_numVertices;


	sFaceU16 *m_pU16FacesList;
	sFaceU32 *m_pU32FacesList;
	cVec4 *m_pNormalFaceList;
	unsigned long m_numFaces;
	unsigned long m_numTriangles;
	int m_face_type; // 1 for u16 and 2 for unsigned long.

	unsigned long m_vbo_position;
	unsigned long m_vbo_normal;
	unsigned long m_ibo;
	int m_update_vbo_position;
	int m_update_vbo_normal;
	int m_update_ibo;

};



#endif
