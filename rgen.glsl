

#version 460

#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(push_constant) uniform PushConstants {
  uint flag;
  uint frame;
}
pushConstants;

layout(rgba16f, set = 0, binding = 0) uniform image2D imageOutput;

layout(r32ui, set = 0, binding = 2) uniform uimage2D positionImage;

layout(set = 0, binding = 3, r8ui) uniform readonly uimage3D voxelData[512];
layout(set = 0, binding = 4, r16ui) uniform readonly uimage3D voxelChunkMap;

layout(set = 0, binding = 5, r32ui) uniform uimage2D lightImageX;
layout(set = 0, binding = 6, r32ui) uniform uimage2D lightImageY;
layout(set = 0, binding = 7, r32ui) uniform uimage2D lightImageZ;
layout(set = 0, binding = 8, r32ui) uniform uimage2D lightImageW;

vec3 rayOrigin = vec3(0);

layout(set = 0, binding = 1) uniform Transform {
  mat4 view;
  mat4 invproj;
}
transform;

//=================
//  Materials
//-----------------
#define MAT_AIR 0
#define MAT_STONE 1
#define MAT_STONE2 2
#define MAT_STONE3 3
#define MAT_GRASS 9
#define MAT_FLOWER 10
#define MAT_WATER 19
#define MAT_PLANT_CORE 20
// ================

float semiRandomFloat(ivec3 inp) {
  // A simple hashing function using the input vector components

  uint hash = inp.x * 123456789 + inp.y * 987654321 + inp.z * 567890123;

  // A bitwise operation to mix the hash value
  hash = (hash ^ (hash >> 21)) * 2654435761u;
  hash = hash ^ (hash >> 21);
  hash = hash * 668265263;
  hash = hash ^ (hash >> 21);

  // Return a float between 0 and 1 based on the hash
  return float(hash & 0xFFFFFFF) / float(0xFFFFFFF);
}

bool RayIntersectsCube(vec3 corner, float size, vec3 rayOrigin, vec3 rayDir,
                       out float tMin, out float tMax) {
  // The cube is defined from corner (0, 0, 0) to corner (size, size, size)
  vec3 minBound = corner;              // Minimum corner at (0,0,0)
  vec3 maxBound = corner + vec3(size); // Maximum corner at (size, size, size)

  vec3 invDir = 1.0 / rayDir;
  vec3 t1 = (minBound - rayOrigin) * invDir;
  vec3 t2 = (maxBound - rayOrigin) * invDir;

  vec3 tMinVec = min(t1, t2);
  vec3 tMaxVec = max(t1, t2);

  tMin = max(max(tMinVec.x, tMinVec.y), tMinVec.z);
  tMax = min(min(tMaxVec.x, tMaxVec.y), tMaxVec.z);

  return tMax >= tMin && tMax >= 0.0;
}

uint GetVoxel(ivec3 position) {
  // Replace division/modulo with bitwise ops (faster for powers of 2)
  ivec3 chunkPos = position >> 7; // position / 128
  uint chunkID = imageLoad(voxelChunkMap, chunkPos).r;
  ivec3 voxelPos = position & 127; // position % 128

  return imageLoad(voxelData[nonuniformEXT(chunkID)], voxelPos).r;
}
vec3 calculateVoxelNormal(ivec3 pos) {
  // Get material value (0 = air, nonzero = solid)
  uint cx = GetVoxel(pos) & 31;

  // If the center voxel is air, return zero normal
  if (cx == 0) {
    return vec3(0.0);
  }

  vec3 normal = vec3(0.0);
  float weightTotal = 0.0;

  // Loop through a 7x7x7 neighborhood (radius = 3)
  for (int x = -2; x <= 2; x++) {
    for (int y = -2; y <= 2; y++) {
      for (int z = -2; z <= 2; z++) {
        if (x == 0 && y == 0 && z == 0)
          continue; // Skip the center voxel

        ivec3 neighborPos = pos + ivec3(x, y, z);
        uint material = GetVoxel(neighborPos) & 31;
        float weight =
            1.0 /
            (1.0 + length(vec3(x, y, z))); // Closer neighbors contribute more

        if (material > 0) { // If solid, contribute to normal
          normal += weight * vec3(x, y, z);
          weightTotal += weight;
        }
      }
    }
  }

  // Normalize the result if there was any contribution
  return weightTotal > 0.0 ? normalize(normal) : vec3(0.0);
}

