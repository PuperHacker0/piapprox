/*
Approximation of pi with Monte Carlo & Multithreading

Logic:
	-Take a unit circle insribed in a square (side = 2)
	-Generate random points inside the square
	-Count how many are <= 1 unit away (circle's radius) from the circle's center (=> inside the circle) or outside it (> 1 unit away)
	-The formula for the distance of two points, assuming that the center is at (0, 0): 
		sqrt((x - axisStartX) ^ 2 + (y - axisStartY) ^ 2) = sqrt((x - 0) ^ 2 + (y - 0) ^ 2) = sqrt(x ^ 2 + y ^ 2)
	-Area of circle: pi * r ^ 2 (unit circle => r = 1 => r ^ 2 = 1) => pi
	-Area of square in which the unit circle is inscribed: side ^ 2 = 2 ^ 2 = 4
	-As the points start populating the surface of the circle, pointsIn / pointsTotal ~= areaOfCircle / areaOfSquare = pi / 4
	=> pi = 4 * pointsIn / pointsTotal
*/

#include <iostream>
#include <windows.h>
#include <string>
#include <thread>
#include <vector>
#include <random>

typedef unsigned long long int bigint;

class ThreadObject {
	bigint pointsInsideCircle = 0, pointsOutsideCircle = 0;

	std::random_device seed;
	std::mt19937 generator; //Mersenne twister engine
	std::uniform_real_distribution<double> distribution;

	bool iterationsFinished = false;

	inline double pow2(double num) const { return num * num; }

public:
	ThreadObject() {
		generator = std::mt19937(seed());
		distribution = std::uniform_real_distribution<double>(-1, 1);
	};

	inline bigint getPoinsInsideCircle() const { return pointsInsideCircle; }

	inline bigint getPoinsOutsideCircle() const { return pointsOutsideCircle; }

	inline double randomCoord(int lowerBound = -1, int upperBound = 1) { return distribution(generator); }

	inline void processPoint(double x, double y) { pow2(x) + pow2(y) <= 1 ? ++pointsInsideCircle : ++pointsOutsideCircle; }

	inline void setFinished() { iterationsFinished = true; }

	inline bool isFinished() const { return iterationsFinished; }
};

class PiCalculator {
	double computeFromData(std::vector<ThreadObject>& ThreadObjects) {
		bigint totalPointsInCircle = 0, totalPoints = 0;

		for (auto& i : ThreadObjects) {
			totalPointsInCircle += i.getPoinsInsideCircle();
			totalPoints += i.getPoinsOutsideCircle();
		}
		totalPoints += totalPointsInCircle;

		return 4 * static_cast<double>(totalPointsInCircle) / static_cast<double>(totalPoints);
	}

	inline bool allThreadsFinished(std::vector<ThreadObject>& ThreadObjects) {
		for (auto& t : ThreadObjects)
			if (!t.isFinished())
				return false;
		return true;
	}

	void displayData(std::vector<ThreadObject>& ThreadObjects, int threadCount) {
		bigint totalPoints = 0, currentThreadPoints = 0;

		for (int i = 0; i < threadCount; ++i) {
			currentThreadPoints = ThreadObjects[i].getPoinsInsideCircle() + ThreadObjects[i].getPoinsOutsideCircle();
			printf("T%d: %lld\n", i, currentThreadPoints);

			totalPoints += currentThreadPoints;
		}

		printf("Total points: %lld\nPi: %f\n", totalPoints, computeFromData(ThreadObjects));
	}

public:
	void threadLoop(ThreadObject& t, int maxIterations) {
		for (auto i = 0; i < maxIterations; ++i)
			t.processPoint(t.randomCoord(), t.randomCoord());
		t.setFinished();
	}

	void calculatePi(bigint maxIterationsPerThread) {
		int threadCount = std::thread::hardware_concurrency();

		std::vector<ThreadObject> threadObjects(threadCount);
		std::vector<std::thread> threads;

		for (int i = 0; i < threadCount; ++i) { //Start threads
			threads.push_back(std::thread(&PiCalculator::threadLoop, this, std::ref(threadObjects[i]), maxIterationsPerThread));
			//							pointer to member func, the actual object which is 'this', and the arguments
			threads[i].detach(); //All threads are independent from main()
		}

		while (!allThreadsFinished(threadObjects)) {
			displayData(threadObjects, threadCount);
			Sleep(2000); //Pause, sample again after a time period
			system("CLS");
		}

		displayData(threadObjects, threadCount); //Display final findings
	}
};

int main() {
	SetConsoleTitle(TEXT("Pi approximation with uniform distribution"));

	PiCalculator p;
	p.calculatePi(static_cast<bigint>(1e8));

	return 0;
}