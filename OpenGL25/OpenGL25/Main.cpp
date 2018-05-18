#define _CRT_SECURE_NO_WARNINGS
#define GLEW_STATIC
#define FREEGLUT_STATIC
#define _LIB
#define FREEGLUT_LIB_PRAGMAS 0
#pragma comment(lib, "libfbxsdk-mt.lib")
#pragma comment(lib, "libpng16.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "freeglut_static.lib")
#pragma comment(lib, "glfw3.lib")
#include <stdio.h>
#include <stdlib.h>
#include <glew.h>
#include <freeglut.h>
#include <glfw3.h>
// Include GLM
#include <glm\glm.hpp>
#include <glm\vec3.hpp>
#include <glm\vec4.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

#include <fbxsdk.h>
#include <png.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <time.h>
#include <math.h>

#include "object.h"

#include "Camera.h"
#include "load_shaders.h"
#include "load_textures.h"
#include "objloader.h"
#include "vboindexer.h"
#include "quaternion_utils.h"

using namespace glm;

GLFWwindow*window;
int SCREEN_WIDTH = 800, SCREEN_HEIGHT = 600;
GLuint programID;

vec3 gOrientation1;
vec3 gPosition1(-1.5f, 0.0f, 0.0f);
vec3 gPosition2( 1.5f, 0.0f, 0.0f);
quat gOrientation2;

// Initial position : on +Z
float eyeX = 0, eyeY = 1, eyeZ = 5;
glm::vec3 eye = glm::vec3(eyeX, eyeY, eyeZ);
float modelX = 0, modelY = 0, modelZ = 0;
float horizontalAngle = 3.14f; // Initial horizontal angle : toward -Z
float verticalAngle = 0.0f; // Initial vertical angle : none
float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.005f;



