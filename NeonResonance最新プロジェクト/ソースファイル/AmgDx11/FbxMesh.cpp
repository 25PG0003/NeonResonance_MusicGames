#include <io.h>
#include "FbxMesh.h"

#pragma comment( lib, "libfbxsdk.lib")

//****************************************************************
//
//	更新
//
//****************************************************************
//------------------------------------------------
//	更新
//------------------------------------------------
void FBXMesh::Update()
{
	transform.TRS(position, rotation, scale);
}

//------------------------------------------------
//	アニメーション
//------------------------------------------------
void FBXMesh::Animate(float sec)
{
	int beforeFrame = (int)Frame;

	float DeltaTime = sec;
	//	モーション時間の更新
	Frame += DeltaTime * 60;
	//	ループチェック
	if (Frame >= motion[MotionName].NumFrame - 1)
	{
		// ループ
		Frame = 0;		// 全体をループ
		IsMotionLooped = true;
	}
}

float FBXMesh::GetMotionTimeRate()
{
	// モーションのフレーム数
	float endF = motion[MotionName].NumFrame - 2.0f;
	return Frame/endF;
}

//****************************************************************
//
//	描画
//
//****************************************************************
//------------------------------------------------
//	描画
//------------------------------------------------
void FBXMesh::Render() {

	//	モーションが存在する場合はSkinning
	if (motion[MotionName].NumFrame > 0) {
		Skinning();
	}

	// 頂点バッファ設定
	UINT stride = sizeof(PolygonVertex);
	UINT offset = 0;
	DxSystem::DeviceContext->IASetVertexBuffers(0, 1, &VertexBuffer, &stride, &offset);
	DxSystem::DeviceContext->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	DxSystem::DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	int start = 0;
	//	マテリアルに分けて描画
	for (int m = 0; m<NumMesh; m++) {
		int material_no = MeshMaterial[m];
		if (Textures[material_no] != NULL) {
			Textures[material_no]->Set();
		} else {

		}
		//	描画
		DxSystem::DeviceContext->DrawIndexed(MaterialFaces[m]*3, start, 0);
		start += MaterialFaces[m] * 3;
	}
}


//****************************************************************
//
//	初期化
//
//****************************************************************
FBXMesh::FBXMesh()
{
}

//****************************************************************
//
//	解放
//
//****************************************************************
FBXMesh::~FBXMesh()
{
	//	頂点情報解放
	delete[] Vertices;
	if (VertexBuffer) { VertexBuffer->Release(); VertexBuffer = NULL; }

	delete[] Indices;
	delete[] Weights;
	delete[] VerticesSrc;
	//	材質関連解放
	delete[] MaterialFaces;
	delete[] MeshMaterial;
	//	テクスチャ解放
	for (int i = 0; i < NumMesh; i++) {
		if (Textures[i] != NULL)
			delete Textures[i];
	}
	delete[] Textures;

	//	モーション関連解放
	for (auto i = motion.begin(); i != motion.end(); i++) {
		Motion* M = &i->second;
		for (int bone = 0; bone < NumBone; bone++) {
			delete[] M->key[bone];
		}
	}
	motion.clear();

	if (IndexBuffer) { IndexBuffer->Release(); IndexBuffer = NULL; }
}

//****************************************************************
//
//	ファイル読み込み
//
//****************************************************************
void FBXMesh::Create(const char* filename)
{
	//ファイル名を取り除く
	strcpy_s(FBXDir, sizeof(FBXDir), filename);
	for (int n = (int)strlen(FBXDir) - 1; n >= 0; n--)
	{
		if (FBXDir[n] == '/' || FBXDir[n] == '\\')
		{
			FBXDir[n + 1] = '\0';
			break;
		}
	}

	Load(filename);

	// 頂点バッファ生成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(PolygonVertex) * NumVertices;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		// サブリソースの設定.
		D3D11_SUBRESOURCE_DATA initData;
		ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
		initData.pSysMem = Vertices;
		HRESULT hr = DxSystem::Device->CreateBuffer(&bd, &initData, &VertexBuffer);
	}

	// インデックスバッファ生成
	{
		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(D3D11_BUFFER_DESC));

		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.ByteWidth = sizeof(DWORD) * NumFaces * 3;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData;
		ZeroMemory(&InitData, sizeof(InitData));
		InitData.pSysMem = Indices;

		HRESULT hr = DxSystem::Device->CreateBuffer(&bd, &InitData, &IndexBuffer);
	}

	//	情報初期化
	Frame = 0;
	position = Vector3(0, 0, 0);
	rotation = Vector3(0, 0, 0);
	scale = Vector3(1, 1, 1);
	Update();
}

