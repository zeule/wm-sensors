// SPDX-License-Identifier: LGPL-3.0+

#include "./motherboard.hxx"

#include "../../sensor_tree.hxx"
#include "../smbios.hxx"
#include "./LPC/lpc_io.hxx"
#include "./LPC/superio/super_io_sensor_chip.hxx"

std::string wm_sensors::hardware::motherboard::Motherboard::boardName(const SMBios& smbios)
{
	if (smbios.board()) {
		if (!smbios.board()->productName().empty()) {
			if (getManufacturer(smbios.board()->manufacturerName()) == Manufacturer::Unknown) {
				return smbios.board()->productName();
			} else {
				return smbios.board()->manufacturerName() + " " + smbios.board()->productName();
			}
		} else {
			return smbios.board()->manufacturerName();
		}
	} else {
		return "Unknown";
	}
}

bool wm_sensors::hardware::motherboard::Motherboard::probe(SensorChipTreeNode& sensorsTree)
{
	const SMBios bios;
	const MotherboardId boardId{
	    bios.board() ? getManufacturer(bios.board()->manufacturerName()) : Manufacturer::Unknown,
	    bios.board() ? getModel(bios.board()->productName()) : Model::Unknown};

	lpc::LpcIo lpcio{boardId};
	SensorChipTreeNode& lpcNode = sensorsTree.child("/motherboard/lpc");
	for (auto& c: lpcio.chips()) {
		lpcNode.addPayload(std::move(c));
	}

	// TODO EC

	return lpcio.chips().size() > 0;
}

static wm_sensors::impl::PersistentHardwareRegistrator<wm_sensors::hardware::motherboard::Motherboard> registrator;
