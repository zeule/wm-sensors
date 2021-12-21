#include "./controller.hxx"

#include "./custom_messages.h"
#include "./settings.hxx"
#include "./utility.hxx"

#include "ui/resource.h"

#include <lib/utility/string.hxx>
#include <spdlog/spdlog.h>
#include <lib/visitor/chip_visitor.hxx>

#include <map>
#include <stack>

namespace {
	std::map<std::string, wsensors::Controller::NodeIcon> typeIcons = {
		{"cpu", wsensors::Controller::NodeIcon::cpu},
	    {"memory", wsensors::Controller::NodeIcon::memory},
	};
}

wsensors::Controller::Controller(wm_sensors::SensorsTree&& sensors, ui::SensorsTree& view, const Settings& settings)
    : sensors_{std::move(sensors)}
    , model_{sensors_.chips().transform(&wsensors::ChipData::fromSensorChip)}
    , view_{view}
    , settings_{settings}
    , mainThreadId_{::GetCurrentThreadId()}
    , shutdown_{false}
{
	// create tree items for the model
	struct ViewPopulationVisitor: public wm_sensors::SensorChipVisitor {
		ViewPopulationVisitor(
			Controller* pThis,
		    ui::SensorsTree& view, ChipDataTreeNode& model, std::map<ChipData*, ChipUpdateData>& chipToViewMap)
		    : pThis_{pThis}
			, view_{view}
		    , currentPath_{"/"}
		    , model_{model}
		    , chipToViewMap_{chipToViewMap}
		{
			treeItems_.push(view_.rootItem());
		}

		void visit(const NodeAddress& addr, const wm_sensors::TreeNode& /*node*/) override
		{
			if (!addr.nodeName.empty()) {
				auto newItem =
				    CTreeItem({view_.m_ctrlTree.InsertItem(toTString(addr.nodeName).c_str(), treeItems_.top(), TVI_LAST)});
				if (typeIcons.contains(std::string(addr.nodeName))) {
					int iconIndex = pThis_->nodeIconIndices_[typeIcons[std::string(addr.nodeName)]];
					view_.m_ctrlTree.SetItemImage(newItem, iconIndex, iconIndex);
				}
				view_.m_ctrlTree.Expand(treeItems_.top());
				treeItems_.push(newItem);
				currentPath_ = addr.fullPath;
			}
		}

		void ascend() override
		{
			treeItems_.pop();
		}

		void visit(const NodeAddress& addr, std::size_t index, const wm_sensors::SensorChip& chip) override
		{
			ChipDataTreeNode& modelNode = model_.child(addr.fullPath);
			chipToViewMap_.insert(
			    {&modelNode.payload(index), {pThis_->createItemsForSensor(treeItems_.top(), addr.nodeName, chip)}});
			view_.m_ctrlTree.Expand(treeItems_.top());
		}

		ViewPopulationVisitor(const ViewPopulationVisitor&) = delete;
		ViewPopulationVisitor& operator=(const ViewPopulationVisitor&) = delete;
		ViewPopulationVisitor& operator=(ViewPopulationVisitor&&) = delete;

		Controller* pThis_; 
		ui::SensorsTree& view_;
		std::string currentPath_;
		std::stack<CTreeItem> treeItems_;
		ChipDataTreeNode& model_;
		std::map<ChipData*, ChipUpdateData>& chipToViewMap_;
	};

	auto treeControl = view_.GetTreeControl();

	TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = ARRAYSIZE(computerName);
	if (!::GetComputerName(computerName, &size)) {
		spdlog::error(wm_sensors::windowsLastErrorMessage());
	}
	view_.SetSubItemText(view_.rootItem(), 0, computerName);
	loadSensorIcons();
	treeControl.SetImageList(treeIcons_);
	int rootImageIndex = nodeIconIndices_[systemIsLaptop() ? NodeIcon::laptop : NodeIcon::desktop];
	treeControl.SetItemImage(view_.rootItem(), rootImageIndex, rootImageIndex);

	ViewPopulationVisitor populationVisitor{this, view_, *model_, chipToViewMap_};
	sensors_.chips().accept(populationVisitor);
	updateThread_ = std::thread([this]() { this->updateThread(); });
}

