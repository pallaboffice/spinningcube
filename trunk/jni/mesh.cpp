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
			//LOGI("grabbing for i=%d and indices=%d,%d,%d", i,m_pU16FacesList[i].a,m_pU16FacesList[i].b,m_pU16FacesList[i].c );
		}

	}
}

/*void cMesh::buildTorus( float radius, float section_radius, int vertex_density )
{
	int i=0, tu=0;
	int ring = 0;

	freeMeshData(); 

	allocVertexData( (vertex_density+1)*(vertex_density+1) );
	allocFaceData( 2*vertex_density*vertex_density );


	cVec4 vPos1, vPos2;

	//calculate the first ring - inner radius, outer radius
	for( i=0; i<vertex_density+1; i++ )
	{
		vPos1.set( section_radius, 0.0, 0.0 );
		vPos2.set( radius, 0.0, 0.0 );

		vPos1.rotateZ( (float)i/(float)vertex_density * 360.0f );
		vPos1 += vPos2;
		
		m_pVertexPosition[i] = vPos1;

		m_pVertexTexCoord0[i].x = 0.0;
		m_pVertexTexCoord0[i].y = (float)i/(float)vertex_density;
	}

	
	cVec4 vTmp;

	//rotate this to get other rings
	for( ring=1; ring<vertex_density+1; ring++ )
	{
		for( i=0; i<vertex_density+1; i++)
		{
			vTmp = m_pVertexPosition[i];
			vTmp.rotateY( (float)ring/(float)vertex_density * 360.0f );

			m_pVertexPosition[ring*(vertex_density+1)+i] = vTmp;

			m_pVertexTexCoord0[ring*(vertex_density+1)+i].x = 2.0f*(float)ring/(float)vertex_density;
			m_pVertexTexCoord0[ring*(vertex_density+1)+i].y = m_pVertexTexCoord0[i].y;
		}
	}


	//calculate the indices
	if( m_face_type==2 )
	{
		for( ring=0; ring<vertex_density; ring++ )
		{
			for(i=0; i<vertex_density; i++)
			{
				m_pU32FacesList[((ring*vertex_density+i)*2)].a = ring*(vertex_density+1)+i;
				m_pU32FacesList[((ring*vertex_density+i)*2)].b = (ring+1)*(vertex_density+1)+i;
				m_pU32FacesList[((ring*vertex_density+i)*2)].c = ring*(vertex_density+1)+i+1;
				
				m_pU32FacesList[((ring*vertex_density+i)*2+1)].a = ring*(vertex_density+1)+i+1;
				m_pU32FacesList[((ring*vertex_density+i)*2+1)].b = (ring+1)*(vertex_density+1)+i;
				m_pU32FacesList[((ring*vertex_density+i)*2+1)].c = (ring+1)*(vertex_density+1)+i+1;
			}
		}
	}
	else
	{
		for( ring=0; ring<vertex_density; ring++ )
		{
			for(i=0; i<vertex_density; i++)
			{
				m_pU16FacesList[((ring*vertex_density+i)*2)].a = ring*(vertex_density+1)+i;
				m_pU16FacesList[((ring*vertex_density+i)*2)].b = (ring+1)*(vertex_density+1)+i;
				m_pU16FacesList[((ring*vertex_density+i)*2)].c = ring*(vertex_density+1)+i+1;
				
				m_pU16FacesList[((ring*vertex_density+i)*2+1)].a = ring*(vertex_density+1)+i+1;
				m_pU16FacesList[((ring*vertex_density+i)*2+1)].b = (ring+1)*(vertex_density+1)+i;
				m_pU16FacesList[((ring*vertex_density+i)*2+1)].c = (ring+1)*(vertex_density+1)+i+1;
			}
		}
	}


	setVerticesColor( 1.0f, 1.0f, 1.0 );
}*/


