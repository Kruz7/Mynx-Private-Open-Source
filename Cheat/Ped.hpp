struct Bones {
	Vector3 head;
	char padding0[0x4]{};
	Vector3 leftFoot;
	char padding1[0x4]{};
	Vector3 rightFoot;
	char padding2[0x4]{};
	Vector3 leftAnkle;
	char padding3[0x4]{};
	Vector3 rightAnkle;
	char padding4[0x4]{};
	Vector3 leftHand;
	char padding5[0x4]{};
	Vector3 rightHand;
	char padding6[0x4]{};
	Vector3 neck;
	char padding7[0x4]{};
	Vector3 hip;
};

enum Bone : int {
	Head,
	Neck,
	Hip,
	LeftHand,
	RightHand,
	LeftUpperArm,
	RightUpperArm,
	LeftForearm,
	RightForearm,
	LeftFoot,
	RightFoot,
	LeftAnkle,
	RightAnkle,
	LeftThigh,
	RightThigh,
	Spine0,
	Spine1,
	Spine2,
	Spine3,
	SpineRoot,
	LeftClavicle,
	RightClavicle,
	Spine4,
	Spine5,
	LeftToe,
	RightToe,
	Jaw,
	Chest,
	Neck0,
	Neck2,
	Root,
	LeftElbow,
	RightElbow,
	LeftKnee,
	RightKnee
};

class Ped {
public:
	uintptr_t pointer;
	uintptr_t playerInfo;
	uintptr_t weaponManager;
	uintptr_t currentWeapon;

	float armor;
	float health;
	Vector3 position;
	Matrix boneMatrix;
	Vector3 boneList[35]{};

	bool GetPlayer(uintptr_t& base) {
		pointer = base;
		return pointer != NULL;
	}

	std::string GetWeaponName() {
		uintptr_t weaponManager = ReadMemory<uintptr_t>(pointer + Offsets.WeaponManager);
		uintptr_t step1 = ReadMemory<uintptr_t>(weaponManager + 0x20);
		return ReadString(ReadMemory<uintptr_t>(step1 + 0x5F0));
	}

	uintptr_t GetWeapon() {
		return ReadMemory<uintptr_t>(ReadMemory<uintptr_t>(ReadMemory<uintptr_t>(pointer + Offsets.WeaponManager) + 0x20) + 0x10);
	}

	int GetId() {
		return ReadMemory<int>(ReadMemory<uint64_t>(pointer + Offsets.PlayerInfo) + Offsets.Id);
	}

	std::string GetDiscordId() {
		if (!playerInfo) return "";
		
		std::string discordStr = ReadString(playerInfo + 0x40);
		if (!discordStr.empty() && discordStr != "0" && discordStr.length() > 5) return discordStr;
		
		uint64_t discordId = ReadMemory<uint64_t>(playerInfo + 0x48);
		if (discordId != 0 && discordId < 1000000000000000000ULL) return std::to_string(discordId);
		
		discordId = ReadMemory<uint64_t>(playerInfo + 0x50);
		if (discordId != 0 && discordId < 1000000000000000000ULL) return std::to_string(discordId);
		
		discordStr = ReadString(playerInfo + 0x58);
		if (!discordStr.empty() && discordStr != "0" && discordStr.length() > 5) return discordStr;
		
		if (!Offsets.CitizenPlayernamesBase) return "";
		
		uint64_t playerNames = ReadMemory<uint64_t>(Offsets.CitizenPlayernamesBase + 0x30D38);
		if (!playerNames) return "";

		int playerId = GetId();
		uintptr_t list = ReadMemory<uintptr_t>(playerNames + 0x8);
		for (uint32_t i = 0; i < 500; ++i) {
			if (!list) break;

			if (playerId == ReadMemory<int>(list + 0x10)) {
				discordStr = ReadString(list + 0x20);
				if (!discordStr.empty() && discordStr != "0" && discordStr.length() > 5) return discordStr;
				
				discordId = ReadMemory<uint64_t>(list + 0x28);
				if (discordId != 0 && discordId < 1000000000000000000ULL) return std::to_string(discordId);
				
				discordId = ReadMemory<uint64_t>(list + 0x30);
				if (discordId != 0 && discordId < 1000000000000000000ULL) return std::to_string(discordId);
			}
			list = ReadMemory<uintptr_t>(list + 0x8);
		}
		return "";
	}

