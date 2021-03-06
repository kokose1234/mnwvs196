#include "ItemInfo.h"
#include "Wz\WzResMan.hpp"
#include "..\Database\GW_ItemSlotBase.h"

ItemInfo::ItemInfo()
{
}


ItemInfo::~ItemInfo()
{
}

ItemInfo * ItemInfo::GetInstance()
{
	static ItemInfo* pInstance = new ItemInfo;
	return pInstance;
}

void ItemInfo::Initialize()
{
	IterateMapString();
	printf("[ItemInfo::Initialize]開始載入所有物品名稱[IterateItemString Start]....\n");
	IterateItemString(nullptr);
	printf("[ItemInfo::Initialize]物品名稱載入完成[IterateItemString Done]....\n");

	static auto& eqpWz = stWzResMan->GetWz(Wz::Character);
	printf("[ItemInfo::Initialize]開始載入所有裝備[IterateEquipItem Start]....\n");
	IterateEquipItem(&eqpWz);
	printf("[ItemInfo::Initialize]裝備載入完成[IterateEquipItem Done]....\n");

	printf("[ItemInfo::Initialize]開始載入所有物品[IterateBundleItem Start]....\n");
	IterateBundleItem();
	printf("[ItemInfo::Initialize]物品載入完成[IterateBundleItem Done]....\n");
	IterateCashItem();
	RegisterSpecificItems();
	RegisterNoRollbackItem();
	RegisterSetHalloweenItem();
	stWzResMan->ReleaseMemory();
	printf("[ItemInfo::Initialize]釋放ItemInfo所有Wz記憶體[ReleaseMemory Done]....\n");
}

void ItemInfo::IterateMapString()
{
}

void ItemInfo::IterateItemString(void *dataNode)
{
	static WZ::Node Img[] = { stWzResMan->GetWz(Wz::String)["Eqp"]
		, stWzResMan->GetWz(Wz::String)["Etc"]
		, stWzResMan->GetWz(Wz::String)["Consume"]
		, stWzResMan->GetWz(Wz::String)["Ins"] };
	if (dataNode == nullptr)
		for (auto& img : Img)
			IterateItemString((void*)&img);
	else
	{
		auto& dataImg = (*((WZ::Node*)dataNode));
		for (auto& img : dataImg)
		{
			if (!isdigit(img.Name()[0]) || atoi(img.Name().c_str()) < 1000)
				IterateItemString((void*)&img);
			else
				m_mItemString[atoi(img.Name().c_str())] = img["name"];
		}
	}
}

void ItemInfo::IterateEquipItem(void *dataNode)
{
	auto& dataImg = (*((WZ::Node*)dataNode));
	int nItemID = 0;
	
	for (auto& data : dataImg)
	{
		if (!isdigit(data.Name()[0])) //展開資料夾
			IterateEquipItem((void*)(&data));
		else
		{
			nItemID = atoi(data.Name().c_str());
			if (nItemID < 1000)
				continue;
			ItemInfo::EquipItem* pNewEquip = new ItemInfo::EquipItem();
			pNewEquip->nItemID = nItemID;
			pNewEquip->sItemName = m_mItemString[nItemID];
			RegisterEquipItemInfo(pNewEquip, nItemID, (void*)&(data));
			m_mEquipItem[nItemID] = pNewEquip;
		}
	}
}

void ItemInfo::IterateBundleItem()
{
	static WZ::Node Img[] = { stWzResMan->GetWz(Wz::Item)["Cash"]
		, stWzResMan->GetWz(Wz::Item)["Consume"]
		, stWzResMan->GetWz(Wz::Item)["Etc"]
		, stWzResMan->GetWz(Wz::Item)["Install"] };
	for (auto& baseImg : Img)
	{
		for (auto& dir : baseImg)
		{
			for (auto& item : dir)
			{
				auto& infoImg = item["info"];
				int nItemID = atoi(item.Name().c_str());
				ItemInfo::BundleItem* pNewBundle = new ItemInfo::BundleItem;
				LoadAbilityStat(pNewBundle->abilityStat, (void*)&infoImg);
				pNewBundle->nItemID = nItemID;
				pNewBundle->sItemName = m_mItemString[nItemID];
				pNewBundle->nMaxPerSlot = infoImg["slotMax"];
				pNewBundle->dSellUnitPrice = (double)infoImg["unitPrice"];
				pNewBundle->nSellPrice = infoImg["price"];
				pNewBundle->nRequiredLEV = infoImg["reqLevel"];
				pNewBundle->nPAD = infoImg["incPAD"]; //飛鏢
				m_mBundleItem[nItemID] = pNewBundle;
				int nItemCategory = nItemID / 10000;
				void* pProp = (void*)&item;
				switch (nItemCategory)
				{
				case 200:
				case 201:
				case 202:
				case 205:
				case 221:
					RegisterStateChangeItem(nItemID, pProp);
					break;
				case 204:
					RegisterUpgradeItem(nItemID, pProp);
					break;
				case 203:
					RegisterPortalScrollItem(nItemID, pProp);
					break;
				case 210:
					RegisterMobSummonItem(nItemID, pProp);
					break;
				case 212:
					RegisterPetFoodItem(nItemID, pProp);
					break;
				case 226:
					RegisterTamingMobFoodItem(nItemID, pProp);
					break;
				case 227:
					RegisterBridleItem(nItemID, pProp);
					break;
				case 228:
				case 229:
					RegisterSkillLearnItem(nItemID, pProp);
					break;
				case 301:
					RegisterPortableChairItem(nItemID, pProp);
					break;
				}
			}
		}
	}
}

