// This has been adapted from the Vulkan tutorial

#include "MyProject.hpp"

//const std::string MODEL_PATH = "Assets/models/Boat.obj";
//const std::string TEXTURE_PATH = "Assets/textures/Boat.bmp";

// The uniform buffer object used in this example
struct GlobalUniformBufferObject {
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct UniformBufferObject {
	alignas(16) glm::mat4 model;
};

struct RockObject {
	DescriptorSet ds;
	glm::vec3 currentPos;
};

struct BoatObject {
	Model model;
	Texture texture;
	DescriptorSet ds;
	glm::vec3 currentPos = glm::vec3(0.f);
};

struct LandscapeObject {
	DescriptorSet grassDs;
	DescriptorSet waterDs;
	float currentPosX = -15.f;
};

const int maxNumberRock = 10;
const int maxNumberLandscape = 5;
const float distanceBetweenRocksX = 10.f;
const float distanceBetweenRocksZ = 4.f;
bool stillPlaying = true;



// MAIN ! 
class MyProject : public BaseProject {
	protected:
	// Here you list all the Vulkan objects you need:
	
	// Descriptor Layouts [what will be passed to the shaders]
	DescriptorSetLayout DSLglobal;
	DescriptorSetLayout DSLobj;

	// Pipelines [Shader couples]
	Pipeline P1;

	// Models, textures and Descriptors (values assigned to the uniforms)
	BoatObject boatObject;

	Model GrassModel;
	Texture GrassTexture;

	Model WaterModel;
	Texture WaterTexture;

	Model Rock1Model;
	Texture Rock1Texture;
	std::vector<RockObject> rockObjects;

	std::vector<LandscapeObject> landscapeObjects;

	DescriptorSet DSglobal;
	
	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 2560;
		windowHeight = 1440;
		windowTitle = "My Project";
		initialBackgroundColor = {51.f, 153.f, 255.f, 1.f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 3 + maxNumberRock + maxNumberLandscape * 2;
		texturesInPool = 2 + maxNumberRock + maxNumberLandscape * 2;
		setsInPool = 3 + maxNumberRock + maxNumberLandscape * 2;

		std::srand(std::time(nullptr));
	}
	
	// Here you load and setup all your Vulkan objects
	void localInit() {
		// Descriptor Layouts [what will be passed to the shaders]
		DSLobj.init(this, {
					// this array contains the binding:
					// first  element : the binding number
					// second element : the time of element (buffer or texture)
					// third  element : the pipeline stage where it will be used
					{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT},
					{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT}
				  });

		DSLglobal.init(this, {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS}
			});

		// Pipelines [Shader couples]
		// The last array, is a vector of pointer to the layouts of the sets that will
		// be used in this pipeline. The first element will be set 0, and so on..
		P1.init(this, "shaders/vert.spv", "shaders/frag.spv", {&DSLglobal, &DSLobj});

