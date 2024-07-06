#version 430

#define PI 3.1415926535897932384626433832795

uniform float time;

#define MAX_BALLS 5
uniform int ballCount;
uniform vec4 balls[MAX_BALLS];

#define MAX_LIGHTS 3
uniform int lightCount;
uniform vec3 lightPoses[MAX_LIGHTS];
uniform vec3 diffuseColors[MAX_LIGHTS];
uniform vec3 specularColors[MAX_LIGHTS];

in vec2 vs_uv;
out vec4 fs_out_col;

uniform samplerCube cubeMap;

uniform float tr;

// camera
uniform vec3 eye;
uniform vec3 at;
uniform vec3 up;

uniform float aspect;
uniform float angle;
uniform float near;
uniform float far;

uniform float windowSizeX;
uniform float windowSizeY;

uniform vec2 mousePos;

vec4 RayMarch(vec3 startPoint, vec3 normalDir, float minD, float maxD);

// SDF

float fcr(vec3 p, vec3 c, float r) {
	float d = distance(c, p) / r;

	// this part must be commented out for the 5th function
	if (d < 0) 
		return 1;  //completely inside
	else if (d > 1)
		return 0;  //outside 0

	float f = 2 * pow(d, 3.f) - 3.f * pow(d, 2.f) + 1; // 1st function // described by assignment
	//float f = 1 - 3 * pow(d, 2) + 3 * pow(d, 4) - pow(d, 6); // 2nd function // Pixar's RenderMan
	//float f = 1 - 6 * pow(d, 5) + 15 * pow(d, 4) - 10 * pow(d, 3); // 3rd function // John Hart
	/*
	float f = 0; // 4th function // Links (piecewise polynomial)
	if(d < 1.f/3.f)
		f = 1 - 3 * d * d;
	else if(d < 1)
		f = 1.5f * pow(1 - d, 2); 
	*/
	//float f = 1 / pow(d, 2); // 5th function // only works if conditions are deleted above
	
	
	return f;
}

float F(vec3 p){
	
	float res = 0;

	for(int i = 0; i < ballCount; i++){
		res += fcr(p, balls[i].xyz, balls[i].w);
	}

	return res - tr;
}

/// NORMAL

vec3 CalculateNormal(vec3 hitPoint){
	vec2 e = vec2(0.0001f, 0.0);

	float x = F(hitPoint + e.xyy) - F(hitPoint - e.xyy);
	float y = F(hitPoint + e.yxy) - F(hitPoint - e.yxy);
	float z = F(hitPoint + e.yyx) - F(hitPoint - e.yyx);

	vec3 res = vec3(x, y, z) / (2*e.x);

	return -normalize(res);
}

// SHADOW

bool ApplyShadow(vec3 point, vec3 toLight, vec3 lightPos, float tMin){
	const int steps = 30;
	float dist = tMin;

	float max_Distance = distance(point, lightPos);
	
	vec4 hitPoint = RayMarch(point, toLight, .3, max_Distance);

	return hitPoint.w != 0;
}

// LIGHTING

// Blinn-Phong Shading
vec3 ApplyLight(vec3 point, vec3 pointNormal, vec3 eyePosition){
	
	float shininess = 2;
	vec3 final = vec3( 0.0 );

	for(int i = 0; i < lightCount; i++){
		vec3 diffColor = diffuseColors[i];
		vec3 specColor = specularColors[i];
		vec3 lightPos = -lightPoses[i];
		vec3 toLight = normalize(point - lightPos);
		float lightDist = length(point - lightPos);

		float attenuation = 1 / (.5 + 0.2 * lightDist + 0.2 * lightDist * lightDist);

		vec3 ambient = vec3(.1);

		float difFactor = max(dot(toLight, pointNormal), 0) * attenuation;
		vec3 diffuse = difFactor * diffColor;
		
		vec3 toOrigin = normalize(eyePosition - point);
		vec3 reflectDir = reflect(toLight, pointNormal);

		float specFactor = pow(max(dot(toOrigin, reflectDir), 0), shininess) * attenuation;
		vec3 specular = specFactor * specColor;


		if(!ApplyShadow(point, -toLight, lightPos, .1))
			final += (ambient + diffuse + specular);

	}

	return final;
}

