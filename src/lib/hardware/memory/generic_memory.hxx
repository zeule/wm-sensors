// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_HARDWARE_MEMORY_GENERIC_HXX
#define WM_SENSORS_HARDWARE_MEMORY_GENERIC_HXX

#include "../../sensor.hxx"

#include <chrono>

namespace wm_sensors::hardware::memory {
	class GenericMemory: public SensorChip {
		using base = SensorChip;

	public:
		GenericMemory();

		Config config() const override;
		int read(SensorType type, u32 attr, std::size_t channel, double& val) const override;
		int read(SensorType type, u32 attr, std::size_t channel, std::string_view& str) const override;

	private:
		void update() const;
		DELETE_COPY_CTOR_AND_ASSIGNMENT(GenericMemory)

		struct MemoryStatus {
			float load;
			unsigned long long totalPhys;
			unsigned long long availPhys;
			unsigned long long totalPageFile;
			unsigned long long availPageFile;
		};
		mutable MemoryStatus status_;
		mutable std::chrono::steady_clock::time_point lastUpdate_;
	};
} // namespace wm_sensors::hardware::memory

#endif // !WM_SENSORS_HARDWARE_MEMORY_GENERIC_HXX
