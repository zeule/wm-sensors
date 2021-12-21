// SPDX-License-Identifier: LGPL-3.0+

#include "./smbios.hxx"

#include <iostream>
#include <type_traits>

namespace {
	const char* unknown = "Unknown";
	const char* other = "Other";
}

std::ostream& wm_sensors::hardware::operator<<(std::ostream& os, wm_sensors::hardware::CacheAssociativity v)
{
	switch (v) {
		case CacheAssociativity::Other:
			os << other;
			break;
		case CacheAssociativity::Unknown:
			os << unknown;
			break;
		case CacheAssociativity::DirectMapped:
			os << "Direct-mapped";
			break;
		case CacheAssociativity::_2Way:
			os << "2-way";
			break;
		case CacheAssociativity::_4Way:
			os << "4-way";
			break;
		case CacheAssociativity::FullyAssociative:
			os << "Fully-associative";
			break;
		case CacheAssociativity::_8Way:
			os << "8-way";
			break;
		case CacheAssociativity::_16Way:
			os << "16-way";
			break;
		case CacheAssociativity::_12Way:
			os << "12-way";
			break;
		case CacheAssociativity::_24Way:
			os << "24-way";
			break;
		case CacheAssociativity::_32Way:
			os << "32-way";
			break;
		case CacheAssociativity::_48Way:
			os << "48-way";
			break;
		case CacheAssociativity::_64Way:
			os << "64-way";
			break;
		case CacheAssociativity::_20Way:
			os << "20-way";
			break;
	}

	return os;
}

std::ostream& wm_sensors::hardware::operator<<(std::ostream& os, wm_sensors::hardware::CacheDesignation v)
{
	switch (v) {
		case CacheDesignation::Other:
			os << other;
			break;
		case CacheDesignation::L1:
			os << "L1";
			break;
		case CacheDesignation::L2:
			os << "L2";
			break;
		case CacheDesignation::L3:
			os << "L3";
			break;
	}

	return os;
}

std::ostream& wm_sensors::hardware::operator<<(std::ostream& os, wm_sensors::hardware::ChassisSecurityStatus v)
{
	switch (v) {
		case ChassisSecurityStatus::Other:
			os << other;
			break;
		case ChassisSecurityStatus::Unknown:
			os << unknown;
			break;
		case ChassisSecurityStatus::None:
			os << "None";
			break;
		case ChassisSecurityStatus::ExternalInterfaceLockedOut:
			os << "External interface locked out";
			break;
		case ChassisSecurityStatus::ExternalInterfaceEnabled:
			os << "External interface enabled";
			break;
	}

	return os;
}

std::ostream& wm_sensors::hardware::operator<<(std::ostream& os, wm_sensors::hardware::ChassisState v)
{
	switch (v) {
		case ChassisState::Other:
			os << other;
			break;
		case ChassisState::Unknown:
			os << unknown;
			break;
		case ChassisState::Safe:
			os << "Safe";
			break;
		case ChassisState::Warning:
			os << "Warning";
			break;
		case ChassisState::Critical:
			os << "Critical";
			break;
		case ChassisState::NonRecoverable:
			os << "Non-recoverable";
			break;
	}

	return os;
}

