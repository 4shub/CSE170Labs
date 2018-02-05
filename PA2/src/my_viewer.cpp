
# include "my_viewer.h"
# include "sn_torus.h"

# include <sigogl/gl_tools.h>
# include <sigogl/ui_button.h>

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l) {
	add_ui ();
	render_torus();
}

void MyViewer::add_ui () {
	UiPanel *p;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( new UiButton ( "Add", EvAdd ) );
	p->add ( new UiButton ( "Info", EvInfo ) );
	p->add ( new UiButton ( "Exit", EvExit ) );
}

// define defaults
float initialRadius = 0.4f;
float initialMajorRadius = 1.0f;
int initialResolution = 300;

void MyViewer::render_torus() {
	rootg()->remove(1);

	SnTorus* mytorus;
	mytorus = new SnTorus;
	mytorus->init.set(0, 0, 0);
	mytorus->radius = initialRadius;
	mytorus->majorRadius = initialMajorRadius;
	mytorus->resolution = initialResolution;
	mytorus->color(GsColor::red);
	rootg()->add(mytorus);

	render();
}

int MyViewer::handle_keyboard ( const GsEvent &e ) {
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	switch ( e.key ) {	
		case GsEvent::KeyEsc : gs_exit(); return 1;
		case 113: { // q
			initialResolution += 10;
			render_torus();
			return 1;
		}
		case 97: { // a
			initialResolution -= 10;
			render_torus();
			return 1;
		};
		case 119: { // w
			initialRadius += 0.01f;
			render_torus();
			return 1;
		};
		case 115: { // s
			initialRadius -= 0.01f;
			render_torus();
			return 1;
		};
		case 101: { // e
			initialMajorRadius += 0.01f;
			render_torus();
			return 1;
		};
		case 100: { // d
			initialMajorRadius -= 0.01f;
			render_torus();
			return 1;
		};
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}

	return 0;
}

int MyViewer::uievent ( int e ) {
	switch ( e ) {	
		case EvAdd: render_torus(); return 1;

		case EvInfo: {	
			if ( output().len()>0 ) { output(""); return 1; }
			output_pos(0,30);
			activate_ogl_context(); // we need an active context
			GsOutput o; o.init(output()); gl_print_info(&o); // print info to viewer
			return 1;
		}

		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