bool DistanceToCubeSurface(vec3 pointInside, vec3 direction, vec3 corner,
                           float size, out float distance) {
  // Define the minimum and maximum bounds of the cube
  direction = normalize(direction);
  vec3 minBound = corner;              // Minimum corner at (0, 0, 0)
  vec3 maxBound = corner + vec3(size); // Maximum corner at (size, size, size)

  // Calculate t values for each axis (distance to each face in the direction)
  vec3 t1 = (minBound - pointInside) / direction;
  vec3 t2 = (maxBound - pointInside) / direction;

  // Swap t1 and t2 if direction is negative (so t1 refers to the closer
  // intersection)
  vec3 tMinVec = min(t1, t2); // Smallest values for entry to the cube
  vec3 tMaxVec = max(t1, t2); // Largest values for exit from the cube

  // The distance is the smallest positive tMax where the direction is outward
  distance = max(max(tMinVec.x, tMinVec.y),
                 tMinVec.z); // Use the smallest positive tMax for the hit point

  // Check if the point is actually inside the cube and the direction is outward
  return (distance >= 0.0);
}

vec4 computer_ray_dir(uvec3 launchID, uvec3 launchSize) {
  float x = (2 * float(launchID.x) + 0.5) / float(launchSize.x) - 1.0;
  float y = (1.0 - 2 * (float(launchID.y) + 0.5) / float(launchSize.y));

  vec4 direction = transform.invproj * vec4(x, y, 1, 1);
  direction = normalize(direction);

  return direction;
}

vec3 getBounceDirection(vec3 normal, vec3 outgoingRay, int sampleIndex,
                        ivec2 seed, float roughness) {
  float alpha2 = roughness * roughness; // Precompute α²
  float u1 = semiRandomFloat(ivec3(seed, sampleIndex));
  float u2 = semiRandomFloat(ivec3(sampleIndex + 1, seed));

  // GGX Importance Sampling
  float cosTheta = sqrt((1.0 - u1) / (1.0 + (alpha2 - 1.0) * u1));
  float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
  float phi = 6.28318530718 * u2; // 2π * u2

  // Convert to world space using optimized orthonormal basis
  vec3 tangent =
      normalize(abs(normal.z) > 0.999 ? vec3(1.0, 0.0, 0.0)
                                      : cross(vec3(0.0, 0.0, 1.0), normal));
  vec3 bitangent = cross(normal, tangent);

  // Compute half-vector
  vec3 halfVector =
      normalize(tangent * cos(phi) * sinTheta +
                bitangent * sin(phi) * sinTheta + normal * cosTheta);

  // Compute final bounce direction
  return normalize(outgoingRay -
                   2.0 * dot(outgoingRay, halfVector) * halfVector);
}

const vec3 FOG_COLOR = vec3(0, 0, 0);
const vec3 FLOWER_COLRS[] = {vec3(0.8, 0.2, 0.2), vec3(0.0, 0.4, 0.2),
                             vec3(0.5, 0.5, 0.1), vec3(0.95, 0.95, 0.95)};

