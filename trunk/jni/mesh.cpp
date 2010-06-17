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

#include "mesh.h"
#include <android/log.h>
#define  LOG_TAG    "libspinningcube"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

cMesh::cMesh()
{
	m_numVertices = 0;	
	m_pVertexPosition = NULL;
	m_pTriangleList = NULL;
	m_pVertexNormal = NULL;
	m_pVertexColor = NULL;
	m_pVertexTexCoord0 = NULL;

	m_numFaces = 0;	
	m_pU16FacesList = NULL;
	m_pU32FacesList = NULL;
	m_face_type = 1;
	m_pNormalFaceList = NULL;

	m_vbo_position = 0;
	m_vbo_normal = 0;
	m_ibo = 0;
	m_update_vbo_position = 1;
	m_update_vbo_normal = 1;
	m_update_ibo = 1;

}


cMesh::~cMesh()
{
	freeMeshData();
}


void cMesh::freeMeshData()
{
	if(m_numVertices>0)
	{
		SAFE_DELETE_ARRAY(m_pVertexPosition);
		SAFE_DELETE_ARRAY(m_pVertexNormal);
		SAFE_DELETE_ARRAY(m_pVertexColor);
		SAFE_DELETE_ARRAY(m_pVertexTexCoord0);
		m_numVertices = 0;
	}

	if(m_numFaces>0)
	{
		SAFE_DELETE_ARRAY(m_pU16FacesList);
		SAFE_DELETE_ARRAY(m_pU32FacesList);
		SAFE_DELETE_ARRAY(m_pNormalFaceList);
		m_numFaces = 0;
	}
	if(m_numTriangles > 0)
	{
		SAFE_DELETE_ARRAY(m_pTriangleList);
	}
}


void cMesh::allocVertexData( unsigned long num_vertices )
{
	m_numVertices = num_vertices;	
		
	m_pVertexPosition = new cVec4[num_vertices];
	m_pVertexNormal = new cVec4[num_vertices];
	m_pVertexColor = new cVec4[num_vertices];
	m_pVertexTexCoord0 = new cVec4[num_vertices];
}


void cMesh::allocFaceData( int num_faces )
{
	m_numFaces = num_faces;	

	if( num_faces < (1<<16) )
	{
		m_pU16FacesList = new sFaceU16[num_faces];
		m_face_type = 1;
	}
	else
	{
		m_pU32FacesList = new sFaceU32[num_faces];
		m_face_type = 2;
	}
		
	m_pNormalFaceList = new cVec4[num_faces];

}

void cMesh::ComputeTriangleData()
{
	m_numTriangles = m_numFaces * 3;
	m_pTriangleList = new cVec4[m_numTriangles];
	for( int i=0; i < m_numFaces; ++i )
	{
		if( m_face_type==2 )
		{		
			m_pTriangleList[i*3] = m_pVertexPosition[m_pU32FacesList[i].a];
			m_pTriangleList[i*3 + 1] = m_pVertexPosition[m_pU32FacesList[i].b];
			m_pTriangleList[i*3 + 2] = m_pVertexPosition[m_pU32FacesList[i].c];
		}
		else
		{
			m_pTriangleList[i*3] = m_pVertexPosition[m_pU16FacesList[i].a];
			m_pTriangleList[i*3 + 1] = m_pVertexPosition[m_pU16FacesList[i].b];
			m_pTriangleList[i*3 + 2] = m_pVertexPosition[m_pU16FacesList[i].c];
		}

	}
}


