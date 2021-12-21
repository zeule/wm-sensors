// SPDX-License-Identifier: LGPL-3.0+

#include "./smbios.hxx"

#include "../utility/unaligned.hxx"

#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/trim.hpp>

#ifdef WIN32
#	include <Windows.h>
#endif

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>


wm_sensors::hardware::BiosData::BiosData(std::span<const std::uint8_t> data, std::span<std::string_view> strings)
    : data_{data}
    , strings_{strings}
{
}

std::uint8_t wm_sensors::hardware::BiosData::byteAt(std::size_t offset, std::uint8_t defaulValue) const
{
	return offset < data_.size() ? data_[offset] : defaulValue;
}

std::uint16_t wm_sensors::hardware::BiosData::wordAt(std::size_t offset, std::uint16_t defaulValue) const
{
	return offset + 1 < data_.size() ? utility::get_unaligned_le<u16>(data_.data() + offset) : defaulValue;
}

std::string wm_sensors::hardware::BiosData::stringAt(std::size_t offset, const std::string& defaultValue) const
{
	return offset < data_.size() && data_[offset] > 0 && data_[offset] <= strings_.size() ?
	           std::string(strings_[data_[offset] - 1u]) :
               defaultValue;
}

namespace {
	std::optional<std::chrono::year_month_day> parseDate(std::string_view s)
	{
		std::string ss{s};
		std::istringstream is{ss};
		int y = 0, m = 0, d = 0;
		char dummy1, dummy2;
		is >> m >> dummy1 >> d >> dummy2 >> y;

		if (dummy1 == '/' && dummy2 == '/' && y > 0 && m > 0 && d > 0) {
			if (m > 12) { std::swap(d, m); }
			if (y < 100) { y += 1900; }
			return std::chrono::year(y) / m / d;
		}
		return {};
	}

	std::optional<std::size_t> biosSize(wm_sensors::hardware::BiosData data)
	{
		auto biosRomSize = data.byteAt(0x09, 0);
		auto extendedBiosRomSize = data.wordAt(0x18);

		bool isExtendedBiosRomSize = biosRomSize == 0xFF && extendedBiosRomSize != 0;
		if (!isExtendedBiosRomSize) return 65536ul * (biosRomSize + 1);


		int unit = (extendedBiosRomSize & 0xC000) >> 14;
		auto extendedSize = static_cast<unsigned long>(extendedBiosRomSize & ~0xC000) * 1024 * 1024;

		switch (unit) {
			case 0x00:
				return extendedSize; // Megabytes
			case 0x01:
				return extendedSize * 1024; // Gigabytes - might overflow in the future
		}

		return {}; // Other patterns not defined in DMI 3.2.0
	}
} // namespace

wm_sensors::hardware::BiosInformation::BiosInformation(
    std::string vendor, std::string version, std::string_view date, std::optional<std::size_t> size)
    : vendor_{std::move(vendor)}
    , version_{std::move(version)}
    , date_{parseDate(date)}
    , size_{std::move(size)}

{
}

wm_sensors::hardware::BiosInformation::BiosInformation(BiosData data)
    : BiosInformation(data.stringAt(0x04), data.stringAt(0x05), data.stringAt(0x08), biosSize(data))
{
}

wm_sensors::hardware::SystemInformation::SystemInformation(
    std::string manufacturerName, std::string productName, std::string version, std::string serialNumber,
    std::string family, wm_sensors::hardware::SystemWakeUp wakeUp)
    : manufacturerName_{std::move(manufacturerName)}
    , productName_{std::move(productName)}
    , version_{std::move(version)}
    , serialNumber_{std::move(serialNumber)}
    , family_{std::move(family)}
    , wakeUp_{wakeUp}
{
}

wm_sensors::hardware::SystemInformation::SystemInformation(wm_sensors::hardware::BiosData data)
    : SystemInformation(
          data.stringAt(0x04), data.stringAt(0x05), data.stringAt(0x06), data.stringAt(0x07), data.stringAt(0x1a),
          data.enumAt<SystemWakeUp>(0x18, SystemWakeUp::Unknown))
{
}

