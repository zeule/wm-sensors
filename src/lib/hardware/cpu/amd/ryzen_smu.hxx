// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_CPU_AMD_RYZEN_SMU_HXX
#define WM_SENSORS_LIB_HARDWARE_CPU_AMD_RYZEN_SMU_HXX

#include "../../../utility/macro.hxx"
#include "../../../wm_sensor_types.hxx"

#include <map>
#include <mutex>
#include <span>

namespace wm_sensors::hardware::cpu {
	class RyzenSMU {
	public:
		RyzenSMU(u32 family, u32 model, u32 packageType);

		enum class CpuCodeName
		{
			Undefined,
			Colfax,
			Renoir,
			Picasso,
			Matisse,
			Threadripper,
			CastlePeak,
			RavenRidge,
			RavenRidge2,
			SummitRidge,
			PinnacleRidge,
			Rembrandt,
			Vermeer,
			Vangogh,
			Cezanne,
			Milan,
			Dali
		};

		struct Address {
			unsigned cmd;
			unsigned rsp;
			unsigned args;
		};

		struct SmuSensorType {
			std::string_view name;
			SensorType type;
			float scale;
		};

		bool isPmTableLayoutDefined() const;
		const std::map<unsigned, SmuSensorType>& pmTableStructure() const;
		std::vector<float> pmTable();

	private:
		DELETE_COPY_CTOR_AND_ASSIGNMENT(RyzenSMU)

		static const std::size_t SMU_REQ_MAX_ARGS = 6;

		bool setupPmTableAddrAndSize();
		bool pmTableVersion(u32& version);
		void setupPmTableSize();
		void setupDramBaseAddr();
		void setupAddrClass1(u32 fn);
		void setupAddrClass2(u32 fn[2]);
		void setupAddrClass3(u32 fn[3]);
		bool transferTableToDRAM();
		std::vector<float> readDRAMToArray();
		bool sendCommand(unsigned msg, std::span<u32, SMU_REQ_MAX_ARGS> args);

		const CpuCodeName cpuCodeName_;
		Address addr_;
		std::mutex mutex_;
		const bool supportedCPU_;

		unsigned dramBaseAddr_;
		unsigned pmTableSize_;
		unsigned pmTableSizeAlt_;
		unsigned pmTableVersion_;
	};
} // namespace wm_sensors::hardware::cpu

#endif
