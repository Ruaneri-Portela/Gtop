#include "gpu.hpp"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <unordered_set>
#include <vector>
#include <algorithm>

static void clearScreen() {
	std::cout << "\033[2J\033[H";
}

/*
 * Impressão compacta das estatísticas globais da GPU
 */
static void printGpuStatsLine(const GPU::PerformanceStatistics &s) {
	constexpr double MB = 1024.0 * 1024.0;

	// Linha 1: utilização e memória
	std::cout
		<< "  Stats: "
		<< "device=" << s.device_utilization << "%  "
		<< "renderer=" << s.renderer_utilization << "%  "
		<< "tiler=" << s.tiler_utilization << "%  "
		<< std::fixed << std::setprecision(1)
		<< "alloc_sys=" << (s.alloc_system_memory / MB) << "MB  "
		<< "in_use_sys=" << (s.in_use_system_memory / MB) << "MB  "
		<< "in_use_drv=" << (s.in_use_system_memory_driver / MB) << "MB  "
		<< "pb_alloc=" << (s.allocated_pb_size / MB) << "MB"
		<< std::endl;

	// Linha 2: cenas, splits, recuperação e HW info
	std::cout
		<< "           "
		<< "tiled=" << (s.tiled_scene_bytes / MB) << "MB  "
		<< "splits=" << s.split_scene_count << "  "
		<< "recov=" << s.recovery_count << "  "
		<< "freq=" << s.gpu_frequency / 1000'000 << "MHz  "
		<< std::fixed << std::setprecision(1)
		<< "volt=" << static_cast<double>(s.gpu_voltage / 1000.0) << "V  "
		<< "watts=" << static_cast<double>(s.milliwatts / 1000.0) << "W  "
		<< "temp=" << s.temp_c << "C"
		<< std::endl;
}

/*
 * Impressão de uma GPU
 */
static void printGPU(const GPU &gpu, size_t index) {
	const auto &stats = gpu.get_statistics();
	const auto &activities = gpu.get_activities();

	std::cout << "GPU #" << index << "\n";
	std::cout
		<< "  Name: " << gpu.get_name()
		<< " (" << gpu.get_core_count() << " cores)\n";

	printGpuStatsLine(stats);
	std::cout << "\n";

	if (activities.empty())
		return;

	std::cout << "\n  Processes:\n";
	std::cout << "    PID     GPU %     API            NAME\n";

	// ---------- ordenar por maior uso ----------
	using ActivityEntry =
		std::pair<const pid_t,
		          std::tuple<GPUActivities, uint64_t, double>>;

	std::vector<const ActivityEntry *> sorted;
	sorted.reserve(activities.size());

	for (const auto &entry : activities)
		sorted.push_back(&entry);

	std::sort(sorted.begin(), sorted.end(),
		[](const ActivityEntry *a, const ActivityEntry *b) {
			return std::get<2>(a->second) > std::get<2>(b->second);
		});

	// ---------- print ----------
	for (const auto *entry : sorted) {
		auto pid = entry->first;
		const auto &currTuple = entry->second;

		auto &activity   = std::get<0>(currTuple);
		auto &percentage = std::get<2>(currTuple);
		auto &name       = activity.name;

		std::unordered_set<std::string> seenApis;
		std::string apiStr;

		for (const auto &u : activity.usage) {
			if (!seenApis.insert(u.api).second)
				continue;

			if (!apiStr.empty())
				apiStr += ",";

			apiStr += u.api;
		}

		std::cout
			<< "    "
			<< std::setw(6) << pid << "  "
			<< std::setw(6) << std::fixed << std::setprecision(1)
			<< percentage << "%   "
			<< std::setw(12) << apiStr << "   "
			<< name << "\n";
	}

	std::cout << "\n";
}

int main() {
	using clock = std::chrono::steady_clock;

	uint64_t lastTotalGpuTime = 0;
	IOGPU ioGPU;

	while (true) {
		clearScreen();

		auto &gpus = ioGPU.get_gpus();

		std::cout << "GPU TOP (IOKit / AGX)\n";
		std::cout << "========================\n\n";

		if (gpus.empty()) {
			std::cout << "No GPUs found.\n";
		} else {
			for (size_t i = 0; i < gpus.size(); ++i) {
				gpus[i].refresh();
				printGPU(gpus[i], i);
			}
		}

		std::cout << "Updated every 1s (Ctrl+C to quit)\n";
		std::cout.flush();

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
