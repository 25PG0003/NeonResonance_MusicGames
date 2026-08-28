// FBXMesh.h
#define _CRT_SECURE_NO_WARNINGS

#pragma once
#include <d3d11.h>
#include <iostream>
#include "Texture.h"
#include "Math\math.h"

#include <map>

#include <fbxsdk.h>

class FBXMesh
{
protected:
	char FbxName[128]; // ファイル名

	//	頂点構造体（シェーダーと一致）
	struct PolygonVertex {
		float x, y, z;		// 座標
		float nx, ny, nz;	// 法線
		float tu, tv;		// UV
		float r,g,b,a;		// 頂点カラー
		float tanx, tany, tanz;	// 接線
		float bx, by, bz;	// 従法線
	};

	int NumMesh;
	int NumVertices;	//全頂点数
	int NumFaces;		//全ポリゴン数
	PolygonVertex* Vertices;	//頂点
	PolygonVertex* VerticesSrc; //頂点元データ
	DWORD* Indices;		//三角形（頂点結び方）

	ID3D11Buffer* VertexBuffer;	// 頂点バッファ
	ID3D11Buffer* IndexBuffer;		// インデックスバッファ

	char FBXDir[128];
	int* MeshMaterial;
	int* MaterialFaces;
	Texture** Textures;

	//	ボーン関連
	struct BONE {
		char	Name[128];
		Matrix	OffsetMatrix;
		Matrix	transform;
		int		parent;
	};
	int NumBone;
	BONE Bone[256];

	void InitializeBone(FbxScene* scene);
	int FindBone( const char* name );
	void LoadBone(FbxMesh* mesh );
	void LoadMeshAnim(FbxMesh* mesh);
	
	//	ウェイト関連
	struct WEIGHT {
		int count;
		int bone[4];
		float weight[4];
	};
	WEIGHT* Weights;

	int StartFrame;
	void Skinning();	// ボーンによる変形

	static const int MOTION_MAX = 256; 
	static const int BONE_MAX = 256; 

	//	アニメーション
	struct Motion {
		int NumFrame;	// フレーム数	
		Matrix* key[BONE_MAX];	// キーフレーム
	};

	int NumMotion;		// モーション数
	std::map<std::string, Motion> motion;	// モーションデータ
	void LoadKeyFrames(std::string name, int bone, FbxNode* bone_node,FbxAnimLayer* AnimLayer);
	float GetCurveValue(FbxAnimCurve* curve, FbxTime T);

	FbxScene* scene;
	void Load(const char* filename);
	void LoadMaterial( int index, FbxSurfaceMaterial* material );

	void LoadPosition(FbxMesh* mesh);
	void LoadNormal(FbxMesh* mesh);
	void LoadTangent(FbxMesh* mesh);

	void LoadUV(FbxMesh* mesh);
	void LoadVertexColor(FbxMesh* mesh);

	void OptimizeVertices();

public:
	void SetHeight( Texture* HeightMap);
	void ReplaceTexture(int index, Texture* tex) {
		Textures[index] = tex;
	}
	FBXMesh();
	virtual ~FBXMesh();

	void Update();

	void Render();	//	描画

	bool IsMotionLooped;
	void Play(std::string name) {
		MotionName = name;
		Frame = 0.0f;
		IsMotionLooped = false;
	}
	void Animate(float sec);	// モーション再生
	float GetMotionTimeRate(); // 現在の再生時間の割合(始め0--->1終了)
	void Create(const char* filename);
	void AddMotion( std::string name, const char* filename);

	//	モーション情報
	float Frame;		// 現在のフレーム
	std::string MotionName;		// 現在のモーション

	//	姿勢情報
	Matrix transform;
	Vector3 position;
	Vector3 rotation;
	Vector3 scale;
	//	ボーン行列取得
	Matrix GetBoneMatrix( int bone ){ return Bone[bone].transform; }

	char TextureName[128][128];

};


