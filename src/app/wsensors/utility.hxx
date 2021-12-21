#pragma once

#include <lib/sensor.hxx>

#include <Windows.h>

#include <memory>
#include <string>
#include <string_view>

namespace wsensors {

	std::basic_string<TCHAR> format(double value, wm_sensors::SensorType type);
	void formatTo(std::basic_string<TCHAR>& str, double value, wm_sensors::SensorType type);

	std::basic_string<TCHAR> toTString(std::string_view s, UINT codePage = CP_ACP);

	bool systemIsLaptop();
	HBITMAP loadPngImageFromResource(int resourceId, SIZE size = {-1, -1});
		
	class GdiPlusInit {
	public:
		GdiPlusInit();
		~GdiPlusInit();

	private:
		ULONG_PTR token_;
	};
} // namespace wsensors
