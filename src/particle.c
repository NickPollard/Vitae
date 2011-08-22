// particle.c
#include "src/common.h"
#include "src/particle.h"
//---------------------
#include "mem/allocator.h"
#include "model.h"
#include "render/render.h"
#include "render/shader.h"
#include "render/texture.h"
#include "system/hash.h"

float property_samplef( property* p, float time );
GLint particle_texture = 0;

particleEmitter* particleEmitter_create() {
	particleEmitter* p = mem_alloc( sizeof( particleEmitter ));
	memset( p, 0, sizeof( particleEmitter ));
	particle_texture = texture_loadTGA( "assets/img/star_rgba64.tga" );
	return p;
}

void particleEmitter_addParticle( particleEmitter* e ) {
	int index = (e->start + e->count) % kmax_particles;
	particle* p = &e->particles[index];
	e->count++;
	if ( e->count > kmax_particles ) {
		e->count--;
		e->start++;
	}
	p->position	= Vector( 0.f, 0.f, 0.f, 1.f );
	p->age = 0.f;
}

void particleEmitter_tick( void* data, float dt ) {
	particleEmitter* e = data;
	// Update existing particles
	vector delta;
	vector_scale ( &delta, &e->velocity, dt );
	for ( int i = 0; i < e->count; i++ ) {
		int index = (e->start + i) % kmax_particles;
		e->particles[index].age += dt;
		Add( &e->particles[index].position, &e->particles[index].position, &delta );
	}

	// Spawn new particle
	e->next_spawn += dt;
	if ( e->next_spawn > e->spawn_interval ) {
		e->next_spawn = 0.f;
		particleEmitter_addParticle( e );
	}
}

typedef struct particle_vertex_s {
	vector	position;
	vector	normal;
	vector	uv;
} particle_vertex;

// Output the 4 verts of the quad to the target vertex array
// both position and normals
void particle_quad( particle_vertex* dst, vector* point, float size ) {
	vector offset = Vector( size, size, 0.f, 0.f );

	vector p = matrixVecMul( modelview, point );

	Add( &dst[0].position, &p, &offset );
	dst[0].normal = Vector( 0.f, 0.f, 1.f, 0.f );
	dst[0].uv = Vector( 1.f, 1.f, 0.f, 0.f );

	Sub( &dst[1].position, &p, &offset );
	dst[1].normal = Vector( 0.f, 0.f, 1.f, 0.f );
	dst[1].uv = Vector( 0.f, 0.f, 0.f, 0.f );

	offset.coord.y = -offset.coord.y;

	Add( &dst[2].position, &p, &offset );
	dst[2].normal = Vector( 0.f, 0.f, 1.f, 0.f );
	dst[2].uv = Vector( 1.f, 0.f, 0.f, 0.f );

	Sub( &dst[3].position, &p, &offset );
	dst[3].normal = Vector( 0.f, 0.f, 1.f, 0.f );
	dst[3].uv = Vector( 0.f, 1.f, 0.f, 0.f );
}