		// Models, textures and Descriptors (values assigned to the uniforms)
		boatObject.model.init(this, "models/Boat.obj");
		boatObject.texture.init(this, "textures/Boat.bmp");
		boatObject.ds.init(this, &DSLobj, {
		// the second parameter, is a pointer to the Uniform Set Layout of this set
		// the last parameter is an array, with one element per binding of the set.
		// first  elmenet : the binding number
		// second element : UNIFORM or TEXTURE (an enum) depending on the type
		// third  element : only for UNIFORMs, the size of the corresponding C++ object
		// fourth element : only for TEXTUREs, the pointer to the corresponding texture object
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &boatObject.texture}
		});

		
		/* INITIALIZING MODEL AND TEXTURE OF GRASS AND WATER + INITIALIZING THE DESCRIPTIVE SET AND POSITION */
		WaterModel.init(this, "models/Water.obj");
		WaterTexture.init(this, "textures/Water.png");
		GrassModel.init(this, "models/Grass.obj");
		GrassTexture.init(this, "textures/Grass.png");
		landscapeObjects.resize(maxNumberLandscape);
		float i = 10.f;
		for (auto& obj : landscapeObjects) {
			obj.waterDs.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &WaterTexture}
				});
			obj.grassDs.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &GrassTexture}
				});
			obj.currentPosX += i;
			i += 10.f;
		}


		/* INITIALIZING MODEL AND TEXTURE OF ROCK1 + INITIALIZING THE DESCRIPTIVE SET */
		Rock1Model.init(this, "models/Rock1.obj");
		Rock1Texture.init(this, "textures/Rock1.png");
		rockObjects.resize(maxNumberRock);
		i = 15.f;
		int even = 1;
		for (auto &obj : rockObjects) {
			obj.ds.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &Rock1Texture}
				});

			/* INITIALIZING THE POSITION OF THE ROCKS */
			obj.currentPos = glm::vec3(i, 0.f, (std::rand()%5 -2)*distanceBetweenRocksZ);
			if ((even % 2) == 0) {
				i += distanceBetweenRocksX;
			}
			even++;
		}

		/*-----------------------------------------------------*/

		/*Descriptor set global*/
		DSglobal.init(this, &DSLglobal, {
						{0, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
		});
	}

	// Here you destroy all the objects you created!		
	void localCleanup() {
		boatObject.ds.cleanup();
		boatObject.texture.cleanup();
		boatObject.model.cleanup();

		for (auto &obj : rockObjects) {
			obj.ds.cleanup();
		}

		for (auto& obj : landscapeObjects) {
			obj.grassDs.cleanup();
			obj.waterDs.cleanup();
		}

		Rock1Texture.cleanup();
		Rock1Model.cleanup();

		GrassTexture.cleanup();
		GrassModel.cleanup();

		DSglobal.cleanup();

		P1.cleanup();
		DSLglobal.cleanup();
		DSLobj.cleanup();
	}
	
	// Here it is the creation of the command buffer:
	// You send to the GPU all the objects you want to draw,
	// with their buffers and textures
	void populateCommandBuffer(VkCommandBuffer commandBuffer, int currentImage) {
				
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, P1.graphicsPipeline);
				
		//Binding the global descriptor set
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 0, 1, &DSglobal.descriptorSets[currentImage],
			0, nullptr);

		/*
		* Creating the buffer and the command for the boat
		*/
		VkBuffer vertexBuffers[] = { boatObject.model.vertexBuffer};
		// property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, boatObject.model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						P1.pipelineLayout, 1, 1, &boatObject.ds.descriptorSets[currentImage],
						0, nullptr);
						
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(boatObject.model.indices.size()), 1, 0, 0, 0);


		/* Creating the buffer and the Command for the RIVER*/

		VkBuffer vertexBuffers4[] = { GrassModel.vertexBuffer };
		VkDeviceSize offsets4[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers4, offsets4);
		vkCmdBindIndexBuffer(commandBuffer, GrassModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		for (auto& obj : landscapeObjects) {

			vkCmdBindDescriptorSets(commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				P1.pipelineLayout, 1, 1, &obj.grassDs.descriptorSets[currentImage],
				0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(GrassModel.indices.size()), 1, 0, 0, 0);

		}


		VkBuffer vertexBuffers1[] = { WaterModel.vertexBuffer };
		VkDeviceSize offsets1[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers1, offsets1);
		vkCmdBindIndexBuffer(commandBuffer, WaterModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		for (auto& obj : landscapeObjects) {

			vkCmdBindDescriptorSets(commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				P1.pipelineLayout, 1, 1, &obj.waterDs.descriptorSets[currentImage],
				0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(WaterModel.indices.size()), 1, 0, 0, 0);

		}
		


		/* CREATING THE BUFFER AND THE COMMAND FOR THE ROCKS */

		VkBuffer vertexBuffers2[] = { Rock1Model.vertexBuffer };
		VkDeviceSize offsets2[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers2, offsets2);
		vkCmdBindIndexBuffer(commandBuffer, Rock1Model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		for (auto& obj : rockObjects) {

			vkCmdBindDescriptorSets(commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				P1.pipelineLayout, 1, 1, &obj.ds.descriptorSets[currentImage],
				0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Rock1Model.indices.size()), 1, 0, 0, 0);
		}

		/*---------------------------------------------------*/

		
	
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		
		/* UPDATE THE GLOBAL UBO */

		updateGlobalUBO(currentImage);

		/*---------------------------------------------------------------------*/

		/*UBO FOR THE BOAT*/

			updateBoat(currentImage);

		/*---------------------------------------------------------------------*/


		/* UBO FOR THE ROCKS */

		updateRocks(currentImage);

		/*---------------------------------------------------------------------*/

		/* UBO FOR THE GRASS AND WATER */

		updateLandscapes(currentImage);

		/*---------------------------------------------------------------------*/
		
		
	}	

	void updateGlobalUBO(uint32_t currentImage) {

		/*Creating the Global UBO and copy the data to the GPU*/
		GlobalUniformBufferObject gubo{};

		gubo.view = glm::lookAt(glm::vec3(-5.0f + boatObject.currentPos.x, 10.0f, 0.0f),
			glm::vec3(boatObject.currentPos.x, 0.f, 0.f),
			glm::vec3(0.0f, 1.0f, 0.0f));

		gubo.proj = glm::perspective(glm::radians(90.0f),
			swapChainExtent.width / (float)swapChainExtent.height,
			0.1f, 100.0f);
		gubo.proj[1][1] *= -1;

		void* data;

		vkMapMemory(device, DSglobal.uniformBuffersMemory[0][currentImage], 0, sizeof(gubo), 0, &data);
		memcpy(data, &gubo, sizeof(gubo));
		vkUnmapMemory(device, DSglobal.uniformBuffersMemory[0][currentImage]);

	}

	void updateLandscapes(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		/*UBO for the River*/
		for (auto& obj : landscapeObjects) {
			//std::cout << obj.currentPosX << "    " << boatObject.currentPosX << "\n";
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(obj.currentPosX, 0.f, 0.f)) * glm::scale(glm::mat4(1.f), glm::vec3(0.05f, 0.05f, 0.05f));
			
			vkMapMemory(device, obj.grassDs.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(device, obj.grassDs.uniformBuffersMemory[0][currentImage]);

			vkMapMemory(device, obj.waterDs.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(device, obj.waterDs.uniformBuffersMemory[0][currentImage]);

		}

		for (auto& obj : landscapeObjects) {
			if (boatObject.currentPos.x > obj.currentPosX + 15.f) {
				obj.currentPosX = obj.currentPosX + (maxNumberLandscape * 10.f);

				ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(obj.currentPosX, 0.f, 0.f)) * glm::scale(glm::mat4(1.f), glm::vec3(0.05f, 0.05f, 0.05f));

				vkMapMemory(device, obj.grassDs.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
				memcpy(data, &ubo, sizeof(ubo));
				vkUnmapMemory(device, obj.grassDs.uniformBuffersMemory[0][currentImage]);

				vkMapMemory(device, obj.waterDs.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
				memcpy(data, &ubo, sizeof(ubo));
				vkUnmapMemory(device, obj.waterDs.uniformBuffersMemory[0][currentImage]);
			}
		}



	}

	void updateRocks(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;
		
		/*
			Size:
			x = 10 * 0.2 = 2
			z = 7 * 0.5 = 3.5
		*/

		for (auto& obj : rockObjects) {
			//std::cout << obj.currentPosX << "\n";
			ubo.model = glm::translate(glm::mat4(1.0f), obj.currentPos) * glm::scale(glm::mat4(1.0), glm::vec3(0.2, 0.5, 0.5))
							* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
			vkMapMemory(device, obj.ds.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(device, obj.ds.uniformBuffersMemory[0][currentImage]);
		}

		for (auto& obj : rockObjects) {
			if (boatObject.currentPos.z +1.f < obj.currentPos.z + 2.8f && boatObject.currentPos.z - 1.f > obj.currentPos.z - 2.8f && 
					(boatObject.currentPos.x + 2.f > obj.currentPos.x - 1.f && boatObject.currentPos.x - 2.4f < obj.currentPos.x + 1.f) ) {
				stillPlaying = false;
			}
			if (boatObject.currentPos.x > obj.currentPos.x + 10.f) {
				obj.currentPos = glm::vec3(obj.currentPos.x + distanceBetweenRocksX * (maxNumberRock/2), 0.f, (std::rand() % 5 - 2) * distanceBetweenRocksZ);
				ubo.model = glm::translate(glm::mat4(1.f), obj.currentPos) * ubo.model;
				vkMapMemory(device, obj.ds.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
				memcpy(data, &ubo, sizeof(ubo));
				vkUnmapMemory(device, obj.ds.uniformBuffersMemory[0][currentImage]);
			}
		}

	}

	void updateBoat(uint32_t currentImage) {

		static auto start = std::chrono::high_resolution_clock::now();
		static float last = 0.f;
		auto now = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(now - start).count();
		float delta = time - last;
		last = time;

		static glm::mat4 mat = glm::mat4(1.0f);
		static glm::vec3 pos = glm::vec3(1.0f, 0.f, 0.f);
		static float angle = glm::radians(0.f);

		void* data;

		UniformBufferObject ubo{};

		/* ALWAYS INCREMENTING THE X POSITION OF 5.f FOR MOVING STRAIGHT */
		if (glfwGetKey(this->window, GLFW_KEY_P))
			stillPlaying = true;
		if (stillPlaying) {
			pos += glm::vec3(5.0f, 0.0f, 0.0f) * delta;

			/* UPDATING FOR MOVING RIGHT OR LEFT */
			if (glfwGetKey(this->window, GLFW_KEY_D)) {
				angle = angle < glm::radians(-10.f) ? glm::radians(-10.f) : angle + glm::radians(-1.f);
				pos += glm::vec3(0.f, 0.f, 5.f) * delta;
				if (pos.z > 10.f - 1.f)
					pos.z = 9.f;
			}
			else if (glfwGetKey(this->window, GLFW_KEY_A)) {
				angle = angle > glm::radians(10.f) ? glm::radians(10.f) : angle + glm::radians(1.f);
				pos -= glm::vec3(0.f, 0.f, 5.f) * delta;
				if (pos.z < -10.f + 1.f)
					pos.z = -9.f;
			}
			else {
				if (angle > glm::radians(0.f))
					angle -= glm::radians(1.f);
				else if (angle < glm::radians(0.f))
					angle += glm::radians(1.f);
			}
		}

		
		


		ubo.model = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0), glm::vec3(0.005, 0.005, 0.005))
			* glm::rotate(glm::mat4(1.0f), static_cast<float>(glm::radians(180.f)), glm::vec3(0.f, 1.f, 0.f))
			* glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.f, 1.f, 0.f));

		boatObject.currentPos = pos;

		//std::cout << boatObject.currentPosX << "\n";

		vkMapMemory(device, boatObject.ds.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, boatObject.ds.uniformBuffersMemory[0][currentImage]);

	}
};



// This is the main: probably you do not need to touch this!
int main() {
    MyProject app;

    try {
        app.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}