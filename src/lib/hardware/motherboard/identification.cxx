// SPDX-License-Identifier: LGPL-3.0+

#include "./identification.hxx"

#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <map>
#include <string>

wm_sensors::hardware::motherboard::Manufacturer wm_sensors::hardware::motherboard::getManufacturer(
    std::string_view name)
{
	auto icontains = [](std::string_view input, std::string_view search) {
		return !boost::algorithm::ifind_first(input, search).empty();
	};

	if (icontains(name, "abit.com.tw")) return Manufacturer::Acer;
	if (name.starts_with("Acer")) return Manufacturer::Acer;
	if (name.starts_with("AMD")) return Manufacturer::AMD;
	if (name == "Alienware") return Manufacturer::Alienware;
	if (name.starts_with("AOpen")) return Manufacturer::AOpen;
	if (name.starts_with("Apple")) return Manufacturer::Apple;
	if (name == "ASRock") return Manufacturer::ASRock;
	if (name.starts_with("ASUSTeK") || name.starts_with("ASUS ")) return Manufacturer::ASUS;
	if (name.starts_with("Biostar")) return Manufacturer::Biostar;
	if (name.starts_with("Clevo")) return Manufacturer::Clevo;
	if (name.starts_with("Dell")) return Manufacturer::Dell;
	if (name == "DFI" || name.starts_with("DFI Inc")) return Manufacturer::DFI;
	if (name == "ECS" || name.starts_with("ELITEGROUP")) return Manufacturer::ECS;
	if (name == "EPoX COMPUTER CO., LTD") return Manufacturer::EPoX;
	if (name.starts_with("EVGA")) return Manufacturer::EVGA;
	if (name == "FIC" || name.starts_with("First International Computer")) return Manufacturer::FIC;
	if (name == "Foxconn") return Manufacturer::Foxconn;
	if (name.starts_with("Fujitsu")) return Manufacturer::Fujitsu;
	if (name.starts_with("Gigabyte")) return Manufacturer::Gigabyte;
	if (name.starts_with("Hewlett-Packard") || name == "HP") return Manufacturer::HP;
	if (name == "IBM") return Manufacturer::IBM;
	if (name == "Intel" || name.starts_with("Intel Corp")) return Manufacturer::Intel;
	if (name.starts_with("Jetway")) return Manufacturer::Jetway;
	if (name.starts_with("Lenovo")) return Manufacturer::Lenovo;
	if (name == "LattePanda") return Manufacturer::LattePanda;
	if (name.starts_with("Medion")) return Manufacturer::Medion;
	if (name.starts_with("Microsoft")) return Manufacturer::Microsoft;
	if (name.starts_with("Micro-Star International") || name == "MSI") return Manufacturer::MSI;
	if (name.starts_with("NEC ") || name == "NEC") return Manufacturer::NEC;
	if (name.starts_with("Pegatron")) return Manufacturer::Pegatron;
	if (name.starts_with("Samsung")) return Manufacturer::Samsung;
	if (name.starts_with("Sapphire")) return Manufacturer::Sapphire;
	if (name.starts_with("Shuttle")) return Manufacturer::Shuttle;
	if (name.starts_with("Sony")) return Manufacturer::Sony;
	if (name.starts_with("Supermicro")) return Manufacturer::Supermicro;
	if (name.starts_with("Toshiba")) return Manufacturer::Toshiba;
	if (name == "XFX") return Manufacturer::XFX;
	if (name.starts_with("Zotac")) return Manufacturer::Zotac;
	if (name == "To be filled by O.E.M.") return Manufacturer::Unknown;

	return Manufacturer::Unknown;
}