void ItemInfo::IteratePetItem()
{
}

void ItemInfo::IterateCashItem()
{
}

void ItemInfo::RegisterSpecificItems()
{
}

void ItemInfo::RegisterNoRollbackItem()
{
}

void ItemInfo::RegisterSetHalloweenItem()
{
}

void ItemInfo::RegisterEquipItemInfo(ItemInfo::EquipItem * pEqpItem, int nItemID, void * pProp)
{
	auto& infoImg = (*((WZ::Node*)pProp))["info"];

	LoadIncrementStat(pEqpItem->incStat, (void*)&infoImg);
	LoadAbilityStat(pEqpItem->abilityStat, (void*)&infoImg);

	pEqpItem->nItemID = nItemID;
	pEqpItem->nrSTR = infoImg["reqSTR"];
	pEqpItem->nrDEX = infoImg["reqDEX"];
	pEqpItem->nrINT = infoImg["reqINT"];
	pEqpItem->nrLUK = infoImg["reqLUK"];
	pEqpItem->nrPOP = infoImg["reqPOP"];
	pEqpItem->nrJob = infoImg["reqJob"];
	pEqpItem->nrLevel = infoImg["reqLevel"];
	//pEqpItem->nrMobLevel = infoImg["reqMobLevel"];
	pEqpItem->nSellPrice = infoImg["price"];
	pEqpItem->nKnockBack = infoImg["knockback"];

	pEqpItem->dwPetAbilityFlag = 0;
	if (nItemID / 10000 == 181)
	{
		if ((int)infoImg["pickupItem"] == 1)
			pEqpItem->dwPetAbilityFlag |= 0x1;

		if ((int)infoImg["longRange"] == 1)
			pEqpItem->dwPetAbilityFlag |= 0x2;

		if ((int)infoImg["pickupOthers"] == 1)
			pEqpItem->dwPetAbilityFlag |= 0x10;

		if ((int)infoImg["consumeHP"] == 1)
			pEqpItem->dwPetAbilityFlag |= 0x20;

		if ((int)infoImg["consumeMP"] == 1)
			pEqpItem->dwPetAbilityFlag |= 0x40;
	}
}

void ItemInfo::RegisterUpgradeItem(int nItemID, void * pProp)
{
	auto& infoImg = (*((WZ::Node*)pProp))["info"];
	ItemInfo::UpgradeItem *pNewUpgradeItem = new ItemInfo::UpgradeItem;

	pNewUpgradeItem->nItemID = nItemID;
	LoadIncrementStat(pNewUpgradeItem->incStat, (void*)&infoImg);
	pNewUpgradeItem->nSuccessRate = infoImg["success"];
	pNewUpgradeItem->nCursedRate = infoImg["cursed"];
	m_mUpgradeItem[nItemID] = pNewUpgradeItem;
}

void ItemInfo::RegisterPortalScrollItem(int nItemID, void * pProp)
{
	ItemInfo::PortalScrollItem *pNewPortalScrollItem = new ItemInfo::PortalScrollItem;
	pNewPortalScrollItem->nItemID = nItemID;
	auto& specImg = (*((WZ::Node*)pProp))["spec"];
	for (auto& effect : specImg)
		pNewPortalScrollItem->spec.push_back({ effect.Name(), (int)effect });
	m_mPortalScrollItem[nItemID] = pNewPortalScrollItem;
}

