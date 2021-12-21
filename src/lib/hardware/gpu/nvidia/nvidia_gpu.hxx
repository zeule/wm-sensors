// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_GPU_NVIDIA_GPU_HXX
#define WM_SENSORS_LIB_HARDWARE_GPU_NVIDIA_GPU_HXX

#include "../../../sensor.hxx"

namespace wm_sensors::hardware::gpu {

	class NVIDIAGPU: public SensorChip {
	private:
		DELETE_COPY_CTOR_AND_ASSIGNMENT(NVIDIAGPU)
	};
} // namespace wm_sensors::hardware::gpu

#endif
