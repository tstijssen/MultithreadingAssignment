#include "Threading.h"
#include <iostream>
#include <string>

using namespace multiThread;
using namespace std;

cMultiThreader::cMultiThreader()
{
	//*********************************************************
	// Start worker threads
	mNumWorkers = std::thread::hardware_concurrency(); // Gives a hint about level of thread concurrency supported by system (0 means no hint given)
	if (mNumWorkers == 0)  mNumWorkers = 8;
	--mNumWorkers; // Decrease by one because this main thread is already running
	for (uint32_t i = 0; i < mNumWorkers; ++i)
	{
		// Start each worker thread running the BlockSpritesThread method. Note the way to construct std::thread to run a member function
		//haveWork[i] = false;
		mCollisionWorkers[i].first.thread = std::thread(&cMultiThreader::CollisionsThread, this, i);
	}
}

void cMultiThreader::Update(tle::IModel** spheres, Vector3f* velocities, Vector3f* positions, SphereStatus* statuses, Int2 selection, Int2* spherePartitions, int threadNumber, std::vector<sPartitionData> partitionData, float speedMod)
{
	for (int i = selection.first; i < selection.last; ++i)
	{
		if (statuses[i] == Active || statuses[i] == Checked)
		{
			statuses[i] = Active;	// reset for this frames collisions
			positions[i].x += velocities[i].x * speedMod;
			positions[i].y += velocities[i].y * speedMod;
			positions[i].z += velocities[i].z * speedMod;

			// check if hit left-right boundary
			if (positions[i].x < kLowerBound || positions[i].x > kUpperBound)
			{
				// reflect
				velocities[i].x = -velocities[i].x;
			}
			// check if hit top-down boundary
			else if (positions[i].y < kLowerBound || positions[i].y > kUpperBound)
			{
				// reflect
				velocities[i].y = -velocities[i].y;
			}
			// check if hit forward-back boundary
			else if (positions[i].z < kLowerBound || positions[i].z > kUpperBound)
			{
				// reflect
				velocities[i].z = -velocities[i].z;
			}

			int newPartitions[2] = {-1};
			int parts = 0;
			for (int p = 0; p < partitionData.size(); ++p)
			{
				if (positions[i].y > partitionData.at(p).pos.y - partitionData.at(p).distToEdge &&
					positions[i].y < partitionData.at(p).pos.y + partitionData.at(p).distToEdge)
				{
					newPartitions[parts] = p;
					parts++;
				}
			}
			spherePartitions[i].first = newPartitions[0];
			spherePartitions[i].last = newPartitions[1];

			//// sphere has gone above current partition
			//if (positions[i].y > partitionData.x + partitionData.y)
			//{
			//	// if not the last thread (top partition)
			//	if (threadNumber < mNumWorkers)
			//	{
			//		spherePartitions[i] = threadNumber + 1;
			//	}
			//}
			//// sphere has gone below current partition
			//else if (positions[i].y < partitionData.x - partitionData.y)
			//{
			//	// if not the first thread (bottom partition)
			//	if (threadNumber > 0)
			//	{
			//		spherePartitions[i] = threadNumber - 1;
			//	}
			//}
			spheres[i]->SetPosition(positions[i].x, positions[i].y, positions[i].z);

		}
		else if(statuses[i] == Destroyed)
		{
			statuses[i] = Inactive;
			spheres[i]->SetY(kHidePartFloor);
		}
	}
}

void cMultiThreader::SphereCollisions(sBallModelData* sphereData, SphereStatus* statuses, Vector3f* velocities, Vector3f* positions, float* radii, Int2 startEnd, int threadNum, std::chrono::steady_clock::time_point time)
{
	for (int i = startEnd.first; i < startEnd.last; ++i)
	{
		if (statuses[i] != Inactive)
		{
			statuses[i] = Active;
		}
		if (statuses[i] == Active)
		{
			for (int c = 0; c < kNumOfSpheres; ++c)
			{
				if (c != i && statuses[c] == Active)
				{

					float x = positions[i].x - positions[c].x;
					float y = positions[i].y - positions[c].y;
					float z = positions[i].z - positions[c].z;

					float r = radii[i] + radii[c];
					float distance = x * x + y * y + z * z;

					if (distance <= r * r)
					{
						Vector3f n = {	positions[c].x - positions[i].x,
										positions[c].y - positions[i].y,
										positions[c].z - positions[i].z };

						n = Normalise(n);


						Vector3f d = { velocities[i].x, velocities[i].y, velocities[i].z };
						float DdotN = dot(d, n);

						velocities[i] = d - (2 * DdotN) * n;



						// check if collider has static velocity, if not, perform reflection
						if (velocities[c].x != 0.0f || velocities[c].y != 0.0f || velocities[c].z != 0.0f)
						{
							Vector3f d2 = { velocities[c].x, velocities[c].y, velocities[c].z };
							Vector3f n2 = {	positions[i].x - positions[c].x,
											positions[i].y - positions[c].y,
											positions[i].z - positions[c].z };

							n2 = Normalise(n2);
							float DdotN2 = dot(d2, n2);
							velocities[c] = d2 - (2 * DdotN2) * n2;

						}


						sphereData[i].m_Health -= 10;
						sphereData[c].m_Health -= 10;

						std::cout << "" << sphereData[i].m_Name << " HP: " << sphereData[i].m_Health <<
							" collided with " << sphereData[c].m_Name << " HP: " << sphereData[c].m_Health << " THREAD = " << threadNum << endl;
						if (g_DETAILED_TIMING)
						{
							auto collisionTime = std::chrono::high_resolution_clock::now();
							std::cout << "At: " << std::chrono::duration<double, std::milli>(collisionTime - time).count() << " ms since loop start" << endl;
						}

						if (g_SPHERE_DEATH)
						{
							if (sphereData[i].m_Health <= 0)
							{
								statuses[i] = Destroyed;
								std::cout << sphereData[i].m_Name << " dies!" << endl;
							}
							if (sphereData[c].m_Health <= 0)
							{
								statuses[c] = Destroyed;
								std::cout << sphereData[c].m_Name << " dies!" << endl;
							}
						}
						std::cout << endl;
					}
				}
			}
			statuses[i] = Checked;
		}
	}
}