// Render a particleEmitter system
void particleEmitter_render( void* data ) {
	glDepthMask( GL_FALSE );
	// switch to particle shader
	shader_activate( resources.shader_particle );

	// Set up uniforms
	render_setUniform_matrix( *resources.uniforms.projection, perspective );
//	render_setUniform_matrix( *resources.uniforms.modelview, modelview );
	render_setUniform_matrix( *resources.uniforms.worldspace, modelview );

	// Textures
	GLint* tex = shader_findConstant( mhash( "tex" ));
	if ( tex )
		render_setUniform_texture( *tex, particle_texture );

	particleEmitter* p = data;

	// reset modelview matrix so we can billboard
	// particle_quad() will manually apply the modelview
	render_resetModelView();
	matrix_mul( modelview, modelview, p->trans->world );

	particle_vertex vertex_buffer[kmax_particle_verts];
	GLushort element_buffer[kmax_particle_verts];

	for ( int i = 0; i < p->count; i++ ) {
		int index = (p->start + i) % kmax_particles;
		float size = property_samplef( p->size, p->particles[index].age );
		particle_quad( &vertex_buffer[index*4], &p->particles[index].position, size );
		assert( i*6 + 5 < kmax_particle_verts );
		// TODO: Indices can be initialised once
		element_buffer[i*6+0] = index*4+0;
		element_buffer[i*6+1] = index*4+1;
		element_buffer[i*6+2] = index*4+2;
		element_buffer[i*6+3] = index*4+0;
		element_buffer[i*6+4] = index*4+1;
		element_buffer[i*6+5] = index*4+3;
	}

	// For Billboard particles; cancel out the rotation of the matrix
	// The transformation has been applied already for particle positions
	matrix_setIdentity( modelview );
	render_setUniform_matrix( *resources.uniforms.modelview, modelview );

	int index_count = 6 * p->count;
	
	// Copy our data to the GPU
	// There are now <index_count> vertices, as we have unrolled them
	GLsizei vertex_buffer_size = index_count * sizeof( particle_vertex );
	GLsizei element_buffer_size = index_count * sizeof( GLushort );
	GLint position = *(shader_findConstant( mhash( "position" )));
	GLint normal = *(shader_findConstant( mhash( "normal" )));
	GLint uv = *(shader_findConstant( mhash( "uv" )));
	// *** Vertex Buffer
	{
		// Activate our buffers
		glBindBuffer( GL_ARRAY_BUFFER, resources.vertex_buffer );
		glBufferData( GL_ARRAY_BUFFER, vertex_buffer_size, vertex_buffer, GL_STREAM_DRAW );
		// Set up position data
		glVertexAttribPointer( position, /*vec4*/ 4, GL_FLOAT, /*Normalized?*/GL_FALSE, sizeof( particle_vertex ), (void*)offsetof( particle_vertex, position) );
		glEnableVertexAttribArray( position );
		// Set up normal data
		glVertexAttribPointer( normal, /*vec4*/ 4, GL_FLOAT, /*Normalized?*/GL_FALSE, sizeof( particle_vertex ), (void*)offsetof( particle_vertex, normal ) );
		glEnableVertexAttribArray( normal );
		// Set up texcoord data
		glVertexAttribPointer( uv, /*vec4*/ 4, GL_FLOAT, /*Normalized?*/GL_FALSE, sizeof( particle_vertex ), (void*)offsetof( particle_vertex, uv ) );
		glEnableVertexAttribArray( uv );
	}
	// *** Element Buffer
	{
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, resources.element_buffer );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, element_buffer_size, element_buffer, GL_STREAM_DRAW );
	}

	// Draw!
	glDrawElements( GL_TRIANGLES, index_count, GL_UNSIGNED_SHORT, (void*)0 );

	// Cleanup
	glDisableVertexAttribArray( position );
	glDisableVertexAttribArray( normal );
	glDisableVertexAttribArray( uv );
}

property* property_create( int stride ) {
	property* p = mem_alloc( sizeof( property ));
	memset( p, 0, sizeof( property ));
	p->stride = stride;
	p->data = mem_alloc( sizeof( float ) * p->stride * kmax_property_values );
	return p;
}

void property_addf( property* p, float time, float value ) {
	assert( p->count < kmax_property_values );
	p->data[p->count * p->stride] = time;
	p->data[p->count * p->stride + 1] = value;
	p->count++;
}

float property_samplef( property* p, float time ) {
	float t_after = 0.f, t_before = 0.f;
	int after = -1;
	while ( t_after < time && after < p->count ) {
		after++;
		t_before = t_after;
		t_after = p->data[after*p->stride];
	}
	int before = clamp( after - 1, 0, p->count-1 );
	after = clamp( after, 0, p->count-1 );
//	assert( t_before <= time );
//	assert( t_after >= time );
	float factor = map_range( time, t_before, t_after );
	if ( after == before )
		factor = 0.f;
	assert( factor <= 1.f && factor >= 0.f );
	float value = lerp( p->data[before*p->stride+1], p->data[after*p->stride+1], factor );
//	printf( "Time: %.2f, T_before: %.2f, T_after: %.2f, Value: %2f\n", time, t_before, t_after, value );

	return value;
}

void test_property() {
	property* p = property_create( 2 );
	property_addf( p, 0.f, 0.f );
	property_addf( p, 1.f, 3.f );
	property_addf( p, 2.f, 2.f );
	property_addf( p, 3.f, 3.f );
	property_samplef( p, 0.75f );
	property_samplef( p, 1.5f );
	property_samplef( p, 3.0f );

//	assert( 0 );
}
