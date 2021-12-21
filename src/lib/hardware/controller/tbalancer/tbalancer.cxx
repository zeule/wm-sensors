// SPDX-License-Identifier: LGPL-3.0+

#include "./tbalancer.hxx"

#include <ftd2xx.h>
#include <memory>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

namespace {
	using namespace wm_sensors::stdtypes;

	struct ftPurgeAndClose {
		void operator()(FT_HANDLE h)
		{
			FT_Purge(h, FT_PURGE_RX | FT_PURGE_TX);
			FT_Close(h);
		}
	};

	DWORD ftBytesToRead(FT_HANDLE h)
	{
		DWORD rx, tx, ev;
		if (::FT_GetStatus(h, &rx, &tx, &ev) == FT_OK) {
			return rx;
		}
		return 0;
	}

	u8 ftReadByte(FT_HANDLE h)
	{
		DWORD res, nrRead;
		if (FT_Read(h, &res, 1, &nrRead) != FT_OK) {
			return 0;
		}
		return res;
	}

	void ftSetup(FT_HANDLE h)
	{
		FT_SetBaudRate(h, 19200);
		FT_SetDataCharacteristics(h, 8, 1, 0);
		FT_SetFlowControl(h, FT_FLOW_RTS_CTS, 0x11, 0x13);
		FT_SetTimeouts(h, 1000, 1000);
		FT_Purge(h, FT_PURGE_RX | FT_PURGE_TX);
	}

	const u8 endFlag = 254;
	const u8 startFlag = 100;
	const u8 reedRequest = 0x38;
	const u8 alternativeReadRequest = 0x37;

	using DataArray = std::array<u8, 285>;
} // namespace

std::vector<std::unique_ptr<wm_sensors::SensorChip>> wm_sensors::hardware::tbalancer::TBalancer::probe()
{
	DWORD nrDevices;
	if (::FT_CreateDeviceInfoList(&nrDevices) != FT_OK || nrDevices == 0) {
		return {};
	}

	std::vector<FT_DEVICE_LIST_INFO_NODE> info;
	info.resize(nrDevices);
	if (::FT_GetDeviceInfoList(info.data(), &nrDevices) != FT_OK) {
		return {};
	}

	std::vector<std::unique_ptr<wm_sensors::SensorChip>> res;
	for (std::size_t i = 0; i < info.size(); ++i) {
		if (info[i].Type != FT_DEVICE_BM) {
			continue;
		}

		std::unique_ptr<FT_HANDLE, ftPurgeAndClose> handle;
		FT_STATUS status = FT_Open(i, handle.get());
		if (status != FT_OK) {
			//_report.AppendLine("Open Status: " + status);
			continue;
		}

		ftSetup(*handle);

		DWORD written;
		status = FT_Write(*handle, const_cast<u8*>(&reedRequest), 1, &written);
		if (status != FT_OK || !written) {
			//_report.AppendLine("Write Status: " + status);
			continue;
		}

		bool isValid = false;
		u8 protocolVersion = 0;

		int j = 0;
		while (ftBytesToRead(*handle) == 0 && j < 2) {
			std::this_thread::sleep_for(100ms);
			j++;
		}

		if (ftBytesToRead(*handle) > 0) {
			if (ftReadByte(*handle) == startFlag) {
				while (ftBytesToRead(*handle) < 284 && j < 5) {
					std::this_thread::sleep_for(100ms);
					j++;
				}

				auto length = ftBytesToRead(*handle);
				if (length >= 284) {
					DataArray data;
					data[0] = startFlag;
					DWORD nrRead = 0;
					FT_Read(*handle, &data[1], 284, &nrRead);
					if (nrRead != 284) {
						continue;
					}

					// check protocol version 2X (protocols seen: 2C, 2A, 28)
					isValid = (data[274] & 0xF0) == 0x20;
					protocolVersion = data[274];
				}
			}
		}

		handle.reset(nullptr);
		if (isValid) {
			res.push_back(std::unique_ptr<SensorChip>(new TBalancer(i, protocolVersion)));
		}
	}
	return res;
}

