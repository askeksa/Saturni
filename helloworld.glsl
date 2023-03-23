// learn about uniforms
// ref: https://thebookofshaders.com/03/

#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;  // Canvas size (width,height)
uniform vec2 u_mouse;       // mouse position in screen pixels
uniform float u_time;       // Time in seconds since load

void main() {
    float r = abs(sin(u_time * 0.8));
    float g = abs(sin(u_time * 1.5));
    float b = abs(sin(u_time * 0.3));

	gl_FragColor = vec4(r,g,b,1.0);
}