
#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

/*****************Screen输入*****************************************************************/
uniform sampler2D lastSample;

uniform float rand;
uniform int samplesCount;

/*****************视口大小******************************************************************/
uniform int width;
uniform int height;

struct HitInfos {
    float t;     // 命中时光线的t
    vec3 origin; // 光线起点
    vec3 dir;    // 命中光线的dir
    vec3 invDir; // 光线dir倒数
    vec3 pos;    // 命中位置
    vec3 normal; // 归一化世界法线
};

struct Ray {
    vec3 ori;
    vec3 dir;
};

struct Camera {
    float focalLength;
    vec3 position;
    vec3 lookAtCenter;
    float width;
    float height;
    float aspectRatio;
};

struct Sphere {
    vec3 center;
    float radius;
};

uniform Camera cam;

const float PI = 3.1415926;
const int bouanceLimit = 5;
Sphere spheres[3];
vec3 viewDir;
vec2 uv;

float random(vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

vec3 sampleCosineHemisphere(vec3 normal, vec2 seed) {
    // 1. 在一个单位圆盘内生成均匀随机点
    float r1 = 2.0 * PI * random(seed);
    float r2 = random(seed + vec2(1.0, 0.0));
    float r = sqrt(r2);

    float x = r * cos(r1);
    float y = r * sin(r1);
    float z = sqrt(1.0 - r2);

    // 2. 构建一个局部正交基
    // 这个基需要一个与 normal 垂直的向量。我们可以通过取叉积来实现。
    vec3 w = normal;
    vec3 u = normalize(cross(abs(w.y) > 0.9 ? vec3(1, 0, 0) : vec3(0, 1, 0), w));
    vec3 v = cross(w, u);

    // 3. 将随机生成的局部向量转换回世界空间
    // 这里的 (x, y, z) 是在局部空间下的余弦分布向量
    return normalize(u * x + v * y + w * z);
}

vec3 RayAt(in Ray ray ,in float t) {
    return ray.ori+t*ray.dir;
}
vec4 getBackgroundColor(vec3 dir) {
    vec3 unit_direction = normalize(dir);
    float a = 0.5f * (unit_direction.y + 1.0f);
    return vec4((1.0f - a) * vec3(1.0f, 1.0f, 1.0f) + a * vec3(0.5f, 0.7f, 1.0f), 1.0f);
}

void intersectSphere(in Ray ray,in vec3 center,in float radius,out HitInfos hitInfos) {
    vec3 oc = center - ray.ori;
    float a = dot(ray.dir, ray.dir);
    float b = -2.0f * dot(ray.dir, oc);
    float c = dot(oc, oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        hitInfos = HitInfos(-1.f,ray.ori,ray.dir,1/ray.dir,vec3(0.0f),vec3(0.0f));
    }
    else {
        float t = (-b - sqrt(discriminant)) / (2.0f * a);
        if (t < 0) {
            hitInfos = HitInfos(-1.f,ray.ori,ray.dir,1/ray.dir,vec3(0.0f),vec3(0.0f));
        }
        vec3 N = normalize(RayAt(ray,t) - center);
        vec3 dir = ray.dir;
        hitInfos = HitInfos(t,ray.ori,ray.dir,1/ray.dir,RayAt(ray,t),N);
    }
}

vec4 castRay(in Ray ray, int traceDepth) {
    vec4 color = vec4(0.0f);
    vec3 throughout = vec3(1.f);
    Ray tracingRay = ray;
    while(true) {
        if (traceDepth > bouanceLimit) {
            break;
        }
        traceDepth++;
        // 深度测试
        HitInfos closestHit;
        closestHit.t = 1.0f/0.0f;
        for (int i =0 ;i<3;++i) {
            HitInfos hitInfos;
            intersectSphere(tracingRay,spheres[i].center,spheres[i].radius,hitInfos);
            if (hitInfos.t>=0.0f && hitInfos.t < closestHit.t) {
                closestHit = hitInfos;
            }
        }
        // 命中
        if (closestHit.t != 1.0f/0.0f) {
            // color+= lambertianIrradiance(closestHit);
            throughout*=vec3(0.7f);
            vec3 rndDir = sampleCosineHemisphere(closestHit.normal,TexCoord*(rand+1.f));   
            tracingRay = Ray(closestHit.pos,rndDir);
            // color = vec4(1.0f,0.0f,0.0f,0.0f);
            continue;
        }
        // 未命中

        color.rgb+= throughout*getBackgroundColor(tracingRay.dir).rgb;
        break;
    }
    return color;
}

void initializeScene() {
    spheres[0] = Sphere(vec3(4.f, 2.f, 1.f), 1.f);
    spheres[1] = Sphere(vec3(-2.f, 3.f, 2.f), 2.f);
    spheres[2] = Sphere(vec3(4.f, -1e4f, 1.f), 1e4f);
}

void initialize() {
    uv = TexCoord;
    // cam.focalLength = 1.0f;
    // cam.lookAtCenter = vec3(0.0f);
    // cam.position = vec3(0.0f, 7.f, -8.f);
    // cam.aspectRatio = width/float(height);

    // cam.width = 2.f;
    // cam.height =  cam.width/cam.aspectRatio;
    viewDir = vec3(
        uv.x * cam.width - cam.width / 2.f,
        uv.y * cam.height - cam.height / 2.f,
        cam.focalLength);

    vec3 absY = vec3(0.f, 1.f, 0.f);
    vec3 z = normalize(cam.lookAtCenter - cam.position);
    vec3 x = normalize(cross(absY, z));
    vec3 y = cross(z, x);
    mat3 rotation = mat3(x, y, z);

    viewDir = rotation*(viewDir);

    initializeScene();
}

void main() {
    initialize();

    Ray ray = Ray(cam.position,viewDir);
    FragColor =(texture(lastSample,TexCoord)*float(samplesCount-1.f)+ castRay(ray,0))/float(samplesCount);

    // int nSamples = 16;
    // for(int i=0;i<nSamples;++i) {
    //     FragColor += castRay(ray,0)/float(nSamples);
    // }

    // FragColor.rgb = getBackgroundColor(viewDir).rgb;
}
