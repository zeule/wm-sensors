#pragma once

#include "./sensors_data_model.hxx"
#include "./ui/sensors_tree.hxx"

#include <lib/sensor_tree.hxx>

#include <map>
#include <thread>

namespace wsensors {
	class Settings;

	class Controller: public CMessageFilter {
	public:
		Controller(
		    wm_sensors::SensorsTree&& sensors, ui::SensorsTree& view, const Settings& settings);
		virtual ~Controller();

		// Inherited via CMessageFilter
		virtual BOOL PreTranslateMessage(MSG* pMsg) override;

		enum class NodeIcon
		{
			none,
			desktop,
			laptop,
			cpu,
			memory,
			chip,
			disk,
			// sensor types
			sensorFan,
			sensorTemperature,
			// vendors
			vendorAMD,
			vendorIntel,
			vendorNVIDIA,
			// warnings
			temperatureHot,
		};

	private:
		void updateThread();
		void updateView();

		void updateChip(ChipData& chipData);
		void loadSensorIcons();

		static NodeIcon nodeIcon(const wm_sensors::HardwareType& id, const std::string& name);

		DELETE_COPY_CTOR_AND_ASSIGNMENT(Controller)

		using ChannelItemsMap = std::map<wm_sensors::SensorType, std::vector<HTREEITEM>>;

		struct ChipUpdateData {
			ChannelItemsMap channelsMap;
		};

		ChannelItemsMap createItemsForSensor(
		    const CTreeItem& parent, const wm_sensors::HardwareType& id, const wm_sensors::SensorChip& chip);
		CTreeItem createTypeNode(const CTreeItem& parent, wm_sensors::SensorType type);

		wm_sensors::SensorsTree sensors_;
		std::unique_ptr<ChipDataTreeNode> model_;
		ui::SensorsTree& view_;
		const Settings& settings_;
		std::map<ChipData*, ChipUpdateData> chipToViewMap_;
		std::thread updateThread_;
		DWORD mainThreadId_;
		std::map<NodeIcon, int> nodeIconIndices_;
		CImageListManaged treeIcons_;
		bool shutdown_;
	};
} // namespace wsensors
