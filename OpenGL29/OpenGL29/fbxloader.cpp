#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "libfbxsdk-mt.lib")
#include <fbxsdk.h>

#include <stdio.h>
#include <vector>
#include <map>

#include <string.h>//to memcmp
#include <string>
#include <cstring>

#include <glm/glm.hpp>
#include "Common.h"

#include <conio.h>

#define MAX_VERTICES 900000 //Max number of vertices(for each object)
#define MAX_UV 900000 //The mapcoord type, 2 texture coordinates for each vertex

struct vertex
{
	float x, y, z, w;
}vertices[MAX_VERTICES], vNormal[MAX_VERTICES], test[MAX_VERTICES];
struct map
{
	float u[9000][3], v[9000][3];
}mapcoords;

//float vtx[MAX_VERTICES];
//int fdraw = (int)fmerge;
int polygonCount, triangleCount, TotalNumVerts = 0, numIndices = 0;//polyloop = 0, polyvert = 0, toggledraw = 0;
int*fbxindices;

int fbxuvIndex[10000];
int fbxnormalIndex[10000];

float myVertices[100000];
float myUVmap[100000];
float myNormals[100000];

std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
std::vector<glm::vec3> temp_vertices, temp_indicesvertices;
std::vector<glm::vec2> temp_uvs;
std::vector<glm::vec3> temp_normals;

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

void DisplayTransformPropagation(FbxNode* pNode)
{
	FBXSDK_printf("    Transformation Propagation\n");
	//Rotation Space
	EFbxRotationOrder lRotationOrder;
	pNode->GetRotationOrder(FbxNode::eSourcePivot, lRotationOrder);
	FBXSDK_printf("        Rotation Space: ");
	
	switch (lRotationOrder)
	{
	case eEulerXYZ:
		FBXSDK_printf("Euler XYZ\n");
		break;
	case eEulerXZY:
		FBXSDK_printf("Euler XZY\n");
		break;
	case eEulerYZX:
		FBXSDK_printf("Euler YZX\n");
		break;
	case eEulerYXZ:
		FBXSDK_printf("Euler YXZ\n");
		break;
	case eEulerZXY:
		FBXSDK_printf("Euler ZXY\n");
		break;
	case eEulerZYX:
		FBXSDK_printf("Euler ZYX\n");
		break;
	case eSphericXYZ:
		FBXSDK_printf("Spheric XYZ\n");
		break;
	}
	//Use the Rotation space only for the limits(keep using eEulerXYZ for the rest)
	FBXSDK_printf("        Use the Rotation Space for Limit specification only: %s\n",
		pNode->GetUseRotationSpaceForLimitOnly(FbxNode::eSourcePivot) ? "Yes" : "No");
	//Inherit Type
	FbxTransform::EInheritType lInheritType;
	pNode->GetTransformationInheritType(lInheritType);
	FBXSDK_printf("        Transformation Inheritance: ");
	switch (lInheritType)
	{
	case FbxTransform::eInheritRrSs:
		FBXSDK_printf("RrSs\n");
		break;
	case FbxTransform::eInheritRSrs:
		FBXSDK_printf("RSrs\n");
		break;
	case FbxTransform::eInheritRrs:
		FBXSDK_printf("Rrs\n");
		break;
	}
}

