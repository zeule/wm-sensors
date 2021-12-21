// SPDX-License-Identifier: LGPL-3.0+

#include "./lpc_io.hxx"

#include "../../impl/ring0.hxx"
#include "./identification.hxx"
#include "./port.hxx"
#include "./superio/f718xx.hxx"
#include "./superio/it87xx.hxx"
#include "./superio/nct677x.hxx"
#include "./superio/w836xx.hxx"

#include <chrono>
#include <thread>

namespace {
	const wm_sensors::hardware::motherboard::lpc::SingleBankAddress registerPorts[] = {
		{0, {0x2e, 0x2f}},
		{0, {0x4e, 0x4f}},
	};

	using wm_sensors::u16;
	using wm_sensors::u8;

	const u8 baseAddressRegister = 0x60;
	const u8 chipIdRegister = 0x20;
	const u8 chipRevisionRegister = 0x21;

	const u8 F71858HardwareMonitorLdn = 0x02;
	const u8 fintekHardwareMonitorLdn = 0x04;
	const u8 IT87EnvironmentControllerLdn = 0x04;
	const u8 IT8705GpioLdn = 0x05;
	const u8 IT87xxGpioLdn = 0x07;
	const u8 winbondNuvotonHardwareMonitorLdn = 0x0b;
	const u16 fintekVendorId = 0x1934;

	const u8 fintekVendorIdRegister = 0x23;
	const u8 IT87ChipVersionRegister = 0x22;
} // namespace

wm_sensors::hardware::motherboard::lpc::LpcIo::LpcIo(MotherboardId board)
{
	// 	if (!Ring0.IsOpen)
	//                 return;
	impl::GlobalMutexTryLock lock{impl::GlobalMutex::ISABus, std::chrono::milliseconds(100)};
	if (lock.failed()) return;
	detect(board);
}

wm_sensors::hardware::motherboard::lpc::LpcIo::~LpcIo() = default;

void wm_sensors::hardware::motherboard::lpc::LpcIo::detect(MotherboardId board)
{
	for (const auto& p: registerPorts) {
		const SingleBankPort port{p};

		if (detectWinbondFintek(board, port)) continue;
		if (detectIT87(board, port)) continue;
		if (detectSmsc(board, port)) continue;
	}
}

void wm_sensors::hardware::motherboard::lpc::LpcIo::reportUnknownChip(const SingleBankPort& /*port*/, std::string_view /*type*/, int /*chip*/)
{
	// 	_report.Append("Chip ID: Unknown ");
	// 	_report.Append(type);
	// 	_report.Append(" with ID 0x");
	// 	_report.Append(chip.ToString("X", CultureInfo.InvariantCulture));
	// 	_report.Append(" at 0x");
	// 	_report.Append(port.RegisterPort.ToString("X", CultureInfo.InvariantCulture));
	// 	_report.Append("/0x");
	// 	_report.AppendLine(port.ValuePort.ToString("X", CultureInfo.InvariantCulture));
	// 	_report.AppendLine();
}


