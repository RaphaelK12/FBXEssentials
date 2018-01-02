#define _CRT_SECURE_NO_WARNINGS
#define GLEW_STATIC
#define FREEGLUT_STATIC
#define _LIB
#define FREEGLUT_LIB_PRAGMAS 0
#define MAX_VERTICES 80000
#pragma comment(lib, "libfbxsdk-mt.lib")
#pragma comment(lib, "libglew32.lib")
#pragma comment(lib, "freeglut_static.lib")
#include <glew.h>
#include <freeglut.h>
#include <stdio.h>
#include <fbxsdk.h>
#include <iostream>
using namespace std;
 GLfloat objangle, xRotated, yRotated, zRotated;
char title[] = "My 3D Model Viewer";
struct vertex
{
	float x, y, z;
};
int numVertices = 0;
int numIndices = 0;
int refreshMS = 15; //15ms = 60fps
vertex vertices[MAX_VERTICES];
int*indices;
float*normals;
FbxString GetAttributeTypeName(FbxNodeAttribute::EType type)
{
	switch(type)
	{
	case fbxsdk::FbxNodeAttribute::eUnknown:          return "UNKNOWN";
	case fbxsdk::FbxNodeAttribute::eNull:             return "NULL";
	case fbxsdk::FbxNodeAttribute::eMarker:           return "MARKER";
	case fbxsdk::FbxNodeAttribute::eSkeleton:         return "SKELETON";
	case fbxsdk::FbxNodeAttribute::eMesh:             return "MESH";
	case fbxsdk::FbxNodeAttribute::eNurbs:            return "NURBS";
	case fbxsdk::FbxNodeAttribute::ePatch:            return "PATCH";
	case fbxsdk::FbxNodeAttribute::eCamera:           return "CAMERA";
	case fbxsdk::FbxNodeAttribute::eCameraStereo:     return "STEREO";
	case fbxsdk::FbxNodeAttribute::eCameraSwitcher:   return "CAMERA_SWITCHER";
	case fbxsdk::FbxNodeAttribute::eLight:            return "LIGHT";
	case fbxsdk::FbxNodeAttribute::eOpticalReference: return "OPTICAL_REFERENCE";
	case fbxsdk::FbxNodeAttribute::eOpticalMarker:    return "MARKER";
	case fbxsdk::FbxNodeAttribute::eNurbsCurve:       return "NURBS_CURVE";
	case fbxsdk::FbxNodeAttribute::eTrimNurbsSurface: return "TRIM_NURBS_SURFACE";
	case fbxsdk::FbxNodeAttribute::eBoundary:         return "BOUNDARY";
	case fbxsdk::FbxNodeAttribute::eNurbsSurface:     return "NURBS_SURFACE";
	case fbxsdk::FbxNodeAttribute::eShape:            return "SHAPE";
	case fbxsdk::FbxNodeAttribute::eLODGroup:         return "LODGROUP";
	case fbxsdk::FbxNodeAttribute::eSubDiv:           return "SUBDIV";
	case fbxsdk::FbxNodeAttribute::eCachedEffect:     return "CACHEDEFFECT";
	case fbxsdk::FbxNodeAttribute::eLine:             return "LINE";
	default:                                          return "UNKNOWN";
	}
}
FbxAMatrix GetNodeGeometryTransform(FbxNode*pNode)
{
	FbxAMatrix matrixGeo;
	matrixGeo.SetIdentity();
	if(pNode->GetNodeAttribute())
	{
		const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
		const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
		const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
		matrixGeo.SetT(lT);
		matrixGeo.SetR(lR);
		matrixGeo.SetS(lS);
	}
	return matrixGeo;
}
FbxAMatrix GetNodeWorldTransform(FbxNode*pNode)
{
	FbxAMatrix matrixL2W;
	matrixL2W.SetIdentity();
	if(NULL == pNode)
	{
		return matrixL2W;
	}
	matrixL2W=pNode->EvaluateGlobalTransform();
	FbxAMatrix matrixGeo=GetNodeGeometryTransform(pNode);
	matrixL2W*=matrixGeo;
	return matrixL2W;
}
void PrintAttribute(FbxNodeAttribute*pAttribute, FbxNode*pNode, FbxScene*pScene, FbxTakeInfo*pTakeInfo)
{
	if(!pAttribute)return;
	FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
	if(typeName ==  GetAttributeTypeName(fbxsdk::FbxNodeAttribute::eMesh))
	{
		FbxMesh*mesh = pNode->GetMesh();
		//================= Get Vertices ====================================
		int numVerts = mesh->GetControlPointsCount();
		for(int j = 0; j < numVerts; j++)
		{
			FbxVector4 vert = mesh->GetControlPointAt(j);
			vertices[numVertices].x=(float)vert.mData[0];
			vertices[numVertices].y=(float)vert.mData[1];
			vertices[numVertices++].z=(float)vert.mData[2];
			printf("MeshVert: x: %f y: %f z: %f\n", vertices[numVertices-1].x, vertices[numVertices-1].y, vertices[numVertices-1].z);
		}
		//================= Get Indices ====================================
		numIndices = mesh->GetPolygonVertexCount();
		int triangleCount = numIndices / 3;
		indices = new int[numIndices];
		indices = mesh->GetPolygonVertices();
		printf("numIndices: %i\n", numIndices);
		printf("TriangleCount: %i\n", triangleCount);
		//================= Get Normals ====================================
		FbxGeometryElementNormal*normalEl = mesh->GetElementNormal();
		if(normalEl)
		{
			int numNormals = mesh->GetPolygonCount()*3;
			normals = new float[numNormals*3];
			int vertexCounter = 0;
			for(int polyCounter = 0 ; polyCounter<mesh->GetPolygonCount(); polyCounter++)
			{
				for(int k = 0; k < 3; k++)
				{
					FbxVector4 normal = normalEl->GetDirectArray().GetAt(vertexCounter);
					normals[vertexCounter*3+0] = (float)normal[0];
					normals[vertexCounter*3+1] = (float)normal[1];
					normals[vertexCounter*3+2] = (float)normal[2];
					//cout<<"\n"<<normals[vertexCounter*3+0]<<" "<<normals[vertexCounter*3+1]<<" "<<normals[vertexCounter*3+2];
					vertexCounter++;
				}
			}
		}



		/*
		////////////////////////////////////////////////////////////////
		//Get the UV names
		FbxStringList UVNames;
		mesh->GetUVSetNames(UVNames);
		//Get the controlpoints of the mesh
		FbxVector4*pVertices = mesh->GetControlPoints();
		//Get the amount of polygons inside this mesh
		int polygonCount = mesh->GetPolygonCount();
		//For each polygon inside the mesh
		////////////////////////////////////////////////////////////////
		*/
		
		printf("MESH MEMORY USAGE: %d\n", mesh->MemoryUsage());
		printf("MESH DEFORMER COUNT: %d\n", mesh->GetDeformerCount(FbxDeformer::EDeformerType::eSkin));
		FbxSkin*skin = (FbxSkin*)(mesh->GetDeformer(0, FbxDeformer::EDeformerType::eSkin));
		if(skin)
		{
			for (int i = 0; i < skin->GetClusterCount(); i++)
			{
				FbxCluster*cluster = skin->GetCluster(i);
				printf("CLUSTER #%i has %i vertices\n", i, cluster->GetControlPointIndicesCount());
				FbxString currJointName = cluster->GetLink()->GetName(); 
				printf("CURRENTJOINT: %s\n", currJointName.Buffer());

				GetNodeGeometryTransform(pNode);
				//GetNodeWorldTransform(pNode);

				FbxNode*linkedskel = cluster->GetLink();
				if(linkedskel == NULL)
				{
					printf("Cluster have no Linked Skeleton!\n");
					continue;
				}
				if(linkedskel->GetNodeAttribute()->GetAttributeType() != FbxNodeAttribute::EType::eSkeleton)
				{
					printf("Linked Node is not Skeleton Type!\n");
					continue;
				}
				cluster->SetTransformLinkMatrix(linkedskel->EvaluateGlobalTransform());

				FbxAnimStack*currAnimStack = FbxAnimStack::Create(pScene, "Stack001");
				FbxString animStackName = currAnimStack->GetName();
				FbxString mAnimationName = animStackName.Buffer();
				printf("StackName: %s\n", mAnimationName);

				FbxTime start = pTakeInfo->mLocalTimeSpan.GetStart();
				FbxTime end = pTakeInfo->mLocalTimeSpan.GetStop();
				float startTime = (float)start.GetSecondDouble();
				float endTime = (float)end.GetSecondDouble();
				printf("START: %f\n", startTime);
				printf("END: %f\n", endTime);

				FbxLongLong mAnimationLength = end.GetFrameCount(FbxTime::eFrames30) - start.GetFrameCount(FbxTime::eFrames30) + 1;
				printf("ANIMATIONLENGTH: %lld\n", mAnimationLength);
				for(FbxLongLong ii = start.GetFrameCount(FbxTime::eFrames30); ii <= end.GetFrameCount(FbxTime::eFrames30); ii++)
				{
					//printf("F: #%d, ", ii);
					FbxTime currTime;
					currTime.SetFrame(ii, FbxTime::eFrames30);
					FbxAMatrix currentTransformOffset = pNode->EvaluateGlobalTransform(currTime) * GetNodeWorldTransform(pNode);
					FbxAMatrix mat = currentTransformOffset.Inverse() * cluster->GetLink()->EvaluateGlobalTransform(currTime);
				}
			}
		}
	}
	FbxString attrName = pAttribute->GetName();
	printf("<Attribute type: %s Name: %s>\n", typeName.Buffer(), attrName.Buffer());
}
void initGL()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Set background color to black and opaque
	glClearDepth(1.0f); //Set background depth to farthest
	glEnable(GL_DEPTH_TEST); //Enable depth testing for z-culling
	glDepthFunc(GL_LEQUAL); //Set the type of depth-test
	glShadeModel(GL_SMOOTH); //Enable smooth shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); //Nice perspective corrections
}
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers
	glMatrixMode(GL_MODELVIEW);     // To operate on model-view matrix
	// Render a color-cube consisting of 6 quads with different colors
	glLoadIdentity();                 // Reset the model-view matrix
	glTranslatef(0.0f, -20.0f, -75.0f);  // Move right and into the screen
	
	//printf("INDICE: %i\n", numIndices);

	glRotatef(objangle, xRotated, yRotated, zRotated);
	for(int i = 0; i < numIndices - 3; i+=3)
	{
		glBegin(GL_LINES);
		glNormal3f(normals[i*3+0], normals[i*3+1], normals[i*3+2]);
		for(int j = i; j <= i + 2; j++)
		{
			glVertex3f(vertices[indices[j]].x, vertices[indices[j]].y, vertices[indices[j]].z);
			glColor3f(1.0f, 1.0f, 1.0f);
		}
	}
	glEnd();
	glFlush();
	glutSwapBuffers();  // Swap the front and back frame buffers (double buffering)

	objangle += 0.3f;
	//xRotated += 0.3f;
	yRotated += 0.3f;
	zRotated += 0.3f;
}
void timer(int value)
{
	glutPostRedisplay(); //Post re-paint request to activate display()
	glutTimerFunc(refreshMS, timer, 0); //next timer call milliseconds later
}
void reshape(GLsizei width, GLsizei height)
{
	//GLsizei for non-negative integer
	//Compute aspect ratio of the new window
	if (height == 0) height = 1; //To prevent divide by 0
	GLfloat aspect = (GLfloat)width / (GLfloat)height;
	glViewport(0, 0, width, height); //Set the viewport to cover the new window
	//Set the aspect ratio of the clipping volume to match the viewport
	glMatrixMode(GL_PROJECTION); //To operate on the Projection matrix
	glLoadIdentity(); //Reset
	gluPerspective(45.0f, aspect, 0.1f, 100.0f); //Enable perspective projection with fovy, aspect, zNear and zFar
}

