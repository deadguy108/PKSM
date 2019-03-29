/*
*   This file is part of PKSM
*   Copyright (C) 2016-2019 Bernardo Giordano, Admiral Fish, piepie62
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*   Additional Terms 7.b and 7.c of GPLv3 apply to this file:
*       * Requiring preservation of specified reasonable legal notices or
*         author attributions in that material or in the Appropriate Legal
*         Notices displayed by works containing it.
*       * Prohibiting misrepresentation of the origin of that material,
*         or requiring that modified versions of such material be marked in
*         reasonable ways as different from the original version.
*/

#include "Sav6.hpp"

u16 Sav6::TID(void) const { return *(u16*)(data + TrainerCard); }
void Sav6::TID(u16 v) { *(u16*)(data + TrainerCard) = v; }

u16 Sav6::SID(void) const { return *(u16*)(data + TrainerCard + 2); }
void Sav6::SID(u16 v) { *(u16*)(data + TrainerCard + 2) = v; }

u8 Sav6::version(void) const { return data[TrainerCard + 4]; }
void Sav6::version(u8 v) { data[TrainerCard + 4] = v; }

u8 Sav6::gender(void) const { return data[TrainerCard + 5]; }
void Sav6::gender(u8 v) { data[TrainerCard + 5] = v; }

u8 Sav6::subRegion(void) const { return data[TrainerCard + 0x26]; }
void Sav6::subRegion(u8 v) { data[TrainerCard + 0x26] = v; }

u8 Sav6::country(void) const { return data[TrainerCard + 0x27]; }
void Sav6::country(u8 v) { data[TrainerCard + 0x27] = v; }

u8 Sav6::consoleRegion(void) const { return data[TrainerCard + 0x2C]; }
void Sav6::consoleRegion(u8 v) { data[TrainerCard + 0x2C] = v; }

u8 Sav6::language(void) const { return data[TrainerCard + 0x2D]; }
void Sav6::language(u8 v) { data[TrainerCard + 0x2D] = v; }

std::string Sav6::otName(void) const { return StringUtils::transString67(StringUtils::getString(data, TrainerCard + 0x48, 13)); }
void Sav6::otName(const std::string& v) { StringUtils::setString(data, StringUtils::transString67(v), TrainerCard + 0x48, 13); }

u32 Sav6::money(void) const { return *(u32*)(data + Trainer2 + 0x8); }
void Sav6::money(u32 v) { *(u32*)(data + Trainer2 + 0x8) = v; }

u32 Sav6::BP(void) const { return *(u32*)(data + Trainer2 + (game == Game::XY ? 0x3C : 0x30)); }
void Sav6::BP(u32 v) { *(u32*)(data + Trainer2 + (game == Game::XY ? 0x3C : 0x30)) = v; }

u16 Sav6::playedHours(void) const { return *(u16*)(data + PlayTime); }
void Sav6::playedHours(u16 v) { *(u16*)(data + PlayTime) = v; }

u8 Sav6::playedMinutes(void) const { return *(u8*)(data + PlayTime + 2); }
void Sav6::playedMinutes(u8 v) { *(u8*)(data + PlayTime + 2) = v; }

u8 Sav6::playedSeconds(void) const { return *(u8*)(data + PlayTime + 3); }
void Sav6::playedSeconds(u8 v) { *(u8*)(data + PlayTime + 3) = v; }

u8 Sav6::currentBox(void) const { return data[LastViewedBox]; }
void Sav6::currentBox(u8 v) { data[LastViewedBox] = v; }

u32 Sav6::boxOffset(u8 box, u8 slot) const { return Box + 232*30*box + 232*slot; }

u32 Sav6::partyOffset(u8 slot) const { return Party + 260*slot; }

std::shared_ptr<PKX> Sav6::pkm(u8 slot) const
{
    u8 tmp[260];
    std::copy(data + partyOffset(slot), data + partyOffset(slot) + 260, tmp);
    return std::make_shared<PK6>(tmp, true, true);
}

void Sav6::pkm(std::shared_ptr<PKX> pk, u8 slot)
{
    u8 buf[260] = {0};
    std::copy(pk->rawData(), pk->rawData() + pk->getLength(), buf);
    std::unique_ptr<PK6> pk6 = std::make_unique<PK6>(buf, false, true);

    if (pk->getLength() != 260)
    {
        for (int i = 0; i < 6; i++)
        {
            pk6->partyStat(i, pk6->stat(i));
        }
        pk6->partyLevel(pk6->level());
        pk6->partyCurrHP(pk6->stat(0));
    }

    pk6->encrypt();
    std::fill(data + partyOffset(slot), data + partyOffset(slot + 1), (u8)0);
    std::copy(pk6->rawData(), pk6->rawData() + pk6->getLength(), data + partyOffset(slot));
}

