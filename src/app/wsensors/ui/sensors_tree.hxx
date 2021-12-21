#pragma once

#include "../stdafx.hxx"

#include "./TreeListView.h"

#include "lib/sensor.hxx"

#include <atltheme.h>

namespace wsensors::ui {
	class SensorsTree: public CTreeListViewImpl<SensorsTree>, private CThemeImpl<SensorsTree> {
	public:
		using ItemType = CTreeItemT<SensorsTree>;

		void createAndSetup(HWND wnd);

		CTreeItem& rootItem()
		{
			return rootItem_;
		}

		struct SensorDataRecord {
			double value;
			double min;
			double max;
			double average;
			double sd;
			int iconIndex;
		};

		void updateSensorData(HTREEITEM item, wm_sensors::SensorType type, const SensorDataRecord& data);
		void updateSensorData(const CTreeItem& item, wm_sensors::SensorType type, const SensorDataRecord& data)
		{
			updateSensorData(item.m_hTreeItem, type, data);
		}

		CTreeItem addSensorItem(const CTreeItem& parent, const std::basic_string<TCHAR>& label, wm_sensors::SensorType type);

	private:
		CTreeItem rootItem_;
	};
} // namespace wsensors::ui
