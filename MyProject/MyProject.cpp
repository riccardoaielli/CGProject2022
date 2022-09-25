// This has been adapted from the Vulkan tutorial

#include "MyProject.hpp"

//const std::string MODEL_PATH = "Assets/models/Boat.obj";
//const std::string TEXTURE_PATH = "Assets/textures/Boat.bmp";

enum GameState {WELCOME_PAGE, PLAYING, LOST, WIN, PAUSE};

enum Level {l0, l1, l2, l3, l4, l5, l6, l7, l8, l9};

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
	glm::vec3 currentPos = glm::vec3(0.f, 0.f, 0.f);
};

struct LandscapeObject {
	DescriptorSet grassDs;
	DescriptorSet waterDs;
	float currentPosX = -15.f;
};

struct DataPersonalization{
	const int maxNumberRock = 60;
	const int maxNumberLandscape = 10;
	int numberRocksLine = 2;
	float distanceBetweenRocksX = 10.f;
	const float distanceBetweenRocksZ = 4.f;
	float distanceFinishLine = 400.f;
	glm::vec3 boatSpeed = glm::vec3(15.f, 0.f, 5.f);
	float posCameraY = 0.f;
};

DataPersonalization level;

GameState state = WELCOME_PAGE;

Level levelLabel = l1;

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

	Model infoModel;
	Texture infoTexture;
	DescriptorSet infoDS;

	Model l1Model;
	Texture l0Texture;
	DescriptorSet l0DS;
	Texture l1Texture;
	DescriptorSet l1DS;
	Texture l2Texture;
	DescriptorSet l2DS;
	Texture l3Texture;
	DescriptorSet l3DS;
	Texture l4Texture;
	DescriptorSet l4DS;
	Texture l5Texture;
	DescriptorSet l5DS;
	Texture l6Texture;
	DescriptorSet l6DS;
	Texture l7Texture;
	DescriptorSet l7DS;
	Texture l8Texture;
	DescriptorSet l8DS;
	Texture l9Texture;
	DescriptorSet l9DS;

	DescriptorSet DSglobal;
	
	// Here you set the main application parameters
	void setWindowParameters() {
		// window size, titile and initial background
		windowWidth = 1224;
		windowHeight = 792;
		windowTitle = "My Project";
		initialBackgroundColor = {0.f, 0.f, 0.f, 1.f};
		
		// Descriptor pool sizes
		uniformBlocksInPool = 15 + level.maxNumberRock + level.maxNumberLandscape * 2 +1 +1;
		texturesInPool = 14 + level.maxNumberRock + level.maxNumberLandscape * 2 +1 +1;
		setsInPool = 15 + level.maxNumberRock + level.maxNumberLandscape * 2 +1 +1;

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

		/* INITIALIZETING THE INFO MODEL AND TEXTURE*/

		infoModel.init(this, "models/Square.obj");
		infoTexture.init(this, "textures/CGInfo.png");
		infoDS.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &infoTexture}
			});

		/*-----------------------------------------------*/

		/* INITIALIZETING THE LEVEL MODEL AND LEVEL'S TEXTURES*/

		l1Model.init(this, "models/Square.obj");
		l0Texture.init(this, "textures/CGL0.png");
		l0DS.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &l0Texture}
			});
		l1Texture.init(this, "textures/CGL1.png");
		l1DS.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &l1Texture}
			});
		l2Texture.init(this, "textures/CGL2.png");
		l2DS.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &l2Texture}
			});
		l3Texture.init(this, "textures/CGL3.png");
		l3DS.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &l3Texture}
			});
		l4Texture.init(this, "textures/CGL4.png");
		l4DS.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &l4Texture}
			});
		l5Texture.init(this, "textures/CGL5.png");
		l5DS.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &l5Texture}
			});
		l6Texture.init(this, "textures/CGL6.png");
		l6DS.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &l6Texture}
			});
		l7Texture.init(this, "textures/CGL7.png");
		l7DS.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &l7Texture}
			});
		l8Texture.init(this, "textures/CGL8.png");
		l8DS.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &l8Texture}
			});
		l9Texture.init(this, "textures/CGL9.png");
		l9DS.init(this, &DSLobj, {
						{0, UNIFORM, sizeof(UniformBufferObject), nullptr},
						{1, TEXTURE, 0, &l9Texture}
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
			//obj.currentPosX += i;
			//i += 10.f;
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
			//obj.currentPos = glm::vec3(-20.f, 0.f, 0.f);
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

		infoDS.cleanup();
		infoTexture.cleanup();
		infoModel.cleanup();

		l0DS.cleanup();
		l0Texture.cleanup();
		l1DS.cleanup();
		l1Texture.cleanup();
		l2DS.cleanup();
		l2Texture.cleanup();
		l3DS.cleanup();
		l3Texture.cleanup();
		l4DS.cleanup();
		l4Texture.cleanup();
		l5DS.cleanup();
		l5Texture.cleanup();
		l6DS.cleanup();
		l6Texture.cleanup();
		l7DS.cleanup();
		l7Texture.cleanup();
		l8DS.cleanup();
		l8Texture.cleanup();
		l9DS.cleanup();
		l9Texture.cleanup();
		l1Model.cleanup();


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

		/* CREATING THE BUFFER FOR THE INFO */
		VkBuffer vertexBuffers18[] = { infoModel.vertexBuffer };
		VkDeviceSize offsets18[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers18, offsets18);
		vkCmdBindIndexBuffer(commandBuffer, infoModel.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &infoDS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(infoModel.indices.size()), 1, 0, 0, 0);
		
		/*---------------------------------------------------*/

		/* CREATING THE BUFFER FOR THE level0 */
		VkBuffer vertexBuffers8[] = { l1Model.vertexBuffer };
		VkDeviceSize offsets8[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers8, offsets8);
		vkCmdBindIndexBuffer(commandBuffer, l1Model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &l0DS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(l1Model.indices.size()), 1, 0, 0, 0);
		
		/*---------------------------------------------------*/

		/* CREATING THE BUFFER FOR THE level1 */
		VkBuffer vertexBuffers9[] = { l1Model.vertexBuffer };
		VkDeviceSize offsets9[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers9, offsets9);
		vkCmdBindIndexBuffer(commandBuffer, l1Model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &l1DS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(l1Model.indices.size()), 1, 0, 0, 0);
		
		/*---------------------------------------------------*/

		/* CREATING THE BUFFER FOR THE level2 */
		VkBuffer vertexBuffers10[] = { l1Model.vertexBuffer };
		VkDeviceSize offsets10[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers10, offsets10);
		vkCmdBindIndexBuffer(commandBuffer, l1Model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &l2DS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(l1Model.indices.size()), 1, 0, 0, 0);
		
		/*---------------------------------------------------*/

		/* CREATING THE BUFFER FOR THE level3 */
		VkBuffer vertexBuffers11[] = { l1Model.vertexBuffer };
		VkDeviceSize offsets11[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers11, offsets11);
		vkCmdBindIndexBuffer(commandBuffer, l1Model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &l3DS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(l1Model.indices.size()), 1, 0, 0, 0);
		
		/*---------------------------------------------------*/

		/* CREATING THE BUFFER FOR THE level4 */
		VkBuffer vertexBuffers12[] = { l1Model.vertexBuffer };
		VkDeviceSize offsets12[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers12, offsets12);
		vkCmdBindIndexBuffer(commandBuffer, l1Model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &l4DS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(l1Model.indices.size()), 1, 0, 0, 0);
		
		/*---------------------------------------------------*/

		/* CREATING THE BUFFER FOR THE level5 */
		VkBuffer vertexBuffers13[] = { l1Model.vertexBuffer };
		VkDeviceSize offsets13[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers13, offsets13);
		vkCmdBindIndexBuffer(commandBuffer, l1Model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &l5DS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(l1Model.indices.size()), 1, 0, 0, 0);
		
		/*---------------------------------------------------*/

		/* CREATING THE BUFFER FOR THE level6 */
		VkBuffer vertexBuffers14[] = { l1Model.vertexBuffer };
		VkDeviceSize offsets14[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers14, offsets14);
		vkCmdBindIndexBuffer(commandBuffer, l1Model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &l6DS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(l1Model.indices.size()), 1, 0, 0, 0);
		
		/*---------------------------------------------------*/

		/* CREATING THE BUFFER FOR THE level7 */
		VkBuffer vertexBuffers15[] = { l1Model.vertexBuffer };
		VkDeviceSize offsets15[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers15, offsets15);
		vkCmdBindIndexBuffer(commandBuffer, l1Model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &l7DS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(l1Model.indices.size()), 1, 0, 0, 0);
		
		/*---------------------------------------------------*/

		/* CREATING THE BUFFER FOR THE level8 */
		VkBuffer vertexBuffers16[] = { l1Model.vertexBuffer };
		VkDeviceSize offsets16[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers16, offsets16);
		vkCmdBindIndexBuffer(commandBuffer, l1Model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &l8DS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(l1Model.indices.size()), 1, 0, 0, 0);
		
		/*---------------------------------------------------*/

		/* CREATING THE BUFFER FOR THE level9 */
		VkBuffer vertexBuffers17[] = { l1Model.vertexBuffer };
		VkDeviceSize offsets17[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers17, offsets17);
		vkCmdBindIndexBuffer(commandBuffer, l1Model.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
		vkCmdBindDescriptorSets(commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			P1.pipelineLayout, 1, 1, &l9DS.descriptorSets[currentImage],
			0, nullptr);

		vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(l1Model.indices.size()), 1, 0, 0, 0);
		
		/*---------------------------------------------------*/
	
	}

	// Here is where you update the uniforms.
	// Very likely this will be where you will be writing the logic of your application.
	void updateUniformBuffer(uint32_t currentImage) {
		
		switch (state) {
		case PLAYING:

			/* UBO FOR THE ROCKS */

			updateRocks(currentImage);

			/*---------------------------------------------------------------------*/

			/* UBO FOR THE GRASS AND WATER */

			updateLandscapes(currentImage);

			/*---------------------------------------------------------------------*/

			/*UBO FOR THE BOAT*/

			updateBoat(currentImage);

			/*---------------------------------------------------------------------*/

			/* UBO FOR THE FINISH LINE */

			updateFinishLine(currentImage);

			/*---------------------------------------------------------------------*/

			/* UBO FOR THE LEVEL */

			updateLevel(currentImage);

			/*---------------------------------------------------------------------*/

			/* UBO FOR THE INFO */

			updateInfo(currentImage);

			/*---------------------------------------------------------------------*/

			/* HIDING ALL PAGES */

			hideWelcomePage(currentImage);
			hideLostPage(currentImage);
			hideWonPage(currentImage);
			/*---------------------------------------------------------------------*/

		break;
		case WELCOME_PAGE:
			updateWelcomePage(currentImage);
			selectLevel(currentImage);
		break;
		case LOST:
			updateInfo(currentImage);
			updateLevel(currentImage);
			updateBoat(currentImage);
			updateLostPage(currentImage);
			selectLevel(currentImage);
		break;
		case WIN:
			updateInfo(currentImage);
			updateLevel(currentImage);
			updateBoat(currentImage);
			updateWonPage(currentImage);
			selectLevel(currentImage);
		break;
		case PAUSE:
			state = PLAYING;
		break;
		}

		/* UPDATE THE GLOBAL UBO */

		updateGlobalUBO(currentImage);

		/*---------------------------------------------------------------------*/

		
	}	

	void selectLevel(uint32_t currentImage) {
		if (glfwGetKey(window, GLFW_KEY_1)) {
			level.numberRocksLine = 1;
			level.distanceBetweenRocksX = 14.f;
			level.distanceFinishLine = 50.f;
			level.boatSpeed.x = 5.f;
			level.boatSpeed.z = 5.f;
			level.posCameraY = 10.f;
			firstTime = true;
			resetLevel(currentImage);
			levelLabel = l1;
		}
		else if (glfwGetKey(window, GLFW_KEY_2)) {
			level.numberRocksLine = 2;
			level.distanceBetweenRocksX = 14.f;
			level.distanceFinishLine = 200.f;
			level.boatSpeed.x = 10.f;
			level.boatSpeed.z = 8.f;
			level.posCameraY = 10.f;
			firstTime = true;
			resetLevel(currentImage);
			levelLabel = l2;

		}

		else if (glfwGetKey(window, GLFW_KEY_3)) {
			level.numberRocksLine = 3;
			level.distanceBetweenRocksX = 14.f;
			level.distanceFinishLine = 360.f;
			level.boatSpeed.x = 12.f;
			level.boatSpeed.z = 11.f;
			level.posCameraY = 10.f;
			firstTime = true;
			resetLevel(currentImage);
			levelLabel = l3;

		}

		else if (glfwGetKey(window, GLFW_KEY_4)) {
			level.numberRocksLine = 3;
			level.distanceBetweenRocksX = 14.f;
			level.distanceFinishLine = 780.f;
			level.boatSpeed.x = 13.f;
			level.boatSpeed.z = 12.f;
			level.posCameraY = 10.f;
			firstTime = true;
			resetLevel(currentImage);
			levelLabel = l4;

		}

		else if (glfwGetKey(window, GLFW_KEY_5)) {
			level.numberRocksLine = 4;
			level.distanceBetweenRocksX = 15.f;
			level.distanceFinishLine = 840.f;
			level.boatSpeed.x = 14.f;
			level.boatSpeed.z = 15.f;
			level.posCameraY = 10.f;
			resetLevel(currentImage);
			levelLabel = l5;
			firstTime = true;

		}

		else if (glfwGetKey(window, GLFW_KEY_6)) {
			level.numberRocksLine = 4;
			level.distanceBetweenRocksX = 16.f;
			level.distanceFinishLine = 960.f;
			level.boatSpeed.x = 16.f;
			level.boatSpeed.z = 18.f;
			level.posCameraY = 10.f;
			resetLevel(currentImage);
			levelLabel = l6;
			firstTime = true;

		}

		else if (glfwGetKey(window, GLFW_KEY_7)) {
			level.numberRocksLine = 4;
			level.distanceBetweenRocksX = 16.f;
			level.distanceFinishLine = 1080.f;
			level.boatSpeed.x = 18.f;
			level.boatSpeed.z = 20.f;
			level.posCameraY = 10.f;
			resetLevel(currentImage);
			levelLabel = l7;
			firstTime = true;

		}

		else if (glfwGetKey(window, GLFW_KEY_8)) {
			level.numberRocksLine = 4;
			level.distanceBetweenRocksX = 18.f;
			level.distanceFinishLine = 1200.f;
			level.boatSpeed.x = 20.f;
			level.boatSpeed.z = 22.f;
			level.posCameraY = 10.f;
			resetLevel(currentImage);
			levelLabel = l8;
			firstTime = true;

		}

		else if (glfwGetKey(window, GLFW_KEY_9)) {
			level.numberRocksLine = 4;
			level.distanceBetweenRocksX = 18.f;
			level.distanceFinishLine = 1440.f;
			level.boatSpeed.x = 24.f;
			level.boatSpeed.z = 26.f;
			level.posCameraY = 10.f;
			resetLevel(currentImage);
			levelLabel = l9;
			firstTime = true;
		}

		else if (glfwGetKey(window, GLFW_KEY_0)) {
			level.numberRocksLine = 4;
			level.distanceBetweenRocksX = 18.f;
			level.distanceFinishLine = 2880.f;
			level.boatSpeed.x = 32.f;
			level.boatSpeed.z = 34.f;
			level.posCameraY = 10.f;
			resetLevel(currentImage);
			levelLabel = l0;
			firstTime = true;
		}
	}

	void resetLevel(uint32_t currentImage) {		

		hideL0(currentImage);
		hideL1(currentImage);
		hideL2(currentImage);
		hideL3(currentImage);
		hideL4(currentImage);
		hideL5(currentImage);
		hideL6(currentImage);
		hideL7(currentImage);
		hideL8(currentImage);
		hideL9(currentImage);
		updateBoat(currentImage);
		hideWelcomePage(currentImage);
		hideLostPage(currentImage);
		hideWonPage(currentImage);
		state = PAUSE;

		boatObject.currentPos.x = -5.f;
		boatObject.currentPos.z = 0.f;

		float i = -10.f;
		for (auto& obj : landscapeObjects) {
			obj.currentPosX = i;
			i += 10.f;
		}


		for (auto& obj : rockObjects) {	
			obj.currentPos = glm::vec3(-20.f, 0.f, 0.f);
		}

	}

	void updateLostPage(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(boatObject.currentPos.x -7.25f, 6.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(51.3f), glm::vec3(0.f, 1.f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 3.22f, 1.f));

		vkMapMemory(device, lostPageDS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, lostPageDS.uniformBuffersMemory[0][currentImage]);

	}

	void updateWonPage(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(boatObject.currentPos.x -7.25f, 6.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(51.3f), glm::vec3(0.f, 1.f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 3.22f, 1.f));

		vkMapMemory(device, wonPageDS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, wonPageDS.uniformBuffersMemory[0][currentImage]);

	}

	void updateLevel(uint32_t currentImage) {

		hideL0(currentImage);
		hideL1(currentImage);
		hideL2(currentImage);
		hideL3(currentImage);
		hideL4(currentImage);
		hideL5(currentImage);
		hideL6(currentImage);
		hideL7(currentImage);
		hideL8(currentImage);
		hideL9(currentImage);

		switch (levelLabel) {
		case l0:
			updateL0(currentImage);
			break;
		case l1:
			updateL1(currentImage);
			break;
		case l2:
			updateL2(currentImage);
			break;
		case l3:
			updateL3(currentImage);
			break;
		case l4:
			updateL4(currentImage);
			break;
		case l5:
			updateL5(currentImage);
			break;
		case l6:
			updateL6(currentImage);
			break;
		case l7:
			updateL7(currentImage);
			break;
		case l8:
			updateL8(currentImage);
			break;
		case l9:
			updateL9(currentImage);
			break;
		}

	}

	void updateInfo(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(boatObject.currentPos.x +0.8f, 8.3f, +7.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(51.3f), glm::vec3(0.f, 1.f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 3.534f, 1.f));

		vkMapMemory(device, infoDS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, infoDS.uniformBuffersMemory[0][currentImage]);

	}

	void updateL0(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(boatObject.currentPos.x +0.8f, 8.3f, -7.9f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(51.3f), glm::vec3(0.f, 1.f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 2.674f, 1.f));

		vkMapMemory(device, l0DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l0DS.uniformBuffersMemory[0][currentImage]);

	}

	void updateL1(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(boatObject.currentPos.x +0.8f, 8.3f, -7.9f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(51.3f), glm::vec3(0.f, 1.f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 2.674f, 1.f));

		vkMapMemory(device, l1DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l1DS.uniformBuffersMemory[0][currentImage]);

	}

	void updateL2(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(boatObject.currentPos.x +0.8f, 8.3f, -7.9f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(51.3f), glm::vec3(0.f, 1.f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 2.674f, 1.f));

		vkMapMemory(device, l2DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l2DS.uniformBuffersMemory[0][currentImage]);

	}

	void updateL3(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(boatObject.currentPos.x +0.8f, 8.3f, -7.9f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(51.3f), glm::vec3(0.f, 1.f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 2.674f, 1.f));

		vkMapMemory(device, l3DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l3DS.uniformBuffersMemory[0][currentImage]);

	}

	void updateL4(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(boatObject.currentPos.x +0.8f, 8.3f, -7.9f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(51.3f), glm::vec3(0.f, 1.f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 2.674f, 1.f));

		vkMapMemory(device, l4DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l4DS.uniformBuffersMemory[0][currentImage]);

	}

	void updateL5(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(boatObject.currentPos.x +0.8f, 8.3f, -7.9f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(51.3f), glm::vec3(0.f, 1.f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 2.674f, 1.f));

		vkMapMemory(device, l5DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l5DS.uniformBuffersMemory[0][currentImage]);

	}

	void updateL6(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(boatObject.currentPos.x +0.8f, 8.3f, -7.9f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(51.3f), glm::vec3(0.f, 1.f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 2.674f, 1.f));

		vkMapMemory(device, l6DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l6DS.uniformBuffersMemory[0][currentImage]);

	}

	void updateL7(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(boatObject.currentPos.x +0.8f, 8.3f, -7.9f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(51.3f), glm::vec3(0.f, 1.f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 2.674f, 1.f));

		vkMapMemory(device, l7DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l7DS.uniformBuffersMemory[0][currentImage]);

	}

	void updateL8(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(boatObject.currentPos.x +0.8f, 8.3f, -7.9f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(51.3f), glm::vec3(0.f, 1.f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 2.674f, 1.f));

		vkMapMemory(device, l8DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l8DS.uniformBuffersMemory[0][currentImage]);

	}

	void updateL9(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(boatObject.currentPos.x +0.8f, 8.3f, -7.9f))
			* glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f))
			* glm::rotate(glm::mat4(1.f), glm::radians(51.3f), glm::vec3(0.f, 1.f, 0.f))
			* glm::scale(glm::mat4(1.f), glm::vec3(1.f, 2.674f, 1.f));

		vkMapMemory(device, l9DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l9DS.uniformBuffersMemory[0][currentImage]);

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

	void hideInfo(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 1000.f, 0.f));

		vkMapMemory(device, infoDS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, infoDS.uniformBuffersMemory[0][currentImage]);

	}

	void hideL0(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 1000.f, 0.f));

		vkMapMemory(device, l0DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l0DS.uniformBuffersMemory[0][currentImage]);

	}

	void hideL1(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 1000.f, 0.f));

		vkMapMemory(device, l1DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l1DS.uniformBuffersMemory[0][currentImage]);

	}

	void hideL2(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 1000.f, 0.f));

		vkMapMemory(device, l2DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l2DS.uniformBuffersMemory[0][currentImage]);

	}

	void hideL3(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 1000.f, 0.f));

		vkMapMemory(device, l3DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l3DS.uniformBuffersMemory[0][currentImage]);

	}

	void hideL4(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 1000.f, 0.f));

		vkMapMemory(device, l4DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l4DS.uniformBuffersMemory[0][currentImage]);

	}

	void hideL5(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 1000.f, 0.f));

		vkMapMemory(device, l5DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l5DS.uniformBuffersMemory[0][currentImage]);

	}

	void hideL6(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 1000.f, 0.f));

		vkMapMemory(device, l6DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l6DS.uniformBuffersMemory[0][currentImage]);

	}

	void hideL7(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 1000.f, 0.f));

		vkMapMemory(device, l7DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l7DS.uniformBuffersMemory[0][currentImage]);

	}

	void hideL8(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 1000.f, 0.f));

		vkMapMemory(device, l8DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l8DS.uniformBuffersMemory[0][currentImage]);

	}

	void hideL9(uint32_t currentImage) {

		UniformBufferObject ubo{};
		void* data;

		ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(0.f, 1000.f, 0.f));

		vkMapMemory(device, l9DS.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
		memcpy(data, &ubo, sizeof(ubo));
		vkUnmapMemory(device, l9DS.uniformBuffersMemory[0][currentImage]);

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

		
		if (firstTime) {
			int even = 0;
			float i = level.distanceBetweenRocksX*2;
			for (auto& obj : rockObjects) {

				obj.currentPos = glm::vec3(i, 0.f, (std::rand() % 5 - 2) * level.distanceBetweenRocksZ);
				even++;
				//std::cout << "CIAO " << i << " " << level.numberRocksLine << "\n";
				if (even >= level.numberRocksLine) {
					i += level.distanceBetweenRocksX;
					even = 0;
				}
				
			}
			firstTime = false;
		}
		

		for (auto& obj : rockObjects) {
			if (boatObject.currentPos.z +1.f < obj.currentPos.z + 3.f && boatObject.currentPos.z - 1.f > obj.currentPos.z - 3.f && 
					(boatObject.currentPos.x + 2.f > obj.currentPos.x - 1.f && boatObject.currentPos.x - 2.4f < obj.currentPos.x + 1.f) ) {
				state = LOST;
			}
		}

		for (auto& obj : rockObjects) {

			if (boatObject.currentPos.x > obj.currentPos.x + 10.f) {
				obj.currentPos = glm::vec3(obj.currentPos.x + level.distanceBetweenRocksX * (level.maxNumberRock / level.numberRocksLine), 0.f, (std::rand() % 5 - 2) * level.distanceBetweenRocksZ);
				ubo.model = glm::translate(glm::mat4(1.f), obj.currentPos) * ubo.model;
				vkMapMemory(device, obj.ds.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
				memcpy(data, &ubo, sizeof(ubo));
				vkUnmapMemory(device, obj.ds.uniformBuffersMemory[0][currentImage]);
			}
			else {
				//std::cout << obj.currentPosX << "\n";
				ubo.model = glm::translate(glm::mat4(1.0f), obj.currentPos) * glm::scale(glm::mat4(1.0), glm::vec3(0.2, 0.5, 0.5))
					* glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0.f, 1.f, 0.f));
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

		static float angle = glm::radians(0.f);

		void* data;

		UniformBufferObject ubo{};

		/* ALWAYS INCREMENTING THE X POSITION OF 5.f FOR MOVING STRAIGHT */
		if (glfwGetKey(this->window, GLFW_KEY_P))
			state = PLAYING;
		
		if (state == PLAYING) {
			boatObject.currentPos += glm::vec3(level.boatSpeed.x, 0.f, 0.f) * delta;

			/* UPDATING FOR MOVING RIGHT OR LEFT */
			if (glfwGetKey(this->window, GLFW_KEY_D) || glfwGetKey(this->window, GLFW_KEY_RIGHT)) {
				angle = angle < glm::radians(-10.f) ? glm::radians(-10.f) : angle + glm::radians(-1.f);
				boatObject.currentPos += glm::vec3(0.f, 0.f, level.boatSpeed.z) * delta;
				if (boatObject.currentPos.z > 10.f - 1.f)
					boatObject.currentPos.z = 9.f;
			}
			else if (glfwGetKey(this->window, GLFW_KEY_A) || glfwGetKey(this->window, GLFW_KEY_LEFT)) {
				angle = angle > glm::radians(10.f) ? glm::radians(10.f) : angle + glm::radians(1.f);
				boatObject.currentPos -= glm::vec3(0.f, 0.f, level.boatSpeed.z) * delta;
				if (boatObject.currentPos.z < -10.f + 1.f)
					boatObject.currentPos.z = -9.f;
			}
			else {
				if (angle > glm::radians(0.f))
					angle -= glm::radians(1.f);
				else if (angle < glm::radians(0.f))
					angle += glm::radians(1.f);
			}

			
		}

		ubo.model = glm::translate(glm::mat4(1.0f), boatObject.currentPos) * glm::scale(glm::mat4(1.0), glm::vec3(0.005, 0.005, 0.005))
				* glm::rotate(glm::mat4(1.0f), static_cast<float>(glm::radians(180.f)), glm::vec3(0.f, 1.f, 0.f))
				* glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.f, 1.f, 0.f));

			//boatObject.currentPos = pos;

			//std::cout << boatObject.currentPos.x << "  " << boatObject.currentPos.z << "\n";

			vkMapMemory(device, boatObject.ds.uniformBuffersMemory[0][currentImage], 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(device, boatObject.ds.uniformBuffersMemory[0][currentImage]);
		


		

	}

	void updateFinishLine(uint32_t currentImage) {
		UniformBufferObject ubo{};
		void* data;

		
		if (boatObject.currentPos.x - 4.f > level.distanceFinishLine - 1.f && boatObject.currentPos.x - 8.4f < level.distanceFinishLine + 1.f) {
			state = WIN;
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