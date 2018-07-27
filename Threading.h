#pragma once
#include <TL-Engine.h>	// TL-Engine include file and namespace
#include <thread>
#include <condition_variable>
#include "TypeDefs.h"

using namespace std;

namespace multiThread
{
	struct WorkerThread
	{
		std::thread				thread;
		std::condition_variable workReady;
		std::mutex				lock;
	};

	struct sThreadParams
	{
		bool complete = true;

		sBallModelData* sphereData;
		float speedMod;
		Int2 selection;
		Int2* spherePartitions;
		std::vector<sPartitionData> partitionData;
		tle::IModel** spheres;
		Vector3f* velocities;
		Vector3f* positions;
		SphereStatus* statuses;
		float* radii;
		std::chrono::steady_clock::time_point time;

		bool testCollisions = false;
	};


	class cMultiThreader
	{
	private:
	public:

		cMultiThreader();
		~cMultiThreader();

		void CollisionsThread(uint32_t thread);

		void Update(tle::IModel** spheres, Vector3f* velocities, Vector3f* positions, SphereStatus* statuses, Int2 selection, Int2* spherePartitions, int threadNumber, std::vector<sPartitionData> partitionData, float speedMod);
		void cMultiThreader::PartitionedCollision(sBallModelData* sphereData, SphereStatus* statuses, Vector3f* velocities, Vector3f* positions, float* radii, Int2* spherePartitions, int threadNum, std::chrono::steady_clock::time_point time);
		void cMultiThreader::SphereCollisions(sBallModelData* spheres, SphereStatus* statuses, Vector3f* velocities, Vector3f* positions, float* radii, Int2 startEnd, int threadNum, std::chrono::steady_clock::time_point time);

		static const uint32_t MAX_WORKERS = 31;
		std::pair<WorkerThread, sThreadParams> mCollisionWorkers[MAX_WORKERS];
		uint32_t mNumWorkers;

	};


}

