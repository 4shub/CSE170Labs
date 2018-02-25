
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

void MyViewer::add_model ( SnModel* s, GsVec p )
{
	SnManipulator* manip = new SnManipulator;
	GsMat m;
	m.translation ( p );
	manip->initial_mat ( m );

	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(s);
	manip->child(g);

	rootg()->add(manip);
}

void MyViewer::rotate(GsMat &m, float deg) {
	float rad = -(deg * 3.14f / 180);
	m.e11 = 1;
	m.e22 = cosf(rad);
	m.e32 = -sinf(rad);
	m.e23 = sinf(rad);
	m.e33 = cosf(rad);
}

float armPartLength = 27.0f;


void MyViewer::rotatePart(int index, float deg) {
	SnManipulator* mlowerarm = rootg()->get<SnManipulator>(index); // access big hand
	GsMat mlowerarmM = mlowerarm->mat();

	MyViewer::rotate(mlowerarmM, deg);
	mlowerarm->initial_mat(mlowerarmM);
}

void MyViewer::getNewCoords(float(&coords)[3], float(&coordsb)[3], float deg) {
	float rad = (-deg * 3.14f / 180);

	gsout << "rad:" << -deg << "\n";
	gsout << "p0   | z1:" << coordsb[2] << ", y1:" << coordsb[1] << "\n";

	coords[1] = (sinf(rad) * armPartLength) + coordsb[1];
	coords[2] = (cosf(rad) * armPartLength) + coordsb[2];

	gsout << "p1   | z1:" << coords[2] - coordsb[2] << ", y1:" << coords[1] - coordsb[1] << "\n";
	gsout << "p2   | z1:" << coords[2] << ", y1:" << coords[1] << "\n";

}

void MyViewer::assignCoords(float(&coords1)[3], float(&coords2)[3]) {
	coords1[0] = coords2[0];
	coords1[1] = coords2[1];
	coords1[2] = coords2[2];
}

void MyViewer::getCoordDifference(float(&coords1)[3], float(&coords2)[3]) {
	coords1[0] = coords2[0] - coords1[0];
	coords1[1] = coords2[1] - coords1[1];
	coords1[2] = coords2[2] - coords1[2];
}

float rhandCoords[] = { 0, 0, 0 };
float rhandRotation = 0;
float rlowerarmCords[] = { 0, -10, -25 };
float rlowerarmRotation = -20.0f;
float rupperarmCords[] = { 0, 0, -50 };
float rupperarmRotation = 20.0f;


void MyViewer::movePart(int index, float deg) {
	SnManipulator* mlowerarm = rootg()->get<SnManipulator>(index); // access big hand
	GsMat mlowerarmM = mlowerarm->mat();
	float coords[] = { 0, 0, 0 };
	float coordsb[] = { 0, 0, 0 };

	if (index == 0) {
		 assignCoords(coords, rhandCoords);
		 assignCoords(coordsb, rlowerarmCords);
	} else {
		assignCoords(coords, rlowerarmCords);
		assignCoords(coordsb, rupperarmCords);
	}

	getNewCoords(coords, coordsb, deg);

	GsMat m;
	m.translation(GsVec(coords[0], coords[1], coords[2]));
	mlowerarm->initial_mat(m);
	render();

	if (index == 1) {
		assignCoords(rlowerarmCords, coords);
	} else {
		assignCoords(rhandCoords, coords);
	}
}

void MyViewer::build_scene () {
	GsModel* rhand = new GsModel();
	GsModel* rlowerarm = new GsModel();
	GsModel* rupperarm = new GsModel();


	rhand->load("/Users/shub/Documents/CSE170/PA4.2/src/rhand.m");
	add_model(new SnModel(rhand), GsVec(rhandCoords[0], rhandCoords[1], rhandCoords[2]));

	rlowerarm->load("/Users/shub/Documents/CSE170/PA4.2/src/rlowerarm.m");
	add_model(new SnModel(rlowerarm), GsVec(rlowerarmCords[0], rlowerarmCords[1], rlowerarmCords[2]));

	rupperarm->load("/Users/shub/Documents/CSE170/PA4.2/src/rupperarm.m");
	add_model(new SnModel(rupperarm), GsVec(rupperarmCords[0], rupperarmCords[1], rupperarmCords[2]));

	rotatePart(2, rupperarmRotation);
	movePart(1, rupperarmRotation);
	rotatePart(1, rlowerarmRotation);
	movePart(0, rlowerarmRotation);
	render();

}

// Below is an example of how to control the main loop of an animation:
void MyViewer::run_animation ()
{
	if ( _animating ) return; // avoid recursive calls
	_animating = true;
	
	int ind = gs_random ( 0, rootg()->size()-1 ); // pick one child
	SnManipulator* manip = rootg()->get<SnManipulator>(ind); // access one of the manipulators
	GsMat m = manip->mat();

	double frdt = 1.0/30.0; // delta time to reach given number of frames per second
	double v = 4; // target velocity is 1 unit per second
	double t=0, lt=0, t0=gs_time();
	do // run for a while:
	{	while ( t-lt<frdt ) { ws_check(); t=gs_time()-t0; } // wait until it is time for next frame
		double yinc = (t-lt)*v;
		if ( t>2 ) yinc=-yinc; // after 2 secs: go down
		lt = t;
		m.e24 += (float)yinc;
		if ( m.e24<0 ) m.e24=0; // make sure it does not go below 0
		manip->initial_mat ( m );
		render(); // notify it needs redraw
		ws_check(); // redraw now
	}	while ( m.e24>0 );
	_animating = false;
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
		case 'n' : { bool b=!_nbut->value(); _nbut->value(b); show_normals(b); return 1; }
		case 'q': { // rotate shoulder up
			rupperarmRotation += 5.0f;
			rotatePart(2, rupperarmRotation);
			movePart(1, rupperarmRotation);
			rotatePart(1, rlowerarmRotation);
			movePart(0, rlowerarmRotation);
			render();
			return 1;
		}
		case 'a': { // rotate shoulder down
			rupperarmRotation -= 5.0f;
			rotatePart(2, rupperarmRotation);
			movePart(1, rupperarmRotation);
			rotatePart(1, rlowerarmRotation);
			movePart(0, rlowerarmRotation);
			render();
			return 1;
		}
		case 'w': { // rotate elbow up
			rlowerarmRotation += 5.0f;
			rotatePart(1, rlowerarmRotation);
			movePart(0, rlowerarmRotation);
			render();
			return 1;
		}
		case 's': { // rotate elbow down
			rlowerarmRotation -= 5.0f;
			rotatePart(1, rlowerarmRotation);
			movePart(0, rlowerarmRotation);
			render();
			return 1;
		}
		case 'e': { // rotate wrist up
			rhandRotation += 5.0f;
			rotatePart(0, rhandRotation);
			render();
			return 1;
			
		}
		case 'd': { // rotate wrist down
			rhandRotation -= 5.0f;
			rotatePart(0, rhandRotation);
			render();
			return 1;
		}
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
