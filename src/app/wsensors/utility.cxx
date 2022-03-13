#include "./utility.hxx"

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/xchar.h>
#include <lib/utility/string.hxx>
#include <spdlog/spdlog.h>

#include <atlbase.h>
#include <comdef.h>
#include <gdiplus.h>

#include <algorithm>
#include <type_traits>

std::basic_string<TCHAR> wsensors::format(double value, wm_sensors::SensorType type)
{
	std::basic_string<TCHAR> res;
	formatTo(res, value, type);
	return res;
}

void wsensors::formatTo(std::basic_string<TCHAR>& str, double value, wm_sensors::SensorType type)
{
	using wm_sensors::SensorType;
	str.clear();
	auto out = std::back_inserter(str);
	switch (type) {
		case SensorType::temp: out = fmt::format_to(out, _T("{:.1f} ℃"), value); break;
		case SensorType::in: out = fmt::format_to(out, _T("{:.3f} V"), value); break;
		case SensorType::curr: out = fmt::format_to(out, _T("{:.3f} A"), value); break;
		case SensorType::power: out = fmt::format_to(out, _T("{:.3f} W"), value); break;
		case SensorType::energy: out = fmt::format_to(out, _T("{:.3f} J"), value); break;
		case SensorType::humidity: out = fmt::format_to(out, _T("{:.3f} %"), value); break;
		case SensorType::fan: out = fmt::format_to(out, _T("{:.0f} RPM"), value); break;
		case SensorType::pwm: out = fmt::format_to(out, _T("{:.2f} %"), value); break;
		case SensorType::data: {
			if (value > 1024ull * 1024ull * 1024ull * 1024ull) {
				out = fmt::format_to(out, _T("{:.3f} TiB"), value / 1024. / 1024. / 1024. / 1024.);
			} else if (value > 1024 * 1024 * 1024) {
				out = fmt::format_to(out, _T("{:.3f} GiB"), value / 1024 / 1024 / 1024);
			} else if (value > 1024 * 1024) {
				out = fmt::format_to(out, _T("{:.3f} MiB"), value / 1024 / 1024);
			} else if (value > 1024) {
				out = fmt::format_to(out, _T("{:.3f} kiB"), value / 1024);
			} else {
				out = fmt::format_to(out, _T("{:.3f} B"), value);
			}
			break;
		}
		case SensorType::dataRate: out = fmt::format_to(out, _T("{:f} B/s"), value); break; // TODO
		case SensorType::duration: out = fmt::format_to(out, _T("{:f} s"), value); break;   // TODO
		case SensorType::frequency: {
			if (value > 1e9) {
				out = fmt::format_to(out, _T("{:.3f} GHz"), value * 1e-9);
			} else if (value > 1e6) {
				out = fmt::format_to(out, _T("{:.3f} MHz"), value * 1e-6);
			} else if (value > 1e3) {
				out = fmt::format_to(out, _T("{:.3f} kHz"), value * 1e-3);
			} else {
				out = fmt::format_to(out, _T("{:.3f} Hz"), value);
			}
			break;
		}
		case SensorType::flow: out = fmt::format_to(out, _T("{:.3f} L/s"), value); break;
		case SensorType::load: out = fmt::format_to(out, _T("{:.1f} %"), value * 1e2); break;
		case SensorType::raw: out = fmt::format_to(out, _T("{:.3f}"), value); break;
		case SensorType::fraction: out = fmt::format_to(out, _T("{:.1f} %"), value * 1e2); break;
		default: out = fmt::format_to(out, _T("{:.3g}"), value); break;
	}
	*out = _T('\0');
}

std::basic_string<TCHAR> wsensors::toTString(std::string_view s, UINT codePage)
{
#ifdef UNICODE
	std::wstring res;
	res.resize(s.size());
	::MultiByteToWideChar(codePage, 0, s.data(), static_cast<int>(s.size()), res.data(), static_cast<int>(res.size()));
	return res;
#else
	return std::string(s);
#endif // UNICODE
}