	std::string GetSteamId() {
		if (!playerInfo) return "";
		
		std::string steamStr = ReadString(playerInfo + 0x60);
		if (!steamStr.empty() && steamStr != "0" && steamStr.length() > 5) return steamStr;
		
		uint64_t steamId = ReadMemory<uint64_t>(playerInfo + 0x68);
		if (steamId != 0 && steamId < 1000000000000000000ULL) return std::to_string(steamId);
		
		steamId = ReadMemory<uint64_t>(playerInfo + 0x70);
		if (steamId != 0 && steamId < 1000000000000000000ULL) return std::to_string(steamId);
		
		steamStr = ReadString(playerInfo + 0x78);
		if (!steamStr.empty() && steamStr != "0" && steamStr.length() > 5) return steamStr;
		
		if (!Offsets.CitizenPlayernamesBase) return "";
		
		uint64_t playerNames = ReadMemory<uint64_t>(Offsets.CitizenPlayernamesBase + 0x30D38);
		if (!playerNames) return "";

		int playerId = GetId();
		uintptr_t list = ReadMemory<uintptr_t>(playerNames + 0x8);
		for (uint32_t i = 0; i < 500; ++i) {
			if (!list) break;

			if (playerId == ReadMemory<int>(list + 0x10)) {
				steamStr = ReadString(list + 0x40);
				if (!steamStr.empty() && steamStr != "0" && steamStr.length() > 5) return steamStr;
				
				steamId = ReadMemory<uint64_t>(list + 0x48);
				if (steamId != 0 && steamId < 1000000000000000000ULL) return std::to_string(steamId);
				
				steamId = ReadMemory<uint64_t>(list + 0x50);
				if (steamId != 0 && steamId < 1000000000000000000ULL) return std::to_string(steamId);
			}
			list = ReadMemory<uintptr_t>(list + 0x8);
		}
		return "";
	}

	uint32_t GetModelHash() {
		uintptr_t modelInfo = ReadMemory<uintptr_t>(pointer + 0x20);
		if (!modelInfo) return 0;
		return ReadMemory<uint32_t>(modelInfo + 0x18);
	}

	bool IsPlayer() const {
		return playerInfo != NULL;
	}

	bool IsDead() const {
		return position == Vector3(0.f, 0.f, 0.f);
	}

	bool IsVisible() {
		enum VisibilityFlags : BYTE {
			InvisibleFlag1 = 36,
			InvisibleFlag2 = 0,
			InvisibleFlag3 = 4,
			IntersectMissionEntityAndTrain = 2,
			IntersectPeds1 = 4,
			IntersectVehicles = 10,
			IntersectVegetation = 256,
			FrustumCulling = 512,
			OcclusionCulling = 1024,
			DistanceCulling = 2048
		};

		BYTE VisibilityFlag = ReadMemory<BYTE>(pointer + Offsets.VisibleFlag);
		if (VisibilityFlag == InvisibleFlag1 ||
			VisibilityFlag == InvisibleFlag2 ||
			VisibilityFlag == InvisibleFlag3 ||
			VisibilityFlag == IntersectMissionEntityAndTrain ||
			VisibilityFlag == IntersectPeds1 ||
			VisibilityFlag == IntersectVehicles ||
			VisibilityFlag == IntersectVegetation ||
			(VisibilityFlag & FrustumCulling) ||
			(VisibilityFlag & OcclusionCulling) ||
			(VisibilityFlag & DistanceCulling))
		{
			return false;
		}

		return true;
	}

	bool Update() {
		playerInfo = ReadMemory<uintptr_t>(pointer + Offsets.PlayerInfo);
		currentWeapon = GetWeapon();
		health = ReadMemory<float>(pointer + Offsets.Health);
		position = ReadMemory<Vector3>(pointer + 0x90);
		armor = ReadMemory<float>(pointer + Offsets.Armor);
		boneMatrix = ReadMemory<Matrix>(pointer + 0x60);
		UpdateBones();
		return true;
	}

