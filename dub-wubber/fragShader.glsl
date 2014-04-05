uniform float uFreqBlock[512];
uniform float uMaxFreqAmp;
uniform vec2 uWinSize;

void main() {
	int index, yFreqPos;
	index = int(128.0 * float(gl_FragCoord.x) / float(uWinSize.x));
	yFreqPos = int(uFreqBlock[index] * uWinSize.y / uMaxFreqAmp);

	if (int(gl_FragCoord.y) <= yFreqPos)
		gl_FragColor = vec4(0.8, 0.8, 0.0, 1.0);
	else if (int(gl_FragCoord.y) <= int(10.0 * uMaxFreqAmp))
	    gl_FragColor = vec4(0.25, 0.0, 0.25, 1.0);
	else 
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
