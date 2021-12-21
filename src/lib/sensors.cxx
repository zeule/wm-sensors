// SPDX-License-Identifier: LGPL-3.0+

#include "./sensors.h"

#include "./error.h"
#include "./sensor_tree.hxx"
#include "./impl/libsensors/chip_data.hxx"
#include "./visitor/chip_visitor.hxx"
#include "./utility/string.hxx"
#include "./utility/utility.hxx"

#include "version.h"

#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <limits>
#include <memory>
#include <map>
#include <stdexcept>
#include <string.h>

#ifdef _MSC_VER
#pragma warning (disable: 4365 4388) // 
#endif

const char* libsensors_version = WMS_VERSION;

using wm_sensors::SensorChip;
using wm_sensors::SensorType;
using namespace wm_sensors::stdtypes;

namespace {
	std::unique_ptr<wm_sensors::SensorsTree> sensors;
	struct ChipInfo {
		sensors_chip_name name;
		wm_sensors::impl::libsensors::ChipData data;

		ChipInfo(sensors_chip_name&& n, wm_sensors::impl::libsensors::ChipData&& d)
		    : name{std::move(n)}
		    , data{std::move(d)}
		{
		}

		DELETE_COPY_CTOR_AND_ASSIGNMENT(ChipInfo)
		DEFAULT_MOVE_CTOR_AND_ASSIGNMENT(ChipInfo)

	};
	std::vector<ChipInfo> chips;


	sensors_chip_name makeChipName(std::string_view path, std::string_view chipType, std::size_t chipIndex, wm_sensors::BusType bus)
	{
		sensors_chip_name res;
		res.bus.type = wm_sensors::utility::to_underlying(bus);
		res.bus.nr = 0; // we do not track bus numbers yet
		
		std::string fullPath = fmt::format("{0}{1}-{2}", path, chipType, chipIndex);
		res.path = strndup(fullPath);

		res.addr = static_cast<int>(chipIndex);
		res.prefix = strndup(chipType.data(), chipType.size());
		return res;
	}

	/** Compare two chips name descriptions, to see whether they could match.
     Return 0 if it does not match, return 1 if it does match. */
	int sensors_match_chip(const sensors_chip_name* chip1, const sensors_chip_name* chip2)
	{
		if ((chip1->prefix != SENSORS_CHIP_NAME_PREFIX_ANY) && (chip2->prefix != SENSORS_CHIP_NAME_PREFIX_ANY) &&
		    strcmp(chip1->prefix, chip2->prefix))
			return 0;

		if ((chip1->bus.type != SENSORS_BUS_TYPE_ANY) && (chip2->bus.type != SENSORS_BUS_TYPE_ANY) &&
		    (chip1->bus.type != chip2->bus.type))
			return 0;

		if ((chip1->bus.nr != SENSORS_BUS_NR_ANY) && (chip2->bus.nr != SENSORS_BUS_NR_ANY) &&
		    (chip1->bus.nr != chip2->bus.nr))
			return 0;

		if ((chip1->addr != chip2->addr) && (chip1->addr != SENSORS_CHIP_NAME_ADDR_ANY) &&
		    (chip2->addr != SENSORS_CHIP_NAME_ADDR_ANY))
			return 0;

		return 1;
	}

	ChipInfo* find_chip(const sensors_chip_name* name)
	{
		const auto it =
		    std::find_if(chips.begin(), chips.end(), [name](const ChipInfo& ci) { return &ci.name == name; });
		return it != chips.end() ? &(*it) : nullptr;
	}
} // namespace

/* Check whether the chip name is an 'absolute' name, which can only match
   one chip, or whether it has wildcards. Returns 0 if it is absolute, 1
   if there are wildcards. */
int sensors_chip_name_has_wildcards(const sensors_chip_name* chip)
{
	if ((chip->prefix == SENSORS_CHIP_NAME_PREFIX_ANY) || (chip->bus.type == SENSORS_BUS_TYPE_ANY) ||
	    (chip->bus.nr == SENSORS_BUS_NR_ANY) || (chip->addr == SENSORS_CHIP_NAME_ADDR_ANY))
		return 1;
	else
		return 0;
}

int sensors_init(FILE* input)
{
	if (input) {
		spdlog::critical("Config files are not upported yet");
		return -EOPNOTSUPP;
	}
	sensors.reset(new wm_sensors::SensorsTree());

	class CollectSensorsVisitor: public wm_sensors::SensorChipVisitor {
	public:
		void visit(const NodeAddress& path, std::size_t index, const SensorChip& chip) override
		{
			chips.emplace_back(makeChipName(path.fullPath, path.nodeName, index, chip.identifier().bus),
			     wm_sensors::impl::libsensors::ChipData(&chip));
		}
		using wm_sensors::SensorChipVisitor::visit;
	};

	CollectSensorsVisitor v;
	sensors->chips().accept(v);
	return 0;
}

