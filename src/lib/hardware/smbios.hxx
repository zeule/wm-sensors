// SPDX-License-Identifier: LGPL-3.0+
#ifndef WM_SENSORS_LIB_HARDWARE_SMBIOS_HXX
#define WM_SENSORS_LIB_HARDWARE_SMBIOS_HXX

#include <concepts>
#include <chrono>
#include <iosfwd>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace wm_sensors::hardware {
	/**
	 * @brief Chassis security status
	 * based on DMTF SMBIOS Reference Specification v.3.3.0, Chapter 7.4.3
	 * @see href="https://www.dmtf.org/dsp/DSP0134"
	 */
	enum class ChassisSecurityStatus: std::uint8_t
	{
		Other = 1,
		Unknown,
		None,
		ExternalInterfaceLockedOut,
		ExternalInterfaceEnabled
	};

	std::ostream& operator<<(std::ostream& os, ChassisSecurityStatus css);

	/**
	 * @brief Chassis state
	 * based on DMTF SMBIOS Reference Specification v.3.3.0, Chapter 7.4.2
	 * @see https://www.dmtf.org/dsp/DSP0134
	 */
	enum class ChassisState: std::uint8_t
	{
		Other = 1,
		Unknown,
		Safe,
		Warning,
		Critical,
		NonRecoverable
	};

	std::ostream& operator<<(std::ostream& os, ChassisState v);

	/**
	 * @brief Chassis type
	 * based on DMTF SMBIOS Reference Specification v.3.3.0, Chapter 7.4.1
	 * @see <see href="https://www.dmtf.org/dsp/DSP0134">
	 */
	enum class ChassisType
	{
		Other = 1,
		Unknown,
		Desktop,
		LowProfileDesktop,
		PizzaBox,
		MiniTower,
		Tower,
		Portable,
		Laptop,
		Notebook,
		HandHeld,
		DockingStation,
		AllInOne,
		SubNotebook,
		SpaceSaving,
		LunchBox,
		MainServerChassis,
		ExpansionChassis,
		SubChassis,
		BusExpansionChassis,
		PeripheralChassis,
		RaidChassis,
		RackMountChassis,
		SealedCasePc,
		MultiSystemChassis,
		CompactPci,
		AdvancedTca,
		Blade,
		BladeEnclosure,
		Tablet,
		Convertible,
		Detachable,
		IoTGateway,
		EmbeddedPc,
		MiniPc,
		StickPc
	};

	std::ostream& operator<<(std::ostream& os, ChassisType v);

	/// Processor family based on <see href="https://www.dmtf.org/dsp/DSP0134">DMTF SMBIOS Reference Specification
	/// v.3.3.0, Chapter 7.5.2</see>.
	enum class ProcessorFamily: std::uint16_t
	{
		Other = 1,
		Intel8086 = 3,
		Intel80286 = 4,
		Intel386,
		Intel486,
		Intel8087,
		Intel80287,
		Intel80387,
		Intel80487,
		IntelPentium,
		IntelPentiumPro,
		IntelPentiumII,
		IntelPentiumMMX,
		IntelCeleron,
		IntelPentiumIIXeon,
		IntelPentiumIII,
		M1,
		M2,
		IntelCeleronM,
		IntelPentium4HT,
		AmdDuron = 24,
		AmdK5,
		AmdK6,
		AmdK62,
		AmdK63,
		AmdAthlon,
		Amd2900,
		AmdK62Plus,
		PowerPc,
		PowerPc601,
		PowerPc603,
		PowerPc603Plus,
		PowerPc604,
		PowerPc620,
		PowerPcx704,
		PowerPc750,
		IntelCoreDuo,
		IntelCoreDuoMobile,
		IntelCoreSoloMobile,
		IntelAtom,
		IntelCoreM,
		IntelCoreM3,
		IntelCoreM5,
		IntelCoreM7,
		Alpha,
		Alpha21064,
		Alpha21066,
		Alpha21164,
		Alpha21164Pc,
		Alpha21164a,
		Alpha21264,
		Alpha21364,
		AmdTurionIIUltraDualCoreMobileM,
		AmdTurionDualCoreMobileM,
		AmdAthlonIIDualCoreM,
		AmdOpteron6100Series,
		AmdOpteron4100Series,
		AmdOpteron6200Series,
		AmdOpteron4200Series,
		AmdFxSeries,
		Mips,
		MipsR4000,
		MipsR4200,
		MipsR4400,
		MipsR4600,
		MipsR10000,
		AmdCSeries,
		AmdESeries,
		AmdASeries,
		AmdGSeries,
		AmdZSeries,
		AmdRSeries,
		AmdOpteron4300Series,
		AmdOpteron6300Series,
		AmdOpteron3300Series,
		AmdFireProSeries,
		Sparc,
		SuperSparc,
		MicroSparcII,
		MicroSparcIIep,
		UltraSparc,
		UltraSparcII,
		UltraSparcIIi,
		UltraSparcIII,
		UltraSparcIIIi,
		Motorola68040 = 96,
		Motorola68xxx,
		Motorola68000,
		Motorola68010,
		Motorola68020,
		Motorola68030,
		AmdAthlonX4QuadCore,
		AmdOpteronX1000Series,
		AmdOpteronX2000Series,
		AmdOpteronASeries,
		AmdOpteronX3000Series,
		AmdZen,
		Hobbit = 112,
		CrusoeTm5000 = 120,
		CrusoeTm3000,
		EfficeonTm8000,
		Weitek = 128,
		IntelItanium = 130,
		AmdAthlon64,
		AmdOpteron,
		AmdSempron,
		AmdTurion64Mobile,
		AmdOpteronDualCore,
		AmdAthlon64X2DualCore,
		AmdTurion64X2Mobile,
		AmdOpteronQuadCore,
		AmdOpteronThirdGen,
		AmdPhenomFXQuadCore,
		AmdPhenomX4QuadCore,
		AmdPhenomX2DualCore,
		AmdAthlonX2DualCore,
		PaRisc,
		PaRisc8500,
		PaRisc8000,
		PaRisc7300LC,
		PaRisc7200,
		PaRisc7100LC,
		PaRisc7100,
		V30 = 160,
		IntelXeon3200QuadCoreSeries,
		IntelXeon3000DualCoreSeries,
		IntelXeon5300QuadCoreSeries,
		IntelXeon5100DualCoreSeries,
		IntelXeon5000DualCoreSeries,
		IntelXeonLVDualCore,
		IntelXeonULVDualCore,
		IntelXeon7100Series,
		IntelXeon5400Series,
		IntelXeonQuadCore,
		IntelXeon5200DualCoreSeries,
		IntelXeon7200DualCoreSeries,
		IntelXeon7300QuadCoreSeries,
		IntelXeon7400QuadCoreSeries,
		IntelXeon7400MultiCoreSeries,
		IntelPentiumIIIXeon,
		IntelPentiumIIISpeedStep,
		IntelPentium4,
		IntelXeon,
		As400,
		IntelXeonMP,
		AmdAthlonXP,
		AmdAthlonMP,
		IntelItanium2,
		IntelPentiumM,
		IntelCeleronD,
		IntelPentiumD,
		IntelPentiumExtreme,
		IntelCoreSolo,
		IntelCore2Duo = 191,
		IntelCore2Solo,
		IntelCore2Extreme,
		IntelCore2Quad,
		IntelCore2ExtremeMobile,
		IntelCore2DuoMobile,
		IntelCore2SoloMobile,
		IntelCoreI7,
		IntelCeleronDualCore,
		Ibm390,
		PowerPcG4,
		PowerPcG5,
		Esa390G6,
		ZArchitecture,
		IntelCoreI5,
		IntelCoreI3,
		IntelCoreI9,
		ViaC7M = 210,
		ViaC7D,
		ViaC7,
		ViaEden,
		IntelXeonMultiCore,
		IntelXeon3xxxDualCoreSeries,
		IntelXeon3xxxQuadCoreSeries,
		ViaNano,
		IntelXeon5xxxDualCoreSeries,
		IntelXeon5xxxQuadCoreSeries,
		IntelXeon7xxxDualCoreSeries = 221,
		IntelXeon7xxxQuadCoreSeries,
		IntelXeon7xxxMultiCoreSeries,
		IntelXeon3400MultiCoreSeries,
		AmdOpteron3000Series = 228,
		AmdSempronII,
		AmdOpteronQuadCoreEmbedded,
		AmdPhenomTripleCore,
		AmdTurionUltraDualCoreMobile,
		AmdTurionDualCoreMobile,
		AmdAthlonDualCore,
		AmdSempronSI,
		AmdPhenomII,
		AmdAthlonII,
		AmdOpteronSixCore,
		AmdSempronM,
		IntelI860 = 250,
		IntelI960,
		ArmV7 = 256,
		ArmV8,
		HitachiSh3 = 260,
		HitachiSh4,
		Arm = 280,
		StrongArm,
		_686 = 300,
		MediaGX,
		MII,
		WinChip = 320,
		Dsp = 350,
		VideoProcessor = 500,
		RiscVRV32 = 512,
		RiscVRV64,
		RiscVRV128
	};

	std::ostream& operator<<(std::ostream& os, ProcessorFamily v);

	/// Processor type based on <see href="https://www.dmtf.org/dsp/DSP0134">DMTF SMBIOS Reference Specification
	/// v.3.3.0, Chapter 7.5.1</see>.
	enum class ProcessorType: std::uint8_t
	{
		Other = 1,
		Unknown,
		CentralProcessor,
		MathProcessor,
		DspProcessor,
		VideoProcessor
	};

	std::ostream& operator<<(std::ostream& os, ProcessorType v);

	/// Processor socket based on <see href="https://www.dmtf.org/dsp/DSP0134">DMTF SMBIOS Reference Specification
	/// v.3.3.0, Chapter 7.5.5</see>.
	enum class ProcessorSocket: std::uint8_t
	{
		Other = 1,
		Unknown,
		DaughterBoard,
		ZifSocket,
		PiggyBack,
		None,
		LifSocket,
		Zif423 = 13,
		A,
		Zif478,
		Zif754,
		Zif940,
		Zif939,
		MPga604,
		Lga771,
		Lga775,
		S1,
		AM2,
		F,
		Lga1366,
		G34,
		AM3,
		C32,
		Lga1156,
		Lga1567,
		Pga988A,
		Bga1288,
		RPga088B,
		Bga1023,
		Bga1224,
		Lga1155,
		Lga1356,
		Lga2011,
		FS1,
		FS2,
		FM1,
		FM2,
		Lga20113,
		Lga13563,
		Lga1150,
		Bga1168,
		Bga1234,
		Bga1364,
		AM4,
		Lga1151,
		Bga1356,
		Bga1440,
		Bga1515,
		Lga36471,
		SP3,
		SP3R2,
		Lga2066,
		Bga1510,
		Bga1528,
		Lga4189
	};

	std::ostream& operator<<(std::ostream& os, ProcessorSocket v);

	/// System wake-up type based on <see href="https://www.dmtf.org/dsp/DSP0134">DMTF SMBIOS Reference Specification
	/// v.3.3.0, Chapter 7.2.2</see>.
	enum class SystemWakeUp: std::uint8_t
	{
		Reserved,
		Other,
		Unknown,
		ApmTimer,
		ModemRing,
		LanRemote,
		PowerSwitch,
		PciPme,
		AcPowerRestored
	};

	std::ostream& operator<<(std::ostream& os, SystemWakeUp sw);

	/// Cache associativity based on <see href="https://www.dmtf.org/dsp/DSP0134">DMTF SMBIOS Reference Specification
	/// v.3.3.0, Chapter 7.8.5</see>.
	enum class CacheAssociativity: std::uint8_t
	{
		Other = 1,
		Unknown,
		DirectMapped,
		_2Way,
		_4Way,
		FullyAssociative,
		_8Way,
		_16Way,
		_12Way,
		_24Way,
		_32Way,
		_48Way,
		_64Way,
		_20Way,
	};

	std::ostream& operator<<(std::ostream& os, CacheAssociativity v);

	/// Processor cache level.
	enum class CacheDesignation
	{
		Other,
		L1,
		L2,
		L3
	};

	std::ostream& operator<<(std::ostream& os, CacheDesignation v);

	class BiosData {
	public:
		BiosData(std::span<const std::uint8_t> data, std::span<std::string_view> strings);

		template <class T, std::size_t dataSize = sizeof(std::underlying_type_t<T>)> requires std::is_enum_v<T>
		T enumAt(std::size_t offset, T defaulValue)
		{
			if constexpr (dataSize == 1) {
				return static_cast<T>(this->byteAt(offset, static_cast<std::uint8_t>(defaulValue)));
			} else if constexpr (dataSize == 2) {
				return static_cast<T>(this->wordAt(offset, static_cast<std::uint8_t>(defaulValue)));
			} else {
				static_assert(!sizeof(T), "No way to read enum of this size");
			}
		}

		std::uint8_t byteAt(std::size_t offset, std::uint8_t defaulValue = 0) const;
		std::uint16_t wordAt(std::size_t offset, std::uint16_t defaulValue = 0) const;
		std::string stringAt(std::size_t offset, const std::string& defaultValue = {}) const;

	private:

		std::span<const std::uint8_t> data_;
		std::span<std::string_view> strings_;
	};

	class SMBios;

	/// Motherboard BIOS information obtained from the SMBIOS table.
	class BiosInformation {
		friend class SMBios;
		BiosInformation(
		    std::string vendor, std::string version, std::string_view date = {}, std::optional<std::size_t> size = {});
		BiosInformation(BiosData data);

	public:
		/// Gets the BIOS release date.
		const std::optional<std::chrono::year_month_day>& date() const
		{
			return date_;
		}

		/// Gets the size of the physical device containing the BIOS.
		std::optional<std::size_t> size() const
		{
			return size_;
		}

		/// Gets the string number of the BIOS Vendor’s Name.
		const std::string& vendor() const
		{
			return vendor_;
		}

		/// Gets the string number of the BIOS Version. This value is a free-form string that may contain Core and OEM
		/// version information.
		const std::string& version() const
		{
			return version_;
		}

	private:
		std::string vendor_;
		std::string version_;
		std::optional<std::chrono::year_month_day> date_;
		std::optional<std::size_t> size_;
	};

	/// System information obtained from the SMBIOS table.
	class SystemInformation {
		friend class SMBios;
		SystemInformation(
		    std::string manufacturerName, std::string productName, std::string version, std::string serialNumber, std::string family,
		    SystemWakeUp wakeUp = SystemWakeUp::Unknown);

		SystemInformation(BiosData data);

	public:
		/**
		 * @brief Gets the family associated with system.
		 *
		 * This text string identifies the family to which a particular computer belongs. A family refers to a set of
		 * computers that are similar but not identical from a hardware or software point of view. Typically, a family
		 * is composed of different computer models, which have different configurations and pricing points. Computers
		 * in the same family often have similar branding and cosmetic features.
		 */
		const std::string& family() const
		{
			return family_;
		}

		/// Gets the manufacturer name associated with system.
		const std::string manufacturerName() const
		{
			return manufacturerName_;
		}

		/// Gets the product name associated with system.
		const std::string productName() const
		{
			return productName_;
		}

		/// Gets the serial number string associated with system.
		const std::string serialNumber() const
		{
			return serialNumber_;
		}

		/// Gets the version string associated with system.
		const std::string version() const
		{
			return version_;
		}

		/// Gets SystemWakeUp
		SystemWakeUp wakeUp() const
		{
			return wakeUp_;
		}

	private:
		std::string manufacturerName_;
		std::string productName_;
		std::string version_;
		std::string serialNumber_;
		std::string family_;
		SystemWakeUp wakeUp_;
	};

	/// Chassis information obtained from the SMBIOS table.
	class ChassisInformation {
		friend class SMBios;
		// TODO add constructor for Linux
		ChassisInformation(BiosData data);

	public:
		/// Gets the asset tag associated with the enclosure or chassis.
		const std::string& assetTag() const
		{
			return assetTag_;
		}

		ChassisState bootUpState() const
		{
			return bootUpState_;
		}

		ChassisType chassisType() const
		{
			return chassisType_;
		}

		bool lockDetected() const
		{
			return lockDetected_;
		}

		/// Gets the string describing the chassis or enclosure manufacturer name.
		const std::string& manufacturerName() const
		{
			return manufacturerName_;
		}

		/// Gets the number of power cords associated with the enclosure or chassis.
		int powerCords() const
		{
			return powerCords_;
		}

		/// Gets the state of the enclosure’s power supply (or supplies) when last booted.
		ChassisState powerSupplyState() const
		{
			return powerSupplyState_;
		}

		/// Gets the height of the enclosure, in 'U's. A U is a standard unit of measure for the height of a rack or
		/// rack-mountable component and is equal to 1.75 inches or 4.445 cm. A value of 0 indicates that the
		/// enclosure height is unspecified.
		int rackHeight() const
		{
			return rackHeight_;
		}

		/// Gets the physical security status of the enclosure when last booted.
		ChassisSecurityStatus securityStatus() const
		{
			return securityStatus_;
		}

		/// Gets the string describing the chassis or enclosure serial number.
		const std::string& serialNumber() const
		{
			return serialNumber_;
		}

		/// Gets the string describing the chassis or enclosure SKU number.
		const std::string& SKU() const
		{
			return SKU_;
		}

		/// Gets the thermal state of the enclosure when last booted.
		ChassisState thermalState() const
		{
			return thermalState_;
		}

		/// Gets the number of null-terminated string representing the chassis or enclosure version.
		const std::string& version() const
		{
			return version_;
		}

	private:
		std::string manufacturerName_;
		std::string version_;
		std::string serialNumber_;
		std::string assetTag_;
		std::uint8_t rackHeight_;
		std::uint8_t powerCords_;
		std::string SKU_;
		bool lockDetected_;
		ChassisType chassisType_;
		ChassisState bootUpState_;
		ChassisState powerSupplyState_;
		ChassisState thermalState_;
		ChassisSecurityStatus securityStatus_;
	};

	/// Motherboard information obtained from the SMBIOS table.
	class BaseBoardInformation {
		friend class SMBios;
		BaseBoardInformation(std::string manufacturerName, std::string productName, std::string version, std::string serialNumber);
		BaseBoardInformation(BiosData data);

		/// Gets the value that represents the manufacturer's name.
	public:
		const std::string manufacturerName() const
		{
			return manufacturerName_;
		}

		/// Gets the value that represents the motherboard's name.
		const std::string productName() const
		{
			return productName_;
		}

		/// Gets the value that represents the motherboard's serial number.
		const std::string serialNumber() const
		{
			return serialNumber_;
		}

		/// Gets the value that represents the motherboard's revision number.
		const std::string version() const
		{
			return version_;
		}

	private:
		std::string manufacturerName_;
		std::string productName_;
		std::string version_;
		std::string serialNumber_;
	};

	/// Processor information obtained from the SMBIOS table.
	class ProcessorInformation {
		friend class SMBios;
		ProcessorInformation(BiosData d);

	public:
		/// Gets the value that represents the number of cores per processor socket.
		std::uint16_t coreCount() const
		{
			return coreCount_;
		}

		/// Gets the value that represents the number of enabled cores per processor socket.
		std::uint16_t coreEnabled() const
		{
			return coreEnabled_;
		}

		/// Gets the value that represents the current processor speed (in MHz).
		std::uint16_t currentSpeed() const
		{
			return currentSpeed_;
		}

		/// Gets the external Clock Frequency, in MHz. If the value is unknown, the field is set to 0.
		std::uint16_t externalClock() const
		{
			return externalClock_;
		}

		/// Gets processor family
		ProcessorFamily family() const
		{
			return family_;
		}

		/// Gets the string number of Processor Manufacturer.
		const std::string& manufacturerName() const
		{
			return manufacturerName_;
		}

		/// Gets the value that represents the maximum processor speed (in MHz) supported by the system for this
		/// processor socket.
		std::uint16_t maxSpeed() const
		{
			return maxSpeed_;
		}

		/// Gets processor type
		ProcessorType processorType() const
		{
			return processorType_;
		}

		/// Gets the value that represents the string number for the serial number of this processor.
		/// <para>This value is set by the manufacturer and normally not changeable.</para>
		const std::string& serial() const
		{
			return serial_;
		}

		/// Gets <inheritdoc cref="LibreHardwareMonitor.Hardware.ProcessorSocket" />
		ProcessorSocket socket() const
		{
			return socket_;
		}

		/// Gets the string number for Reference Designation.
		const std::string& socketDesignation() const
		{
			return socketDesignation_;
		}

		/// Gets the value that represents the number of threads per processor socket.
		std::uint16_t threadCount() const
		{
			return threadCount_;
		}

		/// Gets the value that represents the string number describing the Processor.
		const std::string& version() const
		{
			return version_;
		}

	private:
		std::string socketDesignation_;
		std::string manufacturerName_;
		std::string version_;
		std::uint16_t coreCount_;
		std::uint16_t coreEnabled_;
		std::uint16_t threadCount_;
		std::uint16_t externalClock_;
		std::uint16_t maxSpeed_;
		std::uint16_t currentSpeed_;
		std::string serial_;

		ProcessorType processorType_;
		ProcessorSocket socket_;
		ProcessorFamily family_;
	};

	/// Processor cache information obtained from the SMBIOS table.
	class ProcessorCache {
		friend class SMBios;
		ProcessorCache(BiosData d);

	public:
		CacheAssociativity associativity() const
		{
			return associativity_;
		}

		CacheDesignation designation() const
		{
			return designation_;
		}

		/// Gets the value that represents the installed cache size.
		std::uint16_t size() const
		{
			return size_;
		}

	private:
		CacheDesignation designation_;
		CacheAssociativity associativity_;
		std::uint16_t size_;
	};

	/// Memory information obtained from the SMBIOS table.
	class MemoryDevice {
		friend class SMBios;
		MemoryDevice(BiosData d);

	public:
		/// Gets the string number of the string that identifies the physically labeled bank where the memory device is
		/// located.
		const std::string& bankLocator() const
		{
			return bankLocator_;
		}

		/// Gets the string number of the string that identifies the physically-labeled socket or board position where
		/// the memory device is located.
		const std::string& deviceLocator() const
		{
			return deviceLocator_;
		}

		/// Gets the string number for the manufacturer of this memory device.
		const std::string& manufacturerName() const
		{
			return manufacturerName_;
		}

		/// Gets the string number for the part number of this memory device.
		const std::string& partNumber() const
		{
			return partNumber_;
		}

		/// Gets the string number for the serial number of this memory device.
		const std::string& serialNumber() const
		{
			return serialNumber_;
		}

		/// Gets the size of the memory device. If the value is 0, no memory device is installed in the socket.
		std::uint16_t size() const
		{
			return size_;
		}

		/// Gets the the value that identifies the maximum capable speed of the device, in mega transfers per second
		/// (MT/s).
		std::uint16_t speed() const
		{
			return speed_;
		}

	private:
		std::string deviceLocator_;
		std::string bankLocator_;
		std::string manufacturerName_;
		std::string serialNumber_;
		std::string partNumber_;
		std::uint16_t speed_;
		std::uint16_t size_;
	};

	/// Reads and processes information encoded in an SMBIOS table.
	class SMBios {
	public:
		SMBios();

		const BiosInformation* bios() const
		{
			return bios_.get();
		}

		const BaseBoardInformation* board() const
		{
			return board_.get();
		}

		const ChassisInformation* chassis() const
		{
			return chassis_.get();
		}

		std::span<const MemoryDevice> memoryDevices() const
		{
			return {&memoryDevices_.front(), memoryDevices_.size()};
		}

		const ProcessorInformation* processor() const
		{
			return processor_.get();
		}

		std::span<const ProcessorCache> processorCaches() const
		{
			return {&processorCaches_.front(), processorCaches_.size()};
		}

		const SystemInformation* system() const
		{
			return system_.get();
		}

		std::ostream& operator<<(std::ostream& os) const;

	private:
		SMBios(const SMBios&) = delete;
		SMBios& operator=(const SMBios&) = delete;

		std::unique_ptr<BiosInformation> bios_;
		std::unique_ptr<BaseBoardInformation> board_;
		std::unique_ptr<ChassisInformation> chassis_;
		std::unique_ptr<ProcessorInformation> processor_;
		std::unique_ptr<SystemInformation> system_;
		std::vector<MemoryDevice> memoryDevices_;
		std::vector<ProcessorCache> processorCaches_;
		std::uint8_t verMajor_;
		std::uint8_t verMinor_;
	};
}

#endif