//------------------------------------------------
//	ファイル読み込み
//------------------------------------------------
void FBXMesh::Load(const char* filename)
{
	// ファイル名保存
	strcpy_s(FbxName, sizeof(FbxName), filename);

	FbxManager* manager = FbxManager::Create();
	scene = FbxScene::Create(manager, "");
	//	ファイルからシーンに読み込み
	FbxImporter* importer = FbxImporter::Create(manager, "");
	importer->Initialize(filename);
	importer->Import(scene);
	importer->Destroy();

	//	モーション情報取得
	FbxArray<FbxString*> names;
	scene->FillAnimStackNameArray(names);

	if (names != NULL) {
		//	モーションが存在するとき
		FbxTakeInfo* take = scene->GetTakeInfo(names[0]->Buffer());
		FbxLongLong start = take->mLocalTimeSpan.GetStart().Get();
		FbxLongLong stop = take->mLocalTimeSpan.GetStop().Get();
		FbxLongLong fps60 = FbxTime::GetOneFrameValue(FbxTime::eFrames60);
		StartFrame = (int)(start / fps60);
		motion["default"].NumFrame = (int)((stop - start) / fps60);
	}
	else {
		StartFrame = 0;
		motion["default"].NumFrame = 0;
	}

	//	ボーン情報取得
	int NumBone = scene->GetSrcObjectCount<FbxSkeleton>();
	InitializeBone(scene);

	//	モデルを材質ごとに分割
	FbxGeometryConverter fgc(manager);

/*
	{
		NumMesh = scene->GetSrcObjectCount<FbxMesh>();
		NodeList mesh;
		mesh.Clear();
		for (int m = 0; m < NumMesh; m++) {
			NodeList.ad = scene->GetSrcObject<FbxMesh>(m);
		}
		fgc.MergeMeshes()
	}
*/
	fgc.SplitMeshesPerMaterial(scene, true);
	fgc.Triangulate(scene, true);

	//	メッシュ数
	NumMesh = scene->GetSrcObjectCount<FbxMesh>();

	//	頂点数計算
	int work = 0;
	for (int m = 0; m < NumMesh; m++) {
		FbxMesh* mesh = scene->GetSrcObject<FbxMesh>(m);
		int num = mesh->GetPolygonVertexCount();
		work += num; // 合計頂点数
	}

	//	頂点確保
	Vertices = new PolygonVertex[work];
	Indices = new DWORD[work];
	Weights = new WEIGHT[work];

	NumVertices = 0;
	//	初期化
	for (int v = 0; v < work; v++) {
		Weights[v].count = 0;
	}

	//材質ごとのポリゴン頂点数
	MaterialFaces = new int[NumMesh];
	MeshMaterial = new int[NumMesh];
	Textures = new Texture * [NumMesh];
	for (int m = 0; m < NumMesh; m++)
	{
		Textures[m] = NULL;
	}

	//	頂点読み込み
	for (int m = 0; m < NumMesh; m++) {
		FbxMesh* mesh = scene->GetSrcObject<FbxMesh>(m);
		int num = mesh->GetPolygonVertexCount();

		//	頂点情報読み込み
		LoadPosition(mesh);		//	座標読み込み
		LoadNormal(mesh);		//	法線読み込み
		LoadUV(mesh);			//	テクスチャUV
		LoadTangent(mesh);
		LoadVertexColor(mesh);	//	頂点カラー読み込み

		//	インデックス設定(三角形ごと)
		for (int i = 0; i < num; i += 3) {
			Indices[i + 0 + NumVertices] = i + 0 + NumVertices;
			Indices[i + 1 + NumVertices] = i + 1 + NumVertices;
			Indices[i + 2 + NumVertices] = i + 2 + NumVertices;
		}

		//	ボーン読み込み
		LoadBone(mesh);

		//	メッシュの使用材質取得
		FbxLayerElementMaterial* LEM = mesh->GetElementMaterial();
		if (LEM != NULL) {
			//	ポリゴンに貼られている材質番号
			int material_index = LEM->GetIndexArray().GetAt(0);
			//	メッシュ材質のmaterial_index番目を取得
			FbxSurfaceMaterial* material = mesh->GetNode()->GetSrcObject<FbxSurfaceMaterial>(material_index);
			LoadMaterial(m, material);
		}
		//	使用材質設定
		MeshMaterial[m] = m;
		MaterialFaces[m] = num / 3;

		NumVertices += num;
	}

	NumFaces = NumVertices / 3;
	OptimizeVertices();

	//	頂点元データ保存
	VerticesSrc = new PolygonVertex[NumVertices];
	memcpy(VerticesSrc, Vertices, sizeof(PolygonVertex)*NumVertices);

	//	ウェイト正規化
	// ５本以上にまたっがてる場合のため
	for (int v = 0; v < NumVertices; v++) {
		float n = 0;
		//	頂点のウェイトの合計値
		for (int w = 0; w < Weights[v].count; w++) {
			n += Weights[v].weight[w];
		}
		//	正規化
		for (int w = 0; w < Weights[v].count; w++) {
			Weights[v].weight[w] /= n;
		}
	}

	//	解放
	scene->Destroy();
	manager->Destroy();

	Play("default");
}