void sensors_cleanup(void)
{
	chips.clear();
	sensors.reset();
}

int sensors_parse_chip_name(const char* name, sensors_chip_name* res)
{
	char* dash;

	/* First, the prefix. It's either "*" or a real chip name. */
	if (!strncmp(name, "*-", 2)) {
		res->prefix = SENSORS_CHIP_NAME_PREFIX_ANY;
		name += 2;
	} else {
		if (!(dash = const_cast<char*>(strchr(name, '-'))))
			return -SENSORS_ERR_CHIP_NAME;
		res->prefix = strndup(name, dash - name);
		if (!res->prefix)
			spdlog::critical("{0}: {1}", __func__, "Allocating name prefix");
		name = dash + 1;
	}

	/* Then we have either a sole "*" (all chips with this name) or a bus
	   type and an address. */
	if (!strcmp(name, "*")) {
		res->bus.type = SENSORS_BUS_TYPE_ANY;
		res->bus.nr = SENSORS_BUS_NR_ANY;
		res->addr = SENSORS_CHIP_NAME_ADDR_ANY;
		return 0;
	}

	if (!(dash = const_cast<char*>(strchr(name, '-'))))
		goto ERROR;
	if (!strncmp(name, "i2c", dash - name))
		res->bus.type = SENSORS_BUS_TYPE_I2C;
	else if (!strncmp(name, "isa", dash - name))
		res->bus.type = SENSORS_BUS_TYPE_ISA;
	else if (!strncmp(name, "pci", dash - name))
		res->bus.type = SENSORS_BUS_TYPE_PCI;
	else if (!strncmp(name, "spi", dash - name))
		res->bus.type = SENSORS_BUS_TYPE_SPI;
	else if (!strncmp(name, "virtual", dash - name))
		res->bus.type = SENSORS_BUS_TYPE_VIRTUAL;
	else if (!strncmp(name, "acpi", dash - name))
		res->bus.type = SENSORS_BUS_TYPE_ACPI;
	else if (!strncmp(name, "hid", dash - name))
		res->bus.type = SENSORS_BUS_TYPE_HID;
	else if (!strncmp(name, "mdio", dash - name))
		res->bus.type = SENSORS_BUS_TYPE_MDIO;
	else if (!strncmp(name, "scsi", dash - name))
		res->bus.type = SENSORS_BUS_TYPE_SCSI;
	else
		goto ERROR;
	name = dash + 1;

	/* Some bus types (i2c, spi) have an additional bus number.
	   For these, the next part is either a "*" (any bus of that type)
	   or a decimal number. */
	switch (res->bus.type) {
		case SENSORS_BUS_TYPE_I2C:
		case SENSORS_BUS_TYPE_SPI:
		case SENSORS_BUS_TYPE_HID:
		case SENSORS_BUS_TYPE_SCSI:
			if (!strncmp(name, "*-", 2)) {
				res->bus.nr = SENSORS_BUS_NR_ANY;
				name += 2;
				break;
			}

			res->bus.nr = static_cast<decltype(res->bus.nr)>(strtoul(name, &dash, 10));
			if (*name == '\0' || *dash != '-' || res->bus.nr < 0)
				goto ERROR;
			name = dash + 1;
			break;
		default: res->bus.nr = SENSORS_BUS_NR_ANY;
	}

	/* Last part is the chip address, or "*" for any address. */
	if (!strcmp(name, "*")) {
		res->addr = SENSORS_CHIP_NAME_ADDR_ANY;
	} else {
		res->addr = strtoul(name, &dash, 16);
		if (*name == '\0' || *dash != '\0' || res->addr < 0)
			goto ERROR;
	}

	return 0;

ERROR:
	free(res->prefix);
	return -SENSORS_ERR_CHIP_NAME;
}

void sensors_free_chip_name(sensors_chip_name* chip)
{
	free(chip->prefix);
	free(chip->path);
}

