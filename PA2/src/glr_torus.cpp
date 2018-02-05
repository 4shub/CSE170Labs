
# include "sn_torus.h"
# include "glr_torus.h"

# include <sig/gs_array.h>
# include <sig/gs_quat.h>

# include <sigogl/gl_core.h>
# include <sigogl/gl_context.h>
# include <sigogl/gl_resources.h>

//# define GS_USE_TRACE1 // Constructor and Destructor
//# define GS_USE_TRACE2 // Render
# include <sig/gs_trace.h>

//======================================= GlrLines ====================================

GlrTorus::GlrTorus ()
{
	GS_TRACE1 ( "Constructor" );
	_psize = 0;
}

GlrTorus::~GlrTorus ()
{
	GS_TRACE1 ( "Destructor" );
}

static const GlProgram* Prog=0; // These are static because they are the same for all GlrTorus instances

void GlrTorus::init ( SnShape* s )
{
	GS_TRACE2 ( "Generating program objects" );
	// Initialize program and buffers if needed:
	if ( !Prog )
	{	bool MyShaders=true;
		if ( MyShaders ) // Example of how to load specific shader files:
		{	const GlShader* MyVtxShader  = GlResources::declare_shader ( GL_VERTEX_SHADER, "MyVtxShader", "../../sig/shaders/3dsmoothsc.vert", 0 );
			const GlShader* MyFragShader = GlResources::declare_shader ( GL_FRAGMENT_SHADER, "MyFragShader", "../../sig/shaders/gouraud.frag", 0 );
			const GlProgram* p = GlResources::declare_program ( "MyNodeProgram", 2, MyVtxShader, MyFragShader );
			GlResources::declare_uniform ( p, 0, "vProj" );
			GlResources::declare_uniform ( p, 1, "vView" );
			GlResources::declare_uniform ( p, 2, "vColor" );
			GlResources::compile_program ( p );
			Prog = p; // Save in Prog a direct pointer to the program used by this node!
		}
		else // Here we just reuse an internal shader:
		{	Prog = GlResources::get_program("3dsmoothsc");
		}
	}

	_glo.gen_vertex_arrays ( 1 );
	_glo.gen_buffers ( 1 );
}


const float arcPi = 3.14f * 2.0f;
void GlrTorus::getCoordinates(int i, int j, int radialSegments, int tubularSegments, float radius, float majorRadius, float& x, float& y, float &z) {

	float u = (float(j) / tubularSegments) * arcPi;
	float v = (float(i) / radialSegments) * arcPi;

	x = (majorRadius + radius * float(cos(v))) * float(cos(u));
	y = (majorRadius + radius * float(cos(v))) * float(sin(u));
	z = radius * float(sin(v));


	return;
}

void GlrTorus::render ( SnShape* s, GlContext* ctx ) {
	GS_TRACE2 ( "GL4 Render "<<s->instance_name() );
	SnTorus& c = *((SnTorus*)s);

	// 1. Set buffer data if node has  been changed:
	if ( s->changed()&SnShape::Changed ) {	
		GsPnt o = c.init;
		int resolution = c.resolution;
		int pointCount = 3 * resolution;
		float radius = c.radius; // tube
		float majorRadius = c.majorRadius;
		
		// constants
		
		const int radialSegments = resolution;
		const int tubularSegments = int(resolution*0.75);
		
		GsArray<GsVec> P(0, pointCount); // will hold the points forming my triangles (size 0, but pre-allocate 6 spaces)
		
		for (int i = 0; i < radialSegments; i++) {
			for (int j = 0; j < tubularSegments; j++) {
				float x, y, z, x2, y2, z2, x3, y3, z3, x4, y4, z4 = 0;

				GlrTorus::getCoordinates(i, j, radialSegments, tubularSegments, radius, majorRadius, x, y, z);
				GlrTorus::getCoordinates(i + 1, j, radialSegments, tubularSegments, radius, majorRadius, x2, y2, z2);
				GlrTorus::getCoordinates(i, j + 1, radialSegments, tubularSegments, radius, majorRadius, x3, y3, z3);
				GlrTorus::getCoordinates(i + 1, j + 1, radialSegments, tubularSegments, radius, majorRadius, x4, y4, z4);

				GsPnt location = GsPnt({ x, y, z });
				GsPnt location2 = GsPnt({ x4, y4, z4 });

				P.push() = location;
				P.push() = GsVec(x2, y2, z2);
				P.push() = GsVec(x4, y4, z4);
				P.push() = location;
				P.push() = GsVec(x3, y3, z3);
				P.push() = GsVec(x4, y4, z4);
			}
		}

		glBindVertexArray ( _glo.va[0] );
		glEnableVertexAttribArray ( 0 );
		glBindBuffer ( GL_ARRAY_BUFFER, _glo.buf[0] );
		glBufferData ( GL_ARRAY_BUFFER, P.sizeofarray(), P.pt(), GL_STATIC_DRAW );
		glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
		_psize = P.size(); // after this line array P will be deallocated (data has been sent to graphics card)
	}

	// 2. Enable/bind needed elements and draw:
	if ( _psize )
	{	GS_TRACE2 ( "Rendering w/ single color..." );
		ctx->use_program ( Prog->id ); // ctx tests if the program is being changed

		glUniformMatrix4fv ( Prog->uniloc[0], 1, GLTRANSPMAT, ctx->projection()->e );
		glUniformMatrix4fv ( Prog->uniloc[1], 1, GLTRANSPMAT, ctx->modelview()->e );
		glUniform4fv ( Prog->uniloc[2], 1, s->color().vec4() );

		glBindVertexArray ( _glo.va[0] );
		glDrawArrays ( GL_TRIANGLES, 0, _psize );
	}

	glBindVertexArray ( 0 ); // done - this call protects our data from subsequent OpenGL calls
}

//================================ EOF =================================================