::

    wm_sensors::hardware::ChassisInformation::ChassisInformation(wm_sensors::hardware::BiosData data)
    : manufacturerName_{boost::algorithm::trim_copy(data.stringAt(0x04))}
    , version_{boost::algorithm::trim_copy(data.stringAt(0x06))}
    , serialNumber_{boost::algorithm::trim_copy(data.stringAt(0x07))}
    , assetTag_{boost::algorithm::trim_copy(data.stringAt(0x08))}
    , rackHeight_{data.byteAt(0x11)}
    , powerCords_{data.byteAt(0x12)}
    , SKU_{boost::algorithm::trim_copy(data.stringAt(0x15))}
    , lockDetected_{(data.byteAt(0x05) & 128) == 128}
    , chassisType_{static_cast<ChassisType>(data.byteAt(0x05) & 127)}
    , bootUpState_{data.enumAt<ChassisState>(0x09, ChassisState::Unknown)}
    , powerSupplyState_{data.enumAt<ChassisState>(0x0a, ChassisState::Unknown)}
    , thermalState_{data.enumAt<ChassisState>(0x0b, ChassisState::Unknown)}
    , securityStatus_{data.enumAt<ChassisSecurityStatus>(0x0c, ChassisSecurityStatus::Unknown)}
{
}

wm_sensors::hardware::BaseBoardInformation::BaseBoardInformation(
    std::string manufacturerName, std::string productName, std::string version, std::string serialNumber)
    : manufacturerName_{std::move(manufacturerName)}
    , productName_{std::move(productName)}
    , version_{std::move(version)}
    , serialNumber_{std::move(serialNumber)}
{
}

wm_sensors::hardware::BaseBoardInformation::BaseBoardInformation(wm_sensors::hardware::BiosData data)
    : BaseBoardInformation(
          boost::algorithm::trim_copy(data.stringAt(0x04)), boost::algorithm::trim_copy(data.stringAt(0x05)),
          boost::algorithm::trim_copy(data.stringAt(0x06)), boost::algorithm::trim_copy(data.stringAt(0x07)))
{
}

namespace {
	wm_sensors::hardware::CacheDesignation detectCacheDesignation(const wm_sensors::hardware::BiosData& d)
	{
		using wm_sensors::hardware::CacheDesignation;

		std::string rawCacheType = d.stringAt(0x04);

		if (rawCacheType.find("L1") != std::string::npos) return CacheDesignation::L1;
		if (rawCacheType.find("L2") != std::string::npos) return CacheDesignation::L2;
		if (rawCacheType.find("L3") != std::string::npos) return CacheDesignation::L3;
		return CacheDesignation::Other;
	}
} // namespace

wm_sensors::hardware::ProcessorInformation::ProcessorInformation(wm_sensors::hardware::BiosData d)
    : socketDesignation_{boost::algorithm::trim_copy(d.stringAt(0x04))}
    , manufacturerName_{boost::algorithm::trim_copy(d.stringAt(0x07))}
    , version_{boost::algorithm::trim_copy(d.stringAt(0x10))}
    , coreCount_{d.byteAt(0x23) != 0xff ? static_cast<std::uint16_t>(d.byteAt(0x23)) : d.wordAt(0x2a)}
    , coreEnabled_{d.byteAt(0x24) != 0xff ? static_cast<std::uint16_t>(d.byteAt(0x24)) : d.wordAt(0x2c)}
    , threadCount_{d.byteAt(0x25) != 0xff ? static_cast<std::uint16_t>(d.byteAt(0x25)) : d.wordAt(0x2e)}
    , externalClock_{d.wordAt(0x12)}
    , maxSpeed_{d.wordAt(0x14)}
    , currentSpeed_{d.wordAt(0x16)}
    , serial_{boost::algorithm::trim_copy(d.stringAt(0x20))}
    , processorType_{d.enumAt<ProcessorType>(0x05, ProcessorType::Unknown)}
    , socket_{d.enumAt<ProcessorSocket>(0x19, ProcessorSocket::Other)}
    , family_{
          d.byteAt(0x06) != 0xfe ? d.enumAt<ProcessorFamily, 1>(0x06, ProcessorFamily::Other) :
                                   d.enumAt<ProcessorFamily>(0x28, ProcessorFamily::Other)}
{
}

wm_sensors::hardware::ProcessorCache::ProcessorCache(wm_sensors::hardware::BiosData d)
    : designation_{detectCacheDesignation(d)}
    , associativity_{d.enumAt<CacheAssociativity>(0x12, CacheAssociativity::Unknown)}
    , size_{d.wordAt(0x09)}
{
}