wsensors::Controller::~Controller()
{
	if (updateThread_.joinable()) {
		shutdown_ = true;
		updateThread_.join();
	}
}

BOOL wsensors::Controller::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message) {
		case WM_SENSORS_UPDATED: {
			updateView();
			return TRUE;
		}
		default: return FALSE;
	}
}

void wsensors::Controller::updateThread()
{
	// TODO get update timeout from chips
	while (!shutdown_) {
		for (auto& p : chipToViewMap_) {
			p.first->update();
		}
		::PostThreadMessage(mainThreadId_, WM_SENSORS_UPDATED, 0, 0);
		std::this_thread::sleep_for(std::chrono::milliseconds(settings_.uiUpdateInterval));
	}
}

void wsensors::Controller::updateView() {
	ui::SensorsTree::SensorDataRecord dr;
	for (auto& cd : chipToViewMap_) {
		for (const auto& svp : cd.first->values()) {
			for (const auto& sv : svp.second) {
				dr.value = sv.value();
				dr.min = sv.min();
				dr.max = sv.max();
				dr.average = sv.average();
				dr.sd = sv.stDev();
				const HTREEITEM itm = cd.second.channelsMap.at(svp.first)[sv.channel()];
				ATLASSERT(itm != nullptr);
				view_.updateSensorData(itm, svp.first, dr);
			}
		}
	}
	//view_.m_ctrlTree.Invalidate(FALSE);
}

void wsensors::Controller::updateChip(ChipData& /*chipData*/)
{
}

void wsensors::Controller::loadSensorIcons()
{
	SIZE listIconSize{::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON)};
	treeIcons_.Create(listIconSize.cx, listIconSize.cy, ILC_COLOR32, 4, 2);

	// treeIcons_.Add(NULL);
	const auto addImageListImage = [&](int resourceId, NodeIcon type) {
		HBITMAP hbm = loadPngImageFromResource(resourceId, listIconSize);
		int r = treeIcons_.Add(hbm);
		if (r == -1) {
			spdlog::error("Failed to add image to image list: {0}", wm_sensors::windowsLastErrorMessage());
		}
		::DeleteObject(hbm);
		nodeIconIndices_[type] = r;
	};

	addImageListImage(IDB_ICON_COMPUTER, NodeIcon::desktop);
	addImageListImage(IDB_ICON_COMPUTER_LAPTOP, NodeIcon::laptop);
	addImageListImage(IDB_ICON_DEVICE_CHIP, NodeIcon::chip);
	addImageListImage(IDB_ICON_DEVICE_CPU, NodeIcon::cpu);
	addImageListImage(IDB_ICON_DEVICE_HARDDISK, NodeIcon::disk);
	addImageListImage(IDB_ICON_DEVICE_MEMORY, NodeIcon::memory);
	addImageListImage(IDB_ICON_SENSOR_FAN, NodeIcon::sensorFan);
	addImageListImage(IDB_ICON_SENSOR_TEMPERATURE, NodeIcon::sensorTemperature);
	addImageListImage(IDB_ICON_AMD_MONO, NodeIcon::vendorAMD);
	addImageListImage(IDB_ICON_INTEL_MONO, NodeIcon::vendorIntel);
	addImageListImage(IDB_ICON_NVIDIA_MONO, NodeIcon::vendorNVIDIA);
	addImageListImage(IDB_ICON_TEMPERATURE_HOT, NodeIcon::temperatureHot);

	nodeIconIndices_[NodeIcon::none] = 255;
	// std::numeric_limits<int>::max();
}