void PrintAttribute(FbxNodeAttribute*pAttribute, FbxNode*pNode, FbxScene*pScene, FbxTakeInfo*pTakeInfo)
{
	if(!pAttribute)return;
	FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());

	//DisplayTransformPropagation(pNode);

	if(typeName ==  GetAttributeTypeName(fbxsdk::FbxNodeAttribute::eMesh))
	{
		FbxMesh*mesh = pNode->GetMesh();
		//================= Get Indices ====================================
		numIndices = mesh->GetPolygonVertexCount();
		printf("numIndices: %i\n", numIndices);
		fbxindices = mesh->GetPolygonVertices();

		//for(int i = 0; i < numIndices; i += 3)
			//printf("%d: INDICE %d - %d - %d\n", i, fbxindices[i+0], fbxindices[i+1], fbxindices[i+2]);

		//================= Get Vertices ===================================
		FbxVector4*pVertices = mesh->GetControlPoints();
		TotalNumVerts = mesh->GetControlPointsCount();
		printf("TotalNumVerts: %d\n", TotalNumVerts);
		for(int j = 0; j < TotalNumVerts; j++)
		{
			FbxVector4 vert = mesh->GetControlPointAt(j);
			vertices[j].x=(float)vert.mData[0];
			vertices[j].y=(float)vert.mData[1];
			vertices[j].z=(float)vert.mData[2];
			vertices[j].w = 1.0f;

			glm::vec3 tempvertex;
			tempvertex.x = (float)vert.mData[0];
			tempvertex.y = (float)vert.mData[1];
			tempvertex.z = (float)vert.mData[2];
			temp_vertices.push_back(tempvertex);

			//printf("MeshVert: x: %f y: %f z: %f w: %f\n",
				//vertices[j].x, vertices[j].y, vertices[j].z, vertices[j].w);
			//myVertices[j*3+0] = (float)vert.mData[0];
			//myVertices[j*3+1] = (float)vert.mData[1];
			//myVertices[j*3+2] = (float)vert.mData[2];
			//myVertices[j*4+3] = 1.0f;
		}

		/*
		for(int i = 0; i < TotalNumVerts; i ++)
		{
			glm::vec3 outvertex = temp_vertices[i];
			printf("Vloopnum: %d outV X: %f Y: %f Z: %f\n", i, outvertex[0], outvertex[1], outvertex[2]);
		}
		*/
		
		for(int i = 0; i < numIndices; i++)
		{
			int tempstuff = fbxindices[i];
			//printf("i: %d tempstuff: %d\n", i, tempstuff);
			
			glm::vec3 outvertex = temp_vertices[tempstuff];
			//printf("INDloop: %d %d outV X: %f Y: %f Z: %f\n", i, tempstuff, outvertex[0], outvertex[1], outvertex[2]);
			temp_indicesvertices.push_back(outvertex);
		}
		
		triangleCount = numIndices / 3;
		printf("TriangleCount: %i\n", triangleCount);
		//================= Get Normals ====================================
		FbxGeometryElementNormal*normalEl = mesh->GetElementNormal();
		if(normalEl)
		{
			if(normalEl->GetMappingMode() == FbxGeometryElement::eByControlPoint)printf("NORMALCONTROLPOINT!\n");
			if(normalEl->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)printf("NORMALPOLYGONVERTEX!\n");
			int numNormals = mesh->GetPolygonCount()*3;
			//normals = new float[numNormals*3];
			int vertexCounter = 0;
			for(int polyCounter = 0 ; polyCounter<mesh->GetPolygonCount(); polyCounter++)
			{
				int lPolygonSize = mesh->GetPolygonSize(polyCounter);
				//printf("polygonSize: %d\n", lPolygonSize);
				for(int k = 0; k < 3; k++)
				{
					int lNormalIndex = 0;
					//reference mode is direct, the normal index is same as lIndexByPolygonVertex.
					if(normalEl->GetReferenceMode() == FbxGeometryElement::eDirect)
						lNormalIndex = vertexCounter;
					//reference mode is index-to-direct, get normals by the index-to-direct
					if(normalEl->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
						lNormalIndex = normalEl->GetIndexArray().GetAt(vertexCounter);
					//printf("NORMALINDEXTEST: %d\n", lNormalIndex);
					
					fbxnormalIndex[vertexCounter] = lNormalIndex;
					//printf("nIndex: %d\n", fbxnormalIndex[vertexCounter]);

					FbxVector4 normal = normalEl->GetDirectArray().GetAt(vertexCounter);
					//vNormal[vertexCounter].x = (float)normal[0];
					//vNormal[vertexCounter].y = (float)normal[1];
					//vNormal[vertexCounter].z = (float)normal[2];
					//printf("vNormal X: %f Y: %f Z: %f\n",
						//vNormal[vertexCounter].x, vNormal[vertexCounter].y, vNormal[vertexCounter].z);

					myNormals[vertexCounter*3+0] = (float)normal[0];
					myNormals[vertexCounter*3+1] = (float)normal[1];
					myNormals[vertexCounter*3+2] = (float)normal[2];
					//printf("fbxNormal X: %f Y: %f Z: %f\n",
						//fbxNormal[vertexCounter*3+0], fbxNormal[vertexCounter*3+1], fbxNormal[vertexCounter*3+2]);

					glm::vec3 tempNormal;
					tempNormal.x = (float)normal[0];
					tempNormal.y = (float)normal[1];
					tempNormal.z = (float)normal[2];
					temp_normals.push_back(tempNormal);
					
					vertexCounter++;
				}
			}
		}

		/////////////////////////////////////////////////////////////////
		polygonCount = mesh->GetPolygonCount();
		FbxVector4* controlPoints = mesh->GetControlPoints();
		int controlPointCount = mesh->GetControlPointsCount();
		//int vertexID = 0;

		int iUVIndex = 0;
		int uvIndicesLoop = 0;
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
						//printf("UVINDEX? %d\n", directIndex);
						FbxVector2 uv = geomElementUV->GetDirectArray().GetAt(directIndex);
						//uniqueVert.m_UVs = XMFLOAT2( (float)uv.mData[0], (float)uv.mData[1]);
						mapcoords.u[polygon][polyVert] = (float)uv.mData[0];
						mapcoords.v[polygon][polyVert] = (float)uv.mData[1];
						//printf("UVINDEX: %d Loop: %d:%d UV: %f and %f\n",
							//directIndex, polygon, polyVert, mapcoords.u[polygon][polyVert], mapcoords.v[polygon][polyVert]);

						fbxuvIndex[uvIndicesLoop] = directIndex;
						//printf("uvIndex: %d\n", fbxuvIndex[uvIndicesLoop]);	
						
						myUVmap[uvIndicesLoop*2+0] = (float)uv.mData[0];
						myUVmap[uvIndicesLoop*2+1] = (float)uv.mData[1];

						glm::vec2 tempUV;
						tempUV.x = (float)uv.mData[0];
						tempUV.y = (float)uv.mData[1];
						temp_uvs.push_back(tempUV);

						uvIndicesLoop++;
					}
				}
			}
		}
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