struct wm_sensors::hardware::tbalancer::TBalancer::Impl {
	Impl(std::size_t portIndex, u8 protocolVer);

	void update();
	void readData();
	void readMiniNG(unsigned number, const DataArray& data);

	std::unique_ptr<FT_HANDLE, ftPurgeAndClose> handle;
private:
	u8 protocolVersion_;

        private readonly Sensor[] _analogTemperatures = new Sensor[4];
        private readonly Sensor[] _controls = new Sensor[4];
        private readonly List<ISensor> _deactivating = new List<ISensor>();
        private readonly Sensor[] _digitalTemperatures = new Sensor[8];
        private readonly Sensor[] _fans = new Sensor[4];
        private readonly Sensor[] _miniNgControls = new Sensor[4];
        private readonly Sensor[] _miniNgFans = new Sensor[4];
        private readonly Sensor[] _miniNgTemperatures = new Sensor[4];
        private readonly int _portIndex;
        private readonly byte _protocolVersion;
        private readonly Sensor[] _sensorHubFlows = new Sensor[2];
        private readonly Sensor[] _sensorHubTemperatures = new Sensor[6];
        private byte[] _alternativeData = new byte[0];
        private readonly byte[] _data = new byte[285];
        private Ftd2xx.FT_HANDLE _handle;
        private byte[] _primaryData = new byte[0];
};

wm_sensors::hardware::tbalancer::TBalancer::Impl::Impl(std::size_t portIndex, wm_sensors::stdtypes::u8 protocolVer)
{
	FT_STATUS status = FT_Open(portIndex, handle.get());
	if (status != FT_OK) {
		throw std::runtime_error("Can't open FT2xx device");
	}
	protocolVersion_ = protocolVer;
	ftSetup(*handle);
}

void wm_sensors::hardware::tbalancer::TBalancer::Impl::update()
{
	while (ftBytesToRead(*handle) >= 285) {
		readData();
	}

	if (ftBytesToRead(*handle) == 1) {
		ftReadByte(*handle);
	}

	DWORD written;
	FT_Write(*handle, const_cast<u8*>(&reedRequest), 1, &written);

	std::thread alternativeRequest{[this]() {
		std::this_thread::sleep_for(500ms);
		DWORD written;
		FT_Write(*handle, const_cast<u8*>(&alternativeReadRequest), 1, &written);
	}};
	alternativeRequest.detach();
}

