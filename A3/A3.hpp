#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"

#include <glm/glm.hpp>
#include <stack>
#include <memory>
#include <map>

struct LightSource {
	glm::vec3 position;
	glm::vec3 rgbIntensity;
};

class A3 : public CS488Window {
public:
	// Position Mode : Rotate and translate puppet
	// Joint Mode : Select joints and rotate the selected ones
	enum class InteractionMode { Position = 0, Joint };

	A3(const std::string & luaSceneFile);
	virtual ~A3();

protected:
	// Convenience struct for handling the active transformation
	// when walking the scene graph
	struct MatrixStack {
		MatrixStack() : active_transform(glm::mat4(1.0f)) {}

		// Right multiplies the active transform by m. Requires
		// that this transformation is invertible (for calling pop())
		void push(const glm::mat4 &m); 

		// Removes the most recently pushed matrix
		void pop(); 

		// Empties the stack, resets active_transform to the identity
		void reset();

		// Matrix representing the product of all transformations
		// in the stack matrices
		glm::mat4 active_transform;
		std::stack<glm::mat4> matrices;
	};

	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	//-- Virtual callback methods
	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;

	//-- One time initialization methods:
	void processLuaSceneFile(const std::string & filename);
	void createShaderProgram();
	void enableVertexShaderInputSlots();
	void uploadVertexDataToVbos(const MeshConsolidator & meshConsolidator);
	void mapVboDataToVertexShaderInputLocations();
	void initViewMatrix();
	void initLightSources();

	void initPerspectiveMatrix();
	void uploadCommonSceneUniforms();
	void renderSceneGraph(SceneNode &root);

	void processNode(SceneNode &node);
	void processNodeList(std::list<SceneNode *> &nodes);

	void renderArcCircle();

	glm::mat4 m_perpsective;
	glm::mat4 m_view;

	LightSource m_light;

	//-- GL resources for mesh geometry data:
	GLuint m_vao_meshData;
	GLuint m_vbo_vertexPositions;
	GLuint m_vbo_vertexNormals;
	GLint m_positionAttribLocation;
	GLint m_normalAttribLocation;
	ShaderProgram m_shader;

	//-- GL resources for trackball circle geometry:
	GLuint m_vbo_arcCircle;
	GLuint m_vao_arcCircle;
	GLint m_arc_positionAttribLocation;
	ShaderProgram m_shader_arcCircle;

	// BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
	// object. Each BatchInfo object contains an index offset and the number of indices
	// required to render the mesh with identifier MeshId.
	BatchInfoMap m_batchInfoMap;

	std::string m_luaSceneFile;

	std::shared_ptr<SceneNode> m_rootNode;

	MatrixStack m_matrixStack;

	InteractionMode m_mode = InteractionMode::Position;

	bool m_doPicking; 
	std::map<unsigned int, bool> m_selected; // Stores whether a node ID is selected in picking mode

	// Stores whether a GLFW key is being held
	std::map<int, bool> m_keyHeld;

	// Track position of cursor for rotating and translating the puppet
	glm::vec2 m_mouse{0,0};

	// Keep track of how much to translate the puppet, based on mouse movements
	glm::vec3 m_rootTranslation{0,0,0}; 

	// Degrees to rotate joints on the x and y axis respectively, based on mouse movements
	glm::vec2 m_jointRotation{0,0};
};
