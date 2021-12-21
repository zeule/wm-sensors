#include "./sensors_tree.hxx"

#include "../utility.hxx"

#include <string>

void wsensors::ui::SensorsTree::createAndSetup(HWND wnd)
{
	this->SubclassWindow(wnd);
	LONG style = m_ctrlTree.GetWindowLongW(GWL_STYLE);
	style |= TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;
	m_ctrlTree.SetWindowLongW(GWL_STYLE, style);

	int headerSectionIndex = 0;

	HDITEM col = {0};
	col.mask = HDI_FORMAT | HDI_TEXT | HDI_WIDTH;
	col.fmt = HDF_LEFT;
	col.cxy = 400;
	col.pszText = const_cast<TCHAR*>(_T("Sensor"));
	GetHeaderControl().InsertItem(headerSectionIndex++, &col);

	col.fmt = HDF_RIGHT;
	col.cxy = 75;
	col.pszText = const_cast<TCHAR*>(_T("Value"));
	GetHeaderControl().InsertItem(headerSectionIndex++, &col);

	col.pszText = const_cast<TCHAR*>(_T("Min"));
	GetHeaderControl().InsertItem(headerSectionIndex++, &col);

	col.pszText = const_cast<TCHAR*>(_T("Max"));
	GetHeaderControl().InsertItem(headerSectionIndex++, &col);

	col.pszText = const_cast<TCHAR*>(_T("Average"));
	GetHeaderControl().InsertItem(headerSectionIndex++, &col);

	col.pszText = const_cast<TCHAR*>(_T("sd"));
	GetHeaderControl().InsertItem(headerSectionIndex++, &col);

	::SetWindowTheme(m_ctrlTree.m_hWnd, _T("Explorer"), NULL);

	rootItem_ = GetTreeControl().InsertItem(_T("COMPUTER_NAME"), TVI_ROOT, TVI_ROOT);
}

void wsensors::ui::SensorsTree::updateSensorData(
    HTREEITEM item, wm_sensors::SensorType type, const SensorDataRecord& data)
{
	TLVITEM itemData{0};
	std::basic_string<TCHAR> text;

	const auto setChildText = [=, this, &itemData, &text](int i, double v) {
		formatTo(text, v, type);
		itemData.iSubItem = i;
		itemData.mask = TLVIF_TEXT;
		itemData.pszText = const_cast<TCHAR*>(text.c_str());
		SetSubItem(item, &itemData);
		RECT r;
		m_ctrlTree.GetItemRect(item, &r, FALSE);
		r.right = std::max(m_cxHeader, r.right);
		m_ctrlTree.InvalidateRect(&r);
	};

	setChildText(1, data.value);
	setChildText(2, data.min);
	setChildText(3, data.max);
	setChildText(4, data.average);
	setChildText(5, data.sd);
}

CTreeItem wsensors::ui::SensorsTree::addSensorItem(
    const CTreeItem& parent, const std::basic_string<TCHAR>& label, wm_sensors::SensorType type)
{
	SensorDataRecord nullData = {0.};
	CTreeItem itm = m_ctrlTree.InsertItem(label.c_str(), parent.m_hTreeItem, TVI_LAST);
	updateSensorData(itm, type, nullData);
	return itm;
}
