#define _CRT_SECURE_NO_WARNINGS
#define GLEW_STATIC
#define FREEGLUT_STATIC
#define _LIB
#define FREEGLUT_LIB_PRAGMAS 0
#pragma comment(lib, "libpng16.lib")
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
#include <png.h>

#include "object.h"
#include "load_textures.h"

using namespace std;

char title[] = "My 3D Model Viewer";
int refreshMS = 15; //15ms = 60fps

float xpos = -10, ypos = -30, zpos = 10, xrot = 90, yrot = 0, zrot = 0, lastx, lasty;

GLfloat objangle, xRotated, yRotated, zRotated;
int numVertices = 0, numIndices = 0, polyloop = 0, polyvert = 0, toggledraw = 0;
int polygonCount;
int*indices;
float*normals;

FbxDouble3 translation, rotation, scaling;

int objanimlength = 60;

int matrInt = 0;


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
	if(typeName ==  GetAttributeTypeName(fbxsdk::FbxNodeAttribute::eSkeleton))
	{
		FbxTimeSpan lTimeLineTimeSpan;
		pScene->GetGlobalSettings().GetTimelineDefaultTimeSpan(lTimeLineTimeSpan);
		//globalMatrix = pNode->EvaluateLocalTransform(0);
		//FbxAMatrix globalinverse = globalMatrix.Inverse();
		
		FbxAMatrix worldmatrix = GetNodeWorldTransform(pNode);
		FbxAMatrix worldmatrixinverse = worldmatrix.Inverse();
		FbxVector4 globalvectorT = worldmatrixinverse.GetT();
		FbxVector4 globalvectorR = worldmatrixinverse.GetR();
		FbxVector4 globalvectorS = worldmatrixinverse.GetS();

		maT.x[matrInt][0][0] = globalvectorT.mData[0];
		maT.y[matrInt][0][1] = globalvectorT.mData[1];
		maT.z[matrInt][0][2] = globalvectorT.mData[2];
		maT.x[matrInt][0][0] *= -1;
		maT.y[matrInt][0][1] *= -1;
		maT.z[matrInt][0][2] *= -1;
		printf("MaT X: %f Y: %f Z: %f\n", maT.x[matrInt][0][0], maT.y[matrInt][0][1], maT.z[matrInt][0][2]);

		maR.x[matrInt][0][0] = globalvectorR.mData[0];
		maR.y[matrInt][0][1] = globalvectorR.mData[1];
		maR.z[matrInt][0][2] = globalvectorR.mData[2];
		maR.x[matrInt][0][0] *= -1;
		maR.y[matrInt][0][1] *= -1;
		maR.z[matrInt][0][2] *= -1;
		printf("MaR X: %f Y: %f Z: %f\n", maR.x[matrInt][0][0], maR.y[matrInt][0][1], maR.z[matrInt][0][2]);
		

		translation = pNode->LclTranslation.Get();
		rotation = pNode->LclRotation.Get();
		scaling = pNode->LclScaling.Get();

		LclT.x[matrInt][0][0] = translation[0];
		LclT.y[matrInt][0][1] = translation[1];
		LclT.z[matrInt][0][2] = translation[2];
		printf("LclT: X: %f Y: %f Z: %f\n", LclT.x[matrInt][0][0], LclT.y[matrInt][0][1], LclT.z[matrInt][0][2]);


		FbxAMatrix fbxM;
		fbxM.SetIdentity();
		if(pNode->GetNodeAttribute())
		{
			const FbxVector4 lT = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
			const FbxVector4 lR = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
			const FbxVector4 lS = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
			fbxM.SetT(lT);
			fbxM.SetR(lR);
			fbxM.SetS(lS);
		}
		FbxAMatrix localMatrix = pNode->EvaluateLocalTransform(0);
		FbxAMatrix Matr = localMatrix*fbxM;
		FbxAMatrix MatrInv = Matr.Inverse();

		FbxVector4 MatrTranslation = MatrInv.GetT();
		FbxVector4 MatrRot = MatrInv.GetR();
		FbxVector4 MatrScale = MatrInv.GetS();
		//printf("T X: %f Y: %f Z: %f\n", MatrTranslation.mData[0], MatrTranslation.mData[1], MatrTranslation.mData[2]);
		//printf("R X: %f Y: %f Z: %f\n", MatrRot.mData[0], MatrRot.mData[1], MatrRot.mData[2]);
		//printf("S X: %f Y: %f Z: %f\n", MatrScale.mData[0], MatrScale.mData[1], MatrScale.mData[2]);

		matT.x[matrInt][0][0] = MatrTranslation.mData[0];
		matT.y[matrInt][0][1] = MatrTranslation.mData[1];
		matT.z[matrInt][0][2] = MatrTranslation.mData[2];
		matT.x[matrInt][0][0] *= -1;
		matT.y[matrInt][0][1] *= -1;
		matT.z[matrInt][0][2] *= -1;
		printf("MatrT X: %f Y: %f Z: %f\n", matT.x[matrInt][0][0], matT.y[matrInt][0][1], matT.z[matrInt][0][2]);


		matR.x[matrInt][0][0] = MatrRot.mData[0];
		matR.y[matrInt][0][1] = MatrRot.mData[1];
		matR.z[matrInt][0][2] = MatrRot.mData[2];
		matR.x[matrInt][0][0] *= -1;
		matR.y[matrInt][0][1] *= -1;
		matR.z[matrInt][0][2] *= -1;
		printf("MatrR X: %f Y: %f Z: %f\n", matR.x[matrInt][0][0], matR.y[matrInt][0][1], matR.z[matrInt][0][2]);

		matS.x[matrInt][0][0] = MatrScale.mData[0];
		matS.y[matrInt][0][1] = MatrScale.mData[1];
		matS.z[matrInt][0][2] = MatrScale.mData[2];
		//printf("MatrS X: %f Y: %f Z: %f\n", matS.x[0][0], matS.y[0][1], matS.z[0][2]);

		//FbxQuaternion QuatRotation = fbxM.GetQ();
		//printf("Quat0: x: %f y: %f z: %f w: %f test: %f\n", QuatRotation[0], QuatRotation[1], QuatRotation[2], QuatRotation[3], QuatRotation[4]);
		matrInt++;
	}

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
					//cout<<normals[vertexCounter*3+0]<<" "<<normals[vertexCounter*3+1]<<" "<<normals[vertexCounter*3+2]<<"\n";
					vertexCounter++;
				}
			}
		}
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
				
				controlpoint[MAX_VERTICES].x = (float)controlPoints[cpIndex].mData[0];
				controlpoint[MAX_VERTICES].y = (float)controlPoints[cpIndex].mData[1];
				controlpoint[MAX_VERTICES].z = (float)controlPoints[cpIndex].mData[2];
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
		for(int i = 0; i < materialCount; ++i)
		{
			FbxSurfaceMaterial*material = pScene->GetMaterial(i);  
		}
		/////////////////////////////////////////////////////////////////

		/*
		long long extratime = 0;
		FbxTimeSpan lTimeLineTimeSpan;
		pScene->GetGlobalSettings().GetTimelineDefaultTimeSpan(lTimeLineTimeSpan);

		for(int i = 0; i < objanimlength; i++)
		{
			FbxTime kTime;
			kTime.SetMilliSeconds(extratime);
			FbxAMatrix fbxM;

			fbxM = pNode->EvaluateGlobalTransform(kTime);
			
			FbxVector4 MatrTranslation = fbxM.GetT();
			FbxVector4 MatrRot = fbxM.GetR();
			FbxVector4 MatrScale = fbxM.GetS();

			//printf("T X: %f Y: %f Z: %f\n",
				//MatrTranslation.mData[0], MatrTranslation.mData[1], MatrTranslation.mData[2]);
			//printf("R X: %f Y: %f Z: %f\n", MatrRot.mData[0], MatrRot.mData[1], MatrRot.mData[2]);
			//printf("S X: %f Y: %f Z: %f\n", MatrScale.mData[0], MatrScale.mData[1], MatrScale.mData[2]);
			/////////////////////////////////////
			matT.x[i][0] = MatrTranslation.mData[0];
			matT.y[i][1] = MatrTranslation.mData[1];
			matT.z[i][2] = MatrTranslation.mData[2];
			printf("MatrT X: %f Y: %f Z: %f\n", matT.x[i][0], matT.y[i][1], matT.z[i][2]);
			/////////////////////////////////////
			matR.x[i][0] = MatrRot.mData[0];
			matR.y[i][1] = MatrRot.mData[1];
			matR.z[i][2] = MatrRot.mData[2];
			printf("MatrR X: %f Y: %f Z: %f\n", matR.x[i][0], matR.y[i][1], matR.z[i][2]);
			/////////////////////////////////////
			matS.x[i][0] = MatrScale.mData[0];
			matS.y[i][1] = MatrScale.mData[1];
			matS.z[i][2] = MatrScale.mData[2];
			printf("MatrS X: %f Y: %f Z: %f\n", matS.x[i][0], matS.y[i][1], matS.z[i][2]);
			/////////////////////////////////////
			FbxQuaternion QuatRotation = fbxM.GetQ();
			//printf("Quat0: x: %f y: %f z: %f w: %f test: %f\n", QuatRotation[0], QuatRotation[1], QuatRotation[2], QuatRotation[3], QuatRotation[4]);


			extratime+=30;
		}
		*/

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

					FbxVector4 bPosematrT = transformLinkMatrix.GetT();
					FbxVector4 bPosematrR = transformLinkMatrix.GetR();
					FbxVector4 bPosematrS = transformLinkMatrix.GetS();
					bposeT.xx[i][0] = bPosematrT.mData[0];
					bposeT.yy[i][1] = bPosematrT.mData[1];
					bposeT.zz[i][2] = bPosematrT.mData[2];

					//printf("bPose X: %f Y: %f Z: %f\n", bposeT.x[i][0], bposeT.y[i][1], bposeT.z[i][2]);

					//vS = bindPoseMatrix.GetS();
					//vR = bindPoseMatrix.GetR();
					//vT = bindPoseMatrix.GetT();

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

					FbxAnimLayer* pAnimLayer = FbxAnimLayer::Create(pScene, "Layer0");
					currAnimStack->AddMember(pAnimLayer);
					//Get the camera’s curve node for local translation.
					FbxAnimCurveNode* myAnimCurveNodeRot = pNode->LclRotation.GetCurveNode(pAnimLayer, true);
					//create curve nodes
					FbxAnimCurve* myRotXCurve = myAnimCurveNodeRot->GetCurve(0);
					FbxAnimCurve* myRotYCurve = myAnimCurveNodeRot->GetCurve(1);
					FbxAnimCurve* myRotZCurve = myAnimCurveNodeRot->GetCurve(2);
					//FbxTime lTime; //For the start and stop keys
					int lKeyIndex = 0;
					myRotXCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X, true);
					myRotYCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y, true);
					myRotZCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z, true);
					//printf("X: %d Y: %d Z: %d\n", myRotXCurve, myRotYCurve, myRotZCurve);
					//printf("X: %f Y: %f Z: %f\n", myRotXCurve, myRotYCurve, myRotZCurve);





					
					FbxTime start = pTakeInfo->mLocalTimeSpan.GetStart();
					FbxTime end = pTakeInfo->mLocalTimeSpan.GetStop();
					float startTime = (float)start.GetSecondDouble();
					float endTime = (float)end.GetSecondDouble();
					printf("START: %f END: %f\n", startTime, endTime);

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
void camera()
{
	glRotatef(xrot, -1.0, 0.0, 0.0); //(left and right)
	glRotatef(yrot, 0.0, -1.0, 0.0); //(up and down)
	glRotatef(zrot, 0.0, 0.0, -1.0); //(zoom in and out)
	glTranslatef(-xpos, -ypos, -zpos); //translate the screen to the position of our camera
}
void initGL()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f); //Set background depth to farthest
	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, (float)0.1);

	glEnable(GL_DEPTH_TEST);
	//The following two lines enable semi transparent
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDepthFunc(GL_LEQUAL); //Set the type of depth-test
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST); //Nicest perspective corrections
	
	glShadeModel(GL_FLAT);
	glEnable(GL_TEXTURE_2D);

	int width, height;
	bool hasAlpha;
	char filename[] = "texture.png", filename1[] = "texture1.png";
	
	num_textures[0]=LoadBMP("texture.bmp");
	num_textures[1] = LoadBMP("texture1.bmp");
	printf("TEXTURE: %d - %d\n", num_textures[0], num_textures[1]);

	num_textures[2] = loadPngImage(filename, width, height, hasAlpha, &textureImage);
	num_textures[3] = loadPngImage(filename1, width, height, hasAlpha, &textureImage);
	printf("TEXTURE: %d - %d\n", num_textures[2], num_textures[3]);
}