vec4 CalculateColor(vec3 hitPoint, vec3 rayDirection, vec3 startPoint, int maxReflect){
	vec3 norm = CalculateNormal(hitPoint);

	vec3 light = ApplyLight(hitPoint, norm, startPoint);


	// EXTRA: REFLECT SELF

	vec3 refStartPoint = startPoint;
	vec4 refHitPoint = vec4(hitPoint, 1);
	vec3 refRd = rayDirection;
	vec3 refNorm = norm;
	for(int i = 0; i < maxReflect; i++){
		refRd = reflect( refRd, refNorm);
		
		refHitPoint = RayMarch(refHitPoint.xyz, refRd, 0.3, 30);
		
		if(refHitPoint.w == 0) {// no collision
			light += texture( cubeMap, -refRd ).rgb;
			break;
		}

		refNorm = CalculateNormal(refHitPoint.xyz);

		vec3 reflectedLight = ApplyLight(refHitPoint.xyz, refNorm, refStartPoint);
		
		reflectedLight += texture(cubeMap, -refRd).rgb;

		refStartPoint = refHitPoint.xyz;

		light *= reflectedLight;
	}

	return vec4(light, 1);
}

// RAYMARCH // BINARY SEARCH

// returns the hit point
vec4 RayMarch(vec3 startPoint, vec3 normalDir, float minD, float maxD) { // binary search, stackless https://iquilezles.org/articles/binarysearchsdf/
	int level = 0;
	int segment = 0;

	// approximation
	const float pixel = .0005f; 

	// max depth
	const int maxLevel = 18;

	while (true){

		// binary search : At the middle
		float levelD = (maxD - minD) * exp2(-level);
		float centerD = minD + levelD * (segment+.5f);
		
		float radiusD = levelD * .5f;

		// point 
		vec3 point = startPoint + centerD * normalDir; 

		// check against the SDF
		float d = F(point);

		if(d >= -radiusD){ // if found
			if(radiusD < centerD * pixel || level > maxLevel){ // check approximation
				return vec4(point, 1);
			}
			else {
				level++;
				segment <<= 1;
			}
		}
		else {
			for(; (segment&1) == 1; segment >>= 1, level--);
			segment++;

			if(level == 0)
				break;
		}
	}
	return vec4(0);
}

// CAMERA AND RAY DIRECTION

vec3 RayDirection(vec2 uv, vec3 rayOrigin, vec3 lookat, float zoom, float fov){

	// NO DISTORTION ON SCREEN
	vec2 newUv = vec2(uv.x, uv.y * radians(tan((90 - fov * .5))));
	vec3 forward = normalize(lookat - rayOrigin);
	vec3 right = normalize(cross(up, forward));
	vec3 up = normalize(cross(right, forward));


	vec3 center = forward * zoom;

	vec3 intersect = center + newUv.x * right + newUv.y * up;

	return normalize(intersect);
}

void main(){
	vec2 coord = vs_uv;

	vec2 clampedMouse = vec2(mousePos.x, clamp(mousePos.y, 0, 3.14));
	vec3 ro = eye;

	vec3 rd = RayDirection(coord, ro, up, 2.3, 45);

    vec4 hitPoint = RayMarch(ro, rd, (distance(ro, vec3(0)) - 10), (distance(ro, vec3(0)) + 10));

	if(hitPoint.w == 0){
		fs_out_col = texture(cubeMap, -rd);
		return;
	}
	
	fs_out_col = CalculateColor(hitPoint.xyz, rd, ro, 4);
}