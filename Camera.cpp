#include "Camera.hpp"
#include "ResourceManager.hpp"
#include "VoxelWorld.hpp"
#include "VulkanContext.hpp"
#include "WindowManager.hpp"
#include <cstring>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <memory>
#include <stdexcept>

Camera::Camera(std::unique_ptr<VulkanContext>& vulkanContext) 
{
    uniformBuffer.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDeviceSize bufferSize = sizeof(TransformUBO);

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(vulkanContext->getDevice(), &bufferInfo, nullptr, &uniformBuffer[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create uniform buffer");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(vulkanContext->getDevice(), uniformBuffer[i], &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = ResourceManager::findMemoryType(vulkanContext->getPhysicalDevice(), memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(vulkanContext->getDevice(), &allocInfo, nullptr, &uniformBufferMemory[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate uniform buffer memory!");
        }

        vkBindBufferMemory(vulkanContext->getDevice(), uniformBuffer[i], uniformBufferMemory[i], 0);


        vkMapMemory(vulkanContext->getDevice(), uniformBufferMemory[i], 0, sizeof(TransformUBO), 0, &uniformBuffersMapped[i]);
        memcpy(uniformBuffersMapped[i], &ubo, sizeof(TransformUBO));
        ubo.view = glm::mat4(1.0);
        ubo.proj = glm::perspective(glm::radians(70.0f), vulkanContext->getSwapChainExtent().width / (float)vulkanContext->getSwapChainExtent().height, 0.1f, 1000.0f);
        ubo.proj[1][1] *= -1;
        ubo.proj = glm::inverse(ubo.proj);
    }
}
Camera::~Camera() {}
void Camera::update(std::unique_ptr<WindowManager>& windowManager, std::unique_ptr<VoxelWorld>& voxelWorld, int currentFrame)
{
     struct {
            float& pos;
            int lowerBound, upperBound, offset, modValue, shift;
        } checks[] = {
            {cameraPosition.x, -640, -340, 128, 8, 1},
            {cameraPosition.y, -640, -340, 128, 64, 8},
            {cameraPosition.z, -640, -340, 128, 512, 64}
        };
        
        cameraPosition.z += cameraVelocity.z * windowManager->getDeltaTime();
        cameraTargetPoint.z += cameraVelocity.z * windowManager->getDeltaTime();;

        glm::ivec3 intCameraPosition = glm::ivec3(cameraPosition) * -1;

        #define voxChunk(chunk, x,y,z) chunk[((z) % 128) * 128 * 128 + ((y) % 128) * 128 + ((x) % 128)]
        // floor
        bool is_grounded = false;
        for (int x = -1; x < 2; x++)
        {
            for (int y = -1; y < 2; y++)
            {
                uint16_t chunkID = ((intCameraPosition.z + 15) / 128) * 64 + ((intCameraPosition.y + y) / 128) * 8 + ((intCameraPosition.x + x) / 128);
                is_grounded |= (MAT_HAS_COLLISION(voxel_mat(voxChunk(voxelWorld->voxelData[voxelWorld->voxelChunkMapData[chunkID]].data, intCameraPosition.x + x, intCameraPosition.y + y, intCameraPosition.z + 15)))); //MAT_HAS_COLLISION(voxel_mat(vox(intCameraPosition.x + x, intCameraPosition.y + y, intCameraPosition.z + 30)));
            }
        }
        printf("Got here %d\n", 2);
        // // ceiling
        bool is_ouch = false;
        // for (int x = -1; x < 2; x++)
        // {
        //     for (int y = -1; y < 2; y++)
        //     {
        //         is_ouch |= MAT_HAS_COLLISION(voxel_mat(vox(intCameraPosition.x + x, intCameraPosition.y + y, intCameraPosition.z - 6)));
        //     }
        // }
     
        if (is_grounded)
        {
            cameraVelocity.z = 0;
        }
        else if (is_ouch)
        {
            cameraVelocity.z = -fabsf(cameraVelocity.z) * 0.8;
        }
        else
        {
            cameraVelocity.z -= 20 * 7 * windowManager->getDeltaTime();;
        }

        if (windowManager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            // Hide the cursor and capture mouse movement

            
            windowManager->hideCursor();

            float deltaTime = 0.1f;

            movementSpeed = windowManager->isKeyPressed(GLFW_KEY_LEFT_SHIFT) ? 11.0 : 7.0;
            bool reset = false;
            if (windowManager->isKeyPressed(GLFW_KEY_S)) { reset = true; cameraPosition += movementSpeed * deltaTime * glm::normalize(cameraTargetPoint - cameraPosition) * glm::vec3(1, 1, 0); }
            if (windowManager->isKeyPressed(GLFW_KEY_W)) { reset = true; cameraPosition -= movementSpeed * deltaTime * glm::normalize(cameraTargetPoint - cameraPosition) * glm::vec3(1, 1, 0); }
            if (windowManager->isKeyPressed(GLFW_KEY_D)) { reset = true; cameraPosition -= glm::normalize(glm::cross(cameraTargetPoint - cameraPosition, glm::vec3(0.0f, 0.0f, 1.0f))) * movementSpeed * deltaTime * glm::vec3(1, 1, 0); }
            if (windowManager->isKeyPressed(GLFW_KEY_A)) { reset = true; cameraPosition += glm::normalize(glm::cross(cameraTargetPoint - cameraPosition, glm::vec3(0.0f, 0.0f, 1.0f))) * movementSpeed * deltaTime * glm::vec3(1, 1, 0); }

            // Q and E for vertical movement
            if (windowManager->isKeyPressed(GLFW_KEY_Q)) { cameraPosition.z -= movementSpeed * 2 * deltaTime; }
            if ((windowManager->isKeyPressed(GLFW_KEY_SPACE) || windowManager->isKeyPressed(GLFW_KEY_E)) && is_grounded)
            {
                cameraVelocity.z += 600 * deltaTime;
            }

            if (cameraVelocity.length() != 0 || !is_grounded)
            {
                reset = true;
            }

            double currentMouseX, currentMouseY;
            windowManager->getCursorPos(&currentMouseX, &currentMouseY);

            if (firstMouse) {
                lastMouseX = static_cast<float>(currentMouseX);
                lastMouseY = static_cast<float>(currentMouseY);
                firstMouse = false;
            }

            // Calculate the offset
            float offsetX = static_cast<float>(currentMouseX - lastMouseX);
            float offsetY = static_cast<float>(lastMouseY - currentMouseY);  // Reverse Y for correct orientation
            lastMouseX = static_cast<float>(currentMouseX);
            lastMouseY = static_cast<float>(currentMouseY);

            // Apply sensitivity
            offsetX *= mouseSensitivity;
            offsetY *= mouseSensitivity;

            yaw -= offsetX;
            pitch -= offsetY;

               if (reset || offsetX != 0 || offsetY != 0)
                {
                    for (int i = 0; i < frame.size(); i++)
                    {
                        frame[i] = 0;
                    }
                }


            // Constrain pitch to prevent flipping
            if (pitch > 89.0f) pitch = 89.0f;
            if (pitch < -89.0f) pitch = -89.0f;

            // Update the camera target based on yaw and pitch
            glm::vec3 direction;
            direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction.z = sin(glm::radians(pitch));
            direction.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            direction = glm::normalize(direction);

            cameraTargetPoint = cameraPosition + direction;
        } else {
            // Release the cursor and reset first mouse flag
            windowManager->showCursor();
            firstMouse = true;
        }




        ubo.view = glm::inverse(glm::lookAt(cameraPosition, cameraTargetPoint, glm::vec3(0.0f, 0.0f, 1.0f)));

        printf("\rPOSITION: %i %i %i\tCHUNK: %i %i %i\tDELTA TIME: %f\to: %i", (int)cameraPosition.x, (int)cameraPosition.y, (int)cameraPosition.z, chunkPosition.x, chunkPosition.y, chunkPosition.z, windowManager->getDeltaTime(), is_ouch);


        

        for (auto& check : checks) {
            if (check.pos < check.lowerBound) {
                check.pos += check.offset;
                voxelWorld->updateVoxelChunkMap(check.modValue, check.shift);
            } else if (check.pos > check.upperBound) {
                check.pos -= check.offset;
                voxelWorld->updateVoxelChunkMap(check.modValue, check.modValue - check.shift);
            }
        }

        voxelWorld->sortChunks();

        // Raycast and print distance
        if (windowManager->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
        {
            bool hit = false;
            glm::ivec3 hitPosition = rayCast(voxelWorld,-cameraPosition, cameraTargetPoint - cameraPosition, hit, 25.0f);
            if (hit)
            {   
                printf("HIT: %i, %i %i %i\n", hit, hitPosition.x, hitPosition.y, hitPosition.z);
                voxelWorld->getChunk(hitPosition)[((int)hitPosition.z % 128) * 128*128 + ((int)hitPosition.y % 128) * 128 + ((int)hitPosition.x % 128)] = 0;

                for (int x = -4; x <= 4; x++)
                {
                    for (int y = -4; y <= 4; y++)
                    {
                        for (int z = -4; z <= 4; z++)
                        {
                            glm::ivec3 offset = glm::ivec3(x, y, z);
                            if (voxel_mat(voxelWorld->getChunk(hitPosition + offset)[(((int)hitPosition.z + z) % 128) * 128*128 + (((int)hitPosition.y + y) % 128) * 128 + (((int)hitPosition.x + x) % 128)]) != MAT_AIR)
                            voxelWorld->getChunk(hitPosition + offset)[(((int)hitPosition.z + z) % 128) * 128*128 + (((int)hitPosition.y + y) % 128) * 128 + (((int)hitPosition.x + x) % 128)] = 0;
                        }
                    }
                }
                
                uint16_t chunkID = voxelWorld->getChunkID(hitPosition);
                
                voxelWorld->chunkUpdateQueue[++voxelWorld->chunkUpdateQueue[0]] = chunkID;
            }
        }

        memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(TransformUBO));

}
    
    void Camera::onMouseMove(double xPos, double yPos) {
        mouseX = (float)xPos;
        mouseY = (float)yPos;
    }
    bool Camera::rayIntersectsCube(glm::vec3 corner, float size, glm::vec3 rayOrigin, glm::vec3 rayDir, float& tMin, float& tMax)
    {
        glm::vec3 sizeVec = glm::vec3{size};
        glm::vec3 minBound = corner;
        glm::vec3 maxBound = corner + sizeVec;

        glm::vec3 invDir = 1.0f / rayDir;
        glm::vec3 t1 = (minBound - rayOrigin) * invDir;
        glm::vec3 t2 = (maxBound - rayOrigin) * invDir;

        glm::vec3 tMinVec = glm::min(t1, t2);
        glm::vec3 tMaxVec = glm::max(t1, t2);

        tMin = std::max(std::max(tMinVec.x, tMinVec.y), tMinVec.z);
        tMax = std::min(std::min(tMaxVec.x, tMaxVec.y), tMaxVec.z);

        return tMax >= tMin && tMax >= 0;
    }