void cMultiThreader::PartitionedCollision(sBallModelData* sphereData, SphereStatus* statuses, Vector3f* velocities, Vector3f* positions, float* radii, Int2* spherePartitions, int threadNum, std::chrono::steady_clock::time_point time)
{
	for (int i = kNumOfStatics; i < kNumOfSpheres; ++i)
	{
		if ((spherePartitions[i].first == threadNum || spherePartitions[i].last == threadNum) && statuses[i] == Active)
		{
			for (int c = 0; c < kNumOfSpheres; ++c)
			{
				if ((spherePartitions[c].first == threadNum || spherePartitions[c].last == threadNum) && c != i && statuses[c] == Active)
				{
					// determine the x,y distances between the spheres
					float x = positions[i].x - positions[c].x;
					float y = positions[i].y - positions[c].y;
					float z = positions[i].z - positions[c].z;

					float r = radii[i] + radii[c];
					float distance = x * x + y * y + z * z;

					if (distance <= r * r)
					{
						Vector3f n = { positions[c].x - positions[i].x,
							positions[c].y - positions[i].y,
							positions[c].z - positions[i].z };

						n = Normalise(n);


						Vector3f d = { velocities[i].x, velocities[i].y, velocities[i].z };
						float DdotN = dot(d, n);

						velocities[i] = d - (2 * DdotN) * n;

						// check if collider has static velocity, if not, perform reflection
						if (velocities[c].x != 0.0f || velocities[c].y != 0.0f || velocities[c].z != 0.0f)
						{
							Vector3f d2 = { velocities[c].x, velocities[c].y, velocities[c].z };
							Vector3f n2 = { positions[i].x - positions[c].x,
								positions[i].y - positions[c].y,
								positions[i].z - positions[c].z };

							n2 = Normalise(n2);
							float DdotN2 = dot(d2, n2);
							velocities[c] = d2 - (2 * DdotN2) * n2;
							statuses[c] = Checked;

						}
						statuses[i] = Checked;


						sphereData[i].m_Health -= 10;
						sphereData[c].m_Health -= 10;

						std::cout << "" << sphereData[i].m_Name << " HP: " << sphereData[i].m_Health <<
							" collided with " << sphereData[c].m_Name << " HP: " << sphereData[c].m_Health << " THREAD = " << threadNum << endl;
						if (g_DETAILED_TIMING)
						{
							auto collisionTime = std::chrono::high_resolution_clock::now();
							std::cout << "At: " << std::chrono::duration<double, std::milli>(collisionTime - time).count() << " ms since loop start" << endl;
						}

						if (g_SPHERE_DEATH)
						{
							if (sphereData[i].m_Health <= 0)
							{
								statuses[i] = Destroyed;
								std::cout << sphereData[i].m_Name << " dies!" << endl;
							}
							if (sphereData[c].m_Health <= 0)
							{
								statuses[c] = Destroyed;
								std::cout << sphereData[c].m_Name << " dies!" << endl;
							}
						}
						std::cout << endl;
					}
				}
			}
		}
	}
}

cMultiThreader::~cMultiThreader()
{
	for (uint32_t i = 0; i < mNumWorkers; ++i)
	{
		mCollisionWorkers[i].first.thread.detach();
	}
}

void cMultiThreader::CollisionsThread(uint32_t thread)
{
	auto& worker = mCollisionWorkers[thread].first;
	auto& work = mCollisionWorkers[thread].second;

	while (true)
	{
		// Guard use of haveWork from other thread
		{
			std::unique_lock<std::mutex> lock(worker.lock);
			worker.workReady.wait(lock, [&]() {return !work.complete; });

		}
			if (work.testCollisions)
			{
				//SphereCollisions(work.sphereData, work.statuses, work.velocities, work.positions, work.radii, work.selection, work.time);
				PartitionedCollision(work.sphereData, work.statuses, work.velocities, work.positions, work.radii, work.spherePartitions, thread, work.time);

			}
			else
			{
				Update(work.spheres, work.velocities, work.positions, work.statuses, work.selection, work.spherePartitions, thread, work.partitionData, work.speedMod);
			}
			
		{
			std::unique_lock<std::mutex> lock(worker.lock);
			work.complete = true;
		}

		worker.workReady.notify_one();
	}
}