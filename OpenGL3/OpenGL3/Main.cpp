#define _CRT_SECURE_NO_WARNINGS
#define GLEW_STATIC
#define FREEGLUT_STATIC
#define _LIB
#define FREEGLUT_LIB_PRAGMAS 0
#pragma comment(lib, "libfbxsdk-mt.lib")
#pragma comment(lib, "libglew32.lib")
#pragma comment(lib, "freeglut_static.lib")
#include <glew.h>
#include <freeglut.h>
#include <stdio.h>
#include <fbxsdk.h>
#include <iostream>
#include <Windows.h>
#include <conio.h>

#include "object.h"
#include "load_bmp.h"

using namespace std;
char title[] = "My 3D Model Viewer";
int refreshMS = 15; //15ms = 60fps
GLfloat objangle, xRotated, yRotated, zRotated;
int numVertices = 0;
int numIndices = 0;
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
		FbxVector4*pVertices = mesh->GetControlPoints();
		int numVerts = mesh->GetControlPointsCount();
		for(int j = 0; j < numVerts; j++)
		{
			FbxVector4 vert = mesh->GetControlPointAt(j);
			vertices[numVertices].x=(float)vert.mData[0];
			vertices[numVertices].y=(float)vert.mData[1];
			vertices[numVertices++].z=(float)vert.mData[2];
			//printf("MeshVert: x: %f y: %f z: %f\n", vertices[numVertices-1].x, vertices[numVertices-1].y, vertices[numVertices-1].z);
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
		//int polygonCount = mesh->GetPolygonCount();
		FbxVector4* controlPoints = mesh->GetControlPoints();
		int controlPointCount = mesh->GetControlPointsCount();
		int vertexID = 0;

		int iUVIndex = 0;

		for(int polygon = 0; polygon < mesh->GetPolygonCount(); polygon++)
		{
			//int polyVertCount = mesh->GetPolygonSize(polygon);
			for(int polyVert = 0; polyVert < mesh->GetPolygonSize(polygon); polyVert++)
			{
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
				//printf("CRAP: %d\n", uvElementCount);
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
						mapcoord[MAX_VERTICES].u = (float)uv.mData[0];
						mapcoord[MAX_VERTICES].v = (float)uv.mData[1];

						printf("UV: %f and %f\n", uv.mData[0], uv.mData[1]);
						//_getch();
					}
				}
			}
		}

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
		
		////////////////////////////////////////////////////////////////
		//GET UV
		FbxStringList UVNames;
		mesh->GetUVSetNames(UVNames);
		FbxArray<FbxVector2> uvsets;
		mesh->GetPolygonVertexUVs(UVNames.GetStringAt(0), uvsets);
		////////////////////////////////////////////////////////////////
		//GET MATERIALS
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
void initGL()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); //Set background color to black and opaque
	glClearDepth(1.0f); //Set background depth to farthest
	glEnable(GL_DEPTH_TEST); //Enable depth testing for z-culling
	glDepthFunc(GL_LEQUAL); //Set the type of depth-test
	glShadeModel(GL_SMOOTH); //Enable smooth shading
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); //Nice perspective corrections
	
	glEnable(GL_TEXTURE_2D);
    id_texture=LoadBMP("texture.bmp");
	if(id_texture==-1)
	{
		getchar();
	}
}
void display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT); //Clear color and depth buffers
	glMatrixMode(GL_MODELVIEW); //To operate on model-view matrix
	//Render a color-cube consisting of 6 quads with different colors
	glLoadIdentity(); //Reset the model-view matrix
	glTranslatef(0.0f, -10.0f, -65.0f); //Move right and into the screen

	/*
	if(id_texture!=-1)
	{
		glBindTexture(GL_TEXTURE_2D, id_texture); //We set the active texture 
		glEnable(GL_TEXTURE_2D); //Texture mapping ON
	}
	else
	{
		glDisable(GL_TEXTURE_2D); //Texture mapping OFF
	}
	*/

	glRotatef(objangle, xRotated, yRotated, zRotated);
	for(int i = 0; i < numIndices - 3; i+=3)
	{
		glBegin(GL_TRIANGLES);
		glNormal3f(normals[i*3+0], normals[i*3+1], normals[i*3+2]);
		for(int j = i; j <= i + 2; j++)
		{
			if(j == i)
			{
				//printf("HAHAHAHAHA\n");
				glTexCoord2f(mapcoord[polygon[j].a].u,
					mapcoord[polygon[j].a].v);
			}
			if(j == i+1)
			{
				//printf("NOOOOOOOOOOOOOO\n");
				glTexCoord2f(mapcoord[polygon[j].b].u,
					mapcoord[polygon[j].b].v);
			}
			if(j == i+2)
			{
				//printf("ASDF\n");
				glTexCoord2f(mapcoord[polygon[j].c].u,
					mapcoord[polygon[j].c].v);
			}
			glVertex3f(vertices[indices[j]].x, vertices[indices[j]].y, vertices[indices[j]].z);
			//glColor3f(1.0f, 1.0f, 1.0f);
			//printf("J = %d\n", j);
			//_getch();
		}
	}
	glEnd();
	glFlush();
	glutSwapBuffers(); //Swap the front and back frame buffers(double buffering)

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
	if(height == 0) height = 1;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION); //To operate on the Projection matrix
	glLoadIdentity(); //Reset
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}
void PrintNode(FbxNode*pNode, FbxScene*pScene, FbxTakeInfo*pTakeInfo)
{
	const char*lNodeName = pNode->GetName();
	FbxDouble3 translation = pNode->LclTranslation.Get();
	FbxDouble3 rotaion = pNode->LclRotation.Get();
	FbxDouble3 scaling = pNode->LclScaling.Get();
	printf("Node Name: %s\nTranslation: %f, %f, %f\nRotation: %f, %f, %f\n",//Scale= %f, %f, %f\n",
		lNodeName, translation[0], translation[1], translation[2],
		rotaion[0], rotaion[1], rotaion[2],
		scaling[0], scaling[1], scaling[2]);
	for(int i = 0; i < pNode->GetNodeAttributeCount(); i++)
		PrintAttribute(pNode->GetNodeAttributeByIndex(i), pNode, pScene, pTakeInfo);
	for(int j = 0; j < pNode->GetChildCount(); j++)
		PrintNode(pNode->GetChild(j), pScene, pTakeInfo);
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
			glutInit(&argc, argv);
			glutInitDisplayMode(GLUT_DOUBLE); //Enable double buffered mode
			glutInitWindowSize(640, 480);
			glutInitWindowPosition(5, 5);
			glutCreateWindow(title);
			glutDisplayFunc(display);
			glutReshapeFunc(reshape);
			initGL();
			glutTimerFunc(0, timer, 0);
			glutMainLoop();
		}
		lSdkManager->Destroy();
	}
	return 0;
}