bool wm_sensors::hardware::motherboard::lpc::LpcIo::detectIT87(MotherboardId board, const SingleBankPort& port)
{
	using namespace std::chrono_literals;

	// IT87XX can enter only on port 0x2e
	// IT8792 using 0x4e
	if (port.regs().indexRegOffset != 0x2e && port.regs().indexRegOffset != 0x4e) return false;

	IT87EnterExit guard{port};

	u16 chipId = port.readWord(chipIdRegister);
	Chip chip;
	switch (chipId) {
		case 0x8620: chip = Chip::IT8620E; break;
		case 0x8628: chip = Chip::IT8628E; break;
		case 0x8631: chip = Chip::IT8631E; break;
		case 0x8665: chip = Chip::IT8665E; break;
		case 0x8655: chip = Chip::IT8655E; break;
		case 0x8686: chip = Chip::IT8686E; break;
		case 0x8688: chip = Chip::IT8688E; break;
		case 0x8689: chip = Chip::IT8689E; break;
		case 0x8705: chip = Chip::IT8705F; break;
		case 0x8712: chip = Chip::IT8712F; break;
		case 0x8716: chip = Chip::IT8716F; break;
		case 0x8718: chip = Chip::IT8718F; break;
		case 0x8720: chip = Chip::IT8720F; break;
		case 0x8721: chip = Chip::IT8721F; break;
		case 0x8726: chip = Chip::IT8726F; break;
		case 0x8728: chip = Chip::IT8728F; break;
		case 0x8771: chip = Chip::IT8771E; break;
		case 0x8772: chip = Chip::IT8772E; break;
		case 0x8733: chip = Chip::IT879XE; break;
		default: chip = Chip::unknown; break;
	}

	if (chip == Chip::unknown) {
		if (chipId != 0 && chipId != 0xffff) { reportUnknownChip(port, "ITE", chipId); }
	} else {
		port.select(IT87EnvironmentControllerLdn);

		u16 address = port.readWord(baseAddressRegister);
		std::this_thread::sleep_for(1ms);
		u16 verify = port.readWord(baseAddressRegister);

		u8 version = static_cast<u8>(port.readByte(IT87ChipVersionRegister) & 0x0f);

		u16 gpioAddress;
		u16 gpioVerify;

		if (chip == Chip::IT8705F) {
			port.select(IT8705GpioLdn);
			gpioAddress = port.readWord(baseAddressRegister);
			std::this_thread::sleep_for(1ms);
			gpioVerify = port.readWord(baseAddressRegister);
		} else {
			port.select(IT87xxGpioLdn);
			gpioAddress = port.readWord(baseAddressRegister + 2);
			std::this_thread::sleep_for(1ms);
			gpioVerify = port.readWord(baseAddressRegister + 2);
		}

		if (address != verify || address < 0x100 || (address & 0xF007) != 0) {
			// 			_report.Append("Chip ID: 0x");
			// 			_report.AppendLine(chip.ToString("X"));
			// 			_report.Append("Error: Invalid address 0x");
			// 			_report.AppendLine(address.ToString("X", CultureInfo.InvariantCulture));
			// 			_report.AppendLine();

			return false;
		}

		if (gpioAddress != gpioVerify || gpioAddress < 0x100 || (gpioAddress & 0xF007) != 0) {
			// 			_report.Append("Chip ID: 0x");
			// 			_report.AppendLine(chip.ToString("X"));
			// 			_report.Append("Error: Invalid GPIO address 0x");
			// 			_report.AppendLine(gpioAddress.ToString("X", CultureInfo.InvariantCulture));
			// 			_report.AppendLine();

			return false;
		}

		superioChips_.push_back(std::make_unique<superio::IT87xx>(board, chip, address, gpioAddress, version));
		return true;
	}

	return false;
}

bool wm_sensors::hardware::motherboard::lpc::LpcIo::detectSmsc(MotherboardId /*board*/, const SingleBankPort& port)
{
	SmscEnterExit guard{port};

	u16 chipId = port.readWord(chipIdRegister);
	Chip chip = Chip::unknown;
#if 0
	switch (chipId) {
		default: chip = Chip::unknown; break;
	}
#endif

	if (chip == Chip::unknown) {
		if (chipId != 0 && chipId != 0xffff) {
			reportUnknownChip(port, "SMSC", chipId);
		}
	} else {
		return true;
	}

	return false;
}