std::shared_ptr<PKX> Sav6::pkm(u8 box, u8 slot, bool ekx) const
{
    u8 tmp[232];
    std::copy(data + boxOffset(box, slot), data + boxOffset(box, slot) + 232, tmp);
    return std::make_shared<PK6>(tmp, ekx);
}

void Sav6::pkm(std::shared_ptr<PKX> pk, u8 box, u8 slot)
{
    std::copy(pk->rawData(), pk->rawData() + 232, data + boxOffset(box, slot));
}

void Sav6::cryptBoxData(bool crypted)
{
    for (u8 box = 0; box < boxes; box++)
    {
        for (u8 slot = 0; slot < 30; slot++)
        {
            std::shared_ptr<PKX> pk6 = pkm(box, slot, crypted);
            if (!crypted)
            {
                pk6->encrypt();
            }
            pkm(pk6, box, slot);
        }
    }
}

int Sav6::dexFormIndex(int species, int formct) const
{
    if (formct < 1 || species < 0)
        return -1; // invalid
    
    if (game == Game::ORAS)
    {
        switch (species)
        {
            case 25: return 189; // 7 Pikachu
            case 720: return 196; // 2 Hoopa
            case 15: return 198; // 2 Beedrill
            case 18: return 200; // 2 Pidgeot
            case 80: return 202; // 2 Slowbro
            case 208: return 204; // 2 Steelix
            case 254: return 206; // 2 Sceptile
            case 260: return 208; // 2 Swampert
            case 302: return 210; // 2 Sableye
            case 319: return 212; // 2 Sharpedo
            case 323: return 214; // 2 Camerupt
            case 334: return 216; // 2 Altaria
            case 362: return 218; // 2 Glalie
            case 373: return 220; // 2 Salamence
            case 376: return 222; // 2 Metagross
            case 384: return 224; // 2 Rayquaza
            case 428: return 226; // 2 Lopunny
            case 475: return 228; // 2 Gallade
            case 531: return 230; // 2 Audino
            case 719: return 232; // 2 Diancie
            case 382: return 234; // 2 Kyogre
            case 383: return 236; // 2 Groudon
            case 493: return 238; // 18 Arceus
            case 649: return 256; // 5 Genesect
            case 676: return 261; // 10 Furfrou
        }
    }
    
    switch (species)
    {
        case 666: return 83; // 20 Vivillion
        case 669: return 103; // 5 Flabébé
        case 670: return 108; // 6 Floette
        case 671: return 114; // 5 Florges
        case 710: return 119; // 4 Pumpkaboo
        case 711: return 123; // 4 Gourgeist
        case 681: return 127; // 2 Aegislash
        case 716: return 129; // 2 Xerneas
        case 3: return 131; // 2 Venusaur
        case 6: return 133; // 3 Charizard
        case 9: return 136; // 2 Blastoise
        case 65: return 138; // 2 Alakazam
        case 94: return 140; // 2 Gengar
        case 115: return 142; // 2 Kangaskhan
        case 127: return 144; // 2 Pinsir
        case 130: return 146; // 2 Gyarados
        case 142: return 148; // 2 Aerodactyl
        case 150: return 150; // 3 Mewtwo
        case 181: return 153; // 2 Ampharos
        case 212: return 155; // 2 Scizor
        case 214: return 157; // 2 Heracros
        case 229: return 159; // 2 Houndoom
        case 248: return 161; // 2 Tyranitar
        case 257: return 163; // 2 Blaziken
        case 282: return 165; // 2 Gardevoir
        case 303: return 167; // 2 Mawile
        case 306: return 169; // 2 Aggron
        case 308: return 171; // 2 Medicham
        case 310: return 173; // 2 Manetric
        case 354: return 175; // 2 Banette
        case 359: return 177; // 2 Absol
        case 380: return 179; // 2 Latias
        case 381: return 181; // 2 Latios
        case 445: return 183; // 2 Garchomp
        case 448: return 185; // 2 Lucario
        case 460: return 187; // 2 Abomasnow
        case 646: return 72; // 3 Kyurem
        case 647: return 75; // 2 Keldeo
        case 642: return 77; // 2 Thundurus
        case 641: return 79; // 2 Tornadus
        case 645: return 81; // 2 Landorus
        case 201: return 0; // 28 Unown
        case 386: return 28; // 4 Deoxys
        case 492: return 32; // 2 Shaymin
        case 487: return 34; // 2 Giratina
        case 479: return 36; // 6 Rotom
        case 422: return 42; // 2 Shellos
        case 423: return 44; // 2 Gastrodon
        case 412: return 46; // 3 Burmy
        case 413: return 49; // 3 Wormadam
        case 351: return 52; // 4 Castform
        case 421: return 56; // 2 Cherrim
        case 585: return 58; // 4 Deerling
        case 586: return 62; // 4 Sawsbuck
        case 648: return 66; // 2 Meloetta
        case 555: return 68; // 2 Darmanitan
        case 550: return 70; // 2 Basculin
        default: return -1;
    }
}