void ItemInfo::RegisterMobSummonItem(int nItemID, void * pProp)
{
	auto& infoImg = (*((WZ::Node*)pProp))["info"];
	ItemInfo::MobSummonItem *pNewMobSummonItem = new ItemInfo::MobSummonItem;
	pNewMobSummonItem->nItemID = nItemID;
	pNewMobSummonItem->nType = infoImg["type"];
	auto& mobImg = (*((WZ::Node*)pProp))["mob"];
	for (auto& mob : mobImg)
		pNewMobSummonItem->lMob.push_back({ (int)(mob["id"]), (int)(mob["prob"]) });
	m_mMobSummonItem[nItemID] = pNewMobSummonItem;
}

void ItemInfo::RegisterPetFoodItem(int nItemID, void * pProp)
{
	auto& specImg = (*((WZ::Node*)pProp))["spec"];
	ItemInfo::PetFoodItem *pNewFoodItem = new ItemInfo::PetFoodItem;
	pNewFoodItem->nItemID = nItemID;
	pNewFoodItem->niRepleteness = specImg["inc"];
	for (auto& petID : specImg)
		if (isdigit(petID.Name()[0]))
			pNewFoodItem->ldwPet.push_back((int)petID);
	m_mPetFoodItem[nItemID] = pNewFoodItem;
}

void ItemInfo::RegisterTamingMobFoodItem(int nItemID, void * pProp)
{
	auto& specImg = (*((WZ::Node*)pProp))["spec"];
	ItemInfo::TamingMobFoodItem *pNewTamingMobFoodItem = new ItemInfo::TamingMobFoodItem;
	pNewTamingMobFoodItem->nItemID = nItemID;
	pNewTamingMobFoodItem->niFatigue = specImg["incFatigue"];
	m_mTamingMobFoodItem[nItemID] = pNewTamingMobFoodItem;
}

void ItemInfo::RegisterBridleItem(int nItemID, void * pProp)
{
	auto& infoImg = (*((WZ::Node*)pProp))["info"];
	ItemInfo::BridleItem *pNewBridleItem = new ItemInfo::BridleItem;
	pNewBridleItem->nItemID = nItemID;
	pNewBridleItem->dwTargetMobID = infoImg["mob"];
	pNewBridleItem->nCreateItemID = infoImg["create"];
	pNewBridleItem->nBridleProp = infoImg["bridleProp"];
	pNewBridleItem->nBridleMsgType = infoImg["bridleMsgType"];
	pNewBridleItem->nBridleHP = infoImg["mobHP"];
	pNewBridleItem->nUseDelay = infoImg["useDelay"];
	pNewBridleItem->dBridlePropChg = (double)infoImg["bridlePropChg"];
	m_mBridleItem[nItemID] = pNewBridleItem;
}

void ItemInfo::RegisterPortableChairItem(int nItemID, void * pProp)
{
	auto& infoImg = (*((WZ::Node*)pProp))["info"];
	ItemInfo::PortableChairItem *pNewPortableChairItem = new ItemInfo::PortableChairItem;
	pNewPortableChairItem->nItemID = nItemID;
	pNewPortableChairItem->nReqLevel = infoImg["reqLevel"];
	pNewPortableChairItem->nPortableChairRecoveryRateMP = infoImg["recoveryMP"];
	pNewPortableChairItem->nPortableChairRecoveryRateHP = infoImg["recoveryHP"];
	m_mPortableChairItem[nItemID] = pNewPortableChairItem;
}

void ItemInfo::RegisterSkillLearnItem(int nItemID, void * pProp)
{
	auto& infoImg = (*((WZ::Node*)pProp))["info"];
	ItemInfo::SkillLearnItem *pNewSkillLearnItem = new ItemInfo::SkillLearnItem;
	pNewSkillLearnItem->nItemID = nItemID;
	pNewSkillLearnItem->nMasterLevel = infoImg["masterLevel"];
	pNewSkillLearnItem->nSuccessRate = infoImg["success"];
	if (nItemID / 10000 == 229)
		pNewSkillLearnItem->nReqLevel = infoImg["reqSkillLevel"];
	auto& skillImg = infoImg["skill"];
	for (auto& skill : skillImg)
		pNewSkillLearnItem->aSkill.push_back((int)skill);
	m_mSkillLearnItem[nItemID] = pNewSkillLearnItem;
}

void ItemInfo::RegisterStateChangeItem(int nItemID, void * pProp)
{
	ItemInfo::StateChangeItem *pNewStateChangeItem = new ItemInfo::StateChangeItem;
	pNewStateChangeItem->nItemID = nItemID;
	auto& specImg = (*((WZ::Node*)pProp))["spec"];
	for(auto& effect : specImg)
		pNewStateChangeItem->spec.push_back({ effect.Name(), (int)effect });
	m_mStateChangeItem[nItemID] = pNewStateChangeItem;
}