bool wsensors::systemIsLaptop()
{
	SYSTEM_POWER_STATUS sps{0};
	if (!::GetSystemPowerStatus(&sps)) {
		spdlog::error("GetSystemPowerStatus() failed: {0}", wm_sensors::windowsLastErrorMessage());
	}
	return sps.ACLineStatus != 255;
}

_COM_SMARTPTR_TYPEDEF(IStream, __uuidof(IStream));

namespace {
	IStreamPtr createStreamOnResource(LPCTSTR lpName, LPCTSTR lpType)
	{
		IStreamPtr res;

		HRSRC hrsrc = ::FindResource(NULL, lpName, lpType);
		if (!hrsrc)
			return res;

		DWORD dwResourceSize = ::SizeofResource(NULL, hrsrc);
		HGLOBAL hglbImage = ::LoadResource(NULL, hrsrc);
		if (!hglbImage)
			return res;

		LPVOID pvSourceResourceData = ::LockResource(hglbImage);
		if (pvSourceResourceData == NULL)
			return res;

		// allocate memory to hold the resource data

		CHeapPtr<std::remove_pointer_t<HGLOBAL>, CGlobalAllocator> hgblResourceData{::GlobalAlloc(GMEM_MOVEABLE, dwResourceSize)};
		if (!hgblResourceData)
			return res;

		// get a pointer to the allocated memory

		LPVOID pvResourceData = ::GlobalLock(hgblResourceData);
		if (pvResourceData == NULL)
			return res; // TODO memleak, free hgblResourceData

		// copy the data from the resource to the new memory block

		::CopyMemory(pvResourceData, pvSourceResourceData, dwResourceSize);
		::GlobalUnlock(hgblResourceData);

		// create a stream on the HGLOBAL containing the data

		if (SUCCEEDED(::CreateStreamOnHGlobal(hgblResourceData, TRUE, &res))) {
			hgblResourceData.Detach();
		}

		return res;
	}
}

HBITMAP wsensors::loadPngImageFromResource(int resourceId, SIZE size)
{
	IStreamPtr resStream = createStreamOnResource(MAKEINTRESOURCE(resourceId), _T("PNG"));
	if (!resStream) {
		return NULL;
	}

	Gdiplus::Bitmap bitmap{resStream};
	LONG width = static_cast<LONG>(bitmap.GetWidth());
	LONG height = static_cast<LONG>(bitmap.GetHeight());

	if (size.cx < 0) {
		size.cx = static_cast<LONG>(width);
	}

	if (size.cy < 0) {
		size.cy = static_cast<LONG>(height);
	}

	HBITMAP hBitmap = NULL;
	if (size.cx != width || size.cy != height) {
		Gdiplus::Bitmap gScaledBmp{size.cx, size.cy, bitmap.GetPixelFormat()};
		std::unique_ptr<Gdiplus::Graphics> g{Gdiplus::Graphics::FromImage(&gScaledBmp)};

		// scale the image
		g->SetInterpolationMode(Gdiplus::InterpolationModeBicubic);
		/* g->DrawImage(
		    &bitmap, Gdiplus::Rect(0, 0, size.cx, size.cy), 0, 0, width, height, Gdiplus::UnitPixel, &imgAtt);*/
		g->DrawImage(&bitmap, Gdiplus::Rect(0, 0, size.cx, size.cy), 0, 0, width, height, Gdiplus::UnitPixel);

		gScaledBmp.GetHBITMAP(Gdiplus::Color::MakeARGB(0, 0, 0, 0), &hBitmap);
		
	} else {
		bitmap.GetHBITMAP(Gdiplus::Color::MakeARGB(0, 0, 0, 0), &hBitmap);
	}

	return hBitmap;
}

wsensors::GdiPlusInit::GdiPlusInit()
{
	Gdiplus::GdiplusStartupInput inp;
	/*auto st = */Gdiplus::GdiplusStartup(&token_, &inp, nullptr); // TODO: handle initialization error
}

wsensors::GdiPlusInit::~GdiPlusInit()
{
	Gdiplus::GdiplusShutdown(token_);
}
