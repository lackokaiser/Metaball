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

float fcr(vec3 p, vec3 c, float r) {
	float d = distance(c, p) / r;

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
		vec4 ballPos = vec4(balls[i].xyz, 1);

		res += fcr(p, ballPos.xyz, balls[i].w);
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

	vec3 res = vec3(x, y, z) / (2*e.x);

	return normalize(res);
}

float ApplyShadow(vec3 point, vec3 toLight, float tMin, float tMax){
	float res = 1;
	float t = tMin;
	float ph = 1e10;

	for(int i = 0; i < 32; i++){
		float d = F(point + toLight * t); // sdf evaluation

		float y = d*d / (2*ph); // distance from current point

		float h = sqrt(d*d - y*y); // distance from the point to the closest distance

		res = min(res, d / .1 * max(0, t-y));
		ph = d;
		t += d;

		if(res < .0001 || t > tMax) // if overshot, or res is too small, than end
			break;
	}
	
	res = clamp(res, 0, 1);

	return res*res*(3-2*res);

}

vec3 Fresnel(vec3 ks, vec3 h, vec3 toLight){
	return ks + ( 1.0 - ks ) * pow( clamp( 1.0 - dot( h, toLight ), 0.0, 1.0 ), 5.0 );
}

// light
vec3 ApplyLight(vec3 point, vec3 pointNormal, vec3 rayDirection, vec3 eyePosition){
	float shininess = 16.0;
	
	vec3 final = vec3( 0.0 );
	
	vec3 ref = reflect( rayDirection, pointNormal);
    
    vec3 Ks = vec3( 0.5 );
    vec3 Kd = vec3( 1.0 );

	for(int i = 0; i < lightCount; i++){
		vec3 lightPos = lightPoses[i];
		vec3 light_color = vec3( 0.5 );

		vec3 toLight = normalize(lightPos - point);

		vec3 diffuse = Kd * vec3(max(0, dot(toLight, pointNormal)));
		vec3 specular = vec3(max(0, dot(toLight, ref)));

		vec3 f = Fresnel(Ks, normalize(toLight - rayDirection), toLight);

		specular = pow(specular, vec3(shininess));

		final += light_color * mix(diffuse, specular, f);
	}

	final += texture( cubeMap, ref ).rgb * Fresnel( Ks, pointNormal, -rayDirection );

	return final;
}

vec3 applyLight(vec3 lightPos, vec3 color, vec3 normal, vec3 surfPos, vec3 surfToCamera) { // phong plus shadow
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

vec4 calculateColor(vec3 hitPoint, vec3 rayDirection, vec3 startPoint){
// TODO lighting, shadow, etc

	vec3 norm = calculateNormal(hitPoint);

	vec3 color = ApplyLight(hitPoint, norm, rayDirection, startPoint);

	vec3 light = vec3(1);

	vec3 toLight = normalize(hitPoint - light);

	float diffuse_intensity = max(0.0, dot(norm, toLight));


	//return vec4(vec3(1.0, 0.0, 0.0) * diffuse_intensity, 1);
	return vec4(color, 1);
}

vec4 rayMarch(vec3 startPoint, vec3 normalDir, float minD, float maxD) { // logarithmic search, stackless
//	int level = 0;
//	int segment = 0;
//
//	float pixel = .005f;
//
//	const int maxLevel = 5;
//
//	while (true){
//		float tle = (maxD - minD) * exp2(-float(level));
//		float tce = minD + tle * (float(segment)+.5f);
//		float tra = tle * .5f;
//
//		vec3 point = startPoint + tce * normalDir; 
//
//		float d = F(point);
//
//		if(d < tra){
//			if(tra < tce * pixel || level > maxLevel){
//				return calculateColor(point, startPoint);
//			}
//			else {
//				level++;
//				segment <<= 1;
//			}
//		}
//		else {
//			for(; (segment&1) == 1; segment >>= 1, level--);
//			segment++;
//
//			if(level == 0)
//				break;
//		}
//	}
//	return vec4(0);
	float totalDistance = 0;

	const int steps = 32;
	const float max_Distance = 1000.0;


	for(int i = 0; i < steps; i++){
		vec3 cp = startPoint + (totalDistance * normalDir);

		float closest = F(cp);
		
		if(closest * .003 >= 0)
			return calculateColor(cp, normalDir, startPoint);
		
		if(totalDistance > max_Distance)
			break;

		totalDistance -= closest;// optimization: move faster to targets when the minimum distance is too far
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

//mat4 CalculatePerspectiveMatrix(){
//	return mat4(1 / aspect * tan(angle / 2), 0, 0, 0,
//				0, 1 / tan(angle / 2), 0, 0,
//				0, 0, -(far + near)/(far - near), -(2*near*far)/(far - near),
//				0, 0, -1, 0);
//}
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

vec3 RayDirection(vec2 uv, vec3 rayOrigin, vec3 lookat, float zoom){
	vec3 forward = normalize(lookat - rayOrigin);
	vec3 right = normalize(cross(up, forward));
	vec3 up = normalize(cross(right, forward));

	vec3 center = forward * zoom;

	vec3 intersect = center + uv.x * right + uv.y * up;

	return normalize(intersect);
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

	vec3 rd = RayDirection(coord, ro, up, 2.3);

    vec4 shadedColor = rayMarch(ro, rd, .3, 20);

	if(shadedColor.w == 0){
		fs_out_col = texture(cubeMap, rd);
		return;
	}
	
	fs_out_col = shadedColor;
}