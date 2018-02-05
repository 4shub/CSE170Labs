# pragma once

# include <sigogl/glr_base.h>
# include <sigogl/gl_objects.h>

/*! \class GlrLines glr_lines.h
	Renderer for SnLines */
class GlrTorus : public GlrBase
 { protected :
	GlObjects _glo; // indices for opengl vertex arrays and buffers
	gsuint _psize;
   public :
	GlrTorus ();
	virtual ~GlrTorus ();
	virtual void init ( SnShape* s ) override;
	virtual void render ( SnShape* s, GlContext* c ) override;
	virtual void getCoordinates(int i, int j, int radialSegments, int tubularSegments, float radius, float majorRadius, float& x, float& y, float &z);
};

//================================ End of File =================================================