//****************************************************************
//
//	頂点情報読み込み
//
//****************************************************************
//------------------------------------------------
//	座標読み込み
//------------------------------------------------
void FBXMesh::LoadPosition(FbxMesh* mesh) {
	int* index = mesh->GetPolygonVertices();
	FbxVector4* source = mesh->GetControlPoints();
	// メッシュのトランスフォーム
	FbxVector4 T = mesh->GetNode()->GetGeometricTranslation(FbxNode::eSourcePivot);
	FbxVector4 R = mesh->GetNode()->GetGeometricRotation(FbxNode::eSourcePivot);
	FbxVector4 S = mesh->GetNode()->GetGeometricScaling(FbxNode::eSourcePivot);
	FbxAMatrix TRS = FbxAMatrix(T, R, S);
	//	全頂点変換
	for (int v = 0; v < mesh->GetControlPointsCount(); v++) {
		source[v] = TRS.MultT(source[v]);
	}

	// 頂点座標読み込み
	int num = mesh->GetPolygonVertexCount();
	for (int v = 0; v < num; v++) {
		int vindex = index[v];

		Vertices[v + NumVertices].x = (float)source[vindex][0];
		Vertices[v + NumVertices].y = (float)source[vindex][1];
		Vertices[v + NumVertices].z = (float)source[vindex][2];

		Vertices[v + NumVertices].tu = 0;
		Vertices[v + NumVertices].tv = 0;
		Vertices[v + NumVertices].r = 1;
		Vertices[v + NumVertices].g = 1;
		Vertices[v + NumVertices].b = 1;
		Vertices[v + NumVertices].a = 1;
	}
}

//------------------------------------------------
//	法線読み込み
//------------------------------------------------
void FBXMesh::LoadNormal(FbxMesh* mesh) {
	FbxArray<FbxVector4> normal;
	mesh->GetPolygonVertexNormals(normal);
	for (int v = 0; v < normal.Size(); v++)
	{
		Vertices[v + NumVertices].nx = (float)normal[v][0];
		Vertices[v + NumVertices].ny = (float)normal[v][1];
		Vertices[v + NumVertices].nz = (float)normal[v][2];
	}

}

//------------------------------------------------
//	接線読み込み
//------------------------------------------------
void FBXMesh::LoadTangent(FbxMesh* mesh) {

	mesh->GenerateTangentsData(0, true);
	fbxsdk::FbxGeometryElementTangent* tangents = mesh->GetElementTangent(0);
	if (tangents == NULL) return;

	for (int v = 0; v < mesh->GetPolygonVertexCount() ; v++)
	{
		FbxVector4 t = tangents->GetDirectArray().GetAt(v);
		Vertices[v + NumVertices].tanx = (float)t[0];
		Vertices[v + NumVertices].tany = (float)t[1];
		Vertices[v + NumVertices].tanz = (float)t[2];
	}
}

//------------------------------------------------
//	ＵＶ読み込み
//------------------------------------------------
void FBXMesh::LoadUV(FbxMesh* mesh) {
	FbxStringList names;
	mesh->GetUVSetNames(names);
	FbxArray<FbxVector2> uv;
	mesh->GetPolygonVertexUVs(names.GetStringAt(0), uv);
	for (int v = 0; v < uv.Size(); v++) {
		Vertices[v + NumVertices].tu = (float)(uv[v][0]);
		Vertices[v + NumVertices].tv = (float)(1.0 - uv[v][1]);
	}
}

//------------------------------------------------
//	頂点カラー読み込み
//------------------------------------------------
void FBXMesh::LoadVertexColor(FbxMesh* mesh) {

	int vColorLayerCount = mesh->GetElementVertexColorCount();
	if (mesh->GetElementVertexColorCount() <= 0) return;
	//    頂点カラーレイヤー取得
	fbxsdk::FbxGeometryElementVertexColor* element = mesh->GetElementVertexColor(0);

	//  保存形式の取得
	FbxGeometryElement::EMappingMode mapmode = element->GetMappingMode();
	FbxGeometryElement::EReferenceMode refmode = element->GetReferenceMode();

	//    ポリゴン頂点に対するインデックス参照形式のみ対応
	if (mapmode == FbxGeometryElement::eByPolygonVertex)
	{
		if (refmode == FbxGeometryElement::eIndexToDirect)
		{
			FbxLayerElementArrayTemplate<int>* index = &element->GetIndexArray();
			int indexCount = index->GetCount();
			for (int j = 0; j<indexCount ; j++) {
				// FbxColor取得
				FbxColor c = element->GetDirectArray().GetAt(index->GetAt(j));
				Vertices[j + NumVertices].r = (float)c.mRed;
				Vertices[j + NumVertices].g = (float)c.mGreen;
				Vertices[j + NumVertices].b = (float)c.mBlue;
				Vertices[j + NumVertices].a = (float)c.mAlpha;
			}
		}
	}
}

