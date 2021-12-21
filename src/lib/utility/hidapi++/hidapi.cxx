// SPDX-License-Identifier: LGPL-3.0+

#include "./hidapi.hxx"

#include <hidapi/hidapi.h>

#include <fmt/format.h>

#include <memory>

namespace {
	const std::size_t maxString = 255;
}

// -------------------------- hid --------------------------
hidapi::hid::hid()
{
	int status = hid_init();
	if (status) {
		throw initialization_error(status);
	}
}

hidapi::hid::~hid()
{
	hid_exit();
}

hidapi::hid& hidapi::hid::instance()
{
	static hid hid;
	return hid;
}

std::vector<hidapi::device_info> hidapi::hid::enumerate(std::uint16_t vendorId, std::uint16_t productId)
{
	struct deleter {
		void operator() (hid_device_info* d)
		{
			hid_free_enumeration(d);
		}
	};
	std::unique_ptr<hid_device_info, deleter> devs{hid_enumerate(vendorId, productId)};
	hid_device_info* curDev = devs.get();
	std::vector<device_info> res;
	while (curDev) {
		res.push_back({curDev->path, curDev->vendor_id, curDev->product_id, curDev->serial_number,
			curDev->release_number, curDev->manufacturer_string, curDev->product_string, curDev->usage_page,
			curDev->usage, curDev->interface_number});
		curDev = curDev->next;
	}

	return res;
}

hidapi::device hidapi::hid::open(std::uint16_t vendorId, std::uint16_t productId, const std::wstring& serialNumber)
{
	const auto device_info_list = enumerate(vendorId, productId);
	if (device_info_list.size()) {
		if (serialNumber.empty()) {
			return open(device_info_list.front());
		}

		for (const auto& di: device_info_list) {
			if (di.serial_number == serialNumber) {
				return open(di);
			}
		}
	}
	throw device_open_error();
}

hidapi::device hidapi::hid::open(hidapi::device_info di)
{
	hid_device* dev = hid_open(di.vendor_id, di.product_id, di.serial_number.c_str());
	if (!dev) {
		throw device_open_error();
	}
	return device{dev, std::move(di)};
}

// ------------------------ device ------------------------

hidapi::device::device(hid_device* handle, device_info&& info)
	: handle_{handle, &hid_close}
	, info_{std::move(info)}
{
}

void hidapi::device::set_nonblocking(bool nonBlocking)
{
	hid_set_nonblocking(handle_.get(), nonBlocking);
}

namespace {
	template <class F, class ...Args>
	std::wstring get_device_string(hid_device* dev, F f, Args... args)
	{
		wchar_t buf[maxString];
		buf[0] = 0;
		f(dev, args..., buf, maxString);
		return std::wstring(buf);
	}
}

std::wstring hidapi::device::manufacturer_string() const
{
	return get_device_string(handle_.get(), &hid_get_manufacturer_string);
}

std::wstring hidapi::device::product_string() const
{
	return get_device_string(handle_.get(), &hid_get_product_string);
}

std::wstring hidapi::device::serial_number_string() const
{
	return get_device_string(handle_.get(), &hid_get_serial_number_string);
}

std::wstring hidapi::device::indexed_string(std::size_t index) const
{
	return get_device_string(handle_.get(), &hid_get_indexed_string, static_cast<int>(index));
}

namespace {
	std::size_t check_io_result(int res)
	{
		if (res < 0) {
			throw hidapi::hid_error();
		}
		return static_cast<std::size_t>(res);
	}
}

std::size_t hidapi::device::read(unsigned char* buf, std::size_t len) const
{
	return check_io_result(hid_read(handle_.get(), buf, len));
}

std::size_t hidapi::device::write(const unsigned char* buf, std::size_t len) const
{
	return check_io_result(hid_write(handle_.get(), buf, len));
}

std::size_t hidapi::device::get_feature_report(unsigned char* buf, std::size_t len) const
{
	return check_io_result(hid_get_feature_report(handle_.get(), buf, len));
}

std::size_t hidapi::device::send_feature_report(const unsigned char* buf, std::size_t len)
{
	return check_io_result(hid_send_feature_report(handle_.get(), buf, len));
}

// -------------------------- Exceptions -------------------------------------

hidapi::device_open_error::device_open_error()
	: std::runtime_error("Could not open HID device")
{
}

hidapi::initialization_error::initialization_error(int status)
	: std::runtime_error{fmt::format("HIDAPI initialization error {0}", status)}
{
}

hidapi::hid_error::hid_error()
	: std::runtime_error("HIDAPI error")
{
}