wsensors::Controller::NodeIcon
wsensors::Controller::nodeIcon(const wm_sensors::HardwareType& id, const std::string& name)
{
	if (id == wm_sensors::hardwareTypes::cpu) {
		if (name.find("AMD") != std::string::npos) {
			return NodeIcon::vendorAMD;
		} else if (name.find("Intel") != std::string::npos) {
			return NodeIcon::vendorIntel;
		}
	} else if (id == wm_sensors::hardwareTypes::gpu) {
		if (name.find("AMD") != std::string::npos) {
			return NodeIcon::vendorAMD;
		} else if (name.find("Intel") != std::string::npos) {
			return NodeIcon::vendorIntel;
		} else if (name.find("NVIDIA") != std::string::npos) {
			return NodeIcon::vendorNVIDIA;
		}
	}
	return NodeIcon::none;
}

wsensors::Controller::ChannelItemsMap
wsensors::Controller::createItemsForSensor(const CTreeItem& parent, const wm_sensors::HardwareType& id, const wm_sensors::SensorChip& chip)
{
	ChannelItemsMap channelItems;
	wm_sensors::SensorChip::Config cfg = chip.config();

	auto treeViewControl = view_.GetTreeControl();
	CTreeItem chipNode =
	    treeViewControl.InsertItem(toTString(chip.identifier().name).c_str(), parent.m_hTreeItem, TVI_LAST);
	NodeIcon icon = nodeIcon(id, chip.identifier().name);
	treeViewControl.SetItemImage(chipNode, nodeIconIndices_[icon], nodeIconIndices_[icon]);

	for (const auto& c: cfg.sensors) {
		if (c.second.channelAttributes.size()) {
			CTreeItem group = createTypeNode(chipNode, c.first);
			for (std::size_t i = 0; i < c.second.channelAttributes.size(); ++i) {
				if (chip.isVisible(c.first, wm_sensors::attributes::generic_input, i).any()) {
					channelItems[c.first].push_back(view_.addSensorItem(group, toTString(chip.channelLabel(c.first, i)), c.first));
				} else {
					channelItems[c.first].push_back(nullptr);
				}
			}
			treeViewControl.Expand(group);
		}
	}
	treeViewControl.Expand(chipNode);
	return channelItems;
}

CTreeItem wsensors::Controller::createTypeNode(const CTreeItem& parent, wm_sensors::SensorType type)
{
	using wm_sensors::SensorType;

	const auto addItem = [this, &parent](LPCTSTR label, NodeIcon icon = NodeIcon::none) {
		auto treeViewControl = view_.GetTreeControl();
		auto item = treeViewControl.InsertItem(label, parent.m_hTreeItem, TVI_LAST);
		treeViewControl.SetItemImage(item, nodeIconIndices_[icon], nodeIconIndices_[icon]);
		return item;
	};

	switch (type) {
		case SensorType::temp: return addItem(_T("Temperatures"), NodeIcon::sensorTemperature);
		case SensorType::in: return addItem(_T("Voltages"));
		case SensorType::curr: return addItem(_T("Currents"));
		case SensorType::power: return addItem(_T("Power"));
		case SensorType::energy: return addItem(_T("Energy"));
		case SensorType::humidity: return addItem(_T("Humidity"));
		case SensorType::fan: return addItem(_T("Fans"), NodeIcon::sensorFan);
		case SensorType::pwm: return addItem(_T("Controls"));
		case SensorType::intrusion: return addItem(_T("Intrusion"));
		case SensorType::data: return addItem(_T("Data"));
		case SensorType::dataRate: return addItem(_T("Data rates"));
		case SensorType::duration: return addItem(_T("Durations"));
		case SensorType::frequency: return addItem(_T("Frequencies"));
		case SensorType::flow: return addItem(_T("Flows"));
		case SensorType::load: return addItem(_T("Loads"));
		case SensorType::raw: return addItem(_T("Values"));
		case SensorType::fraction: return addItem(_T("Fractions"));
		default: return addItem(_T("Unknown type"));
	}
}