void PrintNode(FbxNode*pNode, FbxScene*pScene, FbxTakeInfo*pTakeInfo)
{
	const char*lNodeName = pNode->GetName();
	FbxDouble3 translation = pNode->LclTranslation.Get();
	FbxDouble3 rotaion = pNode->LclRotation.Get();
	FbxDouble3 scaling = pNode->LclScaling.Get();
	printf("Node Name: %s\nTranslation: %f, %f, %f\nRotation: %f, %f, %f\nScale= %f, %f, %f\n",
		lNodeName, translation[0], translation[1], translation[2],
		rotaion[0], rotaion[1], rotaion[2],
		scaling[0], scaling[1], scaling[2]);
	for(int i = 0; i < pNode->GetNodeAttributeCount(); i++)
	{
		PrintAttribute(pNode->GetNodeAttributeByIndex(i), pNode, pScene, pTakeInfo);
	}
	for(int j = 0; j < pNode->GetChildCount(); j++)
	{
		PrintNode(pNode->GetChild(j), pScene, pTakeInfo);
	}
}
int main(int argc, char**argv)
{
	//const char *lFilename = "test.fbx";
	//printf("File: %s\n", lFilename);
	for(int i = 1; i < 2; i++)
	{
		char pFilestr[32];
		char Result[12];
		sprintf(Result, "%d", i);
		//printf("INT TO STRING: %s\n", Result);
		strcpy(pFilestr,"test");
		strcat(pFilestr, Result);
		strcat(pFilestr,".fbx");
		//puts(filestr);
		printf("FBX File: %s\n", pFilestr);

		FbxManager *lSdkManager = FbxManager::Create();
		FbxScene *lScene = FbxScene::Create(lSdkManager, "SceneName");
		FbxNode *lRootNode = lScene->GetRootNode();
		FbxDocument*lDoc = lScene;
		FbxIOSettings *lioSettings = FbxIOSettings::Create(lSdkManager, IOSROOT);
		lSdkManager->SetIOSettings(lioSettings);

		int lFileMajor, lFileMinor, lFileRevision, lSDKMajor, lSDKMinor, lSDKRevision;
		//Get the file version number generate by the FBX SDK.
		FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

		FbxImporter *lImporter = FbxImporter::Create(lSdkManager, "");
		lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);
		if(!lImporter->Initialize(pFilestr, -1, lSdkManager->GetIOSettings()))
		{
			FbxString error = lImporter->GetStatus().GetErrorString();
			printf("Call to FbxImporter->Initialize() failed.\n");
			printf("Error returned: %s\n\n", error.Buffer());
			printf("Error returned: %s \n\n", lImporter->GetStatus().GetErrorString());
			if(lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
			{
				printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
				printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilestr, lFileMajor, lFileMinor, lFileRevision);
			}
			printf("Press Enter to continue . . . ");
			getchar();
			exit(-1);
		}
		printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);

		int lAnimStackCount = lImporter->GetAnimStackCount();
		FbxTakeInfo*lTakeInfo = lImporter->GetTakeInfo(lAnimStackCount);
		printf("Number of Animation Stacks: %d\n", lAnimStackCount);
		printf("Current Animation Stack: %s\n", lImporter->GetActiveAnimStackName().Buffer());
		if(lAnimStackCount != 0)
		{
			for(int i = 0; i < lAnimStackCount; i++)
			{
				lTakeInfo = lImporter->GetTakeInfo(i);
				printf("Animation Stack %d\n", i);
				printf("Name: %s\n", lTakeInfo->mName.Buffer());
				printf("Description: %s\n", lTakeInfo->mDescription.Buffer());
				//Change the value of the import name if the animation stack should be imported under a different name.
				printf("Import Name: %s\n", lTakeInfo->mImportName.Buffer());
				//Set the value of the import state to false if the animation stack should be not be imported. 
				printf("Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
			}
		}
		FbxIOSettings*ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
		lSdkManager->SetIOSettings(ios); //Store IO settings here
		(*(lSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_MATERIAL,        true);
		(*(lSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_TEXTURE,         true);
		(*(lSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_LINK,            true);
		(*(lSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_SHAPE,           true);
		(*(lSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_GOBO,            true);
		(*(lSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_ANIMATION,       true);
		(*(lSdkManager->GetIOSettings())).SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
		lImporter->Import(lDoc);
		lImporter->Destroy();
		if(lRootNode)
		{
			for(int ii = 0; ii < lRootNode->GetChildCount(); ii++)
			{
				PrintNode(lRootNode->GetChild(ii), lScene, lTakeInfo);

				glutInit(&argc, argv); //Initialize GLUT
				glutInitDisplayMode(GLUT_DOUBLE); //Enable double buffered mode
				glutInitWindowSize(640, 480); //Set the window's initial width & height
				glutInitWindowPosition(50, 50); //Position the window's initial top-left corner
				glutCreateWindow(title); //Create window with the given title
				glutDisplayFunc(display); //Register callback handler for window re-paint event
				glutReshapeFunc(reshape);
				initGL(); //Our own OpenGL initialization
				glutTimerFunc(0, timer, 0); //First timer call immediately [NEW]
				glutMainLoop();
			}
		}
		lSdkManager->Destroy();
	}
	return 0;
}

/*
void Model::RenderModel()
{
	int i, j;
	for(i = 0; i < numIndices - 3; i+=3)
	{
		glBegin(GL_TRIANGLES);
		glNormal3f(normals[i*3+0], normals[i*3+1], normals[i*3+2]); 
		for(j = i; j <= i + 2; j++)
		{
			glVertex3f(vertices[indices[j]].x,vertices[indices[j]].y,vertices[indices[j]].z);
		}
		glEnd();
	}
}
*/