ItemInfo::EquipItem * ItemInfo::GetEquipItem(int nItemID)
{
	auto findIter = m_mEquipItem.find(nItemID);
	return (findIter != m_mEquipItem.end() ? findIter->second : nullptr);
}

ItemInfo::StateChangeItem * ItemInfo::GetStateChangeItem(int nItemID)
{
	auto findIter = m_mStateChangeItem.find(nItemID);
	return (findIter != m_mStateChangeItem.end() ? findIter->second : nullptr);
}

ItemInfo::BundleItem * ItemInfo::GetBundleItem(int nItemID)
{
	auto findIter = m_mBundleItem.find(nItemID);
	return (findIter != m_mBundleItem.end() ? findIter->second : nullptr);
}

ItemInfo::UpgradeItem * ItemInfo::GetUpgradeItem(int nItemID)
{
	auto findIter = m_mUpgradeItem.find(nItemID);
	return (findIter != m_mUpgradeItem.end() ? findIter->second : nullptr);
}

ItemInfo::PortalScrollItem * ItemInfo::GetPortalScrollItem(int nItemID)
{
	auto findIter = m_mPortalScrollItem.find(nItemID);
	return (findIter != m_mPortalScrollItem.end() ? findIter->second : nullptr);
}

ItemInfo::MobSummonItem * ItemInfo::GetMobSummonItem(int nItemID)
{
	auto findIter = m_mMobSummonItem.find(nItemID);
	return (findIter != m_mMobSummonItem.end() ? findIter->second : nullptr);
}

ItemInfo::PetFoodItem * ItemInfo::GetPetFoodItem(int nItemID)
{
	auto findIter = m_mPetFoodItem.find(nItemID);
	return (findIter != m_mPetFoodItem.end() ? findIter->second : nullptr);
}

ItemInfo::TamingMobFoodItem * ItemInfo::GetTamingMobFoodItem(int nItemID)
{
	auto findIter = m_mTamingMobFoodItem.find(nItemID);
	return (findIter != m_mTamingMobFoodItem.end() ? findIter->second : nullptr);
}

ItemInfo::BridleItem * ItemInfo::GetBridleItem(int nItemID)
{
	auto findIter = m_mBridleItem.find(nItemID);
	return (findIter != m_mBridleItem.end() ? findIter->second : nullptr);
}

ItemInfo::SkillLearnItem * ItemInfo::GetSkillLearnItem(int nItemID)
{
	auto findIter = m_mSkillLearnItem.find(nItemID);
	return (findIter != m_mSkillLearnItem.end() ? findIter->second : nullptr);
}

ItemInfo::PortableChairItem * ItemInfo::GetPortableChairItem(int nItemID)
{
	auto findIter = m_mPortableChairItem.find(nItemID);
	return (findIter != m_mPortableChairItem.end() ? findIter->second : nullptr);
}

bool ItemInfo::ConsumeOnPickup(int nItemID)
{
	StateChangeItem* pItem = nullptr;
	if (nItemID / 1000000 == 2 && (pItem = GetStateChangeItem(nItemID)) != nullptr)
	{
		for (auto& p : pItem->spec)
			if (p.first == "consumeOnPickup" && p.second == 1)
				return true;
	}
	return false;
}

bool ItemInfo::ExpireOnLogout(int nItemID)
{
	if (nItemID / 1000000 != 5)
		if (nItemID / 1000000 == 1)
		{
			auto pItem = GetEquipItem(nItemID);
			if (pItem != nullptr)
				return pItem->abilityStat.bExpireOnLogout;
		}
		else
		{
			auto pItem = GetBundleItem(nItemID);
			if (pItem != nullptr)
				return pItem->abilityStat.bExpireOnLogout;
		}
	return false;
}

int ItemInfo::GetBulletPAD(int nItemID)
{
	auto pItem = GetBundleItem(nItemID);
	if (pItem == nullptr)
		return 0;
	return pItem->nPAD;
}

long ItemInfo::GetItemDateExpire(const std::string & sDate)
{
	std::string sYear = sDate.substr(4);
	std::string sMonth = sDate.substr(4, 2);
	std::string sDay = sDate.substr(6, 2);
	std::string sHour = sDate.substr(8, 2);
	SYSTEMTIME sysTime;
	sysTime.wYear = atoi(sYear.c_str());
	sysTime.wMonth = atoi(sMonth.c_str());
	sysTime.wDay = atoi(sDay.c_str());
	sysTime.wHour = atoi(sHour.c_str());
	sysTime.wMilliseconds = 0;
	sysTime.wSecond = 0;
	sysTime.wMinute = 0;
	sysTime.wDayOfWeek = 0;
	FILETIME ft;
	SystemTimeToFileTime(&sysTime, &ft);
	return *((long long int*)&ft);
}

