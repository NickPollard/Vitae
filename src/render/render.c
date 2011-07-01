// render.c

#include "common.h"
#include "render.h"
//-----------------------
#include "camera.h"
#include "font.h"
#include "light.h"
#include "model.h"
#include "scene.h"
#include "render/debugdraw.h"
#include "render/modelinstance.h"
#include "render/texture.h"
// temp
#include "engine.h"

// GLFW Libraries
#include <GL/glfw.h>

// Private Function declarations
void render_buildShaders();

void render_set3D( int w, int h ) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, (double)w / (double)h, 1.0, 200.0);

	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	glDepthMask( GL_TRUE );
}

void render_set2D() {
	// *** Set an orthographic projection
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho( 0,		// left
			640.0,	// right
		   	480.0,	// bottom
		   	0,		// top
		   	0,		// near
		   	1 );	// far

	glDisable( GL_DEPTH_TEST );
}

// Iterate through each model in the scene
// Translate by their transform
// Then draw all the submeshes
void render_scene(scene* s) {
	for (int i = 0; i < s->model_count; i++) {
		modelInstance_draw( scene_model( s, i ));
	}
}

void render_lighting( scene* s ) {
	// Ambient Light
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, s->ambient);

	// Point Lights	
	// according to www.opengl.org/sdk/dorcs/man/xhtml/glLight.xml this should work
	for ( int i = 0; i < s->light_count; i++)
		light_render( GL_LIGHT0 + i /* according to the oGL spec, this should work */, s->lights[i] );
}

void render_applyCamera(camera* cam) {
//	glLoadIdentity();

	// Negate as we're doing the inverse of camera
	matrix cam_inverse;
	matrix_inverse( cam_inverse, cam->trans->world );
//	matrix_print( cam->trans->world );
//	matrix_print( cam->trans->local );
//	matrix_print( cam_inverse );

/*
	vector v = Vector( 0.4f, 0.2f, -3.f, 1.f );
	vector v2 = matrixVecMul( cam->trans->world, &v );
	v2 = matrixVecMul( cam_inverse, &v2 );
	assert( vector_equal( &v, &v2 ));
	*/

	glMultMatrixf( (float*)cam_inverse );
//	glMultMatrixf( (float*)cam->trans->world );

//	const vector* translation = matrix_getTranslation( cam_inverse );
//	glTranslatef( translation->coord.x, translation->coord.y, translation->coord.z );
//	glMultMatrixf( (float*)cam->trans->local );
//	const vector* v = camera_getTranslation( cam );
//	glTranslatef( -(v->coord.x), -(v->coord.y), -(v->coord.z) );
}

// Clear information from last draw
void render_clear() {
	glClearColor(0.f, 0.f, 0.0, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Initialise the 3D rendering
void render_init() {
	if (!glfwInit())
		printf("ERROR - failed to init glfw.\n");

	glfwOpenWindow(640, 480, 8, 8, 8, 8, 8, 0, GLFW_WINDOW);
	glfwSetWindowTitle("Vitae");
	glfwSetWindowSizeCallback(handleResize);

	printf("RENDERING: Initialising OpenGL rendering settings.\n");
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_NORMALIZE);
	glEnable(GL_TEXTURE_2D);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_COLOR_MATERIAL);

	texture_init();

	render_buildShaders();
}

// Terminate the 3D rendering
void render_terminate() {
	glfwTerminate();
}

// Render the current scene
// This is where the business happens
void render( scene* s, int w, int h ) {

	render_set3D( w, h );
	render_applyCamera( s->cam );

	// Switch to the drawing perspective and initialise to the identity matrix
	glMatrixMode(GL_MODELVIEW); 
	glLoadIdentity();
	glColor4f(1.f, 1.f, 1.f, 1.f);

	render_lighting( s );
	render_scene( s );
/*
	glPushMatrix();
	glBegin(GL_TRIANGLES);
	glColor4f(1.f, 1.f, 1.f, 1.f);

	glTexCoord2f(0.f, 0.f);
	glNormal3f(0.f, 0.f, 1.f);
	glVertex3f(0.f, 0.f, depth);

	glTexCoord2f(1.f, 0.f);
	glColor4f(1.f, 0.f, 1.f, 1.f);
	glVertex3f(1.f, 0.f, depth);

	glTexCoord2f(0.f, 1.f);
	glColor4f(0.f, 1.f, 1.f, 1.f);
	glVertex3f(0.f, 1.f, depth);

	glTexCoord2f(1.f, 1.f);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glVertex3f(1.f, 2.f, depth);

	glTexCoord2f(1.f, 0.f);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glVertex3f(1.f, 0.f, depth);

	glTexCoord2f(0.f, 1.f);
	glColor4f(1.f, 1.f, 1.f, 1.f);
	glVertex3f(0.f, 2.f, depth);
	glEnd();
	glPopMatrix();
	*/

/*
	font_renderString( 10.f, 10.f, "a" );
	font_renderString( 10.f, 30.f, "ABCDEFGHIJKLMNOPQRSTUVWXYZ.,/?|'@#!$%^&" );
	font_renderString( 10.f, 50.f, "This is a test!" );
*/
}

void render_buildShaders() {

}
