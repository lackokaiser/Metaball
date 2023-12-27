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

uniform float dist;

uniform float aspect;
uniform float angle;
uniform float near;
uniform float far;

uniform float windowSizeX;
uniform float windowSizeY;

uniform vec2 mousePos;

mat4 viewProj;

vec4 RayMarch(vec3 startPoint, vec3 normalDir, float minD, float maxD);

float fcr(vec3 p, vec3 c, float r) {
	float d = distance(c, p) / r;

	if (d < 0)
		return 1; // completely inside
	else if (d > 1)
		return 0; // outside 0
	
	float f = 2 * pow(d, 3.f) - 3.f * pow(d, 2.f) + 1;
	//float f = -log(exp(d * -2)) / 2;
	return f; // on surface
}

float F(vec3 p){
	
	float res = 0;

	for(int i = 0; i < ballCount; i++){
		res += fcr(p, balls[i].xyz, balls[i].w);
	}

	return res - tr;
}

vec3 CalculateNormal(vec3 hitPoint){
	vec3 e = vec3(0.002f, 0.0, 0.0);

	float x = F(hitPoint + e.xyy) - F(hitPoint - e.xyy);
	float y = F(hitPoint + e.yxy) - F(hitPoint - e.yxy);
	float z = F(hitPoint + e.yyx) - F(hitPoint - e.yyx);

	vec3 res = vec3(x, y, z) / (2*e.x);

	return normalize(res);
}

bool ApplyShadow(vec3 point, vec3 toLight, vec3 lightPos, float tMin){
//	float res = 1;
//	float t = tMin;
//	float ph = 1e10;
//
//	for(int i = 0; i < 32; i++){
//		float d = F(point + toLight * t); // sdf evaluation
//
//		float y = d*d / (2*ph); // distance from current point
//
//		float h = sqrt(d*d - y*y); // distance from the point to the closest distance
//
//		res = min(res, d / .1 * max(0, t-y));
//		ph = d;
//		t += d;
//
//		if(res < .0001 || t > tMax) // if overshot, or res is too small, than end
//			break;
//	}
//	
//	res = clamp(res, 0, 1);
//
//	return res*res*(3-2*res);
	const int steps = 30;
	float dist = tMin;

	float max_Distance = distance(point, lightPos);

	vec4 hitPoint = RayMarch(point, toLight, .3, max_Distance);

	return hitPoint.w != 0;
}


// Blinn-Phong shading
vec3 ApplyLight(vec3 point, vec3 pointNormal, vec3 rayDirection, vec3 eyePosition){
	
	float smoothness = 20;
	float irradiance = .4;


	vec3 final = vec3( 0.0 );

	for(int i = 0; i < lightCount; i++){
		vec3 diffColor = diffuseColors[i];
		vec3 specColor = specularColors[i];
		vec3 lightPos = lightPoses[i];

		vec3 Kd = diffColor / PI;
		vec3 Ks = specColor * ((smoothness + 8) / (PI * 8));
		
		vec3 halfVec = normalize(lightPos + pointNormal);

		float cosTi = max(dot(pointNormal, lightPos), 0);
		float cosTh = max(dot(pointNormal, halfVec), 0);

		float lightDist = length(point - lightPos);

		float multiplier = 1 / (.5 + .8 * lightDist + .2 * lightDist * lightDist);

		if(!ApplyShadow(point, -normalize(lightPos - point), lightPos, .9))
			final += (Kd + Ks * pow(cosTh, smoothness)) * multiplier * 2 * irradiance * cosTi;

//		vec3 lightPos = lightPoses[i];
//		vec3 light_color = vec3( 0.5 );
//
//		vec3 toLight = -normalize(lightPos - point);
//
//		vec3 diffuse = Kd * vec3(max(0, dot(toLight, pointNormal)));
//		vec3 specular = vec3(max(0, dot(toLight, ref)));
//
//		vec3 f = Fresnel(Ks, normalize(toLight - rayDirection), toLight);
//
//		specular = pow(specular, vec3(shininess));
//
//		if(!ApplyShadow(point, -toLight, lightPos, 1.5))
//			final += light_color * mix(diffuse, specular, f);
	}

//	if(maxReflect > 0)
//		//final += RayMarch(point, ref, .3, 30, maxReflect-1).xyz;
//	else 
	// * Fresnel( Ks, pointNormal, -rayDirection );

	return final;
}

//vec3 applyLight(vec3 lightPos, vec3 color, vec3 normal, vec3 surfPos, vec3 surfToCamera) { // phong plus shadow
//	vec3 surfaceToLight;
//    float attenuation = 1.0;
//
//	surfaceToLight = normalize(lightPos - surfPos);
//    float distanceToLight = length(lightPos - surfPos);
//    attenuation = 1.0 / (1.0 + 1 * pow(distanceToLight, 2));
//
//	//ambient
//    vec3 ambient = color.rgb;
//
//    //diffuse
//    float diffuseCoefficient = max(0.0, dot(normal, surfaceToLight));
//    vec3 diffuse = diffuseCoefficient * color.rgb;
//    
//    //specular
//    float specularCoefficient = 0.0;
//    if(diffuseCoefficient > 0.0)
//        specularCoefficient = pow(max(0.0, dot(surfToCamera, reflect(-surfaceToLight, normal))), 1);
//    vec3 specular = specularCoefficient * color;
//
//    //linear color (color before gamma correction)
//    return ambient + attenuation*(diffuse + specular);
//}