wm_sensors::hardware::MemoryDevice::MemoryDevice(wm_sensors::hardware::BiosData d)
    : deviceLocator_{boost::algorithm::trim_copy(d.stringAt(0x10))}
    , bankLocator_{boost::algorithm::trim_copy(d.stringAt(0x11))}
    , manufacturerName_{boost::algorithm::trim_copy(d.stringAt(0x17))}
    , serialNumber_{boost::algorithm::trim_copy(d.stringAt(0x18))}
    , partNumber_{boost::algorithm::trim_copy(d.stringAt(0x1a))}
    , speed_{d.wordAt(0x15)}
    , size_{d.wordAt(0x0c)}
{
	auto size2 = d.wordAt(0x1c);
	if (size2 > 0) { size_ += size2; }
}

namespace {
#ifndef WIN32
	std::string readSysFsEntry(const std::filesystem::path f)
	{
		if (!std::filesystem::exists(f)) { return {}; }
		std::ifstream inp{f};
		std::string res;
		std::getline(inp, res);
		return res;
	}

	std::string readDmiIdEntry(const std::string_view fn)
	{
		return readSysFsEntry(std::filesystem::path("/sys/class/dmi/id") / fn);
	}
#endif
} // namespace

wm_sensors::hardware::SMBios::SMBios()
{
#ifndef WIN32
	board_.reset(new BaseBoardInformation(
	    readDmiIdEntry("board_vendor"), readDmiIdEntry("board_name"), readDmiIdEntry("board_version"),
	    readDmiIdEntry("chassis_serial")));

	system_.reset(new SystemInformation(
	    readDmiIdEntry("sys_vendor"), readDmiIdEntry("product_name"), readDmiIdEntry("product_version"),
	    readDmiIdEntry("product_serial"),
	    readDmiIdEntry("product_family"))); // TODO add Wake-up Type

	bios_.reset(new BiosInformation(
	    readDmiIdEntry("bios_vendor"), readDmiIdEntry("bios_version"), readDmiIdEntry("bios_date")));
#else
	size_t smbiosTableSize = ::GetSystemFirmwareTable('RSMB', 0, nullptr, 0);
	if (!smbiosTableSize) { return; }
	std::vector<std::uint8_t> raw(smbiosTableSize);
	::GetSystemFirmwareTable('RSMB', 0, &raw.front(), static_cast<DWORD>(raw.size()));

	verMajor_ = raw[1];
	verMinor_ = raw[2];

	std::size_t offset = 8;
	std::uint8_t type = raw[offset];

	while (offset + 4 < smbiosTableSize && type != 127) {
		type = raw[offset];
		auto length = raw[offset + 1];

		if (offset + length > smbiosTableSize) break;

		std::span<const std::uint8_t> data{&raw[offset], length};
		offset += length;

		std::vector<std::string_view> strings;
		if (offset < smbiosTableSize && raw[offset] == 0) offset++;

		while (offset < smbiosTableSize && raw[offset] != 0) {
			auto strBegin = offset;

			for (;offset < smbiosTableSize && raw[offset] != 0; offset++);
			strings.emplace_back(reinterpret_cast<const char*>(&raw[strBegin]), offset - strBegin);

			offset++;
		}

		offset++;
		switch (type) {
			case 0x00: {
				bios_.reset(new BiosInformation({data, strings}));
				break;
			}
			case 0x01: {
				system_.reset(new SystemInformation({data, strings}));
				break;
			}
			case 0x02: {
				board_.reset(new BaseBoardInformation({data, strings}));
				break;
			}
			case 0x03: {
				chassis_.reset(new ChassisInformation({data, strings}));
				break;
			}
			case 0x04: {
				processor_.reset(new ProcessorInformation({data, strings}));
				break;
			}
			case 0x07: {
				processorCaches_.push_back(BiosData{data, strings});
				break;
			}
			case 0x11: {
				memoryDevices_.push_back(BiosData{data, strings});
				break;
			}
		}
	}
#endif
}