FbxDouble3 translation;
FbxDouble3 rotaion;
FbxDouble3 scaling;

void PrintNode(FbxNode*pNode, FbxScene*pScene, FbxTakeInfo*pTakeInfo)
{
	const char*lNodeName = pNode->GetName();
	translation = pNode->LclTranslation.Get();
	rotaion = pNode->LclRotation.Get();
	scaling = pNode->LclScaling.Get();
	printf("Node Name: %s\nTranslation: %f, %f, %f\nRotation: %f, %f, %f\nScale= %f, %f, %f\n",
		lNodeName, translation[0], translation[1], translation[2],
		rotaion[0], rotaion[1], rotaion[2],
		scaling[0], scaling[1], scaling[2]);
	for(int i = 0; i < pNode->GetNodeAttributeCount(); i++)
		PrintAttribute(pNode->GetNodeAttributeByIndex(i), pNode, pScene, pTakeInfo);
	for(int j = 0; j < pNode->GetChildCount(); j++)
	{
		printf("CHILD NODE! %d\n", pNode->GetChildCount());
		PrintNode(pNode->GetChild(j), pScene, pTakeInfo);
		_getch();
	}
}

bool loadmyFBX(
	const char * pFilestr,
	std::vector<glm::vec3> & out_vertices,
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
	)
{
	/*
	//const char *lFilename = "test.fbx";
	//printf("File: %s\n", lFilename);
	//for(int i = 1; i < 2; i++){
		//char pFilestr[32];
		//char Result[12];
		//sprintf(Result, "%d", i);
		//printf("INT TO STRING: %s\n", Result);
		//strcpy(pFilestr,"test");
		//strcat(pFilestr, Result);
		//strcat(pFilestr,".fbx");
		//puts(filestr);
		//printf("FBX File: %s\n", pFilestr);
	*/

	FbxManager*lSdkManager = FbxManager::Create();
	FbxScene*lScene = FbxScene::Create(lSdkManager, "SceneName");
	FbxNode*lRootNode = lScene->GetRootNode();
	FbxDocument*lDoc = lScene;
	FbxIOSettings*lioSettings = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(lioSettings);

	int lFileMajor, lFileMinor, lFileRevision, lSDKMajor, lSDKMinor, lSDKRevision;
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
			out_vertices = temp_indicesvertices;
			out_uvs = temp_uvs;
			out_normals = temp_normals;
		}
	}
	lSdkManager->Destroy();
	return 1;
}