//****************************************************************
//	材質読み込み
//****************************************************************
void FBXMesh::LoadMaterial(int index, FbxSurfaceMaterial * material)
{
	TextureName[index][0] = '\0';
	FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);

	//	テクスチャ読み込み
	const char* path = NULL;
	int fileTextureCount = prop.GetSrcObjectCount<FbxFileTexture>();
	if (fileTextureCount > 0) {
		FbxFileTexture* FileTex = prop.GetSrcObject<FbxFileTexture>(0);
		path = FileTex->GetFileName();
	} else {
		int numLayer = prop.GetSrcObjectCount<FbxLayeredTexture>();
		if (numLayer > 0) {
			FbxLayeredTexture* LayerTex = prop.GetSrcObject<FbxLayeredTexture>(0);
			FbxFileTexture* FileTex = LayerTex->GetSrcObject<FbxFileTexture>(0);
			path = FileTex->GetFileName();
		}
	}
	if( path == NULL ) return;

	//  C:\\AAA\\BBB\\a.fbx  C:/AAA/BBB/a.fbx
	const char* name = &path[strlen(path)];
	for (int i = 0; i < (int)strlen(path); i++)
	{
		name--;
		if (name[0] == '/'){name++; break; }
		if (name[0] == '\\') { name++; break; }
	}
	char work[128];
	strcpy_s(work, sizeof(work), FBXDir);		//"AAA/BBB/";
//	strcat(work, "texture/");	//"AAA/BBB/texture/"
	strcat_s(work, sizeof(work), name);			//"AAA/BBB/texture/a.png

	char filename[128];
	strcpy_s(filename, sizeof(filename), work);
	Textures[index] = new Texture();
	Textures[index]->Load(filename);
	// テクスチャ名保存
	strcpy_s(TextureName[index], sizeof(TextureName[index]), name);

}

//****************************************************************
//
//		ボーン関連
//
//****************************************************************
//------------------------------------------------
//	ボーン情報初期化
//------------------------------------------------
void FBXMesh::InitializeBone(FbxScene* scene)
{
	NumBone = scene->GetSrcObjectCount<FbxSkeleton>();

	for (int bone_no = 0; bone_no < NumBone; bone_no++) {
		FbxSkeleton* skel = scene->GetSrcObject<FbxSkeleton>(bone_no);
		FbxNode* node = skel->GetNode();

		//	ボーン名取得
		const char* name = node->GetName();
		strcpy_s(Bone[bone_no].Name, sizeof(Bone[bone_no].Name), name);

		//	親設定
		Bone[bone_no].parent = NULL;
		FbxNode* parent = node->GetParent();
		int parent_no = -1;
		if (parent) {
			const char* parent_name = parent->GetName();
			if (parent_name) {
				parent_no = FindBone(parent_name);
			}
		}
		Bone[bone_no].parent = parent_no;
	}

	//	メッシュ数
	int NumMesh = scene->GetSrcObjectCount<FbxMesh>();
	for (int m = 0; m < NumMesh; m++) {
		FbxMesh* mesh = scene->GetSrcObject<FbxMesh>(m);
		FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer(0, FbxDeformer::eSkin));
		if (!skin) continue;

		//	ボーン数
		int nBone = skin->GetClusterCount();

		//	全ボーン情報取得
		for (int bone = 0; bone < nBone; bone++)
		{
			//	ボーン情報取得
			FbxCluster* cluster = skin->GetCluster(bone);
			FbxAMatrix trans;
			cluster->GetTransformMatrix(trans);

			//	ボーン検索
			const char* name = cluster->GetLink()->GetName();
			int bone_no = FindBone(name);
			if (bone_no < 0) continue;

			//	オフセット行列作成
			FbxAMatrix LinkMatrix;
			cluster->GetTransformLinkMatrix(LinkMatrix);

			FbxAMatrix Offset = LinkMatrix.Inverse() * trans;
			FbxDouble* OffsetM = (FbxDouble*)Offset;
			//	オフセット行列保存
			for (int i = 0; i < 16; i++) {
				Bone[bone_no].OffsetMatrix.m[i] = (float)OffsetM[i];
			}
		}
	}


}

