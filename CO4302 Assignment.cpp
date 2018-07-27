// CO4302 Assignment.cpp: A program using the TL-Engine

#include <iostream>
#include "Threading.h"

using namespace tle;
using namespace multiThread;

const EKeyCode kQuitKey = Key_Escape;
const EKeyCode kStartKey = Key_Space;	// toggles starting/pausing the simulation
const EKeyCode kSpeedUpKey = Key_E;
const EKeyCode kSpeedDownKey = Key_Q;
const EKeyCode kCameraRight = Key_D;
const EKeyCode kCameraLeft = Key_A;
const EKeyCode kCameraUp = Key_W;
const EKeyCode kCameraDown = Key_S;
const EKeyCode kBackSwitch = Key_Back;
const EKeyCode kReverseKey = Key_R;
const EKeyCode kToggleText = Key_1;
const EKeyCode kToggleDeaths = Key_2;
const EKeyCode kToggleTiming = Key_3;
const EKeyCode kTogglePartitions = Key_4;

enum BackgroundStatus { Skybox, Stars, Void };
sPartitionData partitions[kPartSize][kPartSize][kPartSize];

void main()
{
	// Create a 3D engine (using TLX engine here) and open a window for it
	I3DEngine* myEngine = New3DEngine( kTLX );
	myEngine->StartWindowed(1600, 900);

	// Add default folder for meshes and other media
	myEngine->AddMediaFolder( "Media" );

	/////////////////////START OF MODEL SETUP//////////////////////////

	IMesh* ballMesh = myEngine->LoadMesh("Sphere.x");
	IMesh* skyboxMesh = myEngine->LoadMesh("Skybox.x");
	IMesh* starMesh = myEngine->LoadMesh("Stars.x");
	IMesh* cubeMesh = myEngine->LoadMesh("Cube.x");
	IMesh* floorMesh = myEngine->LoadMesh("Floor.x");

	string redBallSkin = "RedBall.jpg";
	string yellowBallSkin = "YellowBall.jpg";
	string whiteBallSkin = "WhiteBall.jpg";


	BackgroundStatus background = Skybox;

	IModel* skybox = skyboxMesh->CreateModel(0, kLowerBound - 500, 0);
	skybox->Scale(2);

	IModel* starBackground = starMesh->CreateModel(0, kLowerBound * 10,0);
	starBackground->Scale(2);

	IModel* voidBackground = starMesh->CreateModel(0, kLowerBound * 10, 0);
	voidBackground->Scale(2);
	voidBackground->SetSkin("BlackBackground.jpg");


	////////////THREADER
	cMultiThreader threader;
	int numOfPartitions = (threader.mNumWorkers + 1);	// one partition for the main thread
	int checkedSpheres = 0;
	std::vector<IModel*> floors;

	float floatSpheresPT = ceil(1.0f * kNumOfStatics / (threader.mNumWorkers + 1));
	int spheresPerThread = floatSpheresPT;

	float ySize = kUpperBound / (threader.mNumWorkers + 1);
	float startY = ySize;
	std::vector<sPartitionData> partitionLayers;
	std::vector<float> partitionPositions;

	for (int i = 0; i < numOfPartitions / 2; ++i)
	{
		sPartitionData upPartition;
		upPartition.pos = { 0, startY, 0 };
		upPartition.ySize = ySize;
		upPartition.distToEdge = ySize;
		partitionLayers.push_back(upPartition);

		sPartitionData downPartition;
		downPartition.pos = { 0, -startY, 0 };
		downPartition.ySize = ySize;
		downPartition.distToEdge = ySize;
		partitionLayers.push_back(downPartition);

		IModel* newFloor = floorMesh->CreateModel(0, kHidePartFloor, 0);
		partitionPositions.push_back(upPartition.pos.y - ySize);
		IModel* newFloorReverse = floorMesh->CreateModel(0, kHidePartFloor, 0);
		partitionPositions.push_back(upPartition.pos.y + ySize);

		newFloorReverse->RotateX(180);
		floors.push_back(newFloor);
		floors.push_back(newFloorReverse);

		IModel* newFloorDown = floorMesh->CreateModel(0, kHidePartFloor, 0);
		partitionPositions.push_back(downPartition.pos.y - ySize);
		IModel* newFloorReverseDown = floorMesh->CreateModel(0, kHidePartFloor, 0);
		partitionPositions.push_back(downPartition.pos.y + ySize);

		newFloorReverseDown->RotateX(180);
		floors.push_back(newFloorDown);
		floors.push_back(newFloorReverseDown);

		startY += ySize * 2.0f;
	}

	IModel* spheres[kNumOfSpheres];
	sBallModelData modelData[kNumOfSpheres];
	Int2 SpherePartitions[kNumOfSpheres] = { {-1,-1} };

	// to test size of spheres, put 10 in a row, see how many squares it fill
	//IModel* tests[1];
	//for (int i = 0; i < 1; i++)
	//{
	//	tests[i] = ballMesh->CreateModel(i, 0.0f, 10.0f);
	//	tests[i]->SetSkin("RedBall.jpg");
	//	tests[i]->Scale(0.4f);
	//}

	Vector3f velocities[kNumOfSpheres];
	Vector3f positions[kNumOfSpheres];
	SphereStatus statuses[kNumOfSpheres];
	float radii[kNumOfSpheres];

	for (int i = 0; i < kNumOfSpheres; ++i)
	{
		spheres[i] = ballMesh->CreateModel(random(kLowerBound, kUpperBound),
										   random(kLowerBound, kUpperBound), 
										   random(kLowerBound, kUpperBound));

		float scale = random(kMinSphereSize, kMaxSphereSize);
		spheres[i]->Scale(scale);
		statuses[i] = Active;
		positions[i] = Vector3f{spheres[i]->GetX(), spheres[i]->GetY(), spheres[i]->GetZ()};
		radii[i] = scale * kSphereModelSize;

		modelData[i].m_Colour = { 0,0,0 };
		modelData[i].m_Health = 100;

		int newPartitions[2];
		int parts = 0;
		for (int p = 0; p < partitionLayers.size(); ++p)
		{
			if (positions[i].y > partitionLayers.at(p).pos.y - partitionLayers.at(p).distToEdge &&
				positions[i].y < partitionLayers.at(p).pos.y + partitionLayers.at(p).distToEdge)
			{
				newPartitions[parts] = p;
				parts++;
			}
		}
		SpherePartitions[i].first = newPartitions[0];
		SpherePartitions[i].last = newPartitions[1];


		// first half of spheres stationary
		if (i < kNumOfSpheres / 2)
		{
			modelData[i].m_Name = "Static" + std::to_string(i);
			spheres[i]->SetSkin(yellowBallSkin);
			velocities[i] = {0.0f, 0.0f, 0.0f };	// static velocity
		}
		else
		{
			modelData[i].m_Name = "Moving" + std::to_string(i);
			spheres[i]->SetSkin(redBallSkin);

			velocities[i] = {random(kMinXVelocity, kMaxXVelocity),
							 random(kMinYVelocity, kMaxYVelocity),
						     random(kMinYVelocity, kMaxYVelocity) };	// initial velocity
		}
	}
	/////////////////////END OF SCENE SETUP//////////////////////////

	ICamera* myCamera = myEngine->CreateCamera(kManual);
	myCamera->SetPosition(-150, 0, -2200);

	IFont* myFont = myEngine->LoadFont("Arial", 20);
	IFont* reportFont = myEngine->LoadFont("Arial", 16);
	
	bool simulationStarted = false;
	bool reversedSimulation = false;
	bool showPartitions = false;
	bool textEnabled = true;
	bool sphereDeath = false;
	bool playing = false;
	int speedMod = kNormalSpeed;

	float simulationTimer = 0;
	float frameTime = myEngine->Timer();
	unsigned int textColour = kBlack;

	auto t_start = std::chrono::high_resolution_clock::now();
	

	// The main game loop, repeat until engine is stopped
	while (myEngine->IsRunning())
	{
		// Draw the scene
		myEngine->DrawScene();

		// update text
		if (textEnabled)
		{
			myFont->Draw("FrameTime = " + std::to_string(frameTime) + "\n" +
				"FPS = " + std::to_string((1 / frameTime)) + "\n" +
				"Time Elapsed = " + std::to_string(simulationTimer) + "\n\n" +
				"# of Spheres = " + std::to_string(kNumOfSpheres) + "\n" +
				"Sphere Speed = " + std::to_string(speedMod) +  "\n\n" +
				"Collision Reports output to console window \n (note without dying spheres reports will be flooded by spheres that spawned inside another)", 0, 0, textColour);

			myFont->Draw("Press 1 to toggle visible text", 0, 200, textColour);
			myFont->Draw("Press Q and E to decrease and increase sphere speed", 0, 220, textColour);
			myFont->Draw("Press Backspace to change backgrounds", 0, 240, textColour);

			if (g_SPHERE_DEATH)
			{
				myFont->Draw("Press 2 to toggle SphereDeath (ACTIVE)", 0, 260, textColour);
			}
			else
			{
				myFont->Draw("Press 2 to toggle SphereDeath (INACTIVE)", 0, 260, textColour);
			}

			if (g_DETAILED_TIMING)
			{
				myFont->Draw("Press 3 to toggle DetailedTiming (ACTIVE)", 0, 280, textColour);
			}
			else
			{
				myFont->Draw("Press 3 to toggle DetailedTiming (INACTIVE)", 0, 280, textColour);
			}

			if (showPartitions)
			{
				myFont->Draw("Press 4 to toggle Visible partition boundaries (ACTIVE)", 0, 300, textColour);
			}
			else
			{
				myFont->Draw("Press 4 to toggle Visible partition boundaries (INACTIVE)", 0, 300, textColour);
			}
		}

		/**** Update your scene each frame here ****/
		frameTime = myEngine->Timer();

		if (g_DETAILED_TIMING)
		{
			t_start = std::chrono::high_resolution_clock::now();
		}

		if (simulationStarted)
		{
			/////////////MULTITHREADED SPHERE UPDATE///////////////////
			checkedSpheres = kNumOfStatics;
			for (uint32_t t = 0; t < threader.mNumWorkers; ++t)
			{
				// prepare work
				auto& work = threader.mCollisionWorkers[t].second;

				work.testCollisions = false;
				work.spheres = spheres;
				work.selection = { checkedSpheres, checkedSpheres + spheresPerThread };
				work.spherePartitions = SpherePartitions;
				work.partitionData = partitionLayers;
				work.speedMod = speedMod * frameTime;
				work.velocities = velocities;
				work.positions = positions;
				work.statuses = statuses;

				auto& workerThread = threader.mCollisionWorkers[t].first;
				{
					std::unique_lock<std::mutex> lock(workerThread.lock);
					work.complete = false;
				}

				workerThread.workReady.notify_one();

				checkedSpheres += spheresPerThread;
			}

			threader.Update(spheres, velocities, positions, statuses,
				{ checkedSpheres, kNumOfSpheres },
				SpherePartitions, threader.mNumWorkers,
				partitionLayers, speedMod * frameTime);

			for (int t = 0; t < threader.mNumWorkers; ++t)
			{
				auto& work = threader.mCollisionWorkers[t].second;
				auto& workerThread = threader.mCollisionWorkers[t].first;

				std::unique_lock<std::mutex> lock(workerThread.lock);
				workerThread.workReady.wait(lock, [&]() {return work.complete; });

			}
			/////////////////MULTITHREADED COLLISION DETECTION/////////////////
			checkedSpheres = kNumOfStatics;
			for (uint32_t t = 0; t < threader.mNumWorkers; ++t)
			{
				// prepare work
				auto& work = threader.mCollisionWorkers[t].second;

				work.testCollisions = true;
				work.sphereData = modelData;
				work.velocities = velocities;
				work.positions = positions;
				work.radii = radii;
				work.statuses = statuses;
				work.time = t_start;
				work.selection = { checkedSpheres, checkedSpheres + spheresPerThread };
				work.spherePartitions = SpherePartitions;

				auto& workerThread = threader.mCollisionWorkers[t].first;
				{
					std::unique_lock<std::mutex> lock(workerThread.lock);
					work.complete = false;
				}

				workerThread.workReady.notify_one();

				checkedSpheres += spheresPerThread;
			}

			threader.PartitionedCollision(modelData, statuses, velocities, positions, radii, SpherePartitions, threader.mNumWorkers, t_start);
			//threader.SphereCollisions(modelData, statuses, velocities, positions, radii, { checkedSpheres, kNumOfSpheres }, t_start);

			for (int t = 0; t < threader.mNumWorkers; ++t)
			{
				auto& work = threader.mCollisionWorkers[t].second;
				auto& workerThread = threader.mCollisionWorkers[t].first;

				std::unique_lock<std::mutex> lock(workerThread.lock);
				workerThread.workReady.wait(lock, [&]() {return work.complete; });

			}
			//////////////////////END OF MULTITHREADING////////////////////////

			simulationTimer += frameTime;
		}

		if (g_DETAILED_TIMING)
		{
			auto t_end = std::chrono::high_resolution_clock::now();
			cout << "Last loop took: " << std::chrono::duration<double, std::milli>(t_end - t_start).count() << " ms" << endl;
		}
		////////KEY LISTENERS/////////

		if (myEngine->KeyHit(kQuitKey))
		{
			myEngine->Stop();
		}

		if (myEngine->KeyHit(kStartKey))
		{
			simulationStarted = !simulationStarted;
		}

		if (myEngine->KeyHit(kBackSwitch))
		{
			switch (background)
			{
			case Skybox:
				skybox->SetY(kLowerBound * 10);
				starBackground->SetY(0);
				textColour = kWhite;
				background = Stars;
				break;
			case Stars:
				starBackground->SetY(kLowerBound * 10);
				voidBackground->SetY(0);
				textColour = kWhite;
				background = Void;
				break;
			case Void:
				voidBackground->SetY(kLowerBound * 10);
				skybox->SetY(kLowerBound - 500);
				textColour = kBlack;
				background = Skybox;
				break;
			default:
				break;
			}
		}

		if (myEngine->KeyHit(kReverseKey))
		{
			reversedSimulation = !reversedSimulation;
			speedMod = -speedMod;
		}

		if (myEngine->KeyHit(kToggleText))
		{
			textEnabled = !textEnabled;
		}

		if (myEngine->KeyHit(kToggleTiming))
		{
			g_DETAILED_TIMING = !g_DETAILED_TIMING;
		}

		if (myEngine->KeyHit(kToggleDeaths))
		{
			g_SPHERE_DEATH = !g_SPHERE_DEATH;
		}

		if (myEngine->KeyHit(kSpeedUpKey))
		{
			if (reversedSimulation)
				speedMod -= 5;
			else
				speedMod += 5;
		}

		if (myEngine->KeyHit(kSpeedDownKey))
		{
			if (reversedSimulation)
				speedMod += 5;
			else
				speedMod -= 5;
		}



		if (myEngine->KeyHit(kTogglePartitions))
		{
			showPartitions = !showPartitions;
			for (int i = 0; i < floors.size(); ++i)
			{
				if (showPartitions)
					floors[i]->SetY(partitionPositions[i]);

				else
					floors[i]->SetY(kHidePartFloor);
			}
		}

		if (myEngine->KeyHeld(kCameraUp))
		{
			myCamera->MoveLocalZ(kCameraSpeed * frameTime);
		}

		if (myEngine->KeyHeld(kCameraDown))
		{
			myCamera->MoveLocalZ(-kCameraSpeed * frameTime);
		}

		if (myEngine->KeyHeld(kCameraRight))
		{
			myCamera->MoveLocalX(kCameraSpeed * frameTime);
		}

		if (myEngine->KeyHeld(kCameraLeft))
		{
			myCamera->MoveLocalX(-kCameraSpeed * frameTime);
		}

		myCamera->RotateY(myEngine->GetMouseMovementX() * kMouseSpeed * frameTime);
		myCamera->RotateLocalX(myEngine->GetMouseMovementY() * kMouseSpeed * frameTime);
	}

	// Delete the 3D engine now we are finished with it
	myEngine->Delete();
}