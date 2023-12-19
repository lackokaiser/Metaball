#version 430
uniform float time;

#define MAX_BALLS 5
uniform int ballCount;
uniform vec4 balls[MAX_BALLS];

#define MAX_LIGHTS 3
uniform int lightCount;
uniform vec3 lightPoses[MAX_LIGHTS];

in vec2 vs_uv;
out vec4 fs_out_col;

uniform float tr = 1;

// camera
uniform vec3 eye = vec3(0, 0, 1);
uniform vec3 at = vec3(0, 0, -5);
uniform vec3 up = vec3(0, 1, 0);

float distanceTo(vec3 p, vec3 c, float r){
	return distance(c, p) / r;
}

float fcr(vec3 p, vec3 c, float r) {
	float d = distanceTo(p, c, r);

	if (d < 0)
		return 1; // completely inside
	else if (d > 1)
		return 0; // outside 0
	
	float f = 2 * pow(d, 3.f) - 3.f * pow(d, 2.f) + 1;

	return f; // on surface
}

float F(vec3 p){
	
	float res = 0;

	for(int i = 0; i < ballCount; i++){
		res += fcr(p, balls[i].xyz, balls[i].w);
	}

	return res - tr;

	if(res > tr)
		return 1;
	return 0;
}

vec3 calculateNormal(vec3 hitPoint){
	vec3 e = vec3(0.01f, 0.0, 0.0);

	float x = F(hitPoint + e.xyy) - F(hitPoint - e.xyy);
	float y = F(hitPoint + e.yxy) - F(hitPoint - e.yxy);
	float z = F(hitPoint + e.yyx) - F(hitPoint - e.yyx);

	vec3 res = vec3(x, y, z) / 2*e.x;

	return normalize(res);
}

vec3 applyLight(vec3 lightPos, vec3 color, vec3 normal, vec3 surfPos, vec3 surfToCamera) {
	vec3 surfaceToLight;
    float attenuation = 1.0;

	surfaceToLight = normalize(lightPos - surfPos);
    float distanceToLight = length(lightPos - surfPos);
    attenuation = 1.0 / (1.0 + 1 * pow(distanceToLight, 2));

	//ambient
    vec3 ambient = color.rgb;

    //diffuse
    float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
    vec3 diffuse = diffuseCoefficient * color.rgb;
    
    //specular
    float specularCoefficient = 0.0;
    if(diffuseCoefficient > 0.0)
        specularCoefficient = pow(max(0.0, dot(surfToCamera, reflect(-surfaceToLight, normal))), 1);
    vec3 specular = specularCoefficient * color;

    //linear color (color before gamma correction)
    return ambient + attenuation*(diffuse + specular);
}

vec4 calculateColor(vec3 hitPoint){
// TODO lighting, shadow, etc

	vec3 norm = calculateNormal(hitPoint);

	vec3 color = applyLight(vec3(0,1,0), vec3(1, 0, 0), norm, hitPoint, hitPoint - at);

	vec3 light = vec3(1);

	vec3 toLight = normalize(hitPoint - light);

	float diffuse_intensity = max(0.0, dot(norm, toLight));


	return vec4(vec3(1.0, 0.0, 0.0) * diffuse_intensity, 1);
}

vec4 rayMarch(vec3 startPoint, vec3 normalDir) {
	float totalDistance = .0f;

	const int steps = 32;
	const float max_Distance = 1000.0;


	for(int i = 0; i < steps; i++){
		vec3 cp = startPoint + (totalDistance * normalDir);

		float closest = F(cp);
		
		if(closest >= 0)
			return calculateColor(cp);
		
		if(totalDistance > max_Distance)
			break;

		totalDistance -= closest;// optimization: move faster to targets when the minimum distance is too far
	}

	return vec4(0);
}

void main(){
	vec2 coord = vs_uv;

	vec3 ro = at;
    vec3 rd = vec3(coord, 1);

    vec4 shadedColor = rayMarch(ro, normalize(rd));

	if(shadedColor.w == 0)
		discard;
	
	fs_out_col = shadedColor;
}