std::ostream& wm_sensors::hardware::SMBios::operator<<(std::ostream& os) const
{

	if (verMajor_ || verMinor_) {
		os << "SMBios Version: " << verMajor_ << '.' << verMinor_ << '\n';
	}

	if (bios_) {
		os << "BIOS Vendor: " << bios_->vendor() << '\n'
			<< "BIOS Version: " << bios_->version() << '\n';

		if (bios_->date()) {
			const auto date = bios_->date().value();
			os << "BIOS Date: "
				<< static_cast<int>(date.year()) << '-'
				<< static_cast<unsigned>(date.month()) << '-'
				<< static_cast<unsigned>(date.day()) << '\n';
		}

		if (bios_->size()) {
			const int megabyte = 1024 * 1024;
			os << "BIOS Size: ";
			if (bios_->size().value() > megabyte) {
				os << bios_->size().value() / megabyte << " MiB";
			} else {
				os << bios_->size().value() / 1024 << " KiB";
			}
			os << '\n';
		}

		os << '\n';
	}

	if (system_) {
		const auto& si = *system_;
		os
			<< "System Manufacturer: " << si.manufacturerName() << '\n'
			<<"System Name: " << si.productName() << '\n'
			<< "System Version: " << si.version() << '\n'
			<< "System Wakeup: " << si.wakeUp() << "\n\n";
	}

	if (board_) {
		const auto& bi = *board_;
		os
			<< "Motherboard Manufacturer: " << bi.manufacturerName() << '\n'
			<< "Motherboard Name: " << bi.productName() << '\n'
			<< "Motherboard Version: " << bi.version() << '\n'
			<< "Motherboard Serial: " << bi.serialNumber() << "\n\n";
	}

	if (chassis_) {
		const auto& ci = *chassis_;
		os
			<< "Chassis Type: " << ci.chassisType() << '\n'
			<< "Chassis Manufacturer: " << ci.manufacturerName() << '\n'
			<< "Chassis Version: " << ci.version() << '\n'
			<< "Chassis Serial: " << ci.serialNumber() << '\n'
			<< "Chassis Asset Tag: " << ci.assetTag() << '\n';
		if (!ci.SKU().empty()) {
			os << "Chassis SKU: " << ci.SKU() << '\n';
		}

		os
			<< "Chassis Boot Up State: " << ci.bootUpState() << '\n'
			<< "Chassis Power Supply State: " << ci.powerSupplyState() << '\n'
			<< "Chassis Thermal State: " << ci.thermalState() << '\n'
			<< "Chassis Power Cords: " << ci.powerCords() << '\n';
		if (ci.rackHeight()) {
			os << "Chassis Rack Height: " << ci.rackHeight() << '\n';
		}

		os
			<< "Chassis Lock Detected: " << std::boolalpha << ci.lockDetected() << '\n'
			<< "Chassis Security Status: " << ci.securityStatus() << '\n'
			<< '\n';
	}

	if (processor_) {
		const auto& pi = *processor_;
		os << "Processor Manufacturer: " << pi.manufacturerName() << '\n'
			<< "Processor Type: " << pi.processorType() << '\n'
			<< "Processor Version: " << pi.version() << '\n'
			<< "Processor Serial: " << pi.serial() << '\n'
			<< "Processor Socket Designation: " << pi.socketDesignation() << '\n'
			<< "Processor Socket: " << pi.socket() << '\n'
			<< "Processor Version: " << pi.version() << '\n'
			<< "Processor Family: " << pi.family() << '\n'
			<< "Processor Core Count: " << pi.coreCount() << '\n'
			<< "Processor Core Enabled: " << pi.coreEnabled() << '\n'
			<< "Processor Thread Count: " << pi.threadCount() << '\n'
			<< "Processor External Clock: " << pi.externalClock() << " MHz\n"
			<< "Processor Max Speed: " << pi.maxSpeed() << " MHz\n"
			<< "Processor Current Speed: " << pi.currentSpeed() << "MHz\n";
	}

	for (const auto& pc: processorCaches_) {
		os << "Cache [" << pc.designation() << "] Size: " << pc.size() << '\n'
			<< "Cache [" << pc.designation() << "] Associativity: " << pc.associativity() << '\n'
			<< '\n';
	}


	for (std::size_t i = 0; i < memoryDevices_.size(); ++i) {
		const auto& md = memoryDevices_[i];
		os << "Memory Device [" << i << "] Manufacturer: " << md.manufacturerName() << '\n'
			<< "Memory Device [" << i << "] Part Number: " << md.partNumber() << '\n'
			<< "Memory Device [" << i << "] Device Locator: " << md.deviceLocator() << '\n'
			<< "Memory Device [" << i << "] Bank Locator: " << md.bankLocator() << '\n'
			<< "Memory Device [" << i << "] Speed: " << md.speed() << '\n'
			<< "Memory Device [" << i << "] Size: " << md.size() << " MB\n\n";
	}

	/*
	if (_raw != null) {
		string base64 = Convert.ToBase64String(_raw);
		r.AppendLine("SMBios Table");
		r.AppendLine();

		for (int i = 0; i < Math.Ceiling(base64.Length / 64.0); i++) {
			r.Append(" ");
			for (int j = 0; j < 0x40; j++) {
				int index = (i << 6) | j;
				if (index < base64.Length) { r.Append(base64[index]); }
			}

			r.AppendLine();
		}

		r.AppendLine();
	}
	*/

	return os;
}