//------------------------------------------------
//	ボーン読み込み
//------------------------------------------------
void FBXMesh::LoadBone(FbxMesh* mesh)
{
	FbxAnimStack* lAnimStack = scene->GetSrcObject<FbxAnimStack>(0);
	if (!lAnimStack) return;
	FbxAnimLayer* lAnimLayer = lAnimStack->GetMember<FbxAnimLayer>();

	//	メッシュ頂点数
	int num = mesh->GetPolygonVertexCount();

	//	スキン情報の有無
	int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin );
	if( skinCount <= 0 ){
		LoadMeshAnim(mesh);
		return;
	}
	FbxSkin* skin = static_cast<FbxSkin*>(mesh->GetDeformer( 0, FbxDeformer::eSkin) );
	//	ボーン数
	int nBone = skin->GetClusterCount();
	//	全ボーン情報取得
	for( int bone=0 ; bone<nBone ; bone++ )
	{
		//	ボーン情報取得
		FbxCluster* cluster = skin->GetCluster(bone);

		//	ボーン名取得
		const char* name = cluster->GetLink()->GetName();
		//	ボーン検索
		bool isNewBone = false;
		int bone_no = FindBone(name);
		if( bone_no < 0 ){
			continue;
		}

		if( isNewBone ){
			//	キーフレーム読み込み
			LoadKeyFrames("default", bone_no, cluster->GetLink(), lAnimLayer);
		}

		//	ウェイト読み込み
		int wgtcount = cluster->GetControlPointIndicesCount();
		int* wgtindex = cluster->GetControlPointIndices();
		double* wgt = cluster->GetControlPointWeights();
		
		int* index = mesh->GetPolygonVertices();

		for( int i=0 ; i<wgtcount ; i++ ){
			int wgtindex2 = wgtindex[i];
			//	全ポリゴンからwgtindex2番目の頂点検索
			for( int v=0 ; v<num ; v++ ){
				if( index[v] != wgtindex2 ) continue;
				//	頂点にウェイト保存
				int w = Weights[v + NumVertices].count;
				if( w >= 4 ){
					continue;
				}
				Weights[v+NumVertices].bone[w] = bone_no;
				Weights[v+NumVertices].weight[w] = (float)wgt[i];
				Weights[v+NumVertices].count ++;
			}
		}
	}
}

//------------------------------------------------
//	ボーン検索
//------------------------------------------------
int FBXMesh::FindBone(const char* name)
{
	int bone = -1; // 見つからない
	for (int i = 0; i < NumBone; i++) {
		if (strcmp(name, Bone[i].Name) == 0) {
			bone = i;
			break;
		}
	}
	return bone;
}


//****************************************************************
//	モーション関連
//****************************************************************
//------------------------------------------------
//	モーション追加
//------------------------------------------------
void FBXMesh::AddMotion(std::string name, const char * filename)
{
	FbxManager* manager = FbxManager::Create();
	scene = FbxScene::Create(manager, "");
	//	ファイルからシーンに読み込み
	FbxImporter* importer = FbxImporter::Create(manager, "");
	importer->Initialize(filename);
	importer->Import(scene);
	importer->Destroy();

	//	モーション情報取得
	FbxArray<FbxString*> names;
	scene->FillAnimStackNameArray(names);

	FbxTakeInfo* take = scene->GetTakeInfo(names[0]->Buffer());
	FbxLongLong start = take->mLocalTimeSpan.GetStart().Get();
	FbxLongLong stop = take->mLocalTimeSpan.GetStop().Get();
	FbxLongLong fps60 = FbxTime::GetOneFrameValue(FbxTime::eFrames60);

	StartFrame = (int)(start / fps60);
	motion[name].NumFrame = (int)((stop - start) / fps60);
	//	ルートノード取得
	FbxNode* root = scene->GetRootNode();


	FbxAnimStack* lAnimStack = scene->GetSrcObject<FbxAnimStack>(0);
	FbxAnimLayer* lAnimLayer = lAnimStack->GetMember<FbxAnimLayer>();
	//	全ボーン読み込み
	for (int b = 0; b < NumBone; b++) {
		//	ボーンノード検索
		FbxNode* bone = root->FindChild(Bone[b].Name);
		if (bone == NULL) continue;

		//	キーフレーム読み込み
		LoadKeyFrames(name, b, bone, lAnimLayer);
	}

	//	解放
	scene->Destroy();
	manager->Destroy();
}