int TotalNumVerts = 0, numIndices = 0, polyloop = 0, polyvert = 0, toggledraw = 0;
int polygonCount, triangleCount;
int*indices;
float*normals;



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
void PrintAttribute(FbxNodeAttribute*pAttribute, FbxNode*pNode, FbxScene*pScene, FbxTakeInfo*pTakeInfo)
{
	if(!pAttribute)return;
	FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
	if(typeName ==  GetAttributeTypeName(fbxsdk::FbxNodeAttribute::eMesh))
	{
		FbxMesh*mesh = pNode->GetMesh();
		//================= Get Vertices ====================================
		FbxVector4*pVertices = mesh->GetControlPoints();
		TotalNumVerts = mesh->GetControlPointsCount();
		printf("TotalNumVerts: %d\n", TotalNumVerts);
		for(int j = 0; j < TotalNumVerts; j++)
		{

			FbxVector4 vert = mesh->GetControlPointAt(j);
			vtx[j*4+0] = (float)vert.mData[0];
			vtx[j*4+1] = (float)vert.mData[1];
			vtx[j*4+2] = (float)vert.mData[2];
			vtx[j*4+3] = 1.0f;
			printf("MeshVert: x: %f y: %f z: %f w: %f\n",
				vtx[j*4+0], vtx[j*4+1], vtx[j*4+2], vtx[j*4+3]);
		}

		//================= Get Indices ====================================
		numIndices = mesh->GetPolygonVertexCount();
		printf("numIndices: %i\n", numIndices);
		indices = mesh->GetPolygonVertices();
		printf("INDICES: %d - %d - %d - %d - %d - %d\n", indices[0], indices[1], indices[2], indices[3], indices[4], indices[5]);
		
		/*
		int asdf = 0;
		while(indices[asdf] != EOF)
		{
			printf("%d - IND: %d\n", asdf, indices[asdf]);
			asdf++;
		}
		printf("sizeof indice: %d\n", sizeof(indices));
		*/


		triangleCount = numIndices / 3;
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
					//cout<<normals[vertexCounter*3+0]<<" "<<normals[vertexCounter*3+1]<<" "<<normals[vertexCounter*3+2]<<"\n";
					vertexCounter++;
				}
			}
		}

		/*
		int polygonCount = mesh->GetPolygonCount();
		for(int i = 0; i < polygonCount; i++)
		{
			for(unsigned iPolygonVertex = 0; iPolygonVertex < 3; iPolygonVertex++)
			{
				int fbxCornerIndex = mesh->GetPolygonVertex(i, iPolygonVertex);
				// Get texture coordinate
				FbxVector2 fbxUV = FbxVector2(0.0, 0.0);
				FbxLayerElementUV* fbxLayerUV = mesh->GetLayer(0)->GetUVs();
				if(fbxLayerUV)
				{	
					int iUVIndex = 0;
					switch(fbxLayerUV->GetMappingMode())
					{
					case FbxLayerElement::eByControlPoint:
						iUVIndex = fbxCornerIndex;
						break;
					case FbxLayerElement::eByPolygonVertex:
						iUVIndex = mesh->GetTextureUVIndex(i, iPolygonVertex, FbxLayerElement::eTextureDiffuse);
						break;
					}
					fbxUV = fbxLayerUV->GetDirectArray().GetAt(iUVIndex);	
				}
			}
		}
		*/

		/////////////////////////////////////////////////////////////////
		polygonCount = mesh->GetPolygonCount();
		FbxVector4* controlPoints = mesh->GetControlPoints();
		int controlPointCount = mesh->GetControlPointsCount();
		int vertexID = 0;

		int iUVIndex = 0;

		for(int polygon = 0; polygon < mesh->GetPolygonCount(); polygon++)
		{
			//printf("LOOPTEST1: %d\n", polygon);
			//int polyVertCount = mesh->GetPolygonSize(polygon);
			for(int polyVert = 0; polyVert < mesh->GetPolygonSize(polygon); polyVert++)
			{
				//printf("LOOPTEST2: %d\n", polyVert);
				//CMesh::TUniqueVertex uniqueVert;
				int cpIndex = mesh->GetPolygonVertex(polygon, polyVert);
				// Grab our CP index as well our position information
				//uniqueVert.m_nControlPointIndex = cpIndex;
				iUVIndex = cpIndex;
				/*
				uniqueVert.m_vPosition = 
					XMFLOAT3((float)controlPoints[cpIndex].mData[0],
					(float)controlPoints[cpIndex].mData[1],
					(float)controlPoints[cpIndex].mData[2]);
				*/
				
				test[MAX_VERTICES].x = (float)controlPoints[cpIndex].mData[0];
				test[MAX_VERTICES].y = (float)controlPoints[cpIndex].mData[1];
				test[MAX_VERTICES].z = (float)controlPoints[cpIndex].mData[2];
				//printf("TEST X: %f Y: %f Z: %f\n", test[MAX_VERTICES].x, test[MAX_VERTICES].y, test[MAX_VERTICES].z);

				// Grab UVs
				int uvElementCount = mesh->GetElementUVCount();
				//printf("uvElementCount: %d\n", uvElementCount);
				for(int uvElement = 0; uvElement < mesh->GetElementUVCount(); uvElement++)
				{
					FbxGeometryElementUV* geomElementUV = mesh->GetElementUV(uvElement);
					FbxLayerElement::EMappingMode mapMode = geomElementUV->GetMappingMode();
					FbxLayerElement::EReferenceMode refMode = geomElementUV->GetReferenceMode();
					int directIndex = -1;
					if(FbxGeometryElement::eByControlPoint == mapMode)
					{
						if(FbxGeometryElement::eDirect == refMode)directIndex = cpIndex;
						else if(FbxGeometryElement::eIndexToDirect == refMode)
							directIndex = geomElementUV->GetIndexArray().GetAt(cpIndex);
					}
					else if(FbxGeometryElement::eByPolygonVertex == mapMode)
					{
						if(FbxGeometryElement::eDirect == refMode || FbxGeometryElement::eIndexToDirect == refMode)
							directIndex = mesh->GetTextureUVIndex(polygon, polyVert);
					}
					// If we got a UV index
					if(directIndex != -1)
					{
						FbxVector2 uv = geomElementUV->GetDirectArray().GetAt(directIndex);
						//uniqueVert.m_UVs = XMFLOAT2( (float)uv.mData[0], (float)uv.mData[1]);
						mapcoords.u[polygon][polyVert] = (float)uv.mData[0];
						mapcoords.v[polygon][polyVert] = (float)uv.mData[1];
						//printf("Loop: %d:%d UV: %f and %f\n", polygon, polyVert, mapcoords.u[polygon][polyVert], mapcoords.v[polygon][polyVert]);
					}
				}
			}
		}
		
		/*
		for(int asdf=0;asdf<35;asdf++)
		{
			for(int qwer=0;qwer<3;qwer++)
			{
			printf("CRAP: %f and %f\n", mapcoords.u[asdf][qwer], mapcoords.v[asdf][qwer]);
			}
		}
		_getch();
		*/

		/*
		//GET POLYGONS
		int polygonCount = mesh->GetPolygonCount();
		int vertexCount = 0;
		for(int i = 0; i < polygonCount; i++)
		{
			vertexCount += mesh->GetPolygonSize(i);

			for(int j = 0; j < mesh->GetPolygonSize(i); j++)
			{
				int uv[3] = mesh->GetTextureUVIndex(i, j);
				mapcoord[MAX_VERTICES].u = uv[0];
				mapcoord[MAX_VERTICES].v = uv[1];
			}
		}
		*/

		int texCoordCount = mesh->GetTextureUVCount();
		printf("texCoordCount: %d\n", texCoordCount);
		//GET UV////////////////////////////////////////////////////////////////
		FbxStringList UVNames;
		mesh->GetUVSetNames(UVNames);
		FbxArray<FbxVector2> uvsets;
		mesh->GetPolygonVertexUVs(UVNames.GetStringAt(0), uvsets);
		//GET MATERIALS////////////////////////////////////////////////////////////////
		int meshCount = pScene->GetSrcObjectCount<FbxMesh>();
		for (int i = 0; i < meshCount; ++i)
		{
			FbxMesh*mesh = pScene->GetSrcObject<FbxMesh>(i);
		}
		int materialCount = pScene->GetMaterialCount();
		for (int i = 0; i < materialCount; ++i)
		{
			FbxSurfaceMaterial*material = pScene->GetMaterial(i);  
		}
		////////////////////////////////////////////////////////////////
		printf("MESH MEMORY USAGE: %d\n", mesh->MemoryUsage());
		unsigned int numOfDeformers = mesh->GetDeformerCount(FbxDeformer::EDeformerType::eSkin);
		printf("MESH DEFORMER COUNT: %d\n", numOfDeformers);
		for(unsigned int deformerIndex = 0; deformerIndex < numOfDeformers; ++deformerIndex)
		{
			FbxSkin*skin = (FbxSkin*)(mesh->GetDeformer(deformerIndex, FbxDeformer::EDeformerType::eSkin));
			if(skin)
			{
				for (int i = 0; i < skin->GetClusterCount(); i++)
				{
					FbxCluster*cluster = skin->GetCluster(i);
					printf("CLUSTER #%i has %i vertices\n", i, cluster->GetControlPointIndicesCount());
					FbxString currJointName = cluster->GetLink()->GetName(); 
					printf("CURRENTJOINT: %s\n", currJointName.Buffer());

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
					// Get the bind pose
					FbxAMatrix transformMatrix;
					FbxAMatrix transformLinkMatrix;
					FbxAMatrix globalBindposeInverseMatrix;
				
					cluster->GetTransformMatrix(transformMatrix);
					cluster->GetTransformLinkMatrix(transformLinkMatrix);
					globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix *GetNodeWorldTransform(pNode);





					int*pVertexIndices = cluster->GetControlPointIndices();
					double*pVertexWeights = cluster->GetControlPointWeights();
					//Iterate through all the vertices, which are affected by the bone
					//_getch();
					int ncVertexIndices = cluster->GetControlPointIndicesCount();
					for(int iBoneVertexIndex = 0; iBoneVertexIndex < ncVertexIndices; iBoneVertexIndex++)
					{
						//printf("Loop: %d - ", iBoneVertexIndex);
						//vertex
						int niVertex = pVertexIndices[iBoneVertexIndex];
						//printf("nV: %d - ", niVertex);
						//weight
						float fWeight = (float)pVertexWeights[iBoneVertexIndex];
						//printf("fW: %f\n", fWeight);
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
					printf("START: %f END: %f\n", startTime, endTime);

					FbxAnimLayer* pAnimLayer = FbxAnimLayer::Create(pScene, "Layer0");
					currAnimStack->AddMember(pAnimLayer);
					//Get the camera’s curve node for local translation.
					FbxAnimCurveNode* myAnimCurveNodeRot = pNode->LclRotation.GetCurveNode(pAnimLayer, true);
					//create curve nodes
					FbxAnimCurve* myRotXCurve = myAnimCurveNodeRot->GetCurve(0);
					FbxAnimCurve* myRotYCurve = myAnimCurveNodeRot->GetCurve(1);
					FbxAnimCurve* myRotZCurve = myAnimCurveNodeRot->GetCurve(2);
					FbxTime lTime; //For the start and stop keys
					int lKeyIndex = 0;
					myRotXCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
					myRotYCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
					myRotZCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
					//printf("X: %d Y: %d Z: %d\n", myRotXCurve, myRotYCurve, myRotZCurve);
					//printf("X: %f Y: %f Z: %f\n", myRotXCurve, myRotYCurve, myRotZCurve);
					
					FbxLongLong mAnimationLength = end.GetFrameCount(FbxTime::eFrames30) - start.GetFrameCount(FbxTime::eFrames30) + 1;
					//printf("ANIMATIONLENGTH: %lld\n", mAnimationLength);
					for(FbxLongLong ii = start.GetFrameCount(FbxTime::eFrames30); ii <= end.GetFrameCount(FbxTime::eFrames30); ii++)
					{
						//printf("F: #%d, ", ii);
						FbxTime currTime;
						currTime.SetFrame(ii, FbxTime::eFrames30);
						FbxAMatrix currentTransformOffset = pNode->EvaluateGlobalTransform(currTime) * GetNodeWorldTransform(pNode);
						FbxAMatrix matr = currentTransformOffset.Inverse() * cluster->GetLink()->EvaluateGlobalTransform(currTime);
					}
				}
			}
		}
	}
	FbxString attrName = pAttribute->GetName();
	printf("<Attribute type: %s Name: %s>\n", typeName.Buffer(), attrName.Buffer());
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
		PrintAttribute(pNode->GetNodeAttributeByIndex(i), pNode, pScene, pTakeInfo);
	for(int j = 0; j < pNode->GetChildCount(); j++)
		PrintNode(pNode->GetChild(j), pScene, pTakeInfo);
}