void cMesh::buildPlane( float width, float height, int wsegs, int hsegs )
{
	if( wsegs<1 ) wsegs=1;
	if( hsegs<1 ) hsegs=1;

	freeMeshData(); 

	allocVertexData( (wsegs+1)*(hsegs+1) );
	allocFaceData( 2*wsegs*hsegs );
	
	LOGI(">>>>>>>>>>>>>>>>>>Alloced=========================");

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


/*void cMesh::buildBox( float w, float h, float d, int wsegs, int hsegs, int dsegs )
{
	freeMeshData(); 

	if( wsegs<1 ) wsegs=1;
	if( hsegs<1 ) hsegs=1;
	if( dsegs<1 ) dsegs=1;


    int n = 0; 
    int m = 0;
	
	int x_segs=wsegs, y_segs=hsegs, z_segs=dsegs;
	int nTriangles[6];

	nTriangles[0] = (z_segs) * (y_segs) * 2; // +X
	nTriangles[1] = (z_segs) * (y_segs) * 2; // -X
	nTriangles[2] = (x_segs) * (z_segs) * 2; // +Y
	nTriangles[3] = (x_segs) * (z_segs) * 2; // -Y
	nTriangles[4] = (x_segs) * (y_segs) * 2; // +Z
	nTriangles[5] = (x_segs) * (y_segs) * 2; // -Z

    int numSides = 6;

	int nfaces = nTriangles[0] + nTriangles[1] + nTriangles[2] + nTriangles[3] + nTriangles[4] + nTriangles[5];
	int nverts = nfaces * 3;

	allocVertexData( nfaces *3 );
	allocFaceData( nfaces );


	float color = 1.0f;
  	int v=0;
	int x=0, z=0;
	int tu=0;

	float width = 0.0f;
	float width2 = 0.0f;
	float height = 0.0f;
	float height2 = 0.0f;
	float depth = 0.0f;
	float depth2 = 0.0f;
    float dX = 0.0f;
    float dZ = 0.0f;



	width = w;
	width2 = w/2.0f;
	height = h;
	height2 = h/2.0f;
	depth = d;
	depth2 = d/2.0f;

	
	//
	// RIGHT
	//
	n = y_segs;
	m = z_segs;
    dX = 1.0f/(n);
    dZ = 1.0f/(m);
	for( z=0; z<m; z++ )
	{
		for( x=0; x<n; x++ )
		{
			m_pVertexPosition[v].set( width2, height * (x+1)*dX - height2, depth * (z+1)*dZ - depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = 1.0f - (float)(z+1)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width2, height * x*dX - height2, depth * (z+1)*dZ - depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+0)*dX;
			m_pVertexTexCoord0[v].y = 1.0f - (float)(z+1)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width2, height * x*dX - height2,  depth * z*dZ - depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+0)*dX;
			m_pVertexTexCoord0[v].y = 1.0f - (float)(z+0)*dZ;
			v++;

			m_pVertexPosition[v].set( width2, height * (x+1)*dX - height2, depth * (z+1)*dZ - depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = 1.0f - (float)(z+1)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width2, height * x*dX - height2, depth * z*dZ - depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+0)*dX;
			m_pVertexTexCoord0[v].y = 1.0f - (float)(z+0)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width2, height * (x+1)*dX - height2, depth * z*dZ - depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = 1.0f - (float)(z+0)*dZ;
			v++;
		}
	}

	//
	// LEFT
	//
	n = y_segs;
	m = z_segs;
    dX = 1.0f/(n);
    dZ = 1.0f/(m);
	for( z=0; z<m; z++ )
	{
		for( x=0; x<n; x++ )
		{
			m_pVertexPosition[v].set( -width2, height * x*dX-height2,  depth * z*dZ -depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+0)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+0)*dZ;
			v++;
			
			m_pVertexPosition[v].set( -width2, height * x*dX-height2, depth * (z+1)*dZ-depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+0)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+1)*dZ;
			v++;
			
			m_pVertexPosition[v].set( -width2, height * (x+1)*dX-height2, depth * (z+1)*dZ-depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+1)*dZ;
			v++;


			m_pVertexPosition[v].set( -width2, height * x*dX-height2, depth * z*dZ-depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+0)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+0)*dZ;
			v++;
			
			m_pVertexPosition[v].set( -width2, height * (x+1)*dX-height2, depth * (z+1)*dZ-depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+1)*dZ;
			v++;
			
			m_pVertexPosition[v].set( -width2, height * (x+1)*dX-height2, depth * z*dZ -depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+0)*dZ;
			v++;
		}
	}


	//
	// TOP
	//
	n = x_segs;
	m = z_segs;
    dX = 1.0f/(n);
    dZ = 1.0f/(m);
	for( z=0; z<m; z++ )
	{
		for( x=0; x<n; x++ )
		{
			m_pVertexPosition[v].set(width * x*dX-width2, height2, depth * z*dZ -depth2); 
			m_pVertexTexCoord0[v].x = (float)x*dX;
			m_pVertexTexCoord0[v].y = 1.0f - (float)z*dZ;
			v++;
			
			m_pVertexPosition[v].set( width * x*dX-width2, height2, depth * (z+1)*dZ-depth2 ); 
			m_pVertexTexCoord0[v].x = (float)x*dX;
			m_pVertexTexCoord0[v].y = 1.0f - (float)(z+1)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width * (x+1)*dX-width2, height2, depth * (z+1)*dZ-depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = 1.0f - (float)(z+1)*dZ;
			v++;


			m_pVertexPosition[v].set(width * x*dX-width2, height2, depth * z*dZ-depth2); 
			m_pVertexTexCoord0[v].x = (float)x*dX;
			m_pVertexTexCoord0[v].y = 1.0f - (float)z*dZ;
			v++;
			
			m_pVertexPosition[v].set( width * (x+1)*dX-width2, height2, depth * (z+1)*dZ-depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = 1.0f - (float)(z+1)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width * (x+1)*dX-width2, height2, depth * z*dZ -depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = 1.0f - (float)z*dZ;
			v++;
		}
	}

	
	//
	// BOTTOM
	//
	n = x_segs;
	m = z_segs;
    dX = 1.0f/(n);
    dZ = 1.0f/(m);
	for( z=0; z<m; z++ )
	{
		for( x=0; x<n; x++ )
		{
			m_pVertexPosition[v].set(width * (x+1)*dX-width2, -height2, depth * (z+1)*dZ-depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+1)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width * x*dX-width2, -height2, depth * (z+1)*dZ-depth2  ); 
			m_pVertexTexCoord0[v].x = (float)(x+0)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+1)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width * x*dX-width2, -height2, depth * z*dZ -depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+0)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+0)*dZ;
			v++;


			m_pVertexPosition[v].set( width * (x+1)*dX-width2, -height2, depth * z*dZ -depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+0)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width * (x+1)*dX-width2,-height2, depth * (z+1)*dZ-depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+1)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width * x*dX-width2, -height2, depth * z*dZ-depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+0)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+0)*dZ;
			v++;
		}
	}


	//
	// FRONT
	//
	n = x_segs;
	m = y_segs;
    dX = 1.0f/(n);
    dZ = 1.0f/(m);
	for( z=0; z<m; z++ )
	{
		for( x=0; x<n; x++ )
		{
			m_pVertexPosition[v].set( width * (x+1)*dX - width2, height * (z+1)*dZ - height2, depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+1)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width * x*dX - width2, height * (z+1)*dZ - height2, depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+0)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+1)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width * x*dX - width2,  height * z*dZ - height2, depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+0)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+0)*dZ;
			v++;


			m_pVertexPosition[v].set( width * (x+1)*dX - width2, height * z*dZ - height2, depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+0)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width * (x+1)*dX - width2, height * (z+1)*dZ - height2, depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+1)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width * x*dX - width2, height * z*dZ - height2, depth2 ); 
			m_pVertexTexCoord0[v].x = (float)(x+0)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+0)*dZ;
			v++;
		}
	}

	//
	// BACK
	//
	n = x_segs;
	m = y_segs;
    dX = 1.0f/(n);
    dZ = 1.0f/(m);
	for( z=0; z<m; z++ )
	{
		for( x=0; x<n; x++ )
		{
			m_pVertexPosition[v].set( width * x*dX-width2,  height * z*dZ - height2, -depth2 ); 
			m_pVertexTexCoord0[v].x = 1.0f - (float)(x+0)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+0)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width * x*dX-width2, height * (z+1)*dZ - height2, -depth2 ); 
			m_pVertexTexCoord0[v].x = 1.0f - (float)(x+0)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+1)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width * (x+1)*dX-width2, height * (z+1)*dZ - height2, -depth2 ); 
			m_pVertexTexCoord0[v].x = 1.0f - (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+1)*dZ;
			v++;


			m_pVertexPosition[v].set( width * x*dX-width2, height * z*dZ - height2, -depth2 ); 
			m_pVertexTexCoord0[v].x = 1.0f - (float)(x+0)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+0)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width * (x+1)*dX-width2, height * (z+1)*dZ - height2, -depth2 ); 
			m_pVertexTexCoord0[v].x = 1.0f - (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+1)*dZ;
			v++;
			
			m_pVertexPosition[v].set( width * (x+1)*dX-width2, height * z*dZ - height2, -depth2 ); 
			m_pVertexTexCoord0[v].x = 1.0f - (float)(x+1)*dX;
			m_pVertexTexCoord0[v].y = (float)(z+0)*dZ;
			v++;
		}
	}



	//------ Creation de la liste des faces.
	//
	unsigned long i=0;

	if( m_face_type==2 )
	{
		unsigned long j = 0;
		for( i=0; i<m_numFaces; i++ )
		{
			m_pU32FacesList[i].a = j++;
			m_pU32FacesList[i].b = j++;
			m_pU32FacesList[i].c = j++;
		}
	}
	else
	{
		unsigned short j = 0;
		for( i=0; i<m_numFaces; i++ )
		{
			m_pU16FacesList[i].a = j++;
			m_pU16FacesList[i].b = j++;
			m_pU16FacesList[i].c = j++;
		}
	}



	setVerticesColor( 1.0f, 1.0f, 1.0f );
}

*/


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


/*
void cMesh::renderVertexArray( sRenderMeshParams *p )
{

	int stride = 0;
		
	stride = 0;
	glVertexPointer( 4, GL_FLOAT, stride, m_pVertexPosition->v );
	glEnableClientState( GL_VERTEX_ARRAY );


	if(p->use_normals==1)
	{
		stride = sizeof(cVec4);
		glNormalPointer( GL_FLOAT, stride, m_pVertexNormal->v );
		glEnableClientState( GL_NORMAL_ARRAY );
	}


	if(p->use_color==1)
	{
		stride = 0;
		glColorPointer( 4, GL_FLOAT, stride, m_pVertexColor->v );
		glEnableClientState( GL_COLOR_ARRAY );
	}



	stride = 0;
	if( p->use_texture[0]==1 )
	{
		glClientActiveTexture( GL_TEXTURE0 );
		glTexCoordPointer( 4, GL_FLOAT, stride, m_pVertexTexCoord0->v );
		glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	}


	if( m_face_type==1 )
	{
		glDrawElements( GL_TRIANGLES, m_numFaces*3, GL_UNSIGNED_SHORT, m_pU16FacesList );
	}
	else
	{
		glDrawElements( GL_TRIANGLES, m_numFaces*3, GL_UNSIGNED_INT, m_pU32FacesList );
	}


	
	if( p->use_texture[0]==1 )
	{
		glClientActiveTexture( GL_TEXTURE0 );
		glDisableClientState( GL_TEXTURE_COORD_ARRAY );
	}

	if(p->use_normals==1)
	{
		glDisableClientState(GL_NORMAL_ARRAY);
	}

	if(p->use_color==1)
	{
		glDisableClientState( GL_COLOR_ARRAY );
	}


	glDisableClientState(GL_VERTEX_ARRAY);
}




void cMesh::renderVertexBuffer( sRenderMeshParams *p )
{
	int stride = 0;
	
	int objectSize = sizeof(cVec4) * m_numVertices;

	
	if( m_vbo_position==0 )
	{
		glGenBuffers( 1, (GLuint *)&m_vbo_position );

		glBindBuffer( GL_ARRAY_BUFFER_ARB, m_vbo_position  );
		int objectSize = sizeof(cVec4) * m_numVertices;
		glBufferData( GL_ARRAY_BUFFER, objectSize, NULL, p->vbo_mode );
	}
	else
	{
		glBindBuffer( GL_ARRAY_BUFFER_ARB, m_vbo_position  );
	}

	if(m_update_vbo_position)
	{
	    glBufferSubData( GL_ARRAY_BUFFER_ARB,
		                 0,
	  					 objectSize ,
 						 m_pVertexPosition); 
		m_update_vbo_position = 0;
	}

	glVertexPointer( 4, GL_FLOAT, stride, 0 );
	glEnableClientState( GL_VERTEX_ARRAY );






	if( m_vbo_normal==0 )
	{
		glGenBuffers( 1, (GLuint *)&m_vbo_normal );
		glBindBuffer( GL_ARRAY_BUFFER_ARB, m_vbo_normal  );
		glBufferData( GL_ARRAY_BUFFER, objectSize, NULL, p->vbo_mode );
	}
	else
	{
		glBindBuffer( GL_ARRAY_BUFFER_ARB, m_vbo_normal  );
	}
	
	if(m_update_vbo_normal)
	{
		glBufferSubData( GL_ARRAY_BUFFER_ARB,
			             0,
	  					 objectSize ,
 						 m_pVertexNormal); 
		
		m_update_vbo_normal = 0;
	}
	
	stride = sizeof(cVec4);
	glNormalPointer( GL_FLOAT, stride, 0 );
	glEnableClientState( GL_NORMAL_ARRAY );








	if( m_ibo==0 )
	{
		glGenBuffers( 1, (GLuint *)&m_ibo );

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER_ARB, m_ibo );

		if( m_face_type==1 )
		{
			objectSize = sizeof(sFaceU16) * m_numFaces;
		}
		else
		{
			objectSize = sizeof(sFaceU32) * m_numFaces;
		}
		
		glBufferData( GL_ELEMENT_ARRAY_BUFFER_ARB, objectSize, 0, p->ibo_mode );
	}
	else
	{
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER_ARB, m_ibo );
	}



	if( m_face_type==1 )
	{
		if(m_update_ibo)
		{
			objectSize = sizeof(sFaceU16) * m_numFaces;
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER_ARB,
							0,
							objectSize,
						    m_pU16FacesList);

			m_update_ibo = 0;
		}
		
		glDrawElements( GL_TRIANGLES, m_numFaces*3, GL_UNSIGNED_SHORT, 0 );
	}
	else
	{
		if(m_update_ibo)
		{
			objectSize = sizeof(sFaceU32) * m_numFaces;
			glBufferSubData(GL_ELEMENT_ARRAY_BUFFER_ARB,
							0,
							objectSize,
						    m_pU32FacesList);
			
			m_update_ibo = 0;
		}

		glDrawElements( GL_TRIANGLES, m_numFaces*3, GL_UNSIGNED_INT, 0 );
	}

	
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
	glBindBuffer( GL_ARRAY_BUFFER_ARB, 0 );
	
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}
*/
/*
void cMesh::renderVertexBuffer( sRenderMeshParams *p )
{
	int stride = 0;

	if( m_vbo_position==0 )
	{
		glGenBuffers( 1, (GLuint *)&m_vbo_position );


		glBindBuffer( GL_ARRAY_BUFFER_ARB, m_vbo_position  );
		int objectSize = sizeof(cVec4) * m_numVertices;
		glBufferData( GL_ARRAY_BUFFER, objectSize, m_pVertexPosition, p->vbo_mode );
		glVertexPointer( 4, GL_FLOAT, stride, 0 );
		glEnableClientState( GL_VERTEX_ARRAY );

	}
	else
	{
		glBindBuffer( GL_ARRAY_BUFFER_ARB, m_vbo_position  );
		glEnableClientState( GL_VERTEX_ARRAY );
	}


	if( m_vbo_normal==0 )
	{
		glGenBuffers( 1, (GLuint *)&m_vbo_normal );
		
		glBindBuffer( GL_ARRAY_BUFFER_ARB, m_vbo_normal  );
		int objectSize = sizeof(cVec4) * m_numVertices;
		glBufferData( GL_ARRAY_BUFFER, objectSize, m_pVertexNormal, p->vbo_mode );
		glNormalPointer( GL_FLOAT, stride, 0 );
		glEnableClientState( GL_NORMAL_ARRAY );
		stride = sizeof(cVec4);
	}
	else
	{
		glBindBuffer( GL_ARRAY_BUFFER_ARB, m_vbo_normal  );
		glEnableClientState( GL_NORMAL_ARRAY );
	}



	if( m_ibo==0 )
	{
		glGenBuffers( 1, (GLuint *)&m_ibo );

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER_ARB, m_ibo );

		if( m_face_type==1 )
		{
			int objectSize = sizeof(sFaceU16) * m_numFaces;
			glBufferData( GL_ELEMENT_ARRAY_BUFFER_ARB, objectSize, m_pU16FacesList, p->ibo_mode );
		}
		else
		{
			int objectSize = sizeof(sFaceU32) * m_numFaces;
			glBufferData( GL_ELEMENT_ARRAY_BUFFER_ARB, objectSize, m_pU32FacesList, p->ibo_mode );
		}
	}
	else
	{
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER_ARB, m_ibo );
	}

	if( m_face_type==1 )
	{
		//objectSize = sizeof(sFaceU16) * m_numFaces;
		//glBufferData( GL_ELEMENT_ARRAY_BUFFER_ARB, objectSize, m_pU16FacesList, p->ibo_mode );

		glDrawElements( GL_TRIANGLES, m_numFaces*3, GL_UNSIGNED_SHORT, 0 );
	}
	else
	{
		//objectSize = sizeof(sFaceU32) * m_numFaces;
		//glBufferData( GL_ELEMENT_ARRAY_BUFFER_ARB, objectSize, m_pU32FacesList, p->ibo_mode );

		glDrawElements( GL_TRIANGLES, m_numFaces*3, GL_UNSIGNED_INT, 0 );
	}

	
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER_ARB, 0 );
	glBindBuffer( GL_ARRAY_BUFFER_ARB, 0 );
	
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}
*/

