/* clang-format off */
[vertex]

#ifdef USE_GLES_OVER_GL
#define lowp
#define mediump
#define highp
#else
precision highp float;
precision highp int;
#endif

attribute vec2 vertex_attrib; // attrib:0
/* clang-format on */
attribute vec2 uv_in; // attrib:4

varying vec2 uv_interp;

void main() {

	uv_interp = uv_in;
	gl_Position = vec4(vertex_attrib, 0.0, 1.0);
}

/* clang-format off */
[fragment]

#ifdef USE_GLES_OVER_GL
#define lowp
#define mediump
#define highp
#else
#if defined(USE_HIGHP_PRECISION)
precision highp float;
precision highp int;
#else
precision mediump float;
precision mediump int;
#endif
#endif

varying vec2 uv_interp;
/* clang-format on */
uniform sampler2D source_color; //texunit:0

uniform vec2 pixel_size;

void main() {

#ifdef AO_BLUR_H_PASS
	vec3 s;

	gl_FragColor.rgb = vec3(0.0);

	s = texture2D(source_color, uv_interp + pixel_size * vec2(2.0, 0.0)).arg;
	gl_FragColor.rgb = vec3(max(s.r, gl_FragColor.r), gl_FragColor.gb + s.gb * 0.2);

	s = texture2D(source_color, uv_interp + pixel_size * vec2(1.0, 0.0)).arg;
	gl_FragColor.rgb = vec3(max(s.r, gl_FragColor.r), gl_FragColor.gb + s.gb * 0.2);

	s = texture2D(source_color, uv_interp).arg;
	gl_FragColor.a = s.b;
	gl_FragColor.rgb = vec3(max(s.r, gl_FragColor.r), gl_FragColor.gb + s.gb * 0.2);

	s = texture2D(source_color, uv_interp + pixel_size * vec2(-1.0, 0.0)).arg;
	gl_FragColor.rgb = vec3(max(s.r, gl_FragColor.r), gl_FragColor.gb + s.gb * 0.2);

	s = texture2D(source_color, uv_interp + pixel_size * vec2(-2.0, 0.0)).arg;
	gl_FragColor.rgb = vec3(max(s.r, gl_FragColor.r), gl_FragColor.gb + s.gb * 0.2);

	gl_FragColor.b = max(gl_FragColor.a, gl_FragColor.b);

	//gl_FragColor = vec4(texture2D(source_color, uv_interp).ra, 0.0, 0.0); //dbg
#endif

#ifdef AO_BLUR_V_PASS
	vec3 s;

	gl_FragColor.rgb = vec3(0.0);

	s = texture2D(source_color, uv_interp + pixel_size * vec2(0.0, 6.0)).rgb;
	gl_FragColor.rgb = vec3(max(s.r, gl_FragColor.r), gl_FragColor.gb + s.gb * 0.1666666666);

	s = texture2D(source_color, uv_interp + pixel_size * vec2(0.0, 5.0)).rgb;
	gl_FragColor.rgb = vec3(max(s.r, gl_FragColor.r), gl_FragColor.gb + s.gb * 0.1666666666);

	s = texture2D(source_color, uv_interp + pixel_size * vec2(0.0, 4.0)).rgb;
	gl_FragColor.rgb = vec3(max(s.r, gl_FragColor.r), gl_FragColor.gb + s.gb * 0.1666666666);

	s = texture2D(source_color, uv_interp + pixel_size * vec2(0.0, 3.0)).rgb;
	gl_FragColor.rgb = vec3(max(s.r, gl_FragColor.r), gl_FragColor.gb + s.gb * 0.1666666666);

	s = texture2D(source_color, uv_interp + pixel_size * vec2(0.0, 2.0)).rgb;
	gl_FragColor.rgb = vec3(max(s.r, gl_FragColor.r), gl_FragColor.gb + s.gb * 0.1666666666);

	s = texture2D(source_color, uv_interp + pixel_size * vec2(0.0, 1.0)).rgb;
	gl_FragColor.a = s.b;
	gl_FragColor.rgb = vec3(max(s.r, gl_FragColor.r), gl_FragColor.gb + s.gb * 0.1666666666);

	gl_FragColor.b = max(gl_FragColor.a, gl_FragColor.b);

	//gl_FragColor = texture2D(source_color, uv_interp); //dbg
#endif
}
