// skybox.c

#include "common.h"
#include "skybox.h"
//-----------------------
#include "model.h"
#include "model_loader.h"
#include "maths/vector.h"
#include "render/render.h"
#include "render/shader.h"
#include "render/texture.h"
#include "system/hash.h"

model*			skybox_model = NULL;
GLint			skybox_texture = -1;

// Initialise static data for the skybox system
void skybox_init( ) {
	skybox_texture = texture_loadTGA( "dat/img/vitae_sky2_export_flattened.tga" );

	skybox_model = model_load( "dat/model/inverse_cube.s" );
	skybox_model->meshes[0]->texture_diffuse = skybox_texture;
	skybox_model->meshes[0]->shader = resources.shader_skybox;
}

#define SKYBOX_VERTEX_ATTRIB_POINTER( attrib ) \
	glVertexAttribPointer( attrib, /*vec4*/ 4, GL_FLOAT, /*Normalized?*/GL_FALSE, sizeof( skybox_vertex ), (void*)offsetof( skybox_vertex, attrib) ); \
	glEnableVertexAttribArray( attrib );

// Render the skybox
void skybox_render( void* data ) {
	(void)data;
	// Skybox does not write to the depth buffer

	render_resetModelView();
	vector v = Vector( 0.f, 0.f, 0.f, 1.f );
	matrix_setTranslation( modelview, &v );

	// TEMP: Force texture again as delayed tex loading from model can override this
	skybox_model->meshes[0]->texture_diffuse = skybox_texture;

	mesh_render( skybox_model->meshes[0] );
}