int matrtest = 0;

void display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); //Reset the model-view matrix
	camera();

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f);
	
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);
	polyloop = 0;

	//glScalef(matS.x[matrtest][0], matS.y[matrtest][1], matS.z[matrtest][2]);
	//glTranslatef(matT.x[matrtest][0], matT.y[matrtest][1], matT.z[matrtest][2]);
	
	//glRotatef(matR.x[matrtest][0], -1, 0, 0);
	//glRotatef(matR.y[matrtest][1], 0, -1, 0);
	//glRotatef(matR.z[matrtest][2], 0, 0, -1);
	
	for(int i = 0; i <= numIndices - 3; i+=3)
	{
		if(toggledraw == 0)glBegin(GL_TRIANGLES);
		else
		{
			glBegin(GL_LINES);
		}
		//glColor3f(1.0f, 1.0f, 1.0f);
		glNormal3f(normals[i*3+0], normals[i*3+1], normals[i*3+2]);
		for(int j = i; j <= i + 2; j++)
		{
			//printf("LOOP: %d-%d\n", i, j);
			if(polyvert == 3)polyvert = 0;
			glTexCoord2f(mapcoords.u[polyloop][polyvert], mapcoords.v[polyloop][polyvert]);
			//printf("LOOP %d-%d UVMAP: %f - %f\n", polyloop, polyvert,
			//mapcoords.u[polyloop][polyvert], mapcoords.v[polyloop][polyvert]);
			glVertex3f(vertices[indices[j]].x, vertices[indices[j]].y, vertices[indices[j]].z);
			//printf("Vertex X: %f Y: %f Z: %f\n", vertices[indices[j]].x, vertices[indices[j]].y, vertices[indices[j]].z);
			polyvert++;
		}
		polyloop++;
	}
	glEnd();
	//glFlush();
	glPopMatrix();

	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, 1);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f( 20.0f,-20.0f,-5.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f( 20.0f, 20.0f,-5.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-20.0f, 20.0f,-5.0f);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-20.0f,-20.0f,-5.0f);
	glEnd();
	//glFlush();
	glPopMatrix();

	if(matrtest < objanimlength)matrtest++;
	else
	{
		matrtest = 0;
	}

	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glLineWidth(5.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(-100,0,0);
	glVertex3f(100,0,0);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glLineWidth(5.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(0,-100,0);
	glVertex3f(0,100,0);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glLineWidth(5.0f);
	glColor3f(0.0f, 0.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(0,0,-100);
	glVertex3f(0,0,100);
	glEnd();
	glPopMatrix();

	
	///////////////////////////////////////////////

	for(int i = 0; i < 2; i++)//matrInt; i++)
	{
		glPushMatrix();
		glLineWidth(3.0f);
		glColor3f(1.0f, (float)(i+1)/matrInt, (float)(i+1)/matrInt);
		//glRotatef(rotation[0], 1, 0, 0);
		//glRotatef(rotation[1], 0, 1, 0);
		//glRotatef(rotation[2], 0, 0, 1);
		glBegin(GL_LINES);
		glVertex3f(matT.x[i][0][0], matT.y[i][0][1], matT.z[i][0][2]);
		printf("TEST: %f - %f - %f\n", matT.x[i][0][0], matT.y[i][0][1], matT.z[i][0][2]);
		//glVertex3f(1.6f, -4.6f, 0);

		
		glVertex3f(LclT.x[i][0][0], LclT.y[i][0][1], LclT.z[i][0][2]);
		printf("TEST: %f - %f - %f\n", LclT.x[i][0][0], LclT.y[i][0][1], LclT.z[i][0][2]);

		//_getch();
		
		glEnd();
		glPopMatrix();
	}

	//printf("MatrT X: %f Y: %f Z: %f\n", matT.x[0][0], matT.y[0][1], matT.z[0][2]);
	//printf("Translation: %f, %f, %f\nRotation: %f, %f, %f\nScale= %f, %f, %f\n",
		//translation[0], translation[1], translation[2], rotation[0], rotation[1], rotation[2],
		//scaling[0], scaling[1], scaling[2]);






	/*
	float xp = 0.0f, yp = 0.0f, zp = 0.0f, xxp = 0.0f, yyp = 0.0f, zzp = 0.0f,
		xr = 0, yr = 0.0f, zr = 0.0f;

	glPushMatrix();
	glColor3f(1.0f, 1.0f, 1.0f);
	glTranslatef(xp,yp,zp);
	glutWireSphere(0.1, 32, 32);
	glPopMatrix();

	xr = 90;
	glPushMatrix();
	glLineWidth(5.0f);
	glTranslatef(xp,yp,zp);

	glRotatef(xr,1,0,0);
	glRotatef(yr,0,1,0);
	glRotatef(zr,0,0,1);
	
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(xp,yp,zp);
	//printf("TEST: %f - %f - %f\n", xp,yp,zp);
	yp+=1.0f;
	glVertex3f(xp,yp,zp);
	//printf("TEST: %f - %f - %f\n", xp,yp,zp);
	glEnd();
	glPopMatrix();


	glPushMatrix();
	glColor3f(1.0f, 0.0f, 1.0f);
	glTranslatef(xxp,yyp,zzp);
	//printf("RESULT: %f - %f - %f\n", xxp,yyp,zzp);
	glutWireSphere(0.1, 32, 32);
	glPopMatrix();



	
	angle-=90.0f;
	glPushMatrix();
	glLineWidth(5.0f);
	glTranslatef(xp,yp,zp);
	glRotatef(angle,1,0,0);
	glColor3f(1.0f, 1.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(xp,yp,zp);
	printf("TEST2: %f - %f - %f\n", xp,yp,zp);
	yp+=1.5f;
	glVertex3f(xp,zp,zp);
	printf("TEST2: %f - %f - %f\n", xp,yp,zp);
	glEnd();
	glPopMatrix();


	angle-=90.0f;
	glPushMatrix();
	glLineWidth(5.0f);
	glTranslatef(xp,yp,zp);
	glRotatef(angle,0,0,1);
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(xp, yp, zp);
	printf("TEST3: %f - %f - %f\n", xp,yp,zp);
	yp+=0.75f;
	glVertex3f(xp, yp, zp);
	printf("TEST3: %f - %f - %f\n", xp,yp,zp);
	glEnd();
	glPopMatrix();



	glPushMatrix();
	glLineWidth(5.0f);
	//glTranslatef(xp,yp,zp);
	glRotatef(45,0,1,0);
	glColor3f(1.0f, 1.0f, 1.0f);
	glBegin(GL_LINES);
	glVertex3f(-5.0f, 15.0f, 0.0f);
	glVertex3f(5.0f, 1.0f, 0.0f);
	glEnd();
	glPopMatrix();

	glPushMatrix();
	glLineWidth(5.0f);
	//glTranslatef(xp,yp,zp);
	glRotatef(90,0,1,0);
	glColor3f(1.0f, 0.0f, 0.0f);
	glBegin(GL_LINES);
	glVertex3f(-5.0f, 15.0f, 0.0f);
	glVertex3f(5.0f, 1.0f, 0.0f);
	glEnd();
	glPopMatrix();
	
	
	glPushMatrix();
	glColor3f(1.0f, 0.0f, 1.0f);
	glTranslatef(-5.0f, 15.0f, 0.0f);
	glutWireSphere(0.5, 32, 32);
	glPopMatrix();

	glPushMatrix();
	glColor3f(1.0f, 1.0f, 0.0f);
	glTranslatef(5.0f, 1.0f, 0.0f);
	glutWireSphere(0.5, 32, 32);
	//glutSolidSphere(5, 32, 32);
	glPopMatrix();
	*/


	glFlush();
	glutSwapBuffers(); //Swap the front and back frame buffers(double buffering)
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
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width/(GLfloat)height, 0.1f, 1000.0f);
}

int looptest = 0;

void PrintNode(FbxNode*pNode, FbxScene*pScene, FbxTakeInfo*pTakeInfo)
{
	const char*lNodeName = pNode->GetName();
	translation = pNode->LclTranslation.Get();
	rotation = pNode->LclRotation.Get();
	scaling = pNode->LclScaling.Get();
	printf("Node Name: %s\nTranslation: %f, %f, %f\nRotation: %f, %f, %f\nScale= %f, %f, %f\n",
		lNodeName, translation[0], translation[1], translation[2],
		rotation[0], rotation[1], rotation[2],
		scaling[0], scaling[1], scaling[2]);

	printf("LOOP: %d\n", looptest);
	looptest++;

	for(int i = 0; i < pNode->GetNodeAttributeCount(); i++)
		PrintAttribute(pNode->GetNodeAttributeByIndex(i), pNode, pScene, pTakeInfo);
	for(int j = 0; j < pNode->GetChildCount(); j++)
		PrintNode(pNode->GetChild(j), pScene, pTakeInfo);
}
void keyboard(unsigned char key, int x, int y)
{
	if(key=='1')xrot = 90;
	if(key=='2')yrot = 0;
	if(key=='3')zrot = 0;
	if(key=='q')
	{
		xrot += 1;
		if(xrot > 360)xrot -= 360;
	}
	if(key=='z')
	{
		xrot -= 1;
		if(xrot < -360)xrot += 360;
	}
	if(key=='w')
	{
		float xrotrad, yrotrad, zrotrad;
		xrotrad = (xrot/180*3.141592654f);
		yrotrad = (yrot/180*3.141592654f);
		zrotrad = (zrot/180*3.141592654f);
		xpos -= float(sin(zrotrad));
		ypos += float(cos(zrotrad));
		zpos -= float(cos(xrotrad));
	}
	if(key=='s')
	{
		float xrotrad, yrotrad, zrotrad;
		xrotrad = (xrot/180*3.141592654f);
		yrotrad = (yrot/180*3.141592654f);
		zrotrad = (zrot/180*3.141592654f);
		xpos += float(sin(zrotrad));
		ypos -= float(cos(zrotrad));
		zpos += float(cos(xrotrad));
	}
	if(key=='a')
	{
		float zrotrad;
		zrotrad = (zrot/180*3.141592654f);
		xpos -= float(cos(zrotrad))*(float)0.75;
		ypos -= float(sin(zrotrad))*(float)0.75;
	}
	if(key=='d')
	{
		float zrotrad;
		zrotrad = (zrot/180*3.141592654f);
		xpos += float(cos(zrotrad))*(float)0.75;
		ypos += float(sin(zrotrad))*(float)0.75;
	}
	if(key=='u')xpos++;
	if(key=='h')xpos--;
	
	if(key=='i')ypos++;
	if(key=='j')ypos--;

	if(key=='o')zpos++;
	if(key=='k')zpos--;

	if(key=='z')xrot += 15;
	if(key=='x')xrot -= 15;

	if(key=='c')yrot += 15;
	if(key=='v')yrot -= 15;


	if(key=='b')zrot += 15;
	if(key=='n')zrot -= 15;

	if(key=='4')glDisable(GL_TEXTURE_2D);
	if(key=='5')glEnable(GL_TEXTURE_2D);
	if(key=='6')toggledraw=-1;
	if(key=='7')toggledraw=0;
	if(key==27)exit(0);
}
void mouseMovement(int x, int y)
{
	int diffx = x-(int)lastx;
	int diffy = y-(int)lasty;
	lastx = (float)x;
	lasty = (float)y;
	xrot -= (float)diffy; //UP AND DOWN
	//yrot += (float)diffx; //TILT LEFT AND RIGHT
	zrot -= (float)diffx; //LEFT AND RIGHT
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
			_getch();
			glutInit(&argc, argv);
			glutInitDisplayMode(GLUT_DOUBLE); //Enable double buffered mode
			glutInitWindowSize(640, 480);
			glutInitWindowPosition(5, 5);
			glutCreateWindow(title);
			initGL();
			glutDisplayFunc(display);
			glutReshapeFunc(reshape);
			glutPassiveMotionFunc(mouseMovement);
			glutKeyboardFunc(keyboard);
			glutTimerFunc(0, timer, 0);
			glutMainLoop();
		}
		lSdkManager->Destroy();
	}
	return 0;
}

/*
void ProcessSkeletonHierarchyRecursively(FbxNode*inNode, int inDepth, int myIndex, int inParentIndex)
{
	if(inNode->GetNodeAttribute() && inNode->GetNodeAttribute()->GetAttributeType() && inNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		Joint currJoint;
		currJoint.mParentIndex = inParentIndex;
		currJoint.mName = inNode->GetName();
		mSkeleton.mJoints.push_back(currJoint);
	}
	for (int i = 0; i < inNode->GetChildCount(); i++)
	{
		ProcessSkeletonHierarchyRecursively(inNode->GetChild(i), inDepth + 1, mSkeleton.mJoints.size(), myIndex);
	}
}
void ProcessSkeletonHierarchy(FbxNode*inRootNode)
{
	for (int childIndex = 0; childIndex < inRootNode->GetChildCount(); ++childIndex)
	{
		FbxNode* currNode = inRootNode->GetChild(childIndex);
		ProcessSkeletonHierarchyRecursively(currNode, 0, 0, -1);
	}
}
*/