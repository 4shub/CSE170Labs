
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	_nbut=0;
	_animating=false;
	build_ui ();
	build_scene ();
}

void MyViewer::build_ui ()
{
	UiPanel *p, *sp;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( new UiButton ( "View", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( _nbut=new UiCheckButton ( "Normals", EvNormals ) ); 
	}
	p->add ( new UiButton ( "Animate", EvAnimate ) );
	p->add ( new UiButton ( "Exit", EvExit ) ); p->top()->separate();
}

void MyViewer::add_model ( SnShape* s, GsVec p )
{
	SnManipulator* manip = new SnManipulator;
	GsMat m;
	m.translation ( p );
	manip->initial_mat ( m );

	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(s);
	g->add(l);
	manip->child(g);

	rootg()->add(manip);
}

void MyViewer::build_scene ()
{
	SnPrimitive* p;

	// generate clock face
	p = new SnPrimitive(GsPrimitive::Cylinder,10,10,1);
	p->prim().material.diffuse=GsColor::blue;
	p->prim().orientation = GsQuat(1, 1, 1, 1);
	add_model ( p, GsVec(0,0,0) );

	// generate spindle
	p = new SnPrimitive(GsPrimitive::Cylinder, 0.2f, 0.2f, 3);
	p->prim().material.diffuse = GsColor::darkblue;
	p->prim().orientation = GsQuat(1, 1, 1, 1);
	add_model(p, GsVec(0, 0, 5));

	// generate big hand
	p = new SnPrimitive(GsPrimitive::Cylinder, 0.5f, 0.5f, 16);
	p->prim().material.diffuse = GsColor::green;
	p->prim().orientation = GsQuat(1, 0, 0, 0);
	p->prim().center = GsVec(0, 16, 7);
	add_model(p, GsVec(0, 0, 7));

	// generate little hand
	p = new SnPrimitive(GsPrimitive::Cylinder, 0.5f, 0.5f, 12);
	p->prim().material.diffuse = GsColor::red;
	p->prim().orientation = GsQuat(1, 0, 0, 0);
	p->prim().center = GsVec(0, 12, 12);

	add_model(p, GsVec(0, 0, 12));

	// p = new SnPrimitive(GsPrimitive::Sphere,2);
	// p->prim().material.diffuse = GsColor::red;
	// add_model(p, GsVec(-4, 0, 0));
}


void MyViewer::rotate(GsMat &m, float deg) {
	m.e33 = 1;
	m.e11 = cosf(deg);
	m.e12 = -sinf(deg);
	m.e21 = sinf(deg);
	m.e22 = cosf(deg);
}

float pauseAnimation = false;
float stopAnimation = false;

// Below is an example of how to control the main loop of an animation:
void MyViewer::run_animation () {
	stopAnimation = false;

	SnManipulator* bigHand = rootg()->get<SnManipulator>(2); // access big hand
	GsMat bigHandM = bigHand->mat();

	SnManipulator* littleHand = rootg()->get<SnManipulator>(3); // access little hand
	GsMat littleHandM = littleHand->mat();

	const float rotation = 6.0f * 3.14f/180; // move 6 degrees
	const float timer = 1000 / 60; // 1/60th of a second
	const float fullRotation = 6.28319f; // radians in a full rotation
	
	float bigHandPosition = 0, littleHandPosition = 0;

	do {	
		gs_sleep(int(timer));
		if (littleHandPosition >= fullRotation) {
			bigHandPosition += rotation;
			MyViewer::rotate(bigHandM, bigHandPosition);
			littleHandPosition = 0;
			bigHand->initial_mat(bigHandM);
			render(); // notify it needs redraw
			ws_check(); // redraw now
		}

		littleHandPosition += rotation;
		MyViewer::rotate(littleHandM, littleHandPosition);

		littleHand->initial_mat(littleHandM);
		render(); // notify it needs redraw
		ws_check(); // redraw now
	} while (!stopAnimation);
}

void MyViewer::show_normals ( bool b )
{
	// Note that primitives are only converted to meshes in GsModel
	// at the first draw call.
	GsArray<GsVec> fn;
	SnGroup* r = (SnGroup*)root();
	for ( int k=0; k<r->size(); k++ )
	{	SnManipulator* manip = r->get<SnManipulator>(k);
		SnShape* s = manip->child<SnGroup>()->get<SnShape>(0);
		SnLines* l = manip->child<SnGroup>()->get<SnLines>(1);
		if ( !b ) { l->visible(false); continue; }
		l->visible ( true );
		if ( !l->empty() ) continue; // build only once
		l->init();
		if ( s->instance_name()==SnPrimitive::class_name )
		{	GsModel& m = *((SnModel*)s)->model();
			m.get_normals_per_face ( fn );
			const GsVec* n = fn.pt();
			float f = 0.33f;
			for ( int i=0; i<m.F.size(); i++ )
			{	const GsVec& a=m.V[m.F[i].a]; l->push ( a, a+(*n++)*f );
				const GsVec& b=m.V[m.F[i].b]; l->push ( b, b+(*n++)*f );
				const GsVec& c=m.V[m.F[i].c]; l->push ( c, c+(*n++)*f );
			}
		}  
	}
}

int MyViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	switch ( e.key ) {	
		case GsEvent::KeyEsc : gs_exit(); return 1;
		case GsEvent::KeyEnter: {
			stopAnimation = true;
			run_animation();
			return 1;
		}
		case GsEvent::KeySpace: {
			if (pauseAnimation) {
				pauseAnimation = false;
			}

			pauseAnimation = true;
			return 1;
		}
		case 'n' : { bool b=!_nbut->value(); _nbut->value(b); show_normals(b); return 1; }
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}

	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{	case EvNormals: show_normals(_nbut->value()); return 1;
		case EvAnimate: run_animation(); return 1;
		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