vec4 getColor(uint material, float d, vec3 hit) {
  vec3 c = vec3(0.0);
  switch (material) {
  case MAT_STONE:
    c = vec3(0.15) *
        (1.0 -
         (semiRandomFloat(ivec3(
              hit.z + (uint(semiRandomFloat(ivec3(hit / 2)) * 2)), 0, 0)) -
          0.5) *
             0.15);
    break;
  case MAT_STONE2:
    c = vec3(0.15) *
        (1.0 -
         (semiRandomFloat(ivec3(
              hit.z + (uint(semiRandomFloat(ivec3(hit / 3)) * 2)), 0, 0)) -
          0.5) *
             0.15);
    break;
  case MAT_STONE3:
    c = vec3(0.15) *
        (1.0 -
         (semiRandomFloat(ivec3(
              hit.z + (uint(semiRandomFloat(ivec3(hit / 4)) * 2)), 0, 0)) -
          0.5) *
             0.15);
    break;
  case MAT_GRASS:
    c = vec3(0.2, 0.5, 0) *
        (1.0 - (semiRandomFloat(ivec3(hit.z, 0, 0)) - 0.5) * 0.2);
    break;
  case MAT_FLOWER:
    c = FLOWER_COLRS[int(
        floor(min(semiRandomFloat(ivec3(hit.z, 0, 0)) * 4, 3.5)))];
    break;
  case MAT_WATER:
    c = vec3(0.0, 0.1, 0.5);
    break;
  case MAT_PLANT_CORE:
    c = vec3(0.4, 0.2, 0.6);
    break;
  default:
    break;
  }
  // if (material != MAT_AIR)
  // {
  //     c = mix(c, FOG_COLOR, clamp((d - 100.0) / 50.0, 0.0, 1.0));
  // }
  return vec4(c, 1.0);
}

struct Payload {
  vec3 hit;
  float dist;
  uint material;
  vec4 color;
};

bool rayBoxIntersect(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax,
                     out float tMin, out float tMax) {
  vec3 invDir = 1.0 / rayDir;
  vec3 t1 = (boxMin - rayOrigin) * invDir;
  vec3 t2 = (boxMax - rayOrigin) * invDir;
  tMin = max(max(min(t1.x, t2.x), min(t1.y, t2.y)), min(t1.z, t2.z));
  tMax = min(min(max(t1.x, t2.x), max(t1.y, t2.y)), max(t1.z, t2.z));
  return tMax >= tMin && tMax >= 0.0;
}

Payload march(vec3 rayOrigin, vec3 rayDir, float maxDistance) {
  float tMax = 0, tMin = 0;

  ivec3 origin = ivec3(0);
  int size = 1024;

  if (rayOrigin.x > 0 && rayOrigin.y > 0 && rayOrigin.z > 0) {
    origin = ivec3(rayOrigin);
    size = 1;
  }

  uint value = GetVoxel(ivec3(origin));
  uint layer = ((value >> 5) & 7);
  uint material = value & 31;

  vec3 hit = rayOrigin;
  if (RayIntersectsCube((origin / uint(pow(2, layer))) * uint(pow(2, layer)),
                        uint(pow(2, layer)), rayOrigin, rayDir, tMin, tMax)) {
    float distanceToBox = tMin;
    float rayDistance = distanceToBox;

    if (rayOrigin.x > 0 && rayOrigin.y > 0 && rayOrigin.z > 0) {
      distanceToBox = abs(tMax);
    }
    hit += rayDir * distanceToBox; // distanceToBox;
    hit += rayDir * 0.005;

    value = GetVoxel(ivec3(hit));
    layer = ((value >> 5) & 7);
    material = value & 31;

    if (material != MAT_AIR) {
      Payload p;
      p.dist = rayDistance;
      p.hit = hit;
      p.color = getColor(material, length(rayOrigin - hit), hit);
      p.material = material;
      return p;
    }

    for (int i = 0; i < 500; i++) {
      RayIntersectsCube((ivec3(hit) / uint(pow(2, layer))) *
                            uint(pow(2, layer)),
                        uint(pow(2, layer)), rayOrigin, rayDir, tMin, tMax);
      distanceToBox = abs(tMin - tMax);
      hit += rayDir * distanceToBox;
      rayDistance += distanceToBox;

      value = GetVoxel(ivec3(hit));
      layer = ((value >> 5) & 7);
      material = value & 31;

      if (material != MAT_AIR) {
        Payload p;
        p.dist = rayDistance;
        p.hit = hit;
        p.color = getColor(material, length(rayOrigin - hit), hit);
        p.material = material;
        return p;
      }

      if (rayDistance > maxDistance)
        break;
    }
  }
  Payload p;
  p.hit = hit;
  p.material = 0;
  p.color = vec4(FOG_COLOR, 1.0);
  p.dist = 200;
  return p;
}

