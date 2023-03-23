// ref: https://thebookofshaders.com/02/

#ifdef GL_ES
precision mediump float;
#endif

uniform float u_time;

void main() {
	gl_FragColor = vec4(0.318,0.373,1.000,1.000);
}