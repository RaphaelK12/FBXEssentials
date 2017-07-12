#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <fbxsdk.h>
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
		FbxMesh* mesh = pNode->GetMesh();
		printf("MESH MEMORY USAGE: %d\n", mesh->MemoryUsage());
		printf("MESH DEFORMER COUNT: %d\n", mesh->GetDeformerCount(FbxDeformer::EDeformerType::eSkin));
		FbxSkin* skin = (FbxSkin*)(mesh->GetDeformer(0, FbxDeformer::EDeformerType::eSkin));
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
				printf("StackName: \"%s\"\n", mAnimationName);

				FbxTime start = pTakeInfo->mLocalTimeSpan.GetStart();
				FbxTime end = pTakeInfo->mLocalTimeSpan.GetStop();
				float startTime = start.GetSecondDouble();
				float endTime = end.GetSecondDouble();
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
int main(int argc, char**argv[])
{
	//const char *lFilename = "test.fbx";
	//printf("File: %s\n", lFilename);
	for(int i = 1; i < 3; i++)
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
				printf("Name: "%s\n", lTakeInfo->mName.Buffer());
				printf("Description: "%s\n", lTakeInfo->mDescription.Buffer());
				//Change the value of the import name if the animation stack should be imported under a different name.
				printf("Import Name: "%s\n", lTakeInfo->mImportName.Buffer());
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
	printf("Press Enter to exit!\n");
	getchar();
	return 0;
}
