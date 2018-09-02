#include "Common.h"

//#define my_sizeof(type) ((char *)(&type+1)-(char*)(&type))
//int size = my_sizeof(myVertices)/my_sizeof(myVertices[0]);

GLFWwindow*window;
int SCREEN_WIDTH = 800, SCREEN_HEIGHT = 600;
vec3 gOrientation1;
vec3 gPosition1(-1.5f, 0.0f, 0.0f);
vec3 gPosition2( 1.5f, 0.0f, 0.0f);
quat gOrientation2;

float horizontalAngle = 3.14f;//Initial horizontal angle : toward -Z
float verticalAngle = 0.0f;//Initial vertical angle : none
float speed = 3.0f;//3 units / second
float mouseSpeed = 0.005f;

float modelX = 0, modelY = 0, modelZ = 0;
float eyeX = 0, eyeY = 0, eyeZ = 25;//Initial position : on +Z
glm::vec3 eye = glm::vec3(eyeX, eyeY, eyeZ);

GLuint MatrixID;
using namespace glm;

void wait(double seconds)
{
	double endwait;//clock_t
	endwait = clock() + seconds * CLOCKS_PER_SEC;
	while(clock() < endwait){}
}

int main()
{
	if(!glfwInit())//Initialise GLFW
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//Open a window and create its OpenGL context
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "GLSL TEST!", NULL, NULL);
	if(window == NULL)
	{
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	//GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	//const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	//glfwSetWindowPos(window, (mode->width - 200) / 2, (mode->height - 200) / 2);
	
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);//Ensure we can capture the escape key being pressed below
	//Hide the mouse and enable unlimited movement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);//GLFW_CURSOR_DISABLED);
	glfwPollEvents();
	//glfwSetCursorPos(window, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);//Set the mouse at the center of the screen
	//Initialize GLEW
	glewExperimental = true;//Needed for core profile
	if(glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);//Dark blue background
	glEnable(GL_DEPTH_TEST);//Enable depth test
	glDepthFunc(GL_LESS);//Accept fragment if it closer to the camera than the former one
	glEnable(GL_CULL_FACE);//Cull triangles which normal is not towards the camera


	char loadmodel[] = "test1.fbx", loadmodel1[] = "maya.obj";
	char loadtexture[] = "texture.png", loadtexture1[] = "texture1.png";
	
	
	initVBO(loadmodel, loadtexture);//LOAD FBX MODEL
	//initVBO(loadmodel1, loadtexture1);//LOAD OBJ MODEL

	
	int frames = 0;
	float deltaTime;
	double currentTime;
	double limitFPS = 1.0/50.0;
	double lastTime = glfwGetTime();
	double timer = lastTime;
	float rotX = 0, rotY = 0, rotZ = 0;
	bool mousefix = 0;

	for(;;)
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		currentTime = glfwGetTime();
		deltaTime = float(currentTime - lastTime);
		double xpos, ypos, lastxpos, lastypos;//Get mouse position
		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			if(mousefix == 0)
			{
				xpos = float(SCREEN_WIDTH /2);
				ypos = float(SCREEN_HEIGHT/2);
				mousefix = 1;
				//printf("TEST XYPOS: %f - %f\n", xpos, ypos);
			}
			else
			{
				glfwGetCursorPos(window, &xpos, &ypos);
			}
			glfwGetWindowSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
			glfwSetCursorPos(window, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);//Reset mouse position for next frame
			//Compute new orientation
			horizontalAngle += mouseSpeed * float(SCREEN_WIDTH /2 - xpos);
			verticalAngle   += mouseSpeed * float(SCREEN_HEIGHT/2 - ypos);
			//printf("hA: %f vA: %f xPos: %f yPos: %f\n", horizontalAngle, verticalAngle, xpos, ypos);
		}
		if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);//GLFW_CURSOR_DISABLED);
			mousefix = 0;
		}
		
		//Direction : Spherical coordinates to Cartesian coordinates conversion
		glm::vec3 target(cos(verticalAngle) * sin(horizontalAngle), 
						sin(verticalAngle),
						cos(verticalAngle) * cos(horizontalAngle));
		//Right vector
		glm::vec3 right = glm::vec3(sin(horizontalAngle - 3.14f/2.0f),
									0,
									cos(horizontalAngle - 3.14f/2.0f));
		glm::vec3 up = glm::cross(right, target);//Up vector
		glm::mat4 view = LookAt(eye, eye+target, up);
		
		glViewport(0, 0, (GLsizei)SCREEN_WIDTH, (GLsizei)SCREEN_HEIGHT);
		//printf("SW: %d SH: %d\n", SCREEN_WIDTH, SCREEN_HEIGHT);
		glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 1000.0f);
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(projection));
		
		vec3 desiredDir = glm::vec3(rotX, rotY, rotZ);//gPosition1-gPosition2;
		vec3 desiredUp = vec3(0.0f, 1.0f, 0.0f);//+Y

		//EULER TO QUATERNION
		quat targetOrientation = normalize(qLookAt(desiredDir, desiredUp));//Compute the desired orientation
		gOrientation2 = RotateTowards(gOrientation2, targetOrientation, 1.0f*deltaTime);//And interpolate
		//printf("ROT X: %f Y: %f Z: %f W: %f\n", gOrientation2[0], gOrientation2[1], gOrientation2[2], gOrientation2[3]);
		glm::mat4 RotationMatrix = mat4_cast(gOrientation2);

		//MOVE KEYS
		if(glfwGetKey(window, GLFW_KEY_UP)    == GLFW_PRESS)eye += target * deltaTime * speed;//Move forward
		if(glfwGetKey(window, GLFW_KEY_DOWN)  == GLFW_PRESS)eye -= target * deltaTime * speed;//Move backward
		if(glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)eye +=  right * deltaTime * speed;//Strafe right
		if(glfwGetKey(window, GLFW_KEY_LEFT)  == GLFW_PRESS)eye -=  right * deltaTime * speed;//Strafe left
		//EULER X TEST
		if(glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)rotX += deltaTime * 100;
		if(glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)rotX -= deltaTime * 100;
		//EULER Y TEST
		if(glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)rotY += deltaTime * 100;
		if(glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)rotY -= deltaTime * 100;
		//EULER Z TEST
		if(glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)rotZ += deltaTime * 100;
		if(glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)rotZ -= deltaTime * 100;
		//X AXIS
		if(glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)modelX += deltaTime * 10;
		if(glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)modelX -= deltaTime * 10;
		//Y AXIS
		if(glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)modelY += deltaTime * 10;
		if(glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)modelY -= deltaTime * 10;
		//Z AXIS
		if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)modelZ += deltaTime * 10;
		if(glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)modelZ -= deltaTime * 10;

		//glm::mat4 RotationMatrix = eulerAngleYXZ(rotX, rotY, rotZ);
		glm::mat4 TranslationMatrix = translate(mat4(), glm::vec3(modelX, modelY, modelZ));
		glm::mat4 ScalingMatrix = scale(mat4(), vec3(1.0f, 1.0f, 1.0f));//scaling[0], scaling[1], scaling[2]));
		glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix * ScalingMatrix;

		glm::mat4 MVP = projection * view * ModelMatrix;
		//Send our transformation to the currently bound shader in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
			
		//glm::vec3 lightPos = glm::vec3(4,4,-4);
		//glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		loadVBO();


		glfwSwapInterval(0);//UNLOCKFPS?
		glfwSwapBuffers(window);
		glfwPollEvents();

		frames++;
		lastTime = currentTime;
		currentTime = glfwGetTime();
		deltaTime = (float)(currentTime - lastTime);
		wait(limitFPS - deltaTime);
		if(currentTime - timer > 1.0)//Reset after one second
		{
			timer++;
			//printf("FPS: %d - %fms\n", frames, 1000.0/double(frames));
			frames = 0;
		}
		if((glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && glfwWindowShouldClose(window) == 0))break;
	}
	deleteVBO();
	glfwTerminate();
	return 0;
}