vec3 bilinearInterpolation(vec3 C00, vec3 C10, vec3 C01, vec3 C11, vec2 uv) {
  vec3 C0 = mix(C00, C10, uv.x);
  vec3 C1 = mix(C01, C11, uv.x);
  return mix(C0, C1, uv.y);
}

vec3 interpolateColor(vec3 c1, vec3 c2, vec3 c3, vec3 c4, float d1, float d2,
                      float d3, float d4) {
  // Prevent division by zero by clamping distances
  d1 = max(d1, 0.0001);
  d2 = max(d2, 0.0001);
  d3 = max(d3, 0.0001);
  d4 = max(d4, 0.0001);

  // Compute weights using inverse square distance (IDW with p=2)
  float w1 = 1.0 / (d1 * d1);
  float w2 = 1.0 / (d2 * d2);
  float w3 = 1.0 / (d3 * d3);
  float w4 = 1.0 / (d4 * d4);

  // Normalize weights
  float sumW = w1 + w2 + w3 + w4;
  w1 /= sumW;
  w2 /= sumW;
  w3 /= sumW;
  w4 /= sumW;

  // Compute final interpolated color
  return c1 * w1 + c2 * w2 + c3 * w3 + c4 * w4;
}

ivec2 worldToPixel(vec3 worldPos, mat4 view, mat4 proj, ivec2 screenSize) {
  // Transform world position to clip space
  view = inverse(view);
  proj = inverse(proj);
  vec4 clipSpacePos = proj * view * vec4(worldPos, 1.0);

  // Handle w = 0 case to prevent errors
  if (abs(clipSpacePos.w) < 0.0001) {
    return ivec2(-1, -1); // Return invalid coordinates
  }

  // Perform perspective divide to get Normalized Device Coordinates (NDC)
  vec3 ndc = clipSpacePos.xyz / clipSpacePos.w;

  // Convert NDC (-1 to 1) to screen space (0 to screenSize)
  ivec2 pixelCoords = ivec2(((ndc.xy + 1.0) * 0.5) * screenSize);
  return (pixelCoords / 2) + screenSize / 4;
}

