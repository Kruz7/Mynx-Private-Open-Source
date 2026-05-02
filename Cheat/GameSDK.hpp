bool Vec3Empty(const Vector3& value) {
	return value == Vector3(0.f, 0.f, 0.f);
}

Vector3 Vec3Transform(Vector3* vIn, Matrix* mIn) {
	Vector3 vOut{};
	vOut.x = vIn->x * mIn->_11 + vIn->y * mIn->_21 + vIn->z * mIn->_31 + 1.f * mIn->_41;
	vOut.y = vIn->x * mIn->_12 + vIn->y * mIn->_22 + vIn->z * mIn->_32 + 1.f * mIn->_42;
	vOut.z = vIn->x * mIn->_13 + vIn->y * mIn->_23 + vIn->z * mIn->_33 + 1.f * mIn->_43;
	return vOut;
};

void NormalizeAngles(Vector3& angle) {
	while (angle.x > 89.0f)
		angle.x -= 180.f;

	while (angle.x < -89.0f)
		angle.x += 180.f;

	while (angle.y > 180.f)
		angle.y -= 360.f;

	while (angle.y < -180.f)
		angle.y += 360.f;
}

float GetDistance(Vector3 value1, Vector3 value2) {
	float num = value1.x - value2.x;
	float num2 = value1.y - value2.y;
	float num3 = value1.z - value2.z;
	return sqrt(num * num + num2 * num2 + num3 * num3);
}

Vector3 CalcAngle(Vector3 localCam, Vector3 toPoint) {
	Vector3 vOut{};
	float distance = GetDistance(localCam, toPoint);
	vOut.x = (toPoint.x - localCam.x) / distance;
	vOut.y = (toPoint.y - localCam.y) / distance;
	vOut.z = (toPoint.z - localCam.z) / distance;
	return vOut;
}

bool WorldToScreen(const Matrix& viewMatrix, const Vector3& vWorld, Vector2& vOut) {
    Matrix v = viewMatrix.Transpose();
    Vector4 row2 = Vector4(v._21, v._22, v._23, v._24);
    Vector4 row3 = Vector4(v._31, v._32, v._33, v._34);
    Vector4 row4 = Vector4(v._41, v._42, v._43, v._44);

    Vector3 proj;
    proj.x = (row2.x * vWorld.x) + (row2.y * vWorld.y) + (row2.z * vWorld.z) + row2.w;
    proj.y = (row3.x * vWorld.x) + (row3.y * vWorld.y) + (row3.z * vWorld.z) + row3.w;
    proj.z = (row4.x * vWorld.x) + (row4.y * vWorld.y) + (row4.z * vWorld.z) + row4.w;
    if (proj.z <= 0.1f)
        return false;

    float invZ = 1.0f / proj.z;
    proj.x *= invZ;
    proj.y *= invZ;

    float screenW = static_cast<float>(Game.lpRect.right);
    float screenH = static_cast<float>(Game.lpRect.bottom);
    float halfW = screenW * 0.5f;
    float halfH = screenH * 0.5f;
    vOut.x = halfW + (0.5f * proj.x * screenW);
    vOut.y = halfH - (0.5f * proj.y * screenH);
    return true;
}