glm::ivec3 Camera::rayCast(std::unique_ptr<VoxelWorld>& voxelWorld, glm::vec3 rayOrigin, glm::vec3 rayDir, bool& something_hit, float maxDistance)
    {
        float tMax = 0, tMin = 0;

        glm::ivec3 origin = glm::ivec3(rayOrigin);

        uint8_t value = voxelWorld->getChunk(origin)[((int)origin.z % 128) * 128*128 + ((int)origin.y % 128) * 128 + ((int)origin.x % 128)];
        uint8_t layer = ((value >> 5) & 7);
        uint8_t material = value & 31;

        if (rayIntersectsCube((origin / (int)std::pow(2, layer)) * (int)std::pow(2, layer), std::pow(2, layer), rayOrigin, rayDir, tMin, tMax))
        {
            glm::vec3 hit = rayOrigin;
            float distanceToBox = tMin;
            float rayDistance = distanceToBox;

            distanceToBox = std::abs(tMax);
            hit += rayDir * distanceToBox;
            hit += rayDir * 0.001f;

            value = voxelWorld->getChunk(hit)[((int)hit.z % 128) * 128*128 + ((int)hit.y % 128) * 128 + ((int)hit.x % 128)];
            layer = ((value >> 5) & 7);
            material = value & 31;

            if (material != MAT_AIR)
            {
                printf("HIT: %i\n", material);
                something_hit = true;
                return hit;
            }

            for (int i = 0; i < 200; i++)
            {
                rayIntersectsCube((glm::ivec3(hit) / (int)std::pow(2, layer)) * (int)std::pow(2, layer), std::pow(2, layer), rayOrigin, rayDir, tMin, tMax);
                distanceToBox = std::abs(tMin - tMax);
                hit += rayDir * distanceToBox;
                rayDistance += distanceToBox;

                value = voxelWorld->getChunk(hit)[((int)hit.z % 128) * 128*128 + ((int)hit.y % 128) * 128 + ((int)hit.x % 128)];
                layer = ((value >> 5) & 7);
                material = value & 31;

                if (material != MAT_AIR)
                {
                    printf("HIT: %i\n", material);
                    something_hit = true;
                    return hit;
                }

                if (rayDistance > maxDistance) break;
            }
            something_hit = false;
            return hit;
        }
        something_hit = false;
        return glm::vec3(0);
    }

void Camera::processMouseMovement(float xoffset, float yoffset) {}
void Camera::processKeyboardInput(int key, float deltaTime) {}
glm::mat4 Camera::getViewMatrix() const { return glm::mat4(1.0f); }
glm::vec3 Camera::getPosition() const { return glm::vec3(0.0f); }
glm::vec3 Camera::getTarget() const { return glm::vec3(0.0f); }
float Camera::getYaw() const { return 0.0f; }
float Camera::getPitch() const { return 0.0f; } 