	void UpdateBones() {
		Bones bones = ReadMemory<Bones>(pointer + Offsets.BoneList);

		boneList[Head] = Vec3Transform(&bones.head, &boneMatrix);
		boneList[Neck] = Vec3Transform(&bones.neck, &boneMatrix);
		boneList[Hip] = Vec3Transform(&bones.hip, &boneMatrix);

		boneList[LeftHand] = Vec3Transform(&bones.leftHand, &boneMatrix);
		boneList[RightHand] = Vec3Transform(&bones.rightHand, &boneMatrix);
		boneList[LeftUpperArm] = Vec3Transform(&bones.leftHand, &boneMatrix);
		boneList[RightUpperArm] = Vec3Transform(&bones.rightHand, &boneMatrix);
		boneList[LeftForearm] = Vec3Transform(&bones.leftHand, &boneMatrix);
		boneList[RightForearm] = Vec3Transform(&bones.rightHand, &boneMatrix);

		boneList[LeftFoot] = Vec3Transform(&bones.leftFoot, &boneMatrix);
		boneList[RightFoot] = Vec3Transform(&bones.rightFoot, &boneMatrix);
		boneList[LeftAnkle] = Vec3Transform(&bones.leftAnkle, &boneMatrix);
		boneList[RightAnkle] = Vec3Transform(&bones.rightAnkle, &boneMatrix);
		boneList[LeftThigh] = Vec3Transform(&bones.leftAnkle, &boneMatrix);
		boneList[RightThigh] = Vec3Transform(&bones.rightAnkle, &boneMatrix);

		boneList[Spine0] = Vec3Transform(&bones.hip, &boneMatrix);
		boneList[Spine1] = Vec3Transform(&bones.hip, &boneMatrix);
		boneList[Spine2] = Vec3Transform(&bones.neck, &boneMatrix);
		boneList[Spine3] = Vec3Transform(&bones.neck, &boneMatrix);
		boneList[SpineRoot] = Vec3Transform(&bones.hip, &boneMatrix);

		boneList[LeftClavicle] = Vec3Transform(&bones.leftHand, &boneMatrix);
		boneList[RightClavicle] = Vec3Transform(&bones.rightHand, &boneMatrix);

		boneList[Spine4] = Vec3Transform(&bones.neck, &boneMatrix);
		boneList[Spine5] = Vec3Transform(&bones.neck, &boneMatrix);
		boneList[LeftToe] = Vec3Transform(&bones.leftFoot, &boneMatrix);
		boneList[RightToe] = Vec3Transform(&bones.rightFoot, &boneMatrix);
		boneList[Jaw] = Vec3Transform(&bones.head, &boneMatrix);
		boneList[Chest] = Vec3Transform(&bones.neck, &boneMatrix);
		boneList[Neck0] = Vec3Transform(&bones.neck, &boneMatrix);
		boneList[Neck2] = Vec3Transform(&bones.neck, &boneMatrix);
		boneList[Root] = Vec3Transform(&bones.hip, &boneMatrix);

		Vector3 leftElbowPos = Vector3(
			(bones.leftHand.x + bones.neck.x) / 2.0f,
			(bones.leftHand.y + bones.neck.y) / 2.0f,
			(bones.leftHand.z + bones.neck.z) / 2.0f
		);
		boneList[LeftElbow] = Vec3Transform(&leftElbowPos, &boneMatrix);
		
		Vector3 rightElbowPos = Vector3(
			(bones.rightHand.x + bones.neck.x) / 2.0f,
			(bones.rightHand.y + bones.neck.y) / 2.0f,
			(bones.rightHand.z + bones.neck.z) / 2.0f
		);
		boneList[RightElbow] = Vec3Transform(&rightElbowPos, &boneMatrix);
		
		Vector3 leftKneePos = Vector3(
			(bones.leftFoot.x + bones.hip.x) / 2.0f,
			(bones.leftFoot.y + bones.hip.y) / 2.0f,
			(bones.leftFoot.z + bones.hip.z) / 2.0f
		);
		boneList[LeftKnee] = Vec3Transform(&leftKneePos, &boneMatrix);
		
		Vector3 rightKneePos = Vector3(
			(bones.rightFoot.x + bones.hip.x) / 2.0f,
			(bones.rightFoot.y + bones.hip.y) / 2.0f,
			(bones.rightFoot.z + bones.hip.z) / 2.0f
		);
		boneList[RightKnee] = Vec3Transform(&rightKneePos, &boneMatrix);
	}
};