bool wm_sensors::hardware::motherboard::lpc::LpcIo::detectWinbondFintek(MotherboardId board, const SingleBankPort& port)
{
	using namespace std::chrono_literals;

	WinbondNuvotonFintekEnterExit guard{port};

	u8 logicalDeviceNumber = 0;
	u8 id = port.readByte(chipIdRegister);
	u8 revision = port.readByte(chipRevisionRegister);
	Chip chip = Chip::unknown;


	switch (id) {
		case 0x05: {
			switch (revision) {
				case 0x07: {
					chip = Chip::F71858;
					logicalDeviceNumber = F71858HardwareMonitorLdn;
					break;
				}
				case 0x41: {
					chip = Chip::F71882;
					logicalDeviceNumber = fintekHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0x06: {
			switch (revision) {
				case 0x01: {
					chip = Chip::F71862;
					logicalDeviceNumber = fintekHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0x07: {
			switch (revision) {
				case 0x23: {
					chip = Chip::F71889F;
					logicalDeviceNumber = fintekHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0x08: {
			switch (revision) {
				case 0x14: {
					chip = Chip::F71869;
					logicalDeviceNumber = fintekHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0x09: {
			switch (revision) {
				case 0x01: {
					chip = Chip::F71808E;
					logicalDeviceNumber = fintekHardwareMonitorLdn;
					break;
				}
				case 0x09: {
					chip = Chip::F71889ED;
					logicalDeviceNumber = fintekHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0x10: {
			switch (revision) {
				case 0x05: {
					chip = Chip::F71889AD;
					logicalDeviceNumber = fintekHardwareMonitorLdn;
					break;
				}
				case 0x07: {
					chip = Chip::F71869A;
					logicalDeviceNumber = fintekHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0x11: {
			switch (revision) {
				case 0x06: {
					chip = Chip::F71878AD;
					logicalDeviceNumber = fintekHardwareMonitorLdn;
					break;
				}
				case 0x18: {
					chip = Chip::F71811;
					logicalDeviceNumber = fintekHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0x52: {
			switch (revision) {
				case 0x17:
				case 0x3A:
				case 0x41: {
					chip = Chip::W83627HF;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0x82: {
			switch (revision & 0xF0) {
				case 0x80: {
					chip = Chip::W83627THF;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0x85: {
			switch (revision) {
				case 0x41: {
					chip = Chip::W83687THF;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0x88: {
			switch (revision & 0xF0) {
				case 0x50:
				case 0x60: {
					chip = Chip::W83627EHF;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0xA0: {
			switch (revision & 0xF0) {
				case 0x20: {
					chip = Chip::W83627DHG;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0xA5: {
			switch (revision & 0xF0) {
				case 0x10: {
					chip = Chip::W83667HG;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0xB0: {
			switch (revision & 0xF0) {
				case 0x70: {
					chip = Chip::W83627DHGP;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0xB3: {
			switch (revision & 0xF0) {
				case 0x50: {
					chip = Chip::W83667HGB;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0xB4: {
			switch (revision & 0xF0) {
				case 0x70: {
					chip = Chip::NCT6771F;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0xC3: {
			switch (revision & 0xF0) {
				case 0x30: {
					chip = Chip::NCT6776F;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0xC4: {
			switch (revision & 0xF0) {
				case 0x50: {
					chip = Chip::NCT610XD;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0xC5: {
			switch (revision & 0xF0) {
				case 0x60: {
					chip = Chip::NCT6779D;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0xC8: {
			switch (revision) {
				case 0x03: {
					chip = Chip::NCT6791D;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0xC9: {
			switch (revision) {
				case 0x11: {
					chip = Chip::NCT6792D;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
				case 0x13: {
					chip = Chip::NCT6792DA;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0xD1: {
			switch (revision) {
				case 0x21: {
					chip = Chip::NCT6793D;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0xD3: {
			switch (revision) {
				case 0x52: {
					chip = Chip::NCT6795D;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0xD4: {
			switch (revision) {
				case 0x23: {
					chip = Chip::NCT6796D;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
				case 0x2A: {
					chip = Chip::NCT6796DR;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
				case 0x51: {
					chip = Chip::NCT6797D;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
				case 0x2B: {
					chip = Chip::NCT6798D;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 0xD5: {
			switch (revision) {
				case 0x92: {
					chip = Chip::NCT6687D;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
				}
			}

			break;
		}
		case 199: {
			switch (revision) {
				case 50:
					chip = Chip::NCT6683D;
					logicalDeviceNumber = winbondNuvotonHardwareMonitorLdn;
					break;
			}
			break;
		}
	}

	if (chip == Chip::unknown) {
		if (id != 0 && id != 0xff) {
			reportUnknownChip(port, "Winbond/Nuvoton/Fintek", (id << 8) | revision);
		}
	} else {
		port.select(logicalDeviceNumber);
		u16 address = port.readWord(baseAddressRegister);
		std::this_thread::sleep_for(1ms);
		u16 verify = port.readWord(baseAddressRegister);

		u16 vendorId = port.readWord(fintekVendorIdRegister);

		// disable the hardware monitor i/o space lock on NCT679XD chips
		if (address == verify && (chip == Chip::NCT6791D || chip == Chip::NCT6792D || chip == Chip::NCT6792DA ||
		                          chip == Chip::NCT6793D || chip == Chip::NCT6795D || chip == Chip::NCT6796D ||
		                          chip == Chip::NCT6796DR || chip == Chip::NCT6798D || chip == Chip::NCT6797D)) {
			nuvotonDisableIOSpaceLock(port);
		}

		if (address != verify) {
// 			_report.Append("Chip ID: 0x");
// 			_report.AppendLine(chip.ToString("X"));
// 			_report.Append("Chip revision: 0x");
// 			_report.AppendLine(revision.ToString("X", CultureInfo.InvariantCulture));
// 			_report.AppendLine("Error: Address verification failed");
// 			_report.AppendLine();

			return false;
		}

		// some Fintek chips have address register offset 0x05 added already
		if ((address & 0x07) == 0x05) address &= 0xFFF8;

		if (address < 0x100 || (address & 0xF007) != 0) {
// 			_report.Append("Chip ID: 0x");
// 			_report.AppendLine(chip.ToString("X"));
// 			_report.Append("Chip revision: 0x");
// 			_report.AppendLine(revision.ToString("X", CultureInfo.InvariantCulture));
// 			_report.Append("Error: Invalid address 0x");
// 			_report.AppendLine(address.ToString("X", CultureInfo.InvariantCulture));
// 			_report.AppendLine();

			return false;
		}

		switch (chip) {
			case Chip::W83627DHG:
			case Chip::W83627DHGP:
			case Chip::W83627EHF:
			case Chip::W83627HF:
			case Chip::W83627THF:
			case Chip::W83667HG:
			case Chip::W83667HGB:
			case Chip::W83687THF: {
				superioChips_.push_back(std::make_unique<superio::W836xx>(board, chip, revision, address));
				break;
			}
			case Chip::NCT610XD:
			case Chip::NCT6771F:
			case Chip::NCT6776F:
			case Chip::NCT6779D:
			case Chip::NCT6791D:
			case Chip::NCT6792D:
			case Chip::NCT6792DA:
			case Chip::NCT6793D:
			case Chip::NCT6795D:
			case Chip::NCT6796D:
			case Chip::NCT6796DR:
			case Chip::NCT6797D:
			case Chip::NCT6798D:
			case Chip::NCT6687D:
			case Chip::NCT6683D: {
				superioChips_.push_back(std::make_unique<superio::Nct67xx>(board, chip, revision, address, port));
				break;
			}
			case Chip::F71858:
			case Chip::F71862:
			case Chip::F71869:
			case Chip::F71878AD:
			case Chip::F71869A:
			case Chip::F71882:
			case Chip::F71889AD:
			case Chip::F71889ED:
			case Chip::F71889F:
			case Chip::F71808E: {
				if (vendorId != fintekVendorId) {
// 					_report.Append("Chip ID: 0x");
// 					_report.AppendLine(chip.ToString("X"));
// 					_report.Append("Chip revision: 0x");
// 					_report.AppendLine(revision.ToString("X", CultureInfo.InvariantCulture));
// 					_report.Append("Error: Invalid vendor ID 0x");
// 					_report.AppendLine(vendorId.ToString("X", CultureInfo.InvariantCulture));
// 					_report.AppendLine();

					return false;
				}

				superioChips_.push_back(std::make_unique<superio::F718xx>(board, chip, address));
				break;
			}
			default:
				break;
		}

		return true;
	}

	return false;
}
