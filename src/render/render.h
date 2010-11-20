// render.h

#include "scene.h"

// Iterate through each model in the scene
// Translate by their transform
// Then draw all the submeshes
void render_scene(scene* s);

void render_lighting(scene* s);