//------------------------------------------------
//	ボーンのないメッシュのアニメーション
//------------------------------------------------
void FBXMesh::LoadMeshAnim(FbxMesh* mesh)
{
	FbxNode* node = mesh->GetNode();

	int bone_no = NumBone;
	strcpy_s( Bone[bone_no].Name, sizeof(Bone[bone_no].Name), node->GetName());
	Bone[bone_no].parent = NULL;

	//	オフセット行列
	Bone[bone_no].OffsetMatrix.identity();

	//	ウェイト設定
	int num = mesh->GetPolygonVertexCount();
	for( int i=0 ; i<num ; i++ ){
		Weights[i+NumVertices].bone[0] = bone_no;
		Weights[i+NumVertices].weight[0] = 1.0f;
		Weights[i+NumVertices].count = 1;
	}

	NumBone++;
}

float FBXMesh::GetCurveValue(FbxAnimCurve* curve, FbxTime T)
{
	float value = 0;
	if (curve->KeyGetInterpolation(0) != FbxAnimCurveDef::eInterpolationConstant)
	{
		value = curve->Evaluate(T, 0);
	}
	else {
		FbxAnimCurveKey lKey = curve->KeyGet(0);
		value = lKey.GetValue();
	}
	return value;
}

//------------------------------------------------
//	キーフレーム読み込み
//------------------------------------------------
void FBXMesh::LoadKeyFrames(std::string name, int bone, FbxNode* bone_node, FbxAnimLayer* AnimLayer) {

	FbxAnimCurve* AnimCurveX = bone_node->LclRotation.GetCurve(AnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
	FbxAnimCurve* AnimCurveY = bone_node->LclRotation.GetCurve(AnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	FbxAnimCurve* AnimCurveZ = bone_node->LclRotation.GetCurve(AnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	FbxAnimCurve* AnimCurvePX = bone_node->LclTranslation.GetCurve(AnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
	FbxAnimCurve* AnimCurvePY = bone_node->LclTranslation.GetCurve(AnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	FbxAnimCurve* AnimCurvePZ = bone_node->LclTranslation.GetCurve(AnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	FbxAnimCurve* AnimCurveSX = bone_node->LclScaling.GetCurve(AnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
	FbxAnimCurve* AnimCurveSY = bone_node->LclScaling.GetCurve(AnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
	FbxAnimCurve* AnimCurveSZ = bone_node->LclScaling.GetCurve(AnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
	FbxDouble3 rot = bone_node->LclRotation.Get();
	FbxDouble3 pos = bone_node->LclTranslation.Get();
	FbxDouble3 scale = bone_node->LclScaling.Get();
	FbxDouble3 prerot = bone_node->PreRotation.Get();
	Vector3 PreR = Vector3((float)prerot[0], (float)prerot[1], (float)prerot[2]) * 3.1415926535f / 180;

	float x, y, z;
	float px, py, pz;
	float sx, sy, sz;
	x = (float)rot[0]; y = (float)rot[1]; z = (float)rot[2];
	px = (float)pos[0]; py = (float)pos[1]; pz = (float)pos[2];
	sx = (float)scale[0]; sy = (float)scale[1]; sz = (float)scale[2];

	// メモリ確保
	Motion* M = &motion[name];
	M->key[bone] = new Matrix[M->NumFrame + 1];

	double time = StartFrame * (1.0 / 60);
	FbxTime T = 0;

	for (int f = 0; f < motion[name].NumFrame; f++)
	{
		T.SetSecondDouble(time);
		time += (1.0 / 60);

		// カーブデータ取得
		if (AnimCurveX) x = GetCurveValue(AnimCurveX, T);
		if (AnimCurveY) y = GetCurveValue(AnimCurveY, T);
		if (AnimCurveZ) z = GetCurveValue(AnimCurveZ, T);
		if (AnimCurvePX) px = GetCurveValue(AnimCurvePX, T);
		if (AnimCurvePY) py = GetCurveValue(AnimCurvePY, T);
		if (AnimCurvePZ) pz = GetCurveValue(AnimCurvePZ, T);
		if (AnimCurveSX) sx = GetCurveValue(AnimCurveSX, T);
		if (AnimCurveSY) sy = GetCurveValue(AnimCurveSY, T);
		if (AnimCurveSZ) sz = GetCurveValue(AnimCurveSZ, T);

		Vector3 R = Vector3(x, y, z) * 3.1415926535f / 180;
		Matrix mat;
		Quaternion q, preq;

		float xx = sinf(R.x / 2.0f);
		float xw = cosf(R.x / 2.0f);
		float yy = sinf(R.y / 2.0f);
		float yw = cosf(R.y / 2.0f);
		float zz = sinf(R.z / 2.0f);
		float zw = cosf(R.z / 2.0f);
		q.x = (zw * yw) * xx - (zz * yy) * xw;
		q.y = (zz * yw) * xx + (zw * yy) * xw;
		q.z = -(zw * yy) * xx + (zz * yw) * xw;
		q.w = (zz * yy) * xx + (zw * yw) * xw;

		xx = sinf(PreR.x / 2.0f);
		xw = cosf(PreR.x / 2.0f);
		yy = sinf(PreR.y / 2.0f);
		yw = cosf(PreR.y / 2.0f);
		zz = sinf(PreR.z / 2.0f);
		zw = cosf(PreR.z / 2.0f);
		preq.x = (zw * yw) * xx - (zz * yy) * xw;
		preq.y = (zz * yw) * xx + (zw * yy) * xw;
		preq.z = -(zw * yy) * xx + (zz * yw) * xw;
		preq.w = (zz * yy) * xx + (zw * yw) * xw;

		Quaternion qRot = preq * q;
		qRot.toMatrix(mat);

		mat._11 *= sx; mat._12 *= sx; mat._13 *= sx;
		mat._21 *= sy; mat._22 *= sy; mat._23 *= sy;
		mat._31 *= sz; mat._32 *= sz; mat._33 *= sz;

		mat._41 = px; mat._42 = py; mat._43 = pz;
		M->key[bone][f] = mat;
	}
}
//------------------------------------------------
//	スキニング
//------------------------------------------------
void FBXMesh::Skinning()
{
	Motion* M = &motion[MotionName];
	if( M == NULL ) return;
	if (M->key[0] == NULL) return;

	//	配列用変数
	int f = (int)Frame;

	//	行列準備
	Matrix KeyMatrix[256];
	for( int b=0 ; b<NumBone ; b++ )
	{
		//	行列補間
		Matrix m = M->key[b][f];
		m.Interporate( M->key[b][f+1], Frame - (int)Frame );

		if (Bone[b].parent >= 0 ) {
			m *= Bone[Bone[b].parent].transform;
		}

		Bone[b].transform = m;

		//	キーフレーム
		KeyMatrix[b] = Bone[b].OffsetMatrix * m;
	}

	//	頂点変形
	for( int v=0 ; v<NumVertices ; v++ )
	{
		//	頂点 * ボーン行列
		// b = v番目の頂点の影響ボーン[n]
		if (Weights[v].count <= 0) continue;

		Vertices[v].x = 0;
		Vertices[v].y = 0;
		Vertices[v].z = 0;

		Vertices[v].nx = 0;
		Vertices[v].ny = 0;
		Vertices[v].nz = 0;

		Vertices[v].tanx = 0;
		Vertices[v].tany = 0;
		Vertices[v].tanz = 0;

		//	影響個数分ループ
		for( int n=0 ; n<Weights[v].count ; n++ ){
			int b = Weights[v].bone[n];

			float x = VerticesSrc[v].x;
			float y = VerticesSrc[v].y;
			float z = VerticesSrc[v].z;
			//	座標を影響力分移動
			Vertices[v].x += (x*KeyMatrix[b]._11 + y*KeyMatrix[b]._21 + z*KeyMatrix[b]._31 + 1*KeyMatrix[b]._41 )*Weights[v].weight[n];
			Vertices[v].y += (x*KeyMatrix[b]._12 + y*KeyMatrix[b]._22 + z*KeyMatrix[b]._32 + 1*KeyMatrix[b]._42 )*Weights[v].weight[n];
			Vertices[v].z += (x*KeyMatrix[b]._13 + y*KeyMatrix[b]._23 + z*KeyMatrix[b]._33 + 1*KeyMatrix[b]._43 )*Weights[v].weight[n];

			float nx = VerticesSrc[v].nx;
			float ny = VerticesSrc[v].ny;
			float nz = VerticesSrc[v].nz;
			//	法線を影響力分変換
			Vertices[v].nx += (nx*KeyMatrix[b]._11 + ny*KeyMatrix[b]._21 + nz*KeyMatrix[b]._31)*Weights[v].weight[n];
			Vertices[v].ny += (nx*KeyMatrix[b]._12 + ny*KeyMatrix[b]._22 + nz*KeyMatrix[b]._32)*Weights[v].weight[n];
			Vertices[v].nz += (nx*KeyMatrix[b]._13 + ny*KeyMatrix[b]._23 + nz*KeyMatrix[b]._33)*Weights[v].weight[n];

			float tx = VerticesSrc[v].tanx;
			float ty = VerticesSrc[v].tany;
			float tz = VerticesSrc[v].tanz;
			//	法線を影響力分変換
			Vertices[v].tanx += (tx * KeyMatrix[b]._11 + ty * KeyMatrix[b]._21 + tz * KeyMatrix[b]._31) * Weights[v].weight[n];
			Vertices[v].tany += (tx * KeyMatrix[b]._12 + ty * KeyMatrix[b]._22 + tz * KeyMatrix[b]._32) * Weights[v].weight[n];
			Vertices[v].tanz += (tx * KeyMatrix[b]._13 + ty * KeyMatrix[b]._23 + tz * KeyMatrix[b]._33) * Weights[v].weight[n];


		}
	}
	DxSystem::DeviceContext->UpdateSubresource(VertexBuffer, 0, NULL, Vertices, 0, 0);
}

//****************************************************************
//	頂点最適化
//****************************************************************
void FBXMesh::OptimizeVertices()
{
	int currentNum = 0;
	for (int v = 0; v < NumVertices; v++) {
		int sameIndex = -1;
		//	同一頂点検索
		for (int old = 0; old < currentNum; old++) {
			if (Vertices[v].x != Vertices[old].x ||
				Vertices[v].y != Vertices[old].y ||
				Vertices[v].z != Vertices[old].z ||
				Vertices[v].nx != Vertices[old].nx ||
				Vertices[v].ny != Vertices[old].ny ||
				Vertices[v].nz != Vertices[old].nz ||
				Vertices[v].tu != Vertices[old].tu ||
				Vertices[v].tv != Vertices[old].tv/* ||
				Vertices[v].color != Vertices[old].color*/) continue;

			sameIndex = old;
			break;
		}

		int target = v;
		if (sameIndex == -1) {
			//	新規頂点
			CopyMemory(&Vertices[currentNum], &Vertices[v], sizeof(PolygonVertex));
			CopyMemory(&Weights[currentNum], &Weights[v], sizeof(WEIGHT));
			target = currentNum;
			currentNum++;
		}
		else {
			target = sameIndex;
		}
		//	インデックス更新
		for (int i = 0; i < NumVertices; i++) {
			if(Indices[i] == v ) Indices[i] = target;
		}
	}

	//	新バッファ確保
	PolygonVertex* buf = new PolygonVertex[currentNum];
	CopyMemory(buf, Vertices, sizeof(PolygonVertex) * currentNum);
	NumVertices = currentNum;

	delete[] Vertices;
	Vertices = buf;
}

void FBXMesh::SetHeight(Texture* HeightMap)
{
	BYTE* pixels = 
			HeightMap->GetPixels();
	int w = HeightMap->GetWidth();
	int h = HeightMap->GetHeight();

	for (int v=0; v<NumVertices; v++)
	{
		// 値vx(0~500) を 0~1023 に
		// 値vx(0~500)　を 0~1.0　に
		//     0~1.0　を　0~１０２３
		int x = (int)((Vertices[v].x / 500)
					* (w-1));
		int z = (int)((Vertices[v].z / 500)
					* (h-1));
		float height = 
			pixels[(z*w+x)*4] * 0.4f;
		
		Vertices[v].y = height;
	}
	delete[] pixels;

	// 全法線再計算

	for (int v=0; v<NumVertices; v++)
	{
		Vertices[v].nx = 0;
		Vertices[v].ny = 0;
		Vertices[v].nz = 0;
	}
	// 全ポリゴン法線計算
	for (int f = 0; f < NumFaces; f++) {
		// 3頂点
		int a = Indices[f*3 + 0];
		int b = Indices[f*3 + 1];
		int c = Indices[f*3 + 2];
		Vector3 va = Vector3(Vertices[a].x, Vertices[a].y, Vertices[a].z);
		Vector3 vb = Vector3(Vertices[b].x, Vertices[b].y, Vertices[b].z);
		Vector3 vc = Vector3(Vertices[c].x, Vertices[c].y, Vertices[c].z);
		Vector3 l1 = vb - va;
		Vector3 l2 = vc - vb;
		// 面法線
		Vector3 N;
		Vector3::cross(N, l1, l2);
		N.Normalize();
		//
		Vertices[a].nx += N.x; Vertices[a].ny += N.y; Vertices[a].nz += N.z;
		Vertices[b].nx += N.x; Vertices[b].ny += N.y; Vertices[b].nz += N.z;
		Vertices[c].nx += N.x; Vertices[c].ny += N.y; Vertices[c].nz += N.z;
	}
	// 全法線正規化
	for (int v = 0; v < NumVertices; v++)
	{
		Vector3 N = 
			Vector3(Vertices[v].nx, Vertices[v].ny, Vertices[v].nz);
		N.Normalize();
		Vertices[v].nx = N.x;
		Vertices[v].ny = N.y;
		Vertices[v].nz = N.z;
	}


	DxSystem::DeviceContext->UpdateSubresource(
		VertexBuffer, 0, NULL, Vertices, 0, 0);
}