vec4 CalculateColor(vec3 hitPoint, vec3 rayDirection, vec3 startPoint, int maxReflect){
	vec3 norm = CalculateNormal(hitPoint);

	vec3 light = ApplyLight(hitPoint, norm, rayDirection, startPoint);


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

		vec3 reflectedLight = ApplyLight(refHitPoint.xyz, refNorm, refRd, refStartPoint);
		
		reflectedLight += texture(cubeMap, -refRd).rgb;

		refStartPoint = refHitPoint.xyz;

		light *= reflectedLight;
	}


//	vec3 light = vec3(1);
//
//	vec3 toLight = normalize(hitPoint - light);
//
//	float diffuse_intensity = max(0.0, dot(norm, toLight));
//
//
//	return vec4(vec3(1.0, 0.0, 0.0) * diffuse_intensity, 1);
	return vec4(light, 1);
}

// returns the hit point
vec4 RayMarch(vec3 startPoint, vec3 normalDir, float minD, float maxD) { // binary search, stackless https://iquilezles.org/articles/binarysearchsdf/
	int level = 0;
	int segment = 0;

	float pixel = .0005f; 

	const int maxLevel = 18;

	while (true){
		float tle = (maxD - minD) * exp2(-float(level));
		float tce = minD + tle * (float(segment)+.5f);
		float tra = tle * .5f;

		vec3 point = startPoint + tce * normalDir; 

		float d = F(point);

		if(d >= -tra){
			if(tra < tce * pixel || level > maxLevel){
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

//mat4 CalculateViewMatrix(){
//	vec3 forward = normalize(at - eye);
//	vec3 right = normalize(cross(up, forward));
//	vec3 up = normalize(cross(right, forward));
//
//
//
//	mat4 view1 = mat4(forward, 0,
//					right, 0,
//					up, 0,
//					0,0,0,1); 
//	mat4 view2 = mat4(1, 0, 0, -eye.x,
//					0, 1, 0, -eye.y,
//					0, 0, 1, -eye.z,
//					0,0,0,1);
//
//	return view1 * view2;
//}

mat4 CalculatePerspectiveMatrix(){
	return mat4(1 / aspect * tan(angle / 2), 0, 0, 0,
				0, 1 / tan(angle / 2), 0, 0,
				0, 0, -(far + near)/(far - near), -(2*near*far)/(far - near),
				0, 0, -1, 0);
}
//
//mat4 CalculateViewProjectionMatrix(){
//	mat4 viewMat = CalculateViewMatrix();
//	mat4 perspectiveMat = CalculatePerspectiveMatrix();
//
//	return viewMat;
//}

//vec3 rayDirection(float fieldOfView, vec2 size, vec2 fragCoord) {
//    vec2 xy = fragCoord - size / 2.0;
//    float z = size.y / tan(radians(fieldOfView) / 2.0);
//    return normalize(vec3(xy, -z));
//}

vec3 DefaultRayDirection(vec2 uv, vec2 size, float fov){
	vec2 pos = uv - size * .5;

	float fovHalf = radians(tan((90 - fov * .5)));
	float z = size.y * .5 * fovHalf;

	return normalize(vec3(pos, -z));
}

vec3 RayDirection(vec2 uv, vec3 rayOrigin, vec3 lookat, float zoom, float fov){
	vec3 forward = normalize(lookat - rayOrigin);
	vec3 right = normalize(cross(up, forward));
	vec3 up = normalize(cross(right, forward));


	vec3 center = forward * zoom;

	vec3 intersect = center + uv.x * right + uv.y * up;

	return normalize(vec3(intersect));
}

mat2 RotationMX( float angle ) {
	float s = sin(angle), c = cos(angle);

	return mat2(c, -s, s, c);
}

void main(){
	vec2 coord = vs_uv;

	vec2 clampedMouse = vec2(mousePos.x, clamp(mousePos.y, 0, 3.14));
	vec3 ro = eye;
    
	ro.yz *= RotationMX(clampedMouse.y * 3.14);
	ro.xz *= RotationMX(clampedMouse.x * 2 * 3.14);

	vec3 rd = RayDirection(coord, ro, up, 2.3, 45);

    vec4 hitPoint = RayMarch(ro, rd, (distance(ro, vec3(0)) - 10), (distance(ro, vec3(0)) + 10));

	if(hitPoint.w == 0){
		fs_out_col = texture(cubeMap, -rd);
		return;
	}
	
	fs_out_col = CalculateColor(hitPoint.xyz, rd, ro, 4);
}