static std::string GetWeaponName(DWORD hash) {
    switch (hash) {
    case 0x92A27487: return XorString("Dagger");
    case 0x958A4A8F: return XorString("Bat");
    case 0xF9E6AA4B: return XorString("Bottle");
    case 0x84BD7BFD: return XorString("Crowbar");
    case 0x8BB05FD7: return XorString("Flashlight");
    case 0x440E4788: return XorString("GolfClub");
    case 0x4E875F73: return XorString("Hammer");
    case 0xF9DCBF2D: return XorString("Hatchet");
    case 0xD8DF3C3C: return XorString("Knuckle");
    case 0x99B507EA: return XorString("Knife");
    case 0xDD5DF8D9: return XorString("Machete");
    case 0xDFE37640: return XorString("Switchblade");
    case 0x678B81B1: return XorString("Nightstick");
    case 0x19044EE0: return XorString("Wrench");
    case 0xCD274149: return XorString("BattleAxe");
    case 0x94117305: return XorString("PoolCue");
    case 0x3813FC08: return XorString("StoneHatchet");
    case 0x1B06D571: return XorString("Pistol");
    case 0xBFE256D4: return XorString("Pistol Mk2");
    case 0x5EF9FEC4: return XorString("Combat Pistol");
    case 0x22D8FE39: return XorString("APPistol");
    case 0x3656C8C1: return XorString("Stun Gun");
    case 0x99AEEB3B: return XorString("Pistol 50");
    case 0xBFD21232: return XorString("SNS Pistol");
    case 0x88374054: return XorString("SNS Pistol Mk2");
    case 0xD205520E: return XorString("Heavy Pistol");
    case 0x83839C4: return XorString("Vintage Pistol");
    case 0x47757124: return XorString("Flare Gun");
    case 0xDC4DB296: return XorString("Marksman Pistol");
    case 0xC1B3C3D1: return XorString("Revolver");
    case 0xCB96392F: return XorString("Revolver Mk2");
    case 0x97EA20B8: return XorString("Double Action");
    case 0xAF3696A1: return XorString("Ray Pistol");
    case 0x2B5EF5EC: return XorString("Ceramic Pistol");
    case 0x917F6C8C: return XorString("Navy Revolver");
    case 0x13532244: return XorString("Micro SMG");
    case 0x2BE6766B: return XorString("SMG");
    case 0x78A97CD0: return XorString("SMG Mk2");
    case 0xEFE7E2DF: return XorString("Assault SMG");
    case 0xA3D4D34: return XorString("Combat PDW");
    case 0xDB1AA450: return XorString("Machine Pistol");
    case 0xBD248B55: return XorString("Mini SMG");
    case 0x476BF155: return XorString("Ray Carbine");
    case 0x1D073A89: return XorString("Pump Shotgun");
    case 0x555AF99A: return XorString("Pump Shotgun Mk2");
    case 0x7846A318: return XorString("Sawnoff Shotgun");
    case 0xE284C527: return XorString("Assault Shotgun");
    case 0x9D61E50F: return XorString("Bullpup Shotgun");
    case 0xA89CB99E: return XorString("Musket");
    case 0x3AABBBAA: return XorString("Heavy Shotgun");
    case 0xBFEFFF6D: return XorString("Assault Rifle");
    case 0x394F415C: return XorString("Assault Rifle Mk2");
    case 0x83BF0278: return XorString("Carbine Rifle");
    case 0xFAD1F1C9: return XorString("Carbine Rifle Mk2");
    case 0xAF113F99: return XorString("Advanced Rifle");
    case 0xC0A3098D: return XorString("Special Carbine");
    case 0x969C3D67: return XorString("Special Carbine Mk2");
    case 0x7F229F94: return XorString("Bullpup Rifle");
    case 0x84D6FAFD: return XorString("Bullpup Rifle Mk2");
    case 0x624FE830: return XorString("Compact Rifle");
    case 0x9D07F764: return XorString("MG");
    case 0x7FD62962: return XorString("Combat MG");
    case 0xDBBD7280: return XorString("Combat MG Mk2");
    case 0x61012683: return XorString("Gusenberg");
    case 0x5FC3C11: return XorString("Sniper Rifle");
    case 0xC472FE2: return XorString("Heavy Sniper");
    case 0xA914799: return XorString("Heavy Sniper Mk2");
    case 0xC734385A: return XorString("Marksman Rifle");
    case 0x6A6C02E0: return XorString("Marksman Rifle Mk2");
    case 0xB1CA77B1: return XorString("RPG");
    case 0xA284510B: return XorString("Grenade Launcher");
    case 0x42BF8A85: return XorString("Minigun");
    case 0x6D544C99: return XorString("Railgun");
    case 0x63AB0442: return XorString("Homing Launcher");
    case 0x781FE4A: return XorString("Compact Grenade Launcher");
    case 0x93E220BD: return XorString("Grenade");
    case 0x24B17070: return XorString("Molotov");
    case 0x2C3731D9: return XorString("Sticky Bomb");
    case 0xAB564B93: return XorString("Proximity Mine");
    case 0xBA45E8B8: return XorString("Pipe Bomb");
    case 0x34A67B97: return XorString("Petrol Can");
    case 0xFBAB5776: return XorString("Parachute");
    case 0x60EC506: return XorString("Fire Extinguisher");
    default: return XorString("Unknown");
    }
}