void cMesh::buildPlane( float width, float height, int wsegs, int hsegs )
{
	if( wsegs<1 ) wsegs=1;
	if( hsegs<1 ) hsegs=1;

	freeMeshData(); 

	allocVertexData( (wsegs+1)*(hsegs+1) );
	allocFaceData( 2*wsegs*hsegs );
	
	int v = 0;
	int x=0, z=0;
	int tu = 0;

	for( z=0; z<hsegs+1; z++ )
	{
		for( x=0; x<wsegs+1; x++ )
		{
			m_pVertexPosition[v].x = -(width/2.0f)+(float)x*(width/(float)wsegs);
			m_pVertexPosition[v].y = 0.0f;
			m_pVertexPosition[v].z = -(height/2.0f)+(float)z*(height/(float)hsegs);

			m_pVertexNormal[v].set(0.0, 1.0, 0.0, 0.0);
			
			m_pVertexTexCoord0[v].x = x * (1.0f/wsegs);
			m_pVertexTexCoord0[v].y = (hsegs-z) * (1.0f/hsegs);
			
			v++;
		}
	}


	int f=0, i=0, j=0;

	if( m_face_type==2 )
	{
		for( z=0; z<hsegs; z++ )
		{
			for (int x=0; x<wsegs; x++)
			{
				i=6*(z*wsegs+x);
				j=z*(wsegs+1)+x;
			
				m_pU32FacesList[f].a = j;
				m_pU32FacesList[f].b = j+(wsegs+1);
				m_pU32FacesList[f].c = j+1;
				f++;

				m_pU32FacesList[f].a = j+1;
				m_pU32FacesList[f].b = j+(wsegs+1);
				m_pU32FacesList[f].c = j+(wsegs+2);
				f++;
			}
		}
	}
	else
	{
		for( z=0; z<hsegs; z++ )
		{
			for (int x=0; x<wsegs; x++)
			{
				i=6*(z*wsegs+x);
				j=z*(wsegs+1)+x;
			
				m_pU16FacesList[f].a = j;
				m_pU16FacesList[f].b = j+(wsegs+1);
				m_pU16FacesList[f].c = j+1;
				f++;

				m_pU16FacesList[f].a = j+1;
				m_pU16FacesList[f].b = j+(wsegs+1);
				m_pU16FacesList[f].c = j+(wsegs+2);
				f++;
			}
		}
	}

	ComputeTriangleData();
	setVerticesColor( 1.0f, 1.0f, 1.0f );
}

void cMesh::setVerticesColor( float r, float g, float b )
{
		
	cVec4 color;
	color.set( r,g,b ); 	
	for( unsigned long i=0; i<m_numVertices; i++ )		
	{
		m_pVertexColor[i] = color;
	}
}

void cMesh::computeVerticesNormals()
{
	if( m_numVertices==0 || !m_pNormalFaceList ) 
		return;


	int *sharedArray = new int[m_numVertices];

	unsigned long a, b, c;
	cVec4 side0, side1;
	cVec4 vNormal;
	unsigned long i, f;

	for( i=0; i<m_numVertices; i++ )		
	{
		m_pVertexNormal[i].set( 0.0, 0.0, 0.0, 0.0 );
		sharedArray[i] = 0;
	}

	for( f=0; f<m_numFaces; f++ )
	{
		vNormal.set( 0.0, 0.0, 0.0, 0.0 );

		if( m_face_type==2 )
		{
			a = m_pU32FacesList[f].a;
			b = m_pU32FacesList[f].b;
			c = m_pU32FacesList[f].c;
		}
		else
		{
			a = m_pU16FacesList[f].a;
			b = m_pU16FacesList[f].b;
			c = m_pU16FacesList[f].c;
		}


		if( a>=0 && a<m_numVertices && 
			b>=0 && b<m_numVertices && 
			c>=0 && c<m_numVertices )
		{
			side0 = m_pVertexPosition[a] - m_pVertexPosition[b];
			side1 = m_pVertexPosition[c] - m_pVertexPosition[b];

			vNormal = side1 ^ side0;
			vNormal.normalize();
			
			m_pNormalFaceList[f] = vNormal;

			m_pVertexNormal[a] += vNormal; 
			sharedArray[a]++;

			m_pVertexNormal[b] += vNormal; 
			sharedArray[b]++;

			m_pVertexNormal[c] += vNormal; 
			sharedArray[c]++;
		}
	}

	for( i=0; i<m_numVertices; i++ )		
	{
		m_pVertexNormal[i].scalarMult( 1.0f/(float)sharedArray[i] );
		m_pVertexNormal[i].w = 0.0;
		m_pVertexNormal[i].normalize();
	}

	SAFE_DELETE_ARRAY( sharedArray );
};