const std::string & ItemInfo::GetItemName(int nItemID)
{
	auto findResult = m_mItemString.find(nItemID);
	return (findResult == m_mItemString.end() ? "" : findResult->second);
}

bool ItemInfo::IsAbleToEquip(int nGender, int nLevel, int nJob, int nSTR, int nDEX, int nINT, int nLUK, int nPOP, GW_ItemSlotBase * pPetItem, int nItemID)
{
	return false;
}

bool ItemInfo::IsNotSaleItem(int nItemID)
{
	if (nItemID / 1000000 != 5)
		if (nItemID / 1000000 == 1)
		{
			auto pItem = GetEquipItem(nItemID);
			if (pItem != nullptr)
				return pItem->abilityStat.bNotSale;
		}
		else
		{
			auto pItem = GetBundleItem(nItemID);
			if (pItem != nullptr)
				return pItem->abilityStat.bNotSale;
		}
	return false;
}

bool ItemInfo::IsOnlyItem(int nItemID)
{
	if (nItemID / 1000000 != 5)
		if (nItemID / 1000000 == 1)
		{
			auto pItem = GetEquipItem(nItemID);
			if (pItem != nullptr)
				return pItem->abilityStat.bOnly;
		}
		else
		{
			auto pItem = GetBundleItem(nItemID);
			if (pItem != nullptr)
				return pItem->abilityStat.bOnly;
		}
	return false;
}

bool ItemInfo::IsTradeBlockItem(int nItemID)
{
	if (nItemID / 1000000 != 5)
		if (nItemID / 1000000 == 1)
		{
			auto pItem = GetEquipItem(nItemID);
			if (pItem != nullptr)
				return pItem->abilityStat.bTradeBlock;
		}
		else
		{
			auto pItem = GetBundleItem(nItemID);
			if (pItem != nullptr)
				return pItem->abilityStat.bTradeBlock;
		}
	return false;
}

bool ItemInfo::IsQuestItem(int nItemID)
{
	if (nItemID / 1000000 != 5)
		if (nItemID / 1000000 == 1)
		{
			auto pItem = GetEquipItem(nItemID);
			if (pItem != nullptr)
				return pItem->abilityStat.bQuest;
		}
		else
		{
			auto pItem = GetBundleItem(nItemID);
			if (pItem != nullptr)
				return pItem->abilityStat.bQuest;
		}
	return false;
}

void ItemInfo::LoadIncrementStat(ItemInfo::BasicIncrementStat & refStat, void * pProp)
{
	auto& infoImg = (*((WZ::Node*)pProp));
	refStat.niSTR = infoImg["incSTR"];
	refStat.niDEX = infoImg["incDEX"];
	refStat.niINT = infoImg["incINT"];
	refStat.niLUK = infoImg["incLUK"];
	refStat.niMaxHP = infoImg["incMHP"];
	refStat.niMaxMP = infoImg["incMMP"];
	refStat.niPAD = infoImg["incPAD"];
	refStat.niMAD = infoImg["incMAD"];
	refStat.niPDD = infoImg["incPDD"];
	refStat.niMDD = infoImg["incMDD"];
	refStat.niACC = infoImg["incACC"];
	refStat.niEVA = infoImg["incEVA"];
	refStat.niCraft = infoImg["incCraft"];
	refStat.niSpeed = infoImg["incSpeed"];
	refStat.niJump = infoImg["incJump"];
	refStat.niSwim = infoImg["incSwim"];
}

void ItemInfo::LoadAbilityStat(ItemInfo::BasicAbilityStat & refStat, void * pProp)
{
	auto& infoImg = (*((WZ::Node*)pProp));	
	refStat.bTimeLimited = ((int)infoImg["timeLimited"]) == 1;
	refStat.bQuest = ((int)infoImg["quest"]) == 1;
	refStat.bOnly = ((int)infoImg["only"]) == 1;
	refStat.bNotSale = ((int)infoImg["notSale"]) == 1;
	refStat.bTradeBlock = ((int)infoImg["tradeBlock"]) == 1;
	refStat.bExpireOnLogout = ((int)infoImg["expireOnLogout"]) == 1;
}
