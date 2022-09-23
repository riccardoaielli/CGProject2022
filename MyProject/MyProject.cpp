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

struct DataPersonalization{
	const int maxNumberRock = 50;
	const int maxNumberLandscape = 10;
	int numberRocksLine = 2;
	float distanceBetweenRocksX = 10.f;
	const float distanceBetweenRocksZ = 4.f;
	float distanceFinishLine = 400.f;
	glm::vec3 boatSpeed = glm::vec3(15.f, 0.f, 5.f);
	float posCameraY = 0.f;
};

DataPersonalization level;

bool selectLevel = true;
bool stillPlaying = true;
bool firstTime = true;



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

	Model finishLineModel;
	Texture finishLineTexture;
	DescriptorSet finishLineDS;

	Model welcomeModel;
	Texture welcomeTexture;
	DescriptorSet welcomeDS;

	Model lostPageModel;
	Texture lostPageTexture;
	DescriptorSet lostPageDS;

	Model wonPageModel;
	Texture wonPageTexture;
	DescriptorSet wonPageDS;

	DescriptorSet DSglobal;
	
	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 1224;
		windowHeight = 792;
		windowTitle = "My Project";
		initialBackgroundColor = {0.f, 0.f, 0.f, 1.f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 4 + level.maxNumberRock + level.maxNumberLandscape * 2 +1 +1;
		texturesInPool = 3 + level.maxNumberRock + level.maxNumberLandscape * 2 +1 +1;
		setsInPool = 4 + level.maxNumberRock + level.maxNumberLandscape * 2 +1 +1;

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

		/* INITIALIZATING THE BOAT MODEL AND TEXTURE */
		boatObject.model.init(this, "models/Boat.obj");
		boatObject.texture.init(this, "textures/Boat.bmp");
		boatObject.ds.init(this, &DSLobj, {
					{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
					{1, TEXTURE, 0, &boatObject.texture}
		});

		/*--------------------------------------------------*/

		/* INITIALIZETING THE WELCOME PAGE MODEL AND TEXTURE */
		welcomeModel.init(this, "models/Square.obj");
		welcomeTexture.init(this, "textures/CGWelcome.png");
		welcomeDS.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &welcomeTexture}
			});

		/*----------------------------------------------------*/

		/* INITIALIZETING THE LOST PAGE MODEL AND TEXTURE*/

		lostPageModel.init(this, "models/Square.obj");
		lostPageTexture.init(this, "textures/CGYouLost.png");
		lostPageDS.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &lostPageTexture}
			});

		/*-----------------------------------------------*/

		/* INITIALIZETING THE WON PAGE MODEL AND TEXTURE*/

		wonPageModel.init(this, "models/Square.obj");
		wonPageTexture.init(this, "textures/CGYouWon.png");
		wonPageDS.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &wonPageTexture}
			});

		/*-----------------------------------------------*/

		/* INITIALIZATING THE FINISH LINE MODEL AND TEXXTURE*/
		finishLineModel.init(this, "models/FinishLine1.obj");
		finishLineTexture.init(this, "textures/FinishLine.png");
		finishLineDS.init(this, &DSLobj, {
			// the second parameter, is a pointer to the Uniform Set Layout of this set
			// the last parameter is an array, with one element per binding of the set.
			// first  elmenet : the binding number
			// second element : UNIFORM or TEXTURE (an enum) depending on the type
			// third  element : only for UNIFORMs, the size of the corresponding C++ object
			// fourth element : only for TEXTUREs, the pointer to the corresponding texture object
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &finishLineTexture}
			});
		/*---------------------------------------------------*/

		
		/* INITIALIZING MODEL AND TEXTURE OF GRASS AND WATER + INITIALIZING THE DESCRIPTIVE SET AND POSITION */
		WaterModel.init(this, "models/Water.obj");
		WaterTexture.init(this, "textures/Water.png");
		GrassModel.init(this, "models/Grass.obj");
		GrassTexture.init(this, "textures/Grass.png");
		landscapeObjects.resize(level.maxNumberLandscape);
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
		
		/*-------------------------------------------------------------------*/


		/* INITIALIZING MODEL AND TEXTURE OF ROCK1 + INITIALIZING THE DESCRIPTIVE SET */
		Rock1Model.init(this, "models/Rock1.obj");
		Rock1Texture.init(this, "textures/Rock1.png");
		rockObjects.resize(level.maxNumberRock);
		i = 15.f;
		int even = 1;
		for (auto &obj : rockObjects) {
			obj.ds.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &Rock1Texture}
				});

			/* INITIALIZING THE POSITION OF THE ROCKS */
			obj.currentPos = glm::vec3(-20.f, 0.f, 0.f);
			/*obj.currentPos = glm::vec3(i, 0.f, (std::rand()%5 -2)*level.distanceBetweenRocksZ);
			if ((even % level.numberRocksLine) == 0) {
				i += level.distanceBetweenRocksX;
			}
			even++;*/
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

		finishLineDS.cleanup();
		finishLineTexture.cleanup();
		finishLineModel.cleanup();

		welcomeDS.cleanup();
		welcomeTexture.cleanup();
		welcomeModel.cleanup();

		lostPageDS.cleanup();
		lostPageTexture.cleanup();
		lostPageModel.cleanup();

		wonPageDS.cleanup();
		wonPageTexture.cleanup();
		wonPageModel.cleanup();

		WaterTexture.cleanup();
		WaterModel.cleanup();

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


		/* CREATING BUFFER FOR FINISH LINE */
		VkBuffer vertexBuffers1[] = { finishLineModel.vertexBuffer };
		// property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
		VkDeviceSize offsets1[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers1, offsets1);
		vkCmdBindIndexBuffer(commandBuffer, finishLineModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &finishLineDS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(finishLineModel.indices.size()), 1, 0, 0, 0);

		/*-----------------------------------------------------------*/

		/* CREATING BUFFER FOR THE WELCOME */

		VkBuffer vertexBuffers2[] = { welcomeModel.vertexBuffer };
		// property .vertexBuffer of models, contains the VkBuffer handle to its vertex buffer
		VkDeviceSize offsets2[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers2, offsets2);
		vkCmdBindIndexBuffer(commandBuffer, welcomeModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &welcomeDS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(welcomeModel.indices.size()), 1, 0, 0, 0);

		/*-----------------------------------------------------------*/


		/* Creating the buffer and the Command for the RIVER*/

		VkBuffer vertexBuffers3[] = { GrassModel.vertexBuffer };
		VkDeviceSize offsets3[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers3, offsets3);
		vkCmdBindIndexBuffer(commandBuffer, GrassModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		for (auto& obj : landscapeObjects) {

			vkCmdBindDescriptorSets(commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				P1.pipelineLayout, 1, 1, &obj.grassDs.descriptorSets[currentImage],
				0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(GrassModel.indices.size()), 1, 0, 0, 0);

		}

		/*-----------------------------------------------------------*/

		/* CREATING THE BUFFER AND COMMAND FOR THE WATER */

		VkBuffer vertexBuffers4[] = { WaterModel.vertexBuffer };
		VkDeviceSize offsets4[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers4, offsets4);
		vkCmdBindIndexBuffer(commandBuffer, WaterModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		for (auto& obj : landscapeObjects) {

			vkCmdBindDescriptorSets(commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				P1.pipelineLayout, 1, 1, &obj.waterDs.descriptorSets[currentImage],
				0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(WaterModel.indices.size()), 1, 0, 0, 0);

		}
		/*-----------------------------------------------------------*/


		/* CREATING THE BUFFER AND THE COMMAND FOR THE ROCKS */

		VkBuffer vertexBuffers5[] = { Rock1Model.vertexBuffer };
		VkDeviceSize offsets5[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers5, offsets5);
		vkCmdBindIndexBuffer(commandBuffer, Rock1Model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		for (auto& obj : rockObjects) {

			vkCmdBindDescriptorSets(commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				P1.pipelineLayout, 1, 1, &obj.ds.descriptorSets[currentImage],
				0, nullptr);

			vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(Rock1Model.indices.size()), 1, 0, 0, 0);
		}

		/*---------------------------------------------------*/

		/* CREATING THE BUFFER FOR THE LOST PAGE */
		VkBuffer vertexBuffers6[] = { lostPageModel.vertexBuffer };
		VkDeviceSize offsets6[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers6, offsets6);
		vkCmdBindIndexBuffer(commandBuffer, lostPageModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &lostPageDS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(lostPageModel.indices.size()), 1, 0, 0, 0);
		
		/*---------------------------------------------------*/

		/* CREATING THE BUFFER FOR THE WON PAGE */
		VkBuffer vertexBuffers7[] = { wonPageModel.vertexBuffer };
		VkDeviceSize offsets7[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers7, offsets7);
		vkCmdBindIndexBuffer(commandBuffer, wonPageModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &wonPageDS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(wonPageModel.indices.size()), 1, 0, 0, 0);
		
		/*---------------------------------------------------*/
	
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		
		/* UPDATE THE GLOBAL UBO */

		updateGlobalUBO(currentImage);

		/*---------------------------------------------------------------------*/

		if (!selectLevel) {

			/*UBO FOR THE BOAT*/

			updateBoat(currentImage);

			/*---------------------------------------------------------------------*/


			/* UBO FOR THE ROCKS */

			updateRocks(currentImage);

			/*---------------------------------------------------------------------*/

			/* UBO FOR THE GRASS AND WATER */

			updateLandscapes(currentImage);

			/*---------------------------------------------------------------------*/

			/* UBO FOR THE FINISH LINE */

			updateFinishLine(currentImage);

			/*---------------------------------------------------------------------*/

			hideWelcomePage(currentImage);
			

		}
		else {
			updateWelcomePage(currentImage);
			if (glfwGetKey(window, GLFW_KEY_1)) {
				level.numberRocksLine = 1;
				level.distanceBetweenRocksX = 15.f;
				level.distanceFinishLine = 100.f;
				level.boatSpeed.x = 5.f;
				level.boatSpeed.z = 5.f;
				level.posCameraY = 10.f;
				selectLevel = false;
				firstTime = true;
			}
			else if (glfwGetKey(window, GLFW_KEY_2)) {
				level.numberRocksLine = 2;
				level.distanceBetweenRocksX = 15.f;
				level.distanceFinishLine = 240.f;
				level.boatSpeed.x = 8.f;
				level.boatSpeed.z = 8.f;
				level.posCameraY = 10.f;
				selectLevel = false;
				firstTime = true;
			}

			else if (glfwGetKey(window, GLFW_KEY_3)) {
				level.numberRocksLine = 3;
				level.distanceBetweenRocksX = 15.f;
				level.distanceFinishLine = 600.f;
				level.boatSpeed.x = 10.f;
				level.boatSpeed.z = 10.f;
				level.posCameraY = 10.f;
				selectLevel = false;
				firstTime = true;
			}

			else if (glfwGetKey(window, GLFW_KEY_4)) {
				level.numberRocksLine = 3;
				level.distanceBetweenRocksX = 15.f;
				level.distanceFinishLine = 660.f;
				level.boatSpeed.x = 11.f;
				level.boatSpeed.z = 10.f;
				level.posCameraY = 10.f;
				selectLevel = false;
				firstTime = true;
			}

			else if (glfwGetKey(window, GLFW_KEY_5)) {
				level.numberRocksLine = 4;
				level.distanceBetweenRocksX = 17.f;
				level.distanceFinishLine = 600.f;
				level.boatSpeed.x = 10.f;
				level.boatSpeed.z = 11.f;
				level.posCameraY = 10.f;
				selectLevel = false;
				firstTime = true;
			}

			else if (glfwGetKey(window, GLFW_KEY_6)) {
				level.numberRocksLine = 4;
				level.distanceBetweenRocksX = 18.f;
				level.distanceFinishLine = 660.f;
				level.boatSpeed.x = 11.f;
				level.boatSpeed.z = 12.f;
				level.posCameraY = 10.f;
				selectLevel = false;
				firstTime = true;
			}

			else if (glfwGetKey(window, GLFW_KEY_7)) {
				level.numberRocksLine = 4;
				level.distanceBetweenRocksX = 18.f;
				level.distanceFinishLine = 720.f;
				level.boatSpeed.x = 12.f;
				level.boatSpeed.z = 13.f;
				level.posCameraY = 10.f;
				selectLevel = false;
				firstTime = true;
			}

			else if (glfwGetKey(window, GLFW_KEY_8)) {
				level.numberRocksLine = 4;
				level.distanceBetweenRocksX = 18.f;
				level.distanceFinishLine = 780.f;
				level.boatSpeed.x = 13.f;
				level.boatSpeed.z = 15.f;
				level.posCameraY = 10.f;
				selectLevel = false;
				firstTime = true;
			}

			else if (glfwGetKey(window, GLFW_KEY_9)) {
				level.numberRocksLine = 4;
				level.distanceBetweenRocksX = 18.f;
				level.distanceFinishLine = 900.f;
				level.boatSpeed.x = 15.f;
				level.boatSpeed.z = 17.f;
				level.posCameraY = 10.f;
				selectLevel = false;
				firstTime = true;
			}
			
		}
	}	

	void updateLostPage(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(boatObject.currentPos.x -4.f, 5.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(51.3f), glm::vec3(0.f, 1.f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 6.132f, 4.f));

		vkMapMemory(device, lostPageDS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, lostPageDS.uniformBuffersMemory[0][currentImage]);

	}

	void updateWonPage(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(boatObject.currentPos.x -4.f, 5.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(51.3f), glm::vec3(0.f, 1.f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 6.132f, 4.f));

		vkMapMemory(device, wonPageDS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, wonPageDS.uniformBuffersMemory[0][currentImage]);

	}

	void hideLostPage(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 1000.f, 0.f));

		vkMapMemory(device, lostPageDS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, lostPageDS.uniformBuffersMemory[0][currentImage]);

	}

	void hideWonPage(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 1000.f, 0.f));

		vkMapMemory(device, wonPageDS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, wonPageDS.uniformBuffersMemory[0][currentImage]);

	}

	void hideWelcomePage(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 1000.f, 0.f));

		vkMapMemory(device, welcomeDS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, welcomeDS.uniformBuffersMemory[0][currentImage]);

	}

	void updateWelcomePage(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(-4.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 6.132f, 4.f));

		vkMapMemory(device, welcomeDS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, welcomeDS.uniformBuffersMemory[0][currentImage]);

	}

	void updateGlobalUBO(uint32_t currentImage) {

		/*Creating the Global UBO and copy the data to the GPU*/
		GlobalUniformBufferObject gubo{};

		gubo.view = glm::lookAt(glm::vec3(-8.0f + boatObject.currentPos.x, level.posCameraY, 0.f),
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
				obj.currentPosX = obj.currentPosX + (level.maxNumberLandscape * 10.f);

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
		static int pos = 1;
		
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
		

		if (firstTime) {
			int even = 1;
			float i = level.distanceBetweenRocksX*2;
			for (auto& obj : rockObjects) {

				obj.currentPos = glm::vec3(i, 0.f, (std::rand() % 5 - 2) * level.distanceBetweenRocksZ);
				if ((even % (level.numberRocksLine)) == 0) {
					i += level.distanceBetweenRocksX;
				}
				even++;
			}
			firstTime = false;
		}
		

		for (auto& obj : rockObjects) {
			if (boatObject.currentPos.z +1.f < obj.currentPos.z + 2.8f && boatObject.currentPos.z - 1.f > obj.currentPos.z - 2.8f && 
					(boatObject.currentPos.x + 2.f > obj.currentPos.x - 1.f && boatObject.currentPos.x - 2.4f < obj.currentPos.x + 1.f) ) {
				stillPlaying = false;
				updateLostPage(currentImage);
			}
			if (boatObject.currentPos.x > obj.currentPos.x + 10.f) {
				obj.currentPos = glm::vec3(obj.currentPos.x + level.distanceBetweenRocksX * (level.maxNumberRock/level.numberRocksLine), 0.f, (std::rand() % 5 - 2) * level.distanceBetweenRocksZ);
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
			pos += glm::vec3(level.boatSpeed.x, 0.f, 0.f) * delta;

			/* UPDATING FOR MOVING RIGHT OR LEFT */
			if (glfwGetKey(this->window, GLFW_KEY_D) || glfwGetKey(this->window, GLFW_KEY_RIGHT)) {
				angle = angle < glm::radians(-10.f) ? glm::radians(-10.f) : angle + glm::radians(-1.f);
				pos += glm::vec3(0.f, 0.f, level.boatSpeed.z) * delta;
				if (pos.z > 10.f - 1.f)
					pos.z = 9.f;
			}
			else if (glfwGetKey(this->window, GLFW_KEY_A) || glfwGetKey(this->window, GLFW_KEY_LEFT)) {
				angle = angle > glm::radians(10.f) ? glm::radians(10.f) : angle + glm::radians(1.f);
				pos -= glm::vec3(0.f, 0.f, level.boatSpeed.z) * delta;
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

		std::cout << boatObject.currentPos.x<<"  "<< boatObject.currentPos.z << "\n";

		vkMapMemory(device, boatObject.ds.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, boatObject.ds.uniformBuffersMemory[0][currentImage]);

	}

	void updateFinishLine(uint32_t currentImage) {
		UniformBufferObject ubo{};
		void* data;

		
		if (boatObject.currentPos.x - 4.f > level.distanceFinishLine - 1.f && boatObject.currentPos.x - 8.4f < level.distanceFinishLine + 1.f) {
			stillPlaying = false;
			updateWonPage(currentImage);
		}	

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(level.distanceFinishLine, 2.f, -2.f)) * glm::scale(glm::mat4(1.f), glm::vec3(0.05f, 0.03f, 0.08f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(0.f, 1.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(25.f), glm::vec3(1.f, 0.f, 0.f));

		vkMapMemory(device, finishLineDS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, finishLineDS.uniformBuffersMemory[0][currentImage]);
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