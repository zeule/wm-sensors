// SPDX-License-Identifier: LGPL-3.0+

#ifndef HIDAPI_HXX
#define HIDAPI_HXX

#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

// TODO
typedef struct hid_device_ hid_device;

namespace hidapi {

	class hid;

	struct device_info {
		/** Platform-specific device path */
			std::string path;
			/** Device Vendor ID */
			std::uint16_t vendor_id;
			/** Device Product ID */
			std::uint16_t product_id;
			/** Serial Number */
			std::wstring serial_number;
			/** Device Release Number in binary-coded decimal,
			    also known as Device Version Number */
			std::uint16_t release_number;
			/** Manufacturer String */
			std::wstring manufacturer_string;
			/** Product string */
			std::wstring product_string;
			/** Usage Page for this Device/Interface
			    (Windows/Mac/hidraw only) */
			std::uint16_t usage_page;
			/** Usage for this Device/Interface
			    (Windows/Mac/hidraw only) */
			std::uint16_t usage;
			/** The USB interface which this logical device
			    represents.

				* Valid on both Linux implementations in all cases.
				* Valid on the Windows implementation only if the device
				  contains more than one interface.
				* Valid on the Mac implementation if and only if the device
				  is a USB HID device. */
			int interface_number;
	};

	class device {
	public:
		void set_nonblocking(bool nonBlocking);
		std::wstring manufacturer_string() const;
		std::wstring product_string() const;
		std::wstring serial_number_string() const;
		std::wstring indexed_string(std::size_t index) const;
		std::size_t read(unsigned char* buf, std::size_t len) const;
		std::size_t write(const unsigned char* buf, std::size_t len) const;
		std::size_t send_feature_report(const unsigned char* buf, std::size_t len);
		std::size_t get_feature_report(unsigned char* buf, std::size_t len) const;

		const device_info& info() const
		{
			return info_;
		}
	private:
		friend class hid;
		device(hid_device* handle, device_info&& info);

		std::shared_ptr<::hid_device> handle_;
		device_info info_;
	};

	class hid {
	public:
		static hid& instance();

		std::vector<device_info> enumerate(std::uint16_t vendorId = 0, std::uint16_t productId = 0);
		device open(std::uint16_t vendorId, std::uint16_t productId, const std::wstring& serialNumber = {});
		device open(device_info di);

	private:
		hid();
		~hid();
	};

	class initialization_error: public std::runtime_error {
	public:
		initialization_error(int status);
	private:
		int status_;
	};

	class device_open_error: public std::runtime_error {
	public:
		device_open_error();
	};

	class hid_error: public std::runtime_error {
	public:
		hid_error();
	};
}

#endif
