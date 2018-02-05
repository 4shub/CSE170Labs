# include "sn_torus.h"

const char* SnTorus::class_name = "SnTorus"; // static
SN_SHAPE_RENDERER_DEFINITIONS(SnTorus);

//===== SnTorus =====

SnTorus::SnTorus () : SnShape ( class_name )
{
	resolution = 80;
	radius = 0.01f;
	majorRadius = 0.004f;
	if ( !SnTorus::renderer_instantiator ) SnTorusRegisterRenderer ();
}

SnTorus::~SnTorus ()
{
}

void SnTorus::get_bounding_box ( GsBox& b ) const
{
	// the bounding box is needed for camera view_all computations
	float bound = ((majorRadius * 2) + (radius * 2)) / 10;
	b.set ( init, init+GsVec(-0.1f, 0.1f,0) );
}

//===== Renderer Instantiator =====

# include "glr_torus.h"

static SnShapeRenderer* GlrTorusInstantiator ()
{
	return new GlrTorus;
}

void SnTorusRegisterRenderer ()
{
	SnTorus::renderer_instantiator = &GlrTorusInstantiator;
}