void main() {

  ivec2 pixelCoord = ivec2(gl_LaunchIDEXT.xy);

  // vec2 ndc = (vec2(pixelCoord) / gl_LaunchSizeEXT.xy) * 2.0 - 1.0;
  rayOrigin = -transform.view[3].xyz;

  int pixelSpread = 14;
  vec4 rayWorldSpace;
  if (pushConstants.flag == 13) {
    rayWorldSpace = computer_ray_dir(
        (gl_LaunchIDEXT.xyz / pixelSpread) * pixelSpread, gl_LaunchSizeEXT.xyz);
  } else {
    rayWorldSpace = computer_ray_dir(gl_LaunchIDEXT.xyz, gl_LaunchSizeEXT.xyz);
  }
  // rayWorldSpace /= rayWorldSpace.w;
  mat4 copy = transform.view;
  copy[3] = vec4(0.0, 0.0, 0.0, 1.0);
  vec3 rayDir = normalize((copy * rayWorldSpace).xyz);

  if (pushConstants.flag == 0) {

    uint z = 0;
    imageStore(lightImageX, pixelCoord, ivec4(0));
    imageStore(lightImageY, pixelCoord, ivec4(0));
    imageStore(lightImageZ, pixelCoord, ivec4(0));
    imageStore(lightImageW, pixelCoord, ivec4(0));

  } else if (pushConstants.flag == 1) {
    // if (mod(pixelCoord.x, pixelSpread) == 0 && mod(pixelCoord.y, pixelSpread)
    // == 0)
    // {

    Payload p = march(rayOrigin, rayDir, 200.0);
    vec3 hit = p.hit;
    vec4 color1 = p.color;
    float distance1 = p.dist;
    vec3 normal = calculateVoxelNormal(ivec3(hit));

    vec3 contribution = vec3(0);
    const int sampleCount = 5;

    ivec2 screenPos =
        worldToPixel(-ivec3(hit), transform.view, transform.invproj,
                     ivec2(gl_LaunchSizeEXT.xy));
    uint num = imageAtomicAdd(positionImage, screenPos, 1);

    vec3 reflectDirection = reflect(
        rayOrigin - hit, normal); // Already normalized if inputs are normalized
    vec3 baseBounceDirection =
        normalize(-reflectDirection); // Avoid recalculating in loop

    for (int i = 0; i < sampleCount; i++) {
      int sampleIndex = int(num) * sampleCount + i;

      vec3 bounceDirection =
          getBounceDirection(normal, baseBounceDirection, sampleIndex,
                             ivec2(hit.x, hit.y + hit.z), 0.5);

      p = march(ivec3(hit) + bounceDirection, bounceDirection, 100.0);
      vec4 bounceColor = p.color;
      float distance2 = p.dist;
      uint material = p.material;

      // Reduce redundant operations
      vec3 hit2 = p.hit;
      ivec3 hitMod = ivec3(mod(hit, 128.0));

      // Store sparse image only once per loop iteration
      imageStore(positionImage, (pixelCoord / pixelSpread), ivec4(hitMod, 1));

      // Compute light contribution efficiently
      float attenuation = exp(-0.01 * distance2);
      float angularFactor = max(dot(rayDir, bounceDirection), 0.0);
      // float materialFactor = (material == MAT_GRASS) ? 8.0 : (material ==
      // MAT_PLANT_CORE) ? 500.0 : 1.0;

      vec3 lightContribution =
          (bounceColor.rgb * attenuation * angularFactor) / float(sampleCount);
      contribution += lightContribution;
    }
    uvec4 c = uvec4(contribution.xyz * 256, 1);
    imageAtomicAdd(lightImageX, screenPos, c.x);
    imageAtomicAdd(lightImageY, screenPos, c.y);
    imageAtomicAdd(lightImageZ, screenPos, c.z);
    imageAtomicAdd(lightImageW, screenPos, 1);
  } else if (pushConstants.flag == 2) {
    Payload p = march(rayOrigin, rayDir, 200.0);
    vec3 hit = p.hit;
    float distance1 = p.dist;
    vec4 color1 = p.color;
    uint material = p.material;

    vec3 light5 = vec3(0);
    float divide = 0;

    ivec2 screenPos =
        worldToPixel(-ivec3(hit), transform.view, transform.invproj,
                     ivec2(gl_LaunchSizeEXT.xy));
    vec4 light1 = vec4(uint(imageLoad(lightImageX, screenPos)),
                       uint(imageLoad(lightImageY, screenPos)),
                       uint(imageLoad(lightImageZ, screenPos)),
                       uint(imageLoad(lightImageW, screenPos)));

    vec3 lightValue = (light1.xyz / 256.0) / light1.w;
    light5 += lightValue;

    float lightContributionMultiplier = 1.0;
    if (material == MAT_GRASS) {
      lightContributionMultiplier = 0.3;
    } else if (material == MAT_PLANT_CORE) {
      lightContributionMultiplier = 0.1;
    }

    vec3 light = lightContributionMultiplier * light5 * vec3(color1);
    vec4 color =
        vec4(mix(light, FOG_COLOR,
                 clamp((length(rayOrigin - hit) - 100.0) / 75.0, 0.0, 1.0)),
             1.0);
    vec4 finalColor = color;

    imageStore(imageOutput, pixelCoord, finalColor);
    imageStore(positionImage, pixelCoord, ivec4(0));
  }
}