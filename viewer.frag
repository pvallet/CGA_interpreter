// ----------------------------------------------
// Informatique Graphique 3D & Réalité Virtuelle.
// Travaux Pratiques
// Shaders
// Copyright (C) 2015 Tamy Boubekeur
// All rights reserved.
// ----------------------------------------------

// Add here all the value you need to describe the light or the material. 
// At first used const values. 
// Then, use uniform variables and set them from the CPU program.

const vec3 lightPos = vec3 (-20, 50, 30);

varying vec4 P; // fragment-wise position
varying vec3 N; // fragment-wise normal

uniform vec3 camPos;

void main (void) {
    gl_FragColor = vec4 (0.1, 0.1, 0.1, 1.0);
    
    vec3 p = vec3 (gl_ModelViewMatrix * P);
    vec3 n = normalize (gl_NormalMatrix * N);
    vec3 l = normalize (p - lightPos); // For the moment the computed normals are in the wrong side
    
    vec3 lightColor = vec3 (1.0, 1.0, 1.0);
    
    vec3 color = lightColor * 0.5 * dot(n, l);

    if (dot(color,vec3(1.,1.,1.)) < 0.) {
        color = vec3 (0,0,0);
    }
    
    gl_FragColor += vec4(color, 1.0);
}
 