wm_sensors::hardware::motherboard::Model wm_sensors::hardware::motherboard::getModel(std::string_view name)
{
	struct Entry {
		const char* name;
		Model model;
	};

	static Entry models[] = {
	    {"880GMH/USB3", Model::_880GMH_USB3},
	    {"965P-S3", Model::_965P_S3},
	    {"A320M-HDV", Model::A320M_HDV},
	    {"A890GXM-A", Model::A890GXM_A},
	    {"AB350 Pro4", Model::AB350_Pro4},
	    {"AB350-Gaming 3-CF", Model::AB350_Gaming_3},
	    {"AB350M Pro4", Model::AB350M_Pro4},
	    {"AB350M", Model::AB350M},
	    {"AB350M-HDV", Model::AB350M_HDV},
	    {"ASRock AOD790GX/128M", Model::AOD790GX_128M},
	    {"AX370-Gaming 5", Model::AX370_Gaming_5},
	    {"AX370-Gaming K7", Model::AX370_Gaming_K7},
	    {"B350 GAMING PLUS (MS-7A34)", Model::B350_Gaming_Plus},
	    {"B360M PRO-VDH (MS-7B24)", Model::B360M_PRO_VDH},
	    {"B450 Pro4", Model::B450_Pro4},
	    {"B450 Steel Legend", Model::B450_Steel_Legend},
	    {"B450-A PRO (MS-7B86)", Model::B450A_PRO},
	    {"B450M Pro4", Model::B450M_Pro4},
	    {"B450M Steel Legend", Model::B450M_Steel_Legend},
	    {"B560M AORUS ELITE", Model::B560M_AORUS_ELITE},
	    {"B85M-DGS", Model::B85M_DGS},
	    {"Crosshair III Formula", Model::ROG_CROSSHAIR_III_FORMULA},
	    {"EP45-DS3R", Model::EP45_DS3R},
	    {"EP45-UD3R", Model::EP45_UD3R},
	    {"EX58-EXTREME", Model::EX58_EXTREME},
	    {"EX58-UD3R", Model::EX58_UD3R},
	    {"FH67", Model::FH67},
	    {"Fatal1ty AB350 Gaming K4", Model::Fatal1ty_AB350_Gaming_K4},
	    {"G41M-Combo", Model::G41M_COMBO},
	    {"G41MT-S2", Model::G41MT_S2},
	    {"G41MT-S2P", Model::G41MT_S2P},
	    {"GA-970A-UD3", Model::_970A_UD3},
	    {"GA-MA770T-UD3", Model::MA770T_UD3},
	    {"GA-MA770T-UD3P", Model::MA770T_UD3P},
	    {"GA-MA785GM-US2H", Model::MA785GM_US2H},
	    {"GA-MA785GMT-UD2H", Model::MA785GMT_UD2H},
	    {"GA-MA78LM-S2H", Model::MA78LM_S2H},
	    {"GA-MA790X-UD3P", Model::MA790X_UD3P},
	    {"H55-USB3", Model::H55_USB3},
	    {"H55N-USB3", Model::H55N_USB3},
	    {"H61M-DS2 REV 1.2", Model::H61M_DS2_REV_1_2},
	    {"H61M-USB3-B3 REV 2.0", Model::H61M_USB3_B3_REV_2_0},
	    {"H67A-UD3H-B3", Model::H67A_UD3H_B3},
	    {"H67A-USB3-B3", Model::H67A_USB3_B3},
	    {"H81M-HD3", Model::H81M_HD3},
	    {"LP BI P45-T2RS Elite", Model::LP_BI_P45_T2RS_Elite},
	    {"LP DK P55-T3eH9", Model::LP_DK_P55_T3EH9},
	    {"M2N-SLI DELUXE", Model::M2N_SLI_Deluxe},
	    {"M4A79XTD EVO", Model::M4A79XTD_EVO},
	    {"P35-DS3", Model::P35_DS3},
	    {"P35-DS3L", Model::P35_DS3L},
	    {"P55 Deluxe", Model::P55_Deluxe},
	    {"P55-UD4", Model::P55_UD4},
	    {"P55A-UD3", Model::P55A_UD3},
	    {"P55M-UD4", Model::P55M_UD4},
	    {"P5W DH Deluxe", Model::P5W_DH_Deluxe},
	    {"P67A-UD3-B3", Model::P67A_UD3_B3},
	    {"P67A-UD3R-B3", Model::P67A_UD3R_B3},
	    {"P67A-UD4-B3", Model::P67A_UD4_B3},
	    {"P6T", Model::P6T},
	    {"P6X58D-E", Model::P6X58D_E},
	    {"P8P67 EVO", Model::P8P67_EVO},
	    {"P8P67 PRO", Model::P8P67_PRO},
	    {"P8P67 REV 3.1", Model::P8P67},
	    {"P8P67", Model::P8P67},
	    {"P8P67-M PRO", Model::P8P67_M_PRO},
	    {"P8Z68-V PRO", Model::P8Z68_V_PRO},
	    {"P8Z77-V", Model::P8Z77_V},
	    {"P9X79", Model::P9X79},
	    {"PRIME X370-PRO", Model::PRIME_X370_PRO},
	    {"PRIME X570-PRO", Model::PRIME_X570_PRO},
	    {"Pro WS X570-ACE", Model::PRO_WS_X570_ACE},
	    {"ROG CROSSHAIR VIII DARK HERO", Model::ROG_CROSSHAIR_VIII_DARK_HERO},
	    {"ROG CROSSHAIR VIII FORMULA", Model::ROG_CROSSHAIR_VIII_FORMULA},
	    {"ROG CROSSHAIR VIII HERO", Model::ROG_CROSSHAIR_VIII_HERO},
	    {"ROG CROSSHAIR VIII HERO(WI - FI)", Model::ROG_CROSSHAIR_VIII_HERO_WIFI},
	    {"ROG CROSSHAIR VIII IMPACT", Model::ROG_CROSSHAIR_VIII_IMPACT},
	    {"ROG MAXIMUS X APEX", Model::ROG_MAXIMUS_X_APEX},
	    {"ROG STRIX B550-E GAMING", Model::ROG_STRIX_B550_E_GAMING},
	    {"ROG STRIX B550-F GAMING", Model::ROG_STRIX_B550_F_GAMING},
	    {"ROG STRIX B550-F GAMING (WI-FI)", Model::ROG_STRIX_B550_F_GAMING_WIFI},
	    {"ROG STRIX B550-I GAMING", Model::ROG_STRIX_B550_I_GAMING},
	    {"ROG STRIX X470-I GAMING", Model::ROG_STRIX_X470_I},
	    {"ROG STRIX X570-E GAMING", Model::ROG_STRIX_X570_E_GAMING},
	    {"ROG STRIX X570-F GAMING", Model::ROG_STRIX_X570_F_GAMING},
	    {"ROG STRIX X570-I GAMING", Model::ROG_STRIX_X570_I_GAMING},
	    {"ROG ZENITH EXTREME", Model::ROG_ZENITH_EXTREME},
	    {"Rampage Extreme", Model::RAMPAGE_EXTREME},
	    {"Rampage II GENE", Model::RAMPAGE_II_GENE},
	    {"TUF GAMING B550M-PLUS (WI-FI)", Model::TUF_GAMING_B550M_PLUS_WIFI},
	    {"TUF X470-PLUS GAMING", Model::TUF_X470_PLUS_GAMING},
	    {"X38-DS5", Model::X38_DS5},
	    {"X399 AORUS Gaming 7", Model::X399_AORUS_Gaming_7},
	    {"X399 Phantom Gaming 6", Model::X399_Phantom_Gaming_6},
	    {"X470 AORUS GAMING 7 WIFI-CF", Model::X470_AORUS_GAMING_7_WIFI},
	    {"X570 AORUS MASTER", Model::X570_AORUS_MASTER},
	    {"X570 GAMING X", Model::X570_GAMING_X},
	    {"X570 Phantom Gaming-ITX/TB3", Model::X570_Phantom_Gaming_ITX},
	    {"X570 Pro4", Model::X570_Pro4},
	    {"X570 Taichi", Model::X570_Taichi},
	    {"X58 SLI Classified", Model::X58_SLI_Classified},
	    {"X58A-UD3R", Model::X58A_UD3R},
	    {"X79-UD3", Model::X79_UD3},
	    {"Z170-A", Model::Z170_A},
	    {"Z170N-WIFI-CF", Model::Z170N_WIFI},
	    {"Z270 PC MATE (MS-7A72)", Model::Z270_PC_MATE},
	    {"Z270 PC MATE", Model::Z270_PC_MATE},
	    {"Z390 AORUS PRO-CF", Model::Z390_AORUS_PRO},
	    {"Z390 AORUS ULTRA", Model::Z390_AORUS_ULTRA},
	    {"Z390 M GAMING-CF", Model::Z390_M_GAMING},
	    {"Z390 UD", Model::Z390_UD},
	    {"Z68A-D3H-B3", Model::Z68A_D3H_B3},
	    {"Z68AP-D3", Model::Z68AP_D3},
	    {"Z68X-UD3H-B3", Model::Z68X_UD3H_B3},
	    {"Z68X-UD7-B3", Model::Z68X_UD7_B3},
	    {"Z68XP-UD3R", Model::Z68XP_UD3R},
	    {"Z77 Pro4-M", Model::Z77Pro4M},
	    //
	    {"Base Board Product Name", Model::Unknown},
	    {"To be filled by O.E.M.", Model::Unknown}};

	const auto end = std::end(models);
	const auto it = std::find_if(
	    std::begin(models), end, [name](const Entry& e) { return boost::algorithm::iequals(name, e.name); });
	return it != end ? it->model : Model::Unknown;
}