std::ostream& wm_sensors::hardware::operator<<(std::ostream& os, wm_sensors::hardware::ChassisType val)
{
	const auto name = [](ChassisType v) {
		switch (v) {
			case ChassisType::Other:
				return other;
			case ChassisType::Unknown:
				return unknown;
			case ChassisType::Desktop:
				return "Desktop";
			case ChassisType::LowProfileDesktop:
				return "Low profile desktop";
			case ChassisType::PizzaBox:
				return "Pizza box";
			case ChassisType::MiniTower:
				return "Mini-tower";
			case ChassisType::Tower:
				return "Tower";
			case ChassisType::Portable:
				return "Portable";
			case ChassisType::Laptop:
				return "Laptop";
			case ChassisType::Notebook:
				return "Notebook";
			case ChassisType::HandHeld:
				return "Handheld";
			case ChassisType::DockingStation:
				return "Docking station";
			case ChassisType::AllInOne:
				return "All-in-one";
			case ChassisType::SubNotebook:
				return "Sub-notebook";
			case ChassisType::SpaceSaving:
				return "Space-saving";
			case ChassisType::LunchBox:
				return "Lauch box";
			case ChassisType::MainServerChassis:
				return "Main server chassis";
			case ChassisType::ExpansionChassis:
				return "Expansion chassis";
			case ChassisType::SubChassis:
				return "Sub-chassis";
			case ChassisType::BusExpansionChassis:
				return "Bus expansion chassis";
			case ChassisType::PeripheralChassis:
				return "Peripheral chassis";
			case ChassisType::RaidChassis:
				return "RAID chassis";
			case ChassisType::RackMountChassis:
				return "Rack-mount chassis";
			case ChassisType::SealedCasePc:
				return "Sealed case PC";
			case ChassisType::MultiSystemChassis:
				return "Multi-system chassis";
			case ChassisType::CompactPci:
				return "Compact PCI";
			case ChassisType::AdvancedTca:
				return "Advanced TCA";
			case ChassisType::Blade:
				return "Blade";
			case ChassisType::BladeEnclosure:
				return "Blade enclosure";
			case ChassisType::Tablet:
				return "Tablet";
			case ChassisType::Convertible:
				return "Convertible";
			case ChassisType::Detachable:
				return "Detachable";
			case ChassisType::IoTGateway:
				return "IoT gateway";
			case ChassisType::EmbeddedPc:
				return "Embedded PC";
			case ChassisType::MiniPc:
				return "Mini-PC";
			case ChassisType::StickPc:
				return "Stick-PC";
			default:
				return "Uknown";
		}
	};

	os << name(val);
	return os;
}