int sensors_snprintf_chip_name(char* str, size_t size, const sensors_chip_name* chip)
{
	if (sensors_chip_name_has_wildcards(chip))
		return -SENSORS_ERR_WILDCARDS;

	switch (chip->bus.type) {
		case SENSORS_BUS_TYPE_ISA: return snprintf(str, size, "%s-isa-%04x", chip->prefix, chip->addr);
		case SENSORS_BUS_TYPE_PCI: return snprintf(str, size, "%s-pci-%04x", chip->prefix, chip->addr);
		case SENSORS_BUS_TYPE_I2C:
			return snprintf(str, size, "%s-i2c-%hd-%02x", chip->prefix, chip->bus.nr, chip->addr);
		case SENSORS_BUS_TYPE_SPI: return snprintf(str, size, "%s-spi-%hd-%x", chip->prefix, chip->bus.nr, chip->addr);
		case SENSORS_BUS_TYPE_VIRTUAL: return snprintf(str, size, "%s-virtual-%x", chip->prefix, chip->addr);
		case SENSORS_BUS_TYPE_ACPI: return snprintf(str, size, "%s-acpi-%x", chip->prefix, chip->addr);
		case SENSORS_BUS_TYPE_HID: return snprintf(str, size, "%s-hid-%hd-%x", chip->prefix, chip->bus.nr, chip->addr);
		case SENSORS_BUS_TYPE_MDIO: return snprintf(str, size, "%s-mdio-%x", chip->prefix, chip->addr);
		case SENSORS_BUS_TYPE_SCSI:
			return snprintf(str, size, "%s-scsi-%hd-%x", chip->prefix, chip->bus.nr, chip->addr);
	}

	return -SENSORS_ERR_CHIP_NAME;
}

const char* sensors_get_adapter_name(const sensors_bus_id* bus)
{
	switch (bus->type) {
		case SENSORS_BUS_TYPE_ISA: return "ISA adapter";
		case SENSORS_BUS_TYPE_PCI: return "PCI adapter";
		/* SPI should not be here, but for now SPI adapters have no name
		   so we don't have any custom string to return. */
		case SENSORS_BUS_TYPE_SPI: return "SPI adapter";
		case SENSORS_BUS_TYPE_VIRTUAL: return "Virtual device";
		case SENSORS_BUS_TYPE_ACPI: return "ACPI interface";
		/* HID should probably not be there either, but I don't know if
		   HID buses have a name nor where to find it. */
		case SENSORS_BUS_TYPE_HID: return "HID adapter";
		case SENSORS_BUS_TYPE_MDIO: return "MDIO adapter";
		case SENSORS_BUS_TYPE_SCSI: return "SCSI adapter";
		default: return "Unknown adapter";
	}
}

char* sensors_get_label(const sensors_chip_name* name, const sensors_feature* feature)
{
	const ChipInfo* ci = find_chip(name);

	return ci ? ci->data.label(feature) : nullptr;
}

int sensors_get_value(const sensors_chip_name* name, int subfeat_nr, double* value)
{
	const ChipInfo* ci = find_chip(name);
	if (!ci || subfeat_nr >= ci->data.subfeatures().size()) {
		*value = std::numeric_limits<double>::quiet_NaN();
		return -EOPNOTSUPP;
	}

	return ci->data.read(subfeat_nr, *value);
}

int sensors_set_value(const sensors_chip_name* /*name*/, int /*subfeat_nr*/, double /*value*/)
{
	return -EOPNOTSUPP; // TODO
}

int sensors_do_chip_sets(const sensors_chip_name* /*name*/)
{
	return 0; // TODO
}

const sensors_chip_name* sensors_get_detected_chips(const sensors_chip_name* match, int* nr)
{
	const sensors_chip_name* res;

	while (*nr < chips.size()) {
		res = &chips[(*nr)++].name;
		if (!match || sensors_match_chip(res, match))
			return res;
	}
	return nullptr;
}

const sensors_feature* sensors_get_features(const sensors_chip_name* name, int* nr)
{
	const ChipInfo* ci = find_chip(name);
	if (!ci) {
		return nullptr;
	}
	if (*nr >= ci->data.features().size() || *nr < 0) {
		return nullptr;
	}

	std::size_t index = static_cast<std::size_t>(*nr); 
	++(*nr);// signed/unsigned mismatch :(
	return &ci->data.features()[index];
}

const sensors_subfeature*
sensors_get_all_subfeatures(const sensors_chip_name* /*name*/, const sensors_feature* /*feature*/, int* /*nr*/)
{
	return nullptr;
}

const sensors_subfeature*
sensors_get_subfeature(const sensors_chip_name* name, const sensors_feature* feature, sensors_subfeature_type type)
{
	const ChipInfo* ci = find_chip(name);
	if (!ci) {
		return nullptr;
	}

	return ci->data.subfeature(feature, type);
}
