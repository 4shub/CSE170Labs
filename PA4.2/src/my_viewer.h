# pragma once

# include <sig/sn_poly_editor.h>
# include <sig/sn_lines2.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ui_button.h>
# include <sigogl/ws_viewer.h>

// Viewer for this example:
class MyViewer : public WsViewer
{  protected :
	enum MenuEv { EvNormals, EvAnimate, EvExit };
	UiCheckButton* _nbut;
	bool _animating;
   public :
	MyViewer ( int x, int y, int w, int h, const char* l );
	void build_ui ();
	void rotate(GsMat &m, float deg);
	void add_model ( SnModel* s, GsVec p );
	void rotatePart(int index, float deg);
	void movePart(int index, float deg);
	void getNewCoords(float (&coords)[3], float(&coordsb)[3], float deg);
	void assignCoords(float(&coords1)[3], float(&coords2)[3]);
	void getCoordDifference(float(&coords1)[3], float(&coords2)[3]);
	void build_scene();
	void show_normals ( bool b );
	void run_animation ();
	virtual int handle_keyboard ( const GsEvent &e ) override;
	virtual int uievent ( int e ) override;
};

