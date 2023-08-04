#include <Novice.h>
#include "Vector3.h"
#include "Matrix4x4.h"
#include <imgui.h>
#define _USE_MATH_DEFINES
#include "stdint.h"
#include "math.h"
#include <algorithm>

#include "Mt4.h"
using namespace std;

const char kWindowTitle[] = "LE2C_13_サイトウ_ユウキ_MT3_02_01";

//Grid
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix);

struct Sphere {
	Vector3 center; //!< 中心点
	float radius;   //!< 半径
};

//Sphere
void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color);

bool IsCollision(const Sphere& s1, const Sphere& s2);



// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = {0};
	char preKeys[256] = {0};

	float kWindowWidth = 1280.0f;
	float kWindowHeight = 720.0f;

	Vector3 rotate{};
	Vector3 translate{};

	Vector3 cameraTranslate{ 0.0f,1.9f,-6.49f };
	Vector3 cameraRotate{ 0.26f,0.0f,0.0f };

	Sphere sphere1 = { 0.0f,0.0f,0.0f,0.5f };

	Sphere sphere2 = { 0.5f,0.5f,0.5f,0.5f };

	unsigned int color = WHITE;

	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		Matrix4x4 cameraMatrix = MakeAffineMatrix({ 1.0f,1.0f,1.0f }, cameraRotate, cameraTranslate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kWindowWidth) / float(kWindowHeight), 0.1f, 100.0f);
		Matrix4x4 viewProjectionMatrix = Multiply(viewMatrix, projectionMatrix);
		Matrix4x4 viewPortMatrix = MakeViewportMatrix(0, 0, float(kWindowWidth), float(kWindowHeight), 0.0f, 1.0f);

		if (IsCollision(sphere1, sphere2)) {
			color = RED;
		}
		else {
			color = WHITE;
		}

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		DrawSphere(sphere1, viewProjectionMatrix, viewPortMatrix, color);

		DrawSphere(sphere2, viewProjectionMatrix, viewPortMatrix, WHITE);

		DrawGrid(viewProjectionMatrix, viewPortMatrix);

		ImGui::Begin("Window");
		ImGui::DragFloat3("CameraTranslate", &cameraTranslate.x, 0.01f);
		ImGui::DragFloat3("CameraRottate", &cameraRotate.x, 0.01f);
		ImGui::DragFloat3("Sphere[0]Center", &sphere1.center.x, 0.01f);
		ImGui::DragFloat("Sphere[0]Radius", &sphere1.radius, 0.01f);
		ImGui::DragFloat3("Sphere[1]Center", &sphere2.center.x, 0.01f);
		ImGui::DragFloat("Sphere[1]Radius", &sphere2.radius, 0.01f);
		ImGui::End();

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}

//Sphere
void DrawSphere(const Sphere& sphere, const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix, uint32_t color) {
	const uint32_t kSubdivision = 12; //分割数
	const float kLonEvery = 2.0f * float(M_PI) / kSubdivision; //経度分割1つ分の角度 φ
	const float kLatEvery = float(M_PI) / kSubdivision;  //緯度分割1つ分の角度 θ
	//緯度の方向に分割 -π/2 ~ π/2
	for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
		float lat = -float(M_PI) / 2.0f + kLatEvery * latIndex; //現在の緯度 θ
		//経度の方向に分割　0~2π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
			float lon = lonIndex * kLonEvery; //現在の経度  φ
			//world座標系でのa,b,cを求める
			Vector3 a
			{
				sphere.radius * cosf(lat) * cosf(lon) + sphere.center.x,
				sphere.radius * sinf(lat) + sphere.center.y,
				sphere.radius * cosf(lat) * sinf(lon) + sphere.center.z
			};
			Vector3 b
			{
				sphere.radius * cosf(lat + kLatEvery) * cosf(lon) + sphere.center.x,
				sphere.radius * sinf(lat + kLatEvery) + sphere.center.y,
				sphere.radius * cosf(lat + kLatEvery) * sinf(lon) + sphere.center.z
			};
			Vector3 c
			{
				sphere.radius * cosf(lat) * cosf(lon + kLonEvery) + sphere.center.x,
				sphere.radius * sinf(lat) + sphere.center.y,
				sphere.radius * cosf(lat) * sinf(lon + kLonEvery) + sphere.center.z
			};
			// a,b,cをScreen座標系まで変換
			Vector3 screenA = Transform(Transform(a, viewProjectionMatrix), viewportMatrix);
			Vector3 screenB = Transform(Transform(b, viewProjectionMatrix), viewportMatrix);
			Vector3 screenC = Transform(Transform(c, viewProjectionMatrix), viewportMatrix);

			//ab,bcで線を引く
			Novice::DrawLine(int(screenA.x), int(screenA.y), int(screenB.x), int(screenB.y), color);
			Novice::DrawLine(int(screenA.x), int(screenA.y), int(screenC.x), int(screenC.y), color);
		}
	}
}

//Grid
void DrawGrid(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewportMatrix) {
	const float kGridHalfWidth = 2.0f; //Gridの半分の幅
	const uint32_t kSubdivision = 10; //分割数
	const float kGridEvery = (kGridHalfWidth * 2.0f) / float(kSubdivision); //1つ分の長さ
	//奥から手前への線を順々に引いていく
	for (uint32_t xIndex = 0; xIndex <= kSubdivision; ++xIndex) {
		float x = -kGridHalfWidth + (xIndex * kGridEvery);

		//上の情報を使ってワールド座標系上の始点と終点を求める
		Vector3 start = { x,0.0f,-kGridHalfWidth };
		Vector3 end = { x,0.0f,kGridHalfWidth };

		//スクリーン座標系まで変換をかける
		Vector3 screenStart = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
		Vector3 screenEnd = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);

		//変換した座標を使って表示。色は薄い灰色(0xAAAAAAFF),原点は黒ぐらいが良いが、何でも良い
		if (x == 0) {
			Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), 0x000000FF);
		}
		else {
			Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), 0xAAAAAAFF);
		}
	}
	//左から右も同じように順々に引いていく
	for (uint32_t zIndex = 0; zIndex <= kSubdivision; ++zIndex) {
		float z = -kGridHalfWidth + (zIndex * kGridEvery);

		//上の情報を使ってワールド座標系上の始点と終点を求める
		Vector3 start = { -kGridHalfWidth,0.0f,z };
		Vector3 end = { kGridHalfWidth,0.0f,z };

		//スクリーン座標系まで変換をかける
		Vector3 screenStart = Transform(Transform(start, viewProjectionMatrix), viewportMatrix);
		Vector3 screenEnd = Transform(Transform(end, viewProjectionMatrix), viewportMatrix);

		//変換した座標を使って表示。色は薄い灰色(0xAAAAAAFF),原点は黒ぐらいが良いが、何でも良い
		if (z == 0) {
			Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), 0x000000FF);
		}
		else {
			Novice::DrawLine(int(screenStart.x), int(screenStart.y), int(screenEnd.x), int(screenEnd.y), 0xAAAAAAFF);
		}
	}
}

bool IsCollision(const Sphere& s1, const Sphere& s2) {
	//2つの球の中心転点間の距離を求める
	float distance = Length(Subtract(s1.center,s2.center));

	//半径の合計よりも短ければ衝突
	if (distance <= s1.radius + s2.radius) {
		//当たった処理を諸々
		return true;
	}
	else {
		return false;
	}
}