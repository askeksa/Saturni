// learn about a varying (ie gl_FragCoord)
// ref: https://thebookofshaders.com/03/

#ifdef GL_ES
precision mediump float;
#endif

uniform vec2 u_resolution;  // Canvas size (width,height)
uniform vec2 u_mouse;       // mouse position in screen pixels
uniform float u_time;       // Time in seconds since load

void main() {
    vec2 st = gl_FragCoord.xy/u_resolution;
    // normalize the coordinate of the fragment by dividing it 
    // by the total resolution of the billboard. 
    // By doing this the values will go between 0.0 and 1.0, 
    // which makes it easy to map the X and Y values to the RED and GREEN channel.
	gl_FragColor = vec4(st.x,st.y,0.0,1.0);
}