void wm_sensors::hardware::tbalancer::TBalancer::Impl::readData()
{
	DataArray data;
	DWORD read;
	auto st = ::FT_Read(*handle, data.data(), data.size(), &read);
        if (data[0] != startFlag)
            {
                FT_Purge(*handle, FT_PURGE_RX);
                return;
            }

            if (data[1] == 255 || data[1] == 88)
            {
                // bigNG

                if (data[274] != protocolVersion_)
                    return;


                if (_primaryData.Length == 0)
                    _primaryData = new byte[_data.Length];

                _data.CopyTo(_primaryData, 0);

                for (int i = 0; i < _digitalTemperatures.Length; i++)
                {
                    if (data[238 + i] > 0)
                    {
                        _digitalTemperatures[i].Value = 0.5f * _data[238 + i] + _digitalTemperatures[i].Parameters[0].Value;
                        ActivateSensor(_digitalTemperatures[i]);
                    }
                    else
                    {
                        DeactivateSensor(_digitalTemperatures[i]);
                    }
                }

                for (int i = 0; i < _analogTemperatures.Length; i++)
                {
                    if (data[260 + i] > 0)
                    {
                        _analogTemperatures[i].Value = 0.5f * _data[260 + i] + _analogTemperatures[i].Parameters[0].Value;
                        ActivateSensor(_analogTemperatures[i]);
                    }
                    else
                    {
                        DeactivateSensor(_analogTemperatures[i]);
                    }
                }

                for (int i = 0; i < _sensorHubTemperatures.Length; i++)
                {
                    if (data[246 + i] > 0)
                    {
                        _sensorHubTemperatures[i].Value = 0.5f * _data[246 + i] + _sensorHubTemperatures[i].Parameters[0].Value;
                        ActivateSensor(_sensorHubTemperatures[i]);
                    }
                    else
                    {
                        DeactivateSensor(_sensorHubTemperatures[i]);
                    }
                }

                for (int i = 0; i < _sensorHubFlows.Length; i++)
                {
                    if (data[231 + i] > 0 && _data[234] > 0)
                    {
                        float pulsesPerSecond = (_data[231 + i] * 4.0f) / _data[234];
                        float pulsesPerLiter = _sensorHubFlows[i].Parameters[0].Value;
                        _sensorHubFlows[i].Value = pulsesPerSecond * 3600 / pulsesPerLiter;
                        ActivateSensor(_sensorHubFlows[i]);
                    }
                    else
                    {
                        DeactivateSensor(_sensorHubFlows[i]);
                    }
                }

                for (int i = 0; i < _fans.Length; i++)
                {
                    float maxRpm = 11.5f * ((data[149 + 2 * i] << 8) | data[148 + 2 * i]);

                    if (_fans[i] == null)
                    {
                        _fans[i] = new Sensor("Fan Channel " + i,
                                              i,
                                              SensorType.Fan,
                                              this,
                                              new[]
                                              {
                                                  new ParameterDescription("MaxRPM",
                                                                           "Maximum revolutions per minute (RPM) of the fan.",
                                                                           maxRpm)
                                              },
                                              _settings);
                    }

                    float value;
                    if ((data[136] & (1 << i)) == 0) // pwm mode
                        value = 0.02f * data[137 + i];
                    else // analog mode
                        value = 0.01f * data[141 + i];

                    _fans[i].Value = _fans[i].Parameters[0].Value * value;
                    ActivateSensor(_fans[i]);

                    _controls[i].Value = 100 * value;
                    ActivateSensor(_controls[i]);
                }
            }
            else if (data[1] == 253)
            {
                // miniNG #1
                if (_alternativeData.Length == 0)
                    _alternativeData = new byte[data.Length];

                _data.CopyTo(_alternativeData, 0);

                readMiniNG(0);
                if (data[66] == 253) // miniNG #2
                    readMiniNG(1);
            }
}

void wm_sensors::hardware::tbalancer::TBalancer::Impl::readMiniNG(unsigned int number, const DataArray& data)
{
	int offset = 1 + number * 65;

            if (data[offset + 61] != endFlag)
                return;


            for (int i = 0; i < 2; i++)
            {
                Sensor sensor = _miniNgTemperatures[number * 2 + i];
                if (data[offset + 7 + i] > 0)
                {
                    sensor.Value = 0.5f * data[offset + 7 + i] +
                                   sensor.Parameters[0].Value;

                    ActivateSensor(sensor);
                }
                else
                {
                    DeactivateSensor(sensor);
                }
            }

            for (int i = 0; i < 2; i++)
            {
                if (_miniNgFans[number * 2 + i] == null)
                    _miniNgFans[number * 2 + i] = new Sensor("miniNG #" + (number + 1) + " Fan Channel " + (i + 1), 4 + number * 2 + i, SensorType.Fan, this, _settings);

                Sensor sensor = _miniNgFans[number * 2 + i];
                sensor.Value = 20.0f * data[offset + 43 + 2 * i];
                ActivateSensor(sensor);
            }

            for (int i = 0; i < 2; i++)
            {
                Sensor sensor = _miniNgControls[number * 2 + i];
                sensor.Value = data[offset + 15 + i];
                ActivateSensor(sensor);
            }
}


wm_sensors::hardware::tbalancer::TBalancer::TBalancer(std::size_t portIndex, u8 protocolVersion)
    : impl_{std::make_unique<Impl>(portIndex, protocolVersion)}
{
}

