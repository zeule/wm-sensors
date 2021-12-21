// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_HARDWARE_CPU_CPUID_HXX
#define WM_SENSORS_LIB_HARDWARE_CPU_CPUID_HXX

#include "../../impl/group_affinity.hxx"
#include "../../wm_sensor_types.hxx"

#include <string>
#include <vector>

namespace wm_sensors::hardware::cpu {
	class CPUIDData {
	public:
		enum class Vendor
		{
			Unknown,
			Intel,
			AMD
		};

		struct Rec {
			unsigned operator[](std::size_t i) const
			{
				return regs[i];
			}

			unsigned regs[4];
		};

	public:
		static CPUIDData get(u16 group, u8 thread);

		wm_sensors::impl::GroupAffinity affinity() const
		{
			return affinity_;
		}

		u32 apicId() const
		{
			return apicId_;
		}

		const std::string& brand() const
		{
			return brand_;
		}

		u32 coreId() const
		{
			return coreId_;
		}

		const std::vector<Rec>& data() const
		{
			return data_;
		}

		unsigned data(std::size_t block, std::size_t index) const
		{
			return data_[block][index];
		}

		unsigned safeData(std::size_t block, std::size_t index, unsigned defaultValue) const
		{
			return data_.size() > block ? data_[block][index] : defaultValue;
		}

		const std::vector<Rec> extData() const
		{
			return extData_;
		}

		unsigned extData(std::size_t block, std::size_t index) const
		{
			return extData_[block][index];
		}

		unsigned safeExtData(std::size_t block, std::size_t index, unsigned defaultValue) const
		{
			return extData_.size() > block ? extData_[block][index] : defaultValue;
		}

		u32 family() const
		{
			return family_;
		}

		u16 group() const
		{
			return group_;
		}

		u32 model() const
		{
			return model_;
		}

		const std::string& name() const
		{
			return name_;
		}

		u32 processorId() const
		{
			return processorId_;
		}

		u32 stepping() const
		{
			return stepping_;
		}

		u8 thread() const
		{
			return thread_;
		}

		unsigned threadId() const
		{
			return threadId_;
		}

		Vendor vendor() const
		{
			return vendor_;
		}

		unsigned pkgType() const
		{
			return pkgType_;
		}

	private:
		CPUIDData(u16 group, u8 thread, wm_sensors::impl::GroupAffinity affinity);

		wm_sensors::impl::GroupAffinity affinity_;
		u32 apicId_;
		u32 coreId_;
		u32 family_;
		u32 model_;
		u32 processorId_;
		u32 stepping_;
		unsigned threadId_;
		Vendor vendor_;
		unsigned pkgType_;
		u16 group_;
		u8 thread_;
		std::string brand_;
		std::string name_;
		std::vector<Rec> data_;
		std::vector<Rec> extData_;
	};

} // namespace wm_sensors::hardware::cpu

#endif
