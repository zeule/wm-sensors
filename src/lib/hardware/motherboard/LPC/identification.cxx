// SPDX-License-Identifier: LGPL-3.0+

#include "./identification.hxx"

std::string_view wm_sensors::hardware::motherboard::lpc::chip_name(Chip chip)
{
	switch (chip) {
		case Chip::ATK0110: return "Asus ATK0110";

		case Chip::F71858: return "Fintek F71858";
		case Chip::F71862: return "Fintek F71862";
		case Chip::F71869: return "Fintek F71869";
		case Chip::F71878AD: return "Fintek F71878AD";
		case Chip::F71869A: return "Fintek F71869A/F71811";
		case Chip::F71882: return "Fintek F71882";
		case Chip::F71889AD: return "Fintek F71889AD";
		case Chip::F71889ED: return "Fintek F71889ED";
		case Chip::F71889F: return "Fintek F71889F";
		case Chip::F71808E: return "Fintek F71808E";

		case Chip::IT8620E: return "ITE IT8620E";
		case Chip::IT8628E: return "ITE IT8628E";
		case Chip::IT8631E: return "ITE IT8631E";
		case Chip::IT8655E: return "ITE IT8655E";
		case Chip::IT8665E: return "ITE IT8665E";
		case Chip::IT8686E: return "ITE IT8686E";
		case Chip::IT8688E: return "ITE IT8688E";
		case Chip::IT8689E: return "ITE IT8689E";
		case Chip::IT8705F: return "ITE IT8705F";
		case Chip::IT8712F: return "ITE IT8712F";
		case Chip::IT8716F: return "ITE IT8716F";
		case Chip::IT8718F: return "ITE IT8718F";
		case Chip::IT8720F: return "ITE IT8720F";
		case Chip::IT8721F: return "ITE IT8721F";
		case Chip::IT8726F: return "ITE IT8726F";
		case Chip::IT8728F: return "ITE IT8728F";
		case Chip::IT8771E: return "ITE IT8771E";
		case Chip::IT8772E: return "ITE IT8772E";
		case Chip::IT879XE: return "ITE IT8792E/IT8795E";

		case Chip::NCT610XD: return "Nuvoton NCT6102D/NCT6104D/NCT6106D";
		case Chip::NCT6771F: return "Nuvoton NCT6771F";
		case Chip::NCT6776F: return "Nuvoton NCT6776F";
		case Chip::NCT6779D: return "Nuvoton NCT6779D";
		case Chip::NCT6791D: return "Nuvoton NCT6791D";
		case Chip::NCT6792D: return "Nuvoton NCT6792D";
		case Chip::NCT6792DA: return "Nuvoton NCT6792D-A";
		case Chip::NCT6793D: return "Nuvoton NCT6793D";
		case Chip::NCT6795D: return "Nuvoton NCT6795D";
		case Chip::NCT6796D: return "Nuvoton NCT6796D";
		case Chip::NCT6796DR: return "Nuvoton NCT6796D-R";
		case Chip::NCT6797D: return "Nuvoton NCT6797D";
		case Chip::NCT6798D: return "Nuvoton NCT6798D";
		case Chip::NCT6687D: return "Nuvoton NCT6687D";
		case Chip::NCT6683D: return "Nuvoton NCT6683D";

		case Chip::W83627DHG: return "Winbond W83627DHG";
		case Chip::W83627DHGP: return "Winbond W83627DHG-P";
		case Chip::W83627EHF: return "Winbond W83627EHF";
		case Chip::W83627HF: return "Winbond W83627HF";
		case Chip::W83627THF: return "Winbond W83627THF";
		case Chip::W83667HG: return "Winbond W83667HG";
		case Chip::W83667HGB: return "Winbond W83667HG-B";
		case Chip::W83687THF: return "Winbond W83687THF";

		default: return "Unknown";
	}
}