void Sav6::dex(std::shared_ptr<PKX> pk)
{
    if (pk->species() == 0)
        return;
    if (pk->species() > 721)
        return;

    const int brSize = 0x60;
    int bit = pk->species() - 1;
    int lang = pk->language() - 1; if (lang > 5) lang--; // 0-6 language vals
    int origin = pk->version();
    int gender = pk->gender() % 2; // genderless -> male
    int shiny = pk->shiny() ? 1 : 0;
    int shiftoff = brSize*(1 + gender + 2*shiny); // after the Owned region
    int bd = bit >> 3; // div8
    int bm = bit & 7; // mod8
    u8 mask = (u8)(1 << bm);
    int ofs = PokeDex + 0x8 + bd;

    // Owned quality flag
    if (origin < 0x18 && bit < 649 && game != Game::ORAS) // Species: 1-649 for X/Y, and not for ORAS; Set the Foreign Owned Flag
        data[ofs + 0x644] |= mask;
    else if (origin >= 0x18 || game == Game::ORAS) // Set Native Owned Flag (should always happen)
        data[ofs + (brSize * 0)] |= mask;

    // Set the [Species/Gender/Shiny] Seen Flag
    data[ofs + shiftoff] |= mask;

    // Set the Display flag if none are set
    bool displayed = false;
    displayed |= (data[ofs + brSize * 5] & mask) != 0;
    displayed |= (data[ofs + brSize * 6] & mask) != 0;
    displayed |= (data[ofs + brSize * 7] & mask) != 0;
    displayed |= (data[ofs + brSize * 8] & mask) != 0;
    if (!displayed) // offset is already biased by brSize, reuse shiftoff but for the display flags.
        data[ofs + brSize * 4 + shiftoff] |= mask;

    // Set the Language
    if (lang < 0) lang = 1;
    data[PokeDexLanguageFlags + (bit * 7 + lang) / 8] |= (u8)(1 << ((bit * 7 + lang) % 8));

    // Set DexNav count (only if not encountered previously)
    if (game == Game::ORAS && *(u16*)(data + EncounterCount + (pk->species() - 1) * 2) == 0)
        *(u16*)(data + EncounterCount + (pk->species() - 1) * 2) = 1;

    // Set Form flags
    int fc = PersonalXYORAS::formCount(pk->species());
    int f = dexFormIndex(pk->species(), fc);
    if (f < 0) return;

    int formLen = game == Game::XY ? 0x18 : 0x26;
    int formDex = PokeDex + 0x8 + brSize*9;
    bit = f + pk->alternativeForm();

    // Set Form Seen Flag
    data[formDex + formLen*shiny + bit/8] |= (u8)(1 << (bit%8));

    // Set Displayed Flag if necessary, check all flags
    for (int i = 0; i < fc; i++)
    {
        bit = f + i;
        if ((data[formDex + formLen*2 + bit/8] & (u8) (1 << (bit%8))) != 0) // Nonshiny
            return; // already set
        if ((data[formDex + formLen*3 + bit/8] & (u8) (1 << (bit%8))) != 0) // Shiny
            return; // already set
    }
    bit = f + pk->alternativeForm();
    data[formDex + formLen * (2 + shiny) + bit / 8] |= (u8)(1 << (bit % 8));
}

void Sav6::mysteryGift(WCX& wc, int& pos)
{
    WC6* wc6 = (WC6*)&wc;
    *(u8*)(data + WondercardFlags + wc6->ID()/8) |= 0x1 << (wc6->ID() % 8);
    std::copy(wc6->rawData(), wc6->rawData() + 264, data + WondercardData + 264*pos);
    pos = (pos + 1) % 24;
}