void loadFBX()
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

		FbxManager*lSdkManager = FbxManager::Create();
		FbxScene*lScene = FbxScene::Create(lSdkManager, "SceneName");
		FbxNode*lRootNode = lScene->GetRootNode();
		FbxDocument*lDoc = lScene;
		FbxIOSettings*lioSettings = FbxIOSettings::Create(lSdkManager, IOSROOT);
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
			}
		}
		lSdkManager->Destroy();
	}
}



void wait(double seconds)
{
	double endwait; //clock_t
	endwait = clock() + seconds * CLOCKS_PER_SEC;
	while(clock() < endwait){}
}
int main()
{
	if(!glfwInit()) //Initialise GLFW
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return -1;
	}
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// Open a window and create its OpenGL context
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Tutorial 09 - VBO Indexing", NULL, NULL);
	if(window == NULL)
	{
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	//Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    //Hide the mouse and enable unlimited movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		//GLFW_CURSOR_NORMAL); 
    //Set the mouse at the center of the screen
	glfwPollEvents();
	glfwSetCursorPos(window, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);

	//Initialize GLEW
	glewExperimental = true; //Needed for core profile
	if(glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
	glEnable(GL_DEPTH_TEST); // Enable depth test
	glDepthFunc(GL_LESS); // Accept fragment if it closer to the camera than the former one
	glEnable(GL_CULL_FACE); // Cull triangles which normal is not towards the camera

	// Create and compile our GLSL program from the shaders
	loadShaders(programID, "shaders/vert_shader.glsl", "shaders/frag_shader.glsl");

	int width, height;
	bool hasAlpha;
	char filename[] = "texture.png", filename1[] = "ntexture.png", filename2[] = "stexture.png";
	// Load the texture
	GLuint Texture = //loadBMP_custom("test.bmp");
		loadPngImage(filename, width, height, hasAlpha, &textureImage);
		//LoadBMP("test.bmp");
		//loadDDS("uvmap.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTexture");
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	/*
	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = loadOBJ("cube.obj", vertices, uvs, normals);

	std::vector<unsigned short> indices;
	std::vector<glm::vec3> indexed_vertices;
	std::vector<glm::vec2> indexed_uvs;
	std::vector<glm::vec3> indexed_normals;
	indexVBO(vertices, uvs, normals, indices, indexed_vertices, indexed_uvs, indexed_normals);
	*/


	loadFBX();


	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	//VBO BUFFERS
	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * TotalNumVerts * 4, vtx, GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);
	/*
	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);
	GLuint normalbuffer;
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);
	*/
	GLuint elementbuffer;
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * numIndices, indices, GL_STATIC_DRAW);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	glUseProgram(programID);
	// Get a handle for our "LightPosition" uniform
	//GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

	int frames = 0;
	int nbFrames = 0;
	double currentTime;
	float deltaTime;
	double limitFPS = 1.0/45.0;
	double lastTime = glfwGetTime();
	double timer = lastTime;

	float rotX = 0, rotY = 0, rotZ = 0;

	for(;;)
	{
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

		currentTime = glfwGetTime();
		deltaTime = float(currentTime - lastTime);

		// Get mouse position
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		// Reset mouse position for next frame
		glfwSetCursorPos(window, SCREEN_WIDTH/2, SCREEN_HEIGHT/2);

		// Compute new orientation
		horizontalAngle += mouseSpeed * float(SCREEN_WIDTH /2 - xpos);
		verticalAngle   += mouseSpeed * float(SCREEN_HEIGHT/2 - ypos);
		//printf("hA: %f vA: %f xPos: %f yPos: %f\n", horizontalAngle, verticalAngle, xpos, ypos);

		// Direction : Spherical coordinates to Cartesian coordinates conversion
		glm::vec3 target(cos(verticalAngle) * sin(horizontalAngle), 
						sin(verticalAngle),
						cos(verticalAngle) * cos(horizontalAngle));
		// Right vector
		glm::vec3 right = glm::vec3(sin(horizontalAngle - 3.14f/2.0f), 
									0,
									cos(horizontalAngle - 3.14f/2.0f));
		glm::vec3 up = glm::cross(right, target); // Up vector

		if(glfwGetKey( window, GLFW_KEY_UP ) == GLFW_PRESS){
			eye += target * deltaTime * speed;} // Move forward
		if(glfwGetKey( window, GLFW_KEY_DOWN ) == GLFW_PRESS){
			eye -= target * deltaTime * speed;} // Move backward
		if(glfwGetKey( window, GLFW_KEY_RIGHT ) == GLFW_PRESS){
			eye += right * deltaTime * speed;} // Strafe right
		if(glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
			eye -= right * deltaTime * speed;} // Strafe left

		//EULER X TEST
		if(glfwGetKey( window, GLFW_KEY_1 ) == GLFW_PRESS){
			rotX += deltaTime * 100;}
		if(glfwGetKey( window, GLFW_KEY_2 ) == GLFW_PRESS){
			rotX -= deltaTime * 100;}
		//EULER Y TEST
		if(glfwGetKey( window, GLFW_KEY_3 ) == GLFW_PRESS){
			rotY += deltaTime * 100;}
		if(glfwGetKey( window, GLFW_KEY_4 ) == GLFW_PRESS){
			rotY -= deltaTime * 100;}
		//EULER Z TEST
		if(glfwGetKey( window, GLFW_KEY_5 ) == GLFW_PRESS){
			rotZ += deltaTime * 100;}
		if(glfwGetKey( window, GLFW_KEY_6 ) == GLFW_PRESS){
			rotZ -= deltaTime * 100;}

		glm::mat4 projection = glm::perspective(glm::radians(60.0f), (float)SCREEN_WIDTH/(float)SCREEN_HEIGHT, 0.1f, 1000.0f);
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(projection));
		glm::mat4 view = LookAt(eye, eye+target, up);
		
		vec3 desiredDir = glm::vec3(rotX, rotY, rotZ); //gPosition1-gPosition2;
		vec3 desiredUp = vec3(0.0f, 1.0f, 0.0f); // +Y

		//EULER TO QUATERNION
		// Compute the desired orientation
		quat targetOrientation = normalize(qLookAt(desiredDir, desiredUp));
		// And interpolate
		gOrientation2 = RotateTowards(gOrientation2, targetOrientation, 1.0f*deltaTime);
		//printf("ROT X: %f Y: %f Z: %f W: %f\n", gOrientation2[0], gOrientation2[1], gOrientation2[2], gOrientation2[3]);
		glm::mat4 RotationMatrix = mat4_cast(gOrientation2);

		if(glfwGetKey( window, GLFW_KEY_I ) == GLFW_PRESS){
			modelX += deltaTime * 10;}
		if(glfwGetKey( window, GLFW_KEY_J ) == GLFW_PRESS){
			modelX -= deltaTime * 10;}

		if(glfwGetKey( window, GLFW_KEY_O ) == GLFW_PRESS){
			modelY += deltaTime * 10;}
		if(glfwGetKey( window, GLFW_KEY_K ) == GLFW_PRESS){
			modelY -= deltaTime * 10;}

		if(glfwGetKey( window, GLFW_KEY_P ) == GLFW_PRESS){
			modelZ += deltaTime * 10;}
		if(glfwGetKey( window, GLFW_KEY_L ) == GLFW_PRESS){
			modelZ -= deltaTime * 10;}

		//glm::mat4 RotationMatrix = eulerAngleYXZ(rotX, rotY, rotZ);
		glm::mat4 TranslationMatrix = translate(mat4(), glm::vec3(modelX, modelY, modelZ));
		glm::mat4 ScalingMatrix = scale(mat4(), vec3(1.0f, 1.0f, 1.0f));
		glm::mat4 ModelMatrix = TranslationMatrix * RotationMatrix * ScalingMatrix;

		glm::mat4 MVP = projection * view * ModelMatrix;
		//Send our transformation to the currently bound shader in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		/*
		glm::vec3 lightPos = glm::vec3(4,4,-4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
		*/

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);
		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
		/*
		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		*/
		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
		glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, (void*)0);
		glDisableVertexAttribArray(0);
		//glDisableVertexAttribArray(1);
		//glDisableVertexAttribArray(2);
		glfwSwapBuffers(window);
		glfwPollEvents();

		frames++;
		lastTime = currentTime;
		currentTime = glfwGetTime();
		deltaTime = (float)(currentTime - lastTime);
		wait(limitFPS - deltaTime);
		if(currentTime - timer > 1.0) //Reset after one second
		{
			timer++;
			printf("FPS: %d - %fms\n", frames, 1000.0/double(frames));
			frames = 0;
		}
		if((glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && glfwWindowShouldClose(window) == 0))break;
	}
	glDeleteBuffers(1, &vertexbuffer);
	//glDeleteBuffers(1, &uvbuffer);
	//glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);
	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}