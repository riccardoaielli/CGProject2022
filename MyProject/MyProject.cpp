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

const int maxNumberRock = 5;
const int maxNumberGrass = 5;


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
	Model BoatModel;
	Texture BoatTexture;

	Model Rock1Model;
	Texture Rock1Texture;

	Model GrassModel;
	Texture GrassTexture;

	Model WaterModel;
	Texture WaterTexture;

	DescriptorSet BoatDS;
	DescriptorSet Rock1DS;
	DescriptorSet WaterDS;

	std::vector<DescriptorSet> GrassDSVector;
	std::vector<DescriptorSet> Rock1DSVector;
	DescriptorSet DSglobal;
	
	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 2560;
		windowHeight = 1440;
		windowTitle = "My Project";
		initialBackgroundColor = {0.0f, 0.0f, 0.0f, 1.0f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 4 + maxNumberRock + maxNumberGrass + 1;
		texturesInPool = 3 + maxNumberRock + maxNumberGrass + 1;
		setsInPool = 4 + maxNumberRock + maxNumberGrass + 1;
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
		BoatModel.init(this, "models/Boat.obj");
		BoatTexture.init(this, "textures/Boat.bmp");
		BoatDS.init(this, &DSLobj, {
		// the second parameter, is a pointer to the Uniform Set Layout of this set
		// the last parameter is an array, with one element per binding of the set.
		// first  elmenet : the binding number
		// second element : UNIFORM or TEXTURE (an enum) depending on the type
		// third  element : only for UNIFORMs, the size of the corresponding C++ object
		// fourth element : only for TEXTUREs, the pointer to the corresponding texture object
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &BoatTexture}
		});

		Rock1Model.init(this, "models/Rock1.obj");
		Rock1Texture.init(this, "textures/Rock1.png");
		Rock1DS.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &Rock1Texture}
		});

		WaterModel.init(this, "models/Water.obj");
		WaterTexture.init(this, "textures/Water.png");
		WaterDS.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &WaterTexture}
			});

		Rock1DSVector.resize(maxNumberRock);
		for (auto &ds : Rock1DSVector) {
			ds.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &Rock1Texture}
				});
		}

		GrassModel.init(this, "models/Grass.obj");
		GrassTexture.init(this, "textures/Grass.png");
		GrassDSVector.resize(maxNumberGrass);
		for (auto& ds : GrassDSVector) {
			ds.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &GrassTexture}
				});
		}

		/*Descriptor set global*/
		DSglobal.init(this, &DSLglobal, {
						{0, UNIFORM, sizeof(GlobalUniformBufferObject), nullptr}
		});
	}

	// Here you destroy all the objects you created!		
	void localCleanup() {
		BoatDS.cleanup();
		BoatTexture.cleanup();
		BoatModel.cleanup();

		Rock1DS.cleanup();

		for (auto &ds : Rock1DSVector) {
			ds.cleanup();
		}

		for (auto& ds : GrassDSVector) {
			ds.cleanup();
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
		VkBuffer vertexBuffers[] = {BoatModel.vertexBuffer};
		// property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
		VkDeviceSize offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, BoatModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						P1.pipelineLayout, 1, 1, &BoatDS.descriptorSets[currentImage],
						0, nullptr);
						
		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(BoatModel.indices.size()), 1, 0, 0, 0);

		VkBuffer vertexBuffers1[] = { WaterModel.vertexBuffer };
		VkDeviceSize offsets1[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers1, offsets1);
		vkCmdBindIndexBuffer(commandBuffer, WaterModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &WaterDS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Rock1Model.indices.size()), 1, 0, 0, 0);

		/*
		* Creating the buffer and the command for the rock
		*/
		VkBuffer vertexBuffers2[] = { Rock1Model.vertexBuffer };
		VkDeviceSize offsets2[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers2, offsets2);
		vkCmdBindIndexBuffer(commandBuffer, Rock1Model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
						VK_PIPELINE_BIND_POINT_GRAPHICS,
						P1.pipelineLayout, 1, 1, &Rock1DS.descriptorSets[currentImage],
						0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Rock1Model.indices.size()), 1, 0, 0, 0);

		/* Creating the buffer and the Command for the RIVER*/

		VkBuffer vertexBuffers4[] = { GrassModel.vertexBuffer };
		VkDeviceSize offsets4[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers4, offsets4);
		vkCmdBindIndexBuffer(commandBuffer, GrassModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		for (auto& ds : GrassDSVector) {

			vkCmdBindDescriptorSets(commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				P1.pipelineLayout, 1, 1, &ds.descriptorSets[currentImage],
				0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(GrassModel.indices.size()), 1, 0, 0, 0);

		}

		/*
		* Creating the buffer and the command for the rocks in the array
		*/
		VkBuffer vertexBuffers3[] = { Rock1Model.vertexBuffer };
		VkDeviceSize offsets3[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers3, offsets3);
		vkCmdBindIndexBuffer(commandBuffer, Rock1Model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		for (auto& ds : Rock1DSVector) {
			
			vkCmdBindDescriptorSets(commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				P1.pipelineLayout, 1, 1, &ds.descriptorSets[currentImage],
				0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Rock1Model.indices.size()), 1, 0, 0, 0);
		}

	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		
		static auto start = std::chrono::high_resolution_clock::now();
		static float last = 0.f;
		auto now = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration<float, std::chrono::seconds::period>(now - start).count();
		float delta = time - last;
		last = time;
					
		static glm::mat4 mat = glm::mat4(1.0f);
		static glm::vec3 pos = glm::vec3(1.0f, 0.f, 0.f);
		float angle = static_cast<float>(glm::radians(0.f));
					
		/*Creating the Global UBO and copy the data to the GPU*/
		GlobalUniformBufferObject gubo{};

		gubo.view = glm::lookAt(glm::vec3(-8.0f, 5.0f, 0.0f) + pos,
			pos,
			glm::vec3(0.0f, 1.0f, 0.0f));

		gubo.proj = glm::perspective(glm::radians(90.0f),
			swapChainExtent.width / (float)swapChainExtent.height,
			0.1f, 100.0f);
		gubo.proj[1][1] *= -1;

		void* data;

		vkMapMemory(device, DSglobal.uniformBuffersMemory[0][currentImage], 0, sizeof(gubo), 0, &data);
		memcpy(data, &gubo, sizeof(gubo));
		vkUnmapMemory(device, DSglobal.uniformBuffersMemory[0][currentImage]);

		/*---------------------------------------------------------------------*/

		/*Creating the local/object UBO and copy the data to the GPU*/
		UniformBufferObject ubo{};

		if (glfwGetKey(this->window, GLFW_KEY_W)) {
			pos += glm::vec3(5.0f, 0.0f, 0.0f) * delta;
		}
		if (glfwGetKey(this->window, GLFW_KEY_S)) {
			pos -= glm::vec3(5.0f, 0.0f, 0.0f) * delta;
		}
		if (glfwGetKey(this->window, GLFW_KEY_D)) {
			angle = static_cast<float>(glm::radians(-15.f));
			pos += glm::vec3(0.f, 0.f, 5.f) * delta;
		}
		if (glfwGetKey(this->window, GLFW_KEY_A)) {
			angle = static_cast<float>(glm::radians(15.f));
			pos -= glm::vec3(0.f, 0.f, 5.f) * delta;
		}
		
		
		ubo.model = glm::translate(glm::mat4(1.0f), pos) * glm::scale(glm::mat4(1.0), glm::vec3(0.005, 0.005, 0.005))
			* glm::rotate(glm::mat4(1.0f), static_cast<float>(glm::radians(180.f)), glm::vec3(0.f, 1.f, 0.f))
			* glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.f, 1.f, 0.f));
		
			
		

		/*UBO for the boat*/
		vkMapMemory(device, BoatDS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, BoatDS.uniformBuffersMemory[0][currentImage]);

		/*UBO for the rock1*/
		ubo.model = glm::scale(glm::mat4(1.0f), glm::vec3(0.25f, 0.25f, 0.25f));
		vkMapMemory(device, Rock1DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, Rock1DS.uniformBuffersMemory[0][currentImage]);

		float i = 5.f;
		/*UBO for the rock in the array*/
		for (auto& ds : Rock1DSVector) {
			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(i, 0.f, 0.f)) * ubo.model;
			vkMapMemory(device, ds.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(device, ds.uniformBuffersMemory[0][currentImage]);
		}

		ubo.model = glm::mat4(1.f) * glm::scale(glm::mat4(1.f), glm::vec3(0.05f, 0.05f, 0.05f));

		/*UBO for the River*/
		i = 10.f;
		for (auto& ds : GrassDSVector) {

			ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(i, 0.f, 0.f)) * ubo.model;
			vkMapMemory(device, ds.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(device, ds.uniformBuffersMemory[0][currentImage]);

		}

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(10.f, 0.f, 0.f)) * glm::scale(glm::mat4(1.f), glm::vec3(0.05f, 0.05f, 0.05f));
		vkMapMemory(device, WaterDS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, WaterDS.uniformBuffersMemory[0][currentImage]);
		
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