std::ostream& wm_sensors::hardware::operator<<(std::ostream& os, wm_sensors::hardware::ProcessorFamily val)
{
	const auto name = [](ProcessorFamily v) {
		switch (v) {
			case ProcessorFamily::Other:
				return other;
			case ProcessorFamily::Intel8086:
				return "Intel 8086";
			case ProcessorFamily::Intel80286:
				return "Intel 80286";
			case ProcessorFamily::Intel386:
				return "Intel 486";
			case ProcessorFamily::Intel486:
				return "Intel 486";
			case ProcessorFamily::Intel8087:
				return "Intel 8087";
			case ProcessorFamily::Intel80287:
				return "Intel 80287";
			case ProcessorFamily::Intel80387:
				return "Intel 80387";
			case ProcessorFamily::Intel80487:
				return "Intel 80487";
			case ProcessorFamily::IntelPentium:
				return "Intel Pentium";
			case ProcessorFamily::IntelPentiumPro:
				return "Intel Pentium Pro";
			case ProcessorFamily::IntelPentiumII:
				return "Intel Pentium II";
			case ProcessorFamily::IntelPentiumMMX:
				return "Intel Pentium MMX";
			case ProcessorFamily::IntelCeleron:
				return "Intel Celeron";
			case ProcessorFamily::IntelPentiumIIXeon:
				return "Intel Pentium II Xeon";
			case ProcessorFamily::IntelPentiumIII:
				return "Intel Pentium III";
			case ProcessorFamily::M1:
				return "M1";
			case ProcessorFamily::M2:
				return "M2";
			case ProcessorFamily::IntelCeleronM:
				return "Intel Celeron M";
			case ProcessorFamily::IntelPentium4HT:
				return "Intel Pentium  4HT";
			case ProcessorFamily::AmdDuron:
				return "AMD Duron";
			case ProcessorFamily::AmdK5:
				return "AMD K5";
			case ProcessorFamily::AmdK6:
				return "AMD K6";
			case ProcessorFamily::AmdK62:
				return "AMD K6-2";
			case ProcessorFamily::AmdK63:
				return "AMD K6-3";
			case ProcessorFamily::AmdAthlon:
				return "AMD Athlon";
			case ProcessorFamily::Amd2900:
				return "AMD 2900";
			case ProcessorFamily::AmdK62Plus:
				return "AMD K6-2+";
			case ProcessorFamily::PowerPc:
				return "PowerPC";
			case ProcessorFamily::PowerPc601:
				return "PowerPC 601";
			case ProcessorFamily::PowerPc603:
				return "PowerPC 603";
			case ProcessorFamily::PowerPc603Plus:
				return "PowerPC 603+";
			case ProcessorFamily::PowerPc604:
				return "PowerPC 604";
			case ProcessorFamily::PowerPc620:
				return "PowerPC 620";
			case ProcessorFamily::PowerPcx704:
				return "PowerPC x704";
			case ProcessorFamily::PowerPc750:
				return "PowerPC 750";
			case ProcessorFamily::IntelCoreDuo:
				return "Intel Core Duo";
			case ProcessorFamily::IntelCoreDuoMobile:
				return "Intel Core Duo Mobile";
			case ProcessorFamily::IntelCoreSoloMobile:
				return "Intel Core Solo Mobile";
			case ProcessorFamily::IntelAtom:
				return "Intel Atom";
			case ProcessorFamily::IntelCoreM:
				return "Intel Core M";
			case ProcessorFamily::IntelCoreM3:
				return "Intel Core M3";
			case ProcessorFamily::IntelCoreM5:
				return "Intel Core M5";
			case ProcessorFamily::IntelCoreM7:
				return "Intel Core M7";
			case ProcessorFamily::Alpha:
				return "Alpha";
			case ProcessorFamily::Alpha21064:
				return "Alpha 21064";
			case ProcessorFamily::Alpha21066:
				return "Alpha 21066";
			case ProcessorFamily::Alpha21164:
				return "Alpha 21164";
			case ProcessorFamily::Alpha21164Pc:
				return "Alpha 21164PC";
			case ProcessorFamily::Alpha21164a:
				return "Alpha 21164a";
			case ProcessorFamily::Alpha21264:
				return "Alpha ";
			case ProcessorFamily::Alpha21364:
				return "Alpha 21364";
			case ProcessorFamily::AmdTurionIIUltraDualCoreMobileM:
				return "AMD Turion II Ultra Dual Core Mobile M";
			case ProcessorFamily::AmdTurionDualCoreMobileM:
				return "AMD Turion Dual Core Mobile M";
			case ProcessorFamily::AmdAthlonIIDualCoreM:
				return "AMD Athlon II Dual Core M";
			case ProcessorFamily::AmdOpteron6100Series:
				return "AMD Opteron 6100 series";
			case ProcessorFamily::AmdOpteron4100Series:
				return "AMD Opteron 4100 series";
			case ProcessorFamily::AmdOpteron6200Series:
				return "AMD Opteron 6200 series";
			case ProcessorFamily::AmdOpteron4200Series:
				return "AMD Opteron 4200 series";
			case ProcessorFamily::AmdFxSeries:
				return "AMD FX-Series";
			case ProcessorFamily::Mips:
				return "MIPS";
			case ProcessorFamily::MipsR4000:
				return "MIPS 4000";
			case ProcessorFamily::MipsR4200:
				return "MIPS R4200";
			case ProcessorFamily::MipsR4400:
				return "MIPS R4400";
			case ProcessorFamily::MipsR4600:
				return "MIPS R4600";
			case ProcessorFamily::MipsR10000:
				return "MIPS R10000";
			case ProcessorFamily::AmdCSeries:
				return "AMD C-Series";
			case ProcessorFamily::AmdESeries:
				return "AMD E-Series";
			case ProcessorFamily::AmdASeries:
				return "AMD A-Series";
			case ProcessorFamily::AmdGSeries:
				return "AMD G-Series";
			case ProcessorFamily::AmdZSeries:
				return "AMD Z-Series";
			case ProcessorFamily::AmdRSeries:
				return "AMD R-Series";
			case ProcessorFamily::AmdOpteron4300Series:
				return "AMD Opteron 4300 Series";
			case ProcessorFamily::AmdOpteron6300Series:
				return "AMD Opteron 6300 Series";
			case ProcessorFamily::AmdOpteron3300Series:
				return "AMD Opteron 3300 Series";
			case ProcessorFamily::AmdFireProSeries:
				return "AMD FirePro Series";
			case ProcessorFamily::Sparc:
				return "SPARC";
			case ProcessorFamily::SuperSparc:
				return "SuperSPARC";
			case ProcessorFamily::MicroSparcII:
				return "microSPARC II";
			case ProcessorFamily::MicroSparcIIep:
				return "microSPARC IIe";
			case ProcessorFamily::UltraSparc:
				return "UltraSPARC";
			case ProcessorFamily::UltraSparcII:
				return "UltraSPARC II";
			case ProcessorFamily::UltraSparcIIi:
				return "UltraSPARC IIi";
			case ProcessorFamily::UltraSparcIII:
				return "UltraSPARC III";
			case ProcessorFamily::UltraSparcIIIi:
				return "UltraSPARC IIIi";
			case ProcessorFamily::Motorola68040:
				return "Motorola 68040";
			case ProcessorFamily::Motorola68xxx:
				return "Motorola 68xxx";
			case ProcessorFamily::Motorola68000:
				return "Motorola 68000";
			case ProcessorFamily::Motorola68010:
				return "Motorola 68010";
			case ProcessorFamily::Motorola68020:
				return "Motorola 68020";
			case ProcessorFamily::Motorola68030:
				return "Motorola 68030";
			case ProcessorFamily::AmdAthlonX4QuadCore:
				return "AMD Athlon X4 Quad Core";
			case ProcessorFamily::AmdOpteronX1000Series:
				return "AMD Opteron X1000 Series";
			case ProcessorFamily::AmdOpteronX2000Series:
				return "AMD Opteron X2000 Series";
			case ProcessorFamily::AmdOpteronASeries:
				return "AMD Opteron A Series";
			case ProcessorFamily::AmdOpteronX3000Series:
				return "AMD Opteron X3000 Series";
			case ProcessorFamily::AmdZen:
				return "AMD Zen";
			case ProcessorFamily::Hobbit:
				return "Hobbit";
			case ProcessorFamily::CrusoeTm5000:
				return "Crusoe TM5000";
			case ProcessorFamily::CrusoeTm3000:
				return "Crusoe TM3000";
			case ProcessorFamily::EfficeonTm8000:
				return "Efficeon TM8000";
			case ProcessorFamily::Weitek:
				return "Weitek";
			case ProcessorFamily::IntelItanium:
				return "Intel Itanium";
			case ProcessorFamily::AmdAthlon64:
				return "AMD Athlon 64";
			case ProcessorFamily::AmdOpteron:
				return "AMD Opteron";
			case ProcessorFamily::AmdSempron:
				return "AMD Semptron";
			case ProcessorFamily::AmdTurion64Mobile:
				return "AMD Turion 64 Mobile";
			case ProcessorFamily::AmdOpteronDualCore:
				return "Dual-Core AMD Opteron";
			case ProcessorFamily::AmdAthlon64X2DualCore:
				return "AMD Athlon 64 X2 Dual-Core";
			case ProcessorFamily::AmdTurion64X2Mobile:
				return "AMD Turion 64 X2 Mobile";
			case ProcessorFamily::AmdOpteronQuadCore:
				return "Quad-Core AMD Opteron";
			case ProcessorFamily::AmdOpteronThirdGen:
				return "Third-Generation AMD Opteron";
			case ProcessorFamily::AmdPhenomFXQuadCore:
				return "AMD Phenom FX Quad-Core";
			case ProcessorFamily::AmdPhenomX4QuadCore:
				return "AMD Phenom X4 Quad-Core";
			case ProcessorFamily::AmdPhenomX2DualCore:
				return "AMD Phenom X2 Dual-Core";
			case ProcessorFamily::AmdAthlonX2DualCore:
				return "AMD Athlon X2 Dual-Core";
			case ProcessorFamily::PaRisc:
				return "PA-RISC";
			case ProcessorFamily::PaRisc8500:
				return "PA-RISC 8500";
			case ProcessorFamily::PaRisc8000:
				return "PA-RISC 8000";
			case ProcessorFamily::PaRisc7300LC:
				return "PA-RISC 7300LC";
			case ProcessorFamily::PaRisc7200:
				return "PA-RISC 7200";
			case ProcessorFamily::PaRisc7100LC:
				return "PA-RISC 7100LC";
			case ProcessorFamily::PaRisc7100:
				return "PA-RISC 7100";
			case ProcessorFamily::V30:
				return "V30";
			case ProcessorFamily::IntelXeon3200QuadCoreSeries:
				return "Quad-Core Intel Xeon 3200 Series";
			case ProcessorFamily::IntelXeon3000DualCoreSeries:
				return "Dual-Core Intel Xeon 300 Series";
			case ProcessorFamily::IntelXeon5300QuadCoreSeries:
				return "Quad-Core Intel Xeon 5300 Series";
			case ProcessorFamily::IntelXeon5100DualCoreSeries:
				return "Dual-Core Intel Xeon 5100 Series";
			case ProcessorFamily::IntelXeon5000DualCoreSeries:
				return "Dual-Core Intel Xeon 5000 Series";
			case ProcessorFamily::IntelXeonLVDualCore:
				return "Dual-Core Intel Xeon LV";
			case ProcessorFamily::IntelXeonULVDualCore:
				return "Dual-Core Intel Xeon ULV";
			case ProcessorFamily::IntelXeon7100Series:
				return "Intel Xeon 7100 Series";
			case ProcessorFamily::IntelXeon5400Series:
				return "Intel Xeon 5400 Series";
			case ProcessorFamily::IntelXeonQuadCore:
				return "Quad-Core Intel Xeon ";
			case ProcessorFamily::IntelXeon5200DualCoreSeries:
				return "Dual-Core Intel Xeon 5200 Series";
			case ProcessorFamily::IntelXeon7200DualCoreSeries:
				return "Dual-Core Intel Xeon 7200 Series";
			case ProcessorFamily::IntelXeon7300QuadCoreSeries:
				return "Quad-Core Intel Xeon 7300 Series";
			case ProcessorFamily::IntelXeon7400QuadCoreSeries:
				return "Quad-Core Intel Xeon 7400 Series";
			case ProcessorFamily::IntelXeon7400MultiCoreSeries:
				return "Multi-Core Intel Xeon 7400 Series";
			case ProcessorFamily::IntelPentiumIIIXeon:
				return "Intel Pentium III Xeon";
			case ProcessorFamily::IntelPentiumIIISpeedStep:
				return "Intel Pentium III Processor with Intel SpeedStep Technology";
			case ProcessorFamily::IntelPentium4:
				return "Intel Pentium 4";
			case ProcessorFamily::IntelXeon:
				return "Intel Xeon";
			case ProcessorFamily::As400:
				return "AS 400";
			case ProcessorFamily::IntelXeonMP:
				return "Intel Xeon MP";
			case ProcessorFamily::AmdAthlonXP:
				return "AMD Athlon XP";
			case ProcessorFamily::AmdAthlonMP:
				return "AMD Athlon MP";
			case ProcessorFamily::IntelItanium2:
				return "Intel Itanium 2";
			case ProcessorFamily::IntelPentiumM:
				return "Intel Pentium M";
			case ProcessorFamily::IntelCeleronD:
				return "Intel Celeron D";
			case ProcessorFamily::IntelPentiumD:
				return "Intel Pentium D";
			case ProcessorFamily::IntelPentiumExtreme:
				return "Intel Pentium Extreme";
			case ProcessorFamily::IntelCoreSolo:
				return "Intel Core Solo";
			case ProcessorFamily::IntelCore2Duo:
				return "Intel Core 2 Duo";
			case ProcessorFamily::IntelCore2Solo:
				return "Intel Core 2 Solo";
			case ProcessorFamily::IntelCore2Extreme:
				return "Intel Core 2 Extreme";
			case ProcessorFamily::IntelCore2Quad:
				return "Intel Core 2 Quad";
			case ProcessorFamily::IntelCore2ExtremeMobile:
				return "Intel Core 2 Extreme Mobile";
			case ProcessorFamily::IntelCore2DuoMobile:
				return "Intel Core 2 Duo Mobile";
			case ProcessorFamily::IntelCore2SoloMobile:
				return "Intel Core 2 Solo Mobile";
			case ProcessorFamily::IntelCoreI7:
				return "Intel Core i7";
			case ProcessorFamily::IntelCeleronDualCore:
				return "Dual-Core Intel Celeron";
			case ProcessorFamily::Ibm390:
				return "IBM 390";
			case ProcessorFamily::PowerPcG4:
				return "G4";
			case ProcessorFamily::PowerPcG5:
				return "G5";
			case ProcessorFamily::Esa390G6:
				return "ESA/390 G4";
			case ProcessorFamily::ZArchitecture:
				return "z/Architecture base";
			case ProcessorFamily::IntelCoreI5:
				return "Intel Core i5";
			case ProcessorFamily::IntelCoreI3:
				return "Intel Core i3";
			case ProcessorFamily::IntelCoreI9:
				return "Intel Core i9";
			case ProcessorFamily::ViaC7M:
				return "VIA C7-M";
			case ProcessorFamily::ViaC7D:
				return "VIA C7-D";
			case ProcessorFamily::ViaC7:
				return "VIA C7";
			case ProcessorFamily::ViaEden:
				return "VIA Eden";
			case ProcessorFamily::IntelXeonMultiCore:
				return "Multi-Core Intel Xeon";
			case ProcessorFamily::IntelXeon3xxxDualCoreSeries:
				return "Dual-Core Intel Xeon 3xxx Series";
			case ProcessorFamily::IntelXeon3xxxQuadCoreSeries:
				return "Quad-Core Intel Xeon 3xxx Series";
			case ProcessorFamily::ViaNano:
				return "VIA Nano";
			case ProcessorFamily::IntelXeon5xxxDualCoreSeries:
				return "Dual-Core Intel Xeon 5xxx Series";
			case ProcessorFamily::IntelXeon5xxxQuadCoreSeries:
				return "Quad-Core Intel Xeon 5xxx Series";
			case ProcessorFamily::IntelXeon7xxxDualCoreSeries:
				return "Dual-Core Intel Xeon 7xxx Series";
			case ProcessorFamily::IntelXeon7xxxQuadCoreSeries:
				return "Quad-Core Intel Xeon 7xxx Series";
			case ProcessorFamily::IntelXeon7xxxMultiCoreSeries:
				return "Multi-Core Intel Xeon 7xxx Series";
			case ProcessorFamily::IntelXeon3400MultiCoreSeries:
				return "Multi-Core Intel Xeon 3400 Series";
			case ProcessorFamily::AmdOpteron3000Series:
				return "AMD Opteron 3000 Series";
			case ProcessorFamily::AmdSempronII:
				return "AMD Sempron II";
			case ProcessorFamily::AmdOpteronQuadCoreEmbedded:
				return "Embedded AMD OpteronTM Quad-Core";
			case ProcessorFamily::AmdPhenomTripleCore:
				return "AMD PhenomTM Triple-Core";
			case ProcessorFamily::AmdTurionUltraDualCoreMobile:
				return "AMD TurionTM Ultra Dual-Core Mobile";
			case ProcessorFamily::AmdTurionDualCoreMobile:
				return "AMD TurionTM Dual-Core Mobile";
			case ProcessorFamily::AmdAthlonDualCore:
				return "AMD AthlonTM Dual-Core";
			case ProcessorFamily::AmdSempronSI:
				return "AMD SempronTM SI";
			case ProcessorFamily::AmdPhenomII:
				return "AMD PhenomTM II";
			case ProcessorFamily::AmdAthlonII:
				return "AMD AthlonTM II";
			case ProcessorFamily::AmdOpteronSixCore:
				return "Six-Core AMD OpteronTM";
			case ProcessorFamily::AmdSempronM:
				return "AMD SempronTM M";
			case ProcessorFamily::IntelI860:
				return "i860";
			case ProcessorFamily::IntelI960:
				return "i960";
			case ProcessorFamily::ArmV7:
				return "ARMv7";
			case ProcessorFamily::ArmV8:
				return "ARMv8";
			case ProcessorFamily::HitachiSh3:
				return "Hitachi SH-3";
			case ProcessorFamily::HitachiSh4:
				return "Hitachi SH-4";
			case ProcessorFamily::Arm:
				return "ARM";
			case ProcessorFamily::StrongArm:
				return "StrongARM";
			case ProcessorFamily::_686:
				return "6x86";
			case ProcessorFamily::MediaGX:
				return "MediaGX";
			case ProcessorFamily::MII:
				return "MII";
			case ProcessorFamily::WinChip:
				return "WinChip";
			case ProcessorFamily::Dsp:
				return "DSP";
			case ProcessorFamily::VideoProcessor:
				return "Video Processor";
			case ProcessorFamily::RiscVRV32:
				return "RISC-V RV32";
			case ProcessorFamily::RiscVRV64:
				return "RISC-V RV64";
			case ProcessorFamily::RiscVRV128:
				return "RISC-V RV128";
			default:
				return "Unknown processor model";
		}
	};

	os << name(val);
	return os;
}

std::ostream& wm_sensors::hardware::operator<<(std::ostream& os, wm_sensors::hardware::ProcessorSocket val)
{
	const auto name = [](ProcessorSocket v) {
		switch (v) {
			case ProcessorSocket::Other:
				return "Other";
			case ProcessorSocket::Unknown:
				return "Unknown";
			case ProcessorSocket::DaughterBoard:
				return "DaughterBoard";
			case ProcessorSocket::ZifSocket:
				return "ZIF";
			case ProcessorSocket::PiggyBack:
				return "PiggyBack";
			case ProcessorSocket::None:
				return "None";
			case ProcessorSocket::LifSocket:
				return "LIF";
			case ProcessorSocket::Zif423:
				return "ZIF 423";
			case ProcessorSocket::A:
				return "A";
			case ProcessorSocket::Zif478:
				return "ZIF 478";
			case ProcessorSocket::Zif754:
				return "ZIF 754";
			case ProcessorSocket::Zif940:
				return "ZIF 940";
			case ProcessorSocket::Zif939:
				return "ZIF 939";
			case ProcessorSocket::MPga604:
				return "M-PGA 604";
			case ProcessorSocket::Lga771:
				return "LGA 771";
			case ProcessorSocket::Lga775:
				return "LGA 775";
			case ProcessorSocket::S1:
				return "S1";
			case ProcessorSocket::AM2:
				return "AM2";
			case ProcessorSocket::F:
				return "F";
			case ProcessorSocket::Lga1366:
				return "LGA 1366";
			case ProcessorSocket::G34:
				return "G34";
			case ProcessorSocket::AM3:
				return "AM3";
			case ProcessorSocket::C32:
				return "C32";
			case ProcessorSocket::Lga1156:
				return "LGA 1156";
			case ProcessorSocket::Lga1567:
				return "LGA 1567";
			case ProcessorSocket::Pga988A:
				return "PGA 988A";
			case ProcessorSocket::Bga1288:
				return "BGA 1288";
			case ProcessorSocket::RPga088B:
				return "RPGA 088B";
			case ProcessorSocket::Bga1023:
				return "BGA 1023";
			case ProcessorSocket::Bga1224:
				return "BGA 1224";
			case ProcessorSocket::Lga1155:
				return "LGA 1155";
			case ProcessorSocket::Lga1356:
				return "LGA 1356";
			case ProcessorSocket::Lga2011:
				return "LGA 2011";
			case ProcessorSocket::FS1:
				return "FS1";
			case ProcessorSocket::FS2:
				return "FS2";
			case ProcessorSocket::FM1:
				return "FM1";
			case ProcessorSocket::FM2:
				return "FM2";
			case ProcessorSocket::Lga20113:
				return "LGA 20113";
			case ProcessorSocket::Lga13563:
				return "LGA 13563";
			case ProcessorSocket::Lga1150:
				return "LGA 1150";
			case ProcessorSocket::Bga1168:
				return "BGA 1168";
			case ProcessorSocket::Bga1234:
				return "BGA 1234";
			case ProcessorSocket::Bga1364:
				return "BGA 1364";
			case ProcessorSocket::AM4:
				return "AM4";
			case ProcessorSocket::Lga1151:
				return "LGA 1151";
			case ProcessorSocket::Bga1356:
				return "BGA 1356";
			case ProcessorSocket::Bga1440:
				return "BGA 1440";
			case ProcessorSocket::Bga1515:
				return "BGA 1515";
			case ProcessorSocket::Lga36471:
				return "LGA 36471";
			case ProcessorSocket::SP3:
				return "SP3";
			case ProcessorSocket::SP3R2:
				return "SP3R2";
			case ProcessorSocket::Lga2066:
				return "LGA 2066";
			case ProcessorSocket::Bga1510:
				return "BGA 1510";
			case ProcessorSocket::Bga1528:
				return "BGA 1528";
			case ProcessorSocket::Lga4189:
				return "LGA 4189";
			default:
				return "Unknown";
		}
	};

	os << name(val);
	return os;
}

std::ostream& wm_sensors::hardware::operator<<(std::ostream& os, wm_sensors::hardware::ProcessorType v)
{
	switch (v) {
		case ProcessorType::CentralProcessor:
			os << "Central processor";
			break;
		case ProcessorType::DspProcessor:
			os << "DSP processor";
			break;
		case ProcessorType::MathProcessor:
			os << "Math processor";
			break;
		case ProcessorType::Other:
			os << "Other";
			break;
		case ProcessorType::Unknown:
			os << "Unknown";
			break;
		case ProcessorType::VideoProcessor:
			os << "Video processor";
			break;
	}

	return os;
}

std::ostream& wm_sensors::hardware::operator<<(std::ostream& os, wm_sensors::hardware::SystemWakeUp v)
{
	switch (v) {
		case SystemWakeUp::AcPowerRestored:
			os << "AC power restored";
			break;
		case SystemWakeUp::ApmTimer:
			os << "APM timer";
			break;
		case SystemWakeUp::LanRemote:
			os << "LAN remote";
			break;
		case SystemWakeUp::ModemRing:
			os << "Modem ring";
			break;
		case SystemWakeUp::Other:
			os << "Other";
			break;
		case SystemWakeUp::PciPme:
			os << "PCI PME";
			break;
		case SystemWakeUp::PowerSwitch:
			os << "Power switch";
			break;
		case SystemWakeUp::Reserved:
			os << "Reserved";
			break;
		case SystemWakeUp::Unknown:
			os << "Unknown";
			break;
	}

	return os;
}