std::string Sav6::boxName(u8 box) const { return StringUtils::transString67(StringUtils::getString(data, PCLayout + 0x22*box, 17)); }

void Sav6::boxName(u8 box, const std::string& name)
{
    StringUtils::setString(data, StringUtils::transString67(name), PCLayout + 0x22*box, 17);
}

u8 Sav6::partyCount(void) const { return data[Party + 6*260]; }
void Sav6::partyCount(u8 v) { data[Party + 6*260] = v; }

std::shared_ptr<PKX> Sav6::emptyPkm() const
{
    static auto empty = std::make_shared<PK6>();
    return empty;
}

int Sav6::emptyGiftLocation(void) const
{
    u8 t;
    bool empty;
    // 24 max wonder cards
    for (t = 0; t < 24; t++)
    {
        empty = true;
        for (u32 j = 0; j < WC6::length; j++)
        {
            if (*(data + WondercardData + t * WC6::length + j) != 0)
            {
                empty = false;
                break;
            }
        }

        if (empty)
        {
            break;
        }
    }

    return !empty ? 23 : t;
}

std::vector<MysteryGift::giftData> Sav6::currentGifts(void) const
{
    std::vector<MysteryGift::giftData> ret;
    u8* wonderCards = data + WondercardData;
    for (int i = 0; i < emptyGiftLocation(); i++)
    {
        if (*(wonderCards + i * WC6::length + 0x51) == 0)
        {
            ret.push_back({ StringUtils::getString(wonderCards + i * WC6::length, 0x2, 36), "", *(u16*)(wonderCards + i * WC6::length + 0x82), *(wonderCards + i * WC6::length + 0x84), *(wonderCards + i * WC6::length + 0xA1)});
        }
        else
        {
            ret.push_back({ StringUtils::getString(wonderCards + i * WC6::length, 0x2, 36), "", -1, -1, -1 });
        }
    }
    return ret;
}

std::unique_ptr<WCX> Sav6::mysteryGift(int pos) const
{
    return std::make_unique<WC6>(data + WondercardData + pos * WC6::length);
}

void Sav6::item(Item& item, Pouch pouch, u16 slot)
{
    Item6 inject = (Item6) item;
    auto write = inject.bytes();
    switch (pouch)
    {
        case NormalItem:
            std::copy(write.first, write.first + write.second, data + PouchHeldItem + slot * 4);
            break;
        case KeyItem:
            std::copy(write.first, write.first + write.second, data + PouchKeyItem + slot * 4);
            break;
        case TM:
            std::copy(write.first, write.first + write.second, data + PouchTMHM + slot * 4);
            break;
        case Medicine:
            std::copy(write.first, write.first + write.second, data + PouchMedicine + slot * 4);
            break;
        case Berry:
            std::copy(write.first, write.first + write.second, data + PouchBerry + slot * 4);
            break;
        default:
            return;
    }
}

std::unique_ptr<Item> Sav6::item(Pouch pouch, u16 slot) const
{
    switch (pouch)
    {
        case NormalItem:
            return std::make_unique<Item6>(data + PouchHeldItem + slot * 4);
        case KeyItem:
            return std::make_unique<Item6>(data + PouchKeyItem + slot * 4);
        case TM:
            return std::make_unique<Item6>(data + PouchTMHM + slot * 4);
        case Medicine:
            return std::make_unique<Item6>(data + PouchMedicine + slot * 4);
        case Berry:
            return std::make_unique<Item6>(data + PouchBerry + slot * 4);
        default:
            return nullptr;
    }
}

std::vector<std::pair<Pouch, int>> Sav6::pouches(void) const
{
    return {
        { NormalItem, game == Game::XY ? 286 : 305 },
        { KeyItem, game == Game::XY ? 31 : 47 },
        { TM, game == Game::XY ? 105 : 107 },
        { Medicine, game == Game::XY ? 51 : 54 },
        { Berry, 67 }
    };
}

std::string Sav6::pouchName(Pouch pouch) const
{
    switch (pouch)
    {
        case NormalItem:
            return i18n::localize("ITEMS");
        case KeyItem:
            return i18n::localize("KEY_ITEMS");
        case TM:
            return i18n::localize("TMHM");
        case Medicine:
            return i18n::localize("MEDICINE");
        case Berry:
            return i18n::localize("BERRIES");
        default:
            return "";
    }
}
