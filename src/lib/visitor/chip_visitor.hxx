// SPDX-License-Identifier: LGPL-3.0+

#ifndef WM_SENSORS_LIB_VISITOR_CHIP_HXX
#define WM_SENSORS_LIB_VISITOR_CHIP_HXX

#include "../source_class.hxx"

#include <string>

#include "wm-sensors_export.h"

namespace wm_sensors {
	class TreeNode;
	class SensorChip;

	class WM_SENSORS_EXPORT SensorTreeVisitorBase {
	public:
		struct NodeAddress {
			std::string_view fullPath;
			std::string_view nodeName;
		};

		virtual void visit(const NodeAddress& address, const TreeNode& node);
		virtual void ascend();

		virtual ~SensorTreeVisitorBase();
	};

	template <class Payload>
	class SensorTreeVisitor: public SensorTreeVisitorBase {
	public:
		using SensorTreeVisitorBase::visit;

		virtual void visit(
		    [[maybe_unused]] const NodeAddress& path, [[maybe_unused]] std::size_t index,
		    [[maybe_unused]] const Payload& payload)
		{
		}
	};

} // namespace wm_sensors

#endif