static std::string GetVehicleName(DWORD hash) {
    switch (hash) {
    case 0x408: return XorString("vacca");
    case 0x800408: return XorString("Gauntlet");
    case 0x817BFA8: return XorString("Gauntlet");
    case 0x820A0: return XorString("Turismo");
    case 0x40820A0: return XorString("Lambo");
    case 0x94B395C5: return XorString("frogger");
    case 0x2B33F9E1: return XorString("Airtug");
    case 0x45D56ADA: return XorString("Akula");
    case 0x83B6C016: return XorString("Akuma");
    case 0x52442435: return XorString("Alpha");
    case 0x8B213907: return XorString("Ambulance");
    case 0x95F4C618: return XorString("Annihilator");
    case 0x431FC24C: return XorString("Apc");
    case 0x2BEF99CB: return XorString("Ardent");
    case 0x97E55D11: return XorString("Asea");
    case 0x94204D89: return XorString("Asterope");
    case 0x8E9254FB: return XorString("Autarch");
    case 0x9B909C94: return XorString("Avarus");
    case 0x81BD9ED1: return XorString("Bagger");
    case 0x34B1680B: return XorString("Baller");
    case 0xCFCA3668: return XorString("Baller2");
    case 0x6FF0F727: return XorString("Banshee");
    case 0x25C5AF13: return XorString("Banshee2");
    case 0x1C5ABD4E: return XorString("Barracks");
    case 0x4008EABB: return XorString("Bati");
    case 0xF9300CC5: return XorString("Bati2");
    case 0x432AA566: return XorString("Benson");
    case 0x82B66778: return XorString("Besra");
    case 0x4C80EB0E: return XorString("BestiaGTS");
    case 0x6BD1C6AB: return XorString("BF400");
    case 0x52832650: return XorString("BfInjection");
    case 0x32B91AF8: return XorString("Biff");
    case 0xEB298297: return XorString("Bifta");
    case 0x810369E2: return XorString("Bison");
    case 0x7AF8F4F1: return XorString("Blade");
    case 0xB820ED5E: return XorString("Blazer");
    case 0xFD231729: return XorString("Blazer2");
    case 0x431D501C: return XorString("Blazer3");
    case 0xC5EFC0B3: return XorString("Blazer4");
    case 0x81AB707A: return XorString("Blazer5");
    case 0x44F314E9: return XorString("Blimp");
    case 0xDB0956DD: return XorString("Blimp2");
    case 0xEB70965F: return XorString("Blista");
    case 0x3DEE5EDA: return XorString("Blista2");
    case 0xDCBC1C3B: return XorString("Blista3");
    case 0x1FD824AF: return XorString("BMX");
    case 0x4BFCF28B: return XorString("BoatTrailer");
    case 0x5BED015C: return XorString("BobcatXL");
    case 0x32F99946: return XorString("Bodhi2");
    case 0x68FF10EB: return XorString("Bombushka");
    case 0x7C9A6684: return XorString("Boxville");
    case 0x821A2D5C: return XorString("Boxville2");
    case 0x617AEC4E: return XorString("Boxville3");
    case 0x1A79847A: return XorString("Boxville4");
    case 0x28AD20E1: return XorString("Boxville5");
    case 0x898ECCEA: return XorString("Brawler");
    case 0x9C669788: return XorString("Brioso");
    case 0x5C55CB39: return XorString("Btype");
    case 0xCE6B35A4: return XorString("Btype2");
    case 0xDC19D101: return XorString("Btype3");
    case 0x2C634FBD: return XorString("Buccaneer");
    case 0xC34F768D: return XorString("Buccaneer2");
    case 0xED7EADA4: return XorString("Buffalo");
    case 0x2BEC3CBE: return XorString("Buffalo2");
    case 0xE2C013E: return XorString("Buffalo3");
    case 0x7074B39D: return XorString("Bullet");
    case 0x9AE6DDA1: return XorString("Bullet2");
    case 0xAF44F260: return XorString("Burrito");
    case 0xC9E8FF76: return XorString("Burrito2");
    case 0x98171BD3: return XorString("Burrito3");
    case 0x353B561D: return XorString("Burrito4");
    case 0x437CF2A0: return XorString("Burrito5");
    case 0xD577C962: return XorString("Bus");
    case 0x2F03547B: return XorString("Buzzard");
    case 0x2C75F497: return XorString("Buzzard2");
    case 0x7F17D0C7: return XorString("Caddy");
    case 0x6F4E9B0C: return XorString("Caddy2");
    case 0x703106A: return XorString("Caddy3");
    case 0x7B8AB45F: return XorString("Carbonizzare");
    case 0x779F23AA: return XorString("Cargobob");
    case 0x60A7EA10: return XorString("Cargobob2");
    case 0x53174EEF: return XorString("Cargobob3");
    case 0x78BC1A3C: return XorString("Cargobob4");
    case 0x15F27762: return XorString("Cargoplane");
    case 0x3822BDFE: return XorString("Casco");
    case 0xD0EB2BE5: return XorString("Cavalcade2");
    case 0x7896E8C3: return XorString("Cheetah");
    case 0xB1D95DA0: return XorString("Cheetah2");
    case 0xD4E5F4D: return XorString("Chimera");
    case 0x6750AF3F: return XorString("Chino");
    case 0xAED64A63: return XorString("Chino2");
    case 0x14D69010: return XorString("Cliffhanger");
    case 0x1B43E495: return XorString("Cognoscenti");
    case 0xDBF2D57B: return XorString("Cognoscenti2");
    case 0x86B0E0A2: return XorString("Comet2");
    case 0x69F5B1BC: return XorString("Comet3");
    case 0x1F0466C7: return XorString("Comet4");
    case 0x3EAB5555: return XorString("Comet5");
    case 0x132D5A1A: return XorString("Coquette");
    case 0x3C4E2113: return XorString("Coquette2");
    case 0x2EC385FE: return XorString("Coquette3");
    case 0x1A48E3E0: return XorString("Cruiser");
    case 0x9D1B9E21: return XorString("Crusader");
    case 0xAC4E93C9: return XorString("Daemon2");
    case 0x4FF77E37: return XorString("Deluxo");
    case 0x586765FB: return XorString("Deveste");
    case 0x5EE005DA: return XorString("Deviant");
    case 0xAC425DF: return XorString("Diablous");
    case 0x6B65F5F2: return XorString("Diablous2");
    case 0xBC993509: return XorString("Dilettante2");
    case 0x107F392C: return XorString("Dinghy2");
    case 0x1E5E54BE: return XorString("Dinghy3");
    case 0x33B9F484: return XorString("Dinghy4");
    case 0xC96E73CE: return XorString("Dominator2");
    case 0xC52C6B93: return XorString("Dominator3");
    case 0xE882E57F: return XorString("Dubsta2");
    case 0xB6410173: return XorString("Dubsta3");
    case 0x462FE277: return XorString("Dukes");
    case 0xEC0F4C8: return XorString("Dukes2");
    case 0x2B26F456: return XorString("Dump");
    case 0x53425886: return XorString("Dune2");
    case 0xDE3D9D22: return XorString("Elegy2");
    case 0x8FC3AADC: return XorString("Emperor2");
    case 0xB5FCF74E: return XorString("Emperor3");
    case 0x8198AEDC: return XorString("Entity2");
    case 0x8A213119: return XorString("Faction2");
    case 0x6B9F2075: return XorString("Faction3");
    case 0x350A1B99: return XorString("Faggio2");
    case 0xB328B188: return XorString("Faggio3");
    case 0x432EA949: return XorString("FBI2");
    case 0x256AB97F: return XorString("FCR2");
    case 0xBC32A33B: return XorString("Feltzer3");
    case 0x742E9AC0: return XorString("Frogger2");
    case 0x27F792A7: return XorString("Hakuchou2");
    case 0x7B7BE9F3: return XorString("Insurgent2");
    case 0x9114EADA: return XorString("Insurgent3");
    case 0xE33A477B: return XorString("ItaliGTB2");
    case 0xBE0E6126: return XorString("Jester2");
    case 0x187D938D: return XorString("Kuruma2");
    default: {
        char hashStr[64];
        sprintf_s(hashStr, sizeof(hashStr), "0x%X", hash);
        return std::string(hashStr);
    }
    }
}