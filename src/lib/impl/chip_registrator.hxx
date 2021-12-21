// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_IMPL_CHIP_REGISTRATOR_HXX
#define WM_SENSORS_LIB_IMPL_CHIP_REGISTRATOR_HXX

#include "../source_class.hxx"
#include "../sensor.hxx"

#include <memory>
#include <vector>

namespace wm_sensors {
	template <class, class>
	class SensorTreeNode;

	using SensorChipTreeNode = SensorTreeNode<HardwareType, std::unique_ptr<SensorChip>>;
}

namespace wm_sensors::impl {

	class ChipProbe {
	public:
		virtual ~ChipProbe();

		virtual bool probe(SensorChipTreeNode& sensorsTree) = 0;
	};

	class PersistentHardwareRegistratorImpl {
	public:
		PersistentHardwareRegistratorImpl(std::unique_ptr<ChipProbe> probe);
		~PersistentHardwareRegistratorImpl();

	private:
		ChipProbe* probe_;
	};

	template <class Probe>
	class PersistentHardwareRegistrator: public PersistentHardwareRegistratorImpl {
	public:
		PersistentHardwareRegistrator()
		    : PersistentHardwareRegistratorImpl(std::unique_ptr<ChipProbe>(new Probe()))
		{
		}
	};

	class ChipProbesRegistry {
	public:
		static ChipProbesRegistry& instance();

		~ChipProbesRegistry();

		void add(std::unique_ptr<ChipProbe> probe);
		void remove(ChipProbe* probe);

		void probeAll(SensorChipTreeNode& tree);

	private:
		ChipProbesRegistry();

		std::vector<std::unique_ptr<ChipProbe>> probes_;
	};
}

#endif
