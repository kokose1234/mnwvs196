#include "GW_ItemSlotEquip.h"
#include "WvsUnified.h"
#include "Poco\Data\MySQL\MySQLException.h"

#define ADD_EQUIP_FLAG(name, container)\
if(n##name != 0) {\
	nFlag |= EQP_##name;\
	VALUE_HOLDER vh;\
	vh.type = sizeof(n##name);\
	if((vh.type) == 1)vh.cValue = n##name;\
	if ((vh.type) == 2)vh.sValue = n##name;\
	if ((vh.type) == 4)vh.iValue = n##name;\
	if ((vh.type) == 8)vh.liValue = n##name;\
	container.push_back(vh);\
}

#define DECODE_EQUIP_FLAG(name)\
if (nFlag & EQP_##name) {\
	if (sizeof(n##name) == 1) n##name = iPacket->Decode1();\
	if (sizeof(n##name) == 2) n##name = iPacket->Decode2();\
	if (sizeof(n##name) == 4) n##name = iPacket->Decode4();\
	if (sizeof(n##name) == 8) n##name = iPacket->Decode8();\
}

GW_ItemSlotEquip::GW_ItemSlotEquip()
{
}


GW_ItemSlotEquip::~GW_ItemSlotEquip()
{
}

void GW_ItemSlotEquip::Load(ATOMIC_COUNT_TYPE SN)
{
	Poco::Data::Statement queryStatement(GET_DB_SESSION);
	queryStatement << "SELECT * FROM ItemSlot_EQP Where SN = " << SN;
	queryStatement.execute();

	Poco::Data::RecordSet recordSet(queryStatement);
	nCharacterID = recordSet["CharacterID"];
	liItemSN = recordSet["ItemSN"];
	nItemID = recordSet["ItemID"];
	liExpireDate = recordSet["ExpireDate"];
	nAttribute = recordSet["Attribute"];
	nPOS = recordSet["POS"];
	nRUC = (char)(unsigned short)recordSet["RUC"];
	nCUC = (char)(unsigned short)recordSet["CUC"];
	nSTR = recordSet["I_STR"];
	nDEX = recordSet["I_DEX"];
	nINT = recordSet["I_INT"];
	nLUK = recordSet["I_LUK"];
	nMaxHP = recordSet["I_MaxHP"];
	nMaxMP = recordSet["I_MaxMP"];
	nPAD = recordSet["I_PAD"];
	nMAD = recordSet["I_MAD"];
	nPDD = recordSet["I_PDD"];
	nMDD = recordSet["I_MDD"];
	nACC = recordSet["I_ACC"];
	nEVA = recordSet["I_EVA"];
	nSpeed = recordSet["I_Speed"];
	nCraft = recordSet["I_Craft"];
	nJump = recordSet["I_Jump"];
	nType = GW_ItemSlotType::EQUIP;
}

void GW_ItemSlotEquip::Save(int nCharacterID, GW_ItemSlotType type)
{
	if (type != GW_ItemSlotType::EQUIP)
		throw std::runtime_error("Invalid Equip Type.");
	Poco::Data::Statement queryStatement(GET_DB_SESSION);
	try 
	{
		if (liItemSN < -1 /*nStatus == GW_ItemSlotStatus::DROPPED*/) //DROPPED or DELETED
		{
			liItemSN *= -1;
			queryStatement << "UPDATE ItemSlot_EQP Set CharacterID = -1 Where CharacterID = " << nCharacterID << " and ItemSN = " << liItemSN;
			queryStatement.execute();
			return;
		}
		if (liItemSN == -1)
		{
			liItemSN = IncItemSN(GW_ItemSlotType::EQUIP);
			queryStatement << "INSERT INTO ItemSlot_EQP (ItemSN, ItemID, CharacterID, ExpireDate, Attribute, POS, RUC, CUC, I_STR, I_DEX, I_INT, I_LUK, I_MaxHP, I_MaxMP, I_PAD, I_MAD, I_PDD, I_MDD, I_ACC, I_EVA, I_Speed, I_Craft, I_Jump) VALUES("
				<< liItemSN << ", "
				<< nItemID << ", "
				<< nCharacterID << ", "
				<< liExpireDate << ", "
				<< nAttribute << ", "
				<< nPOS << ", "
				<< (unsigned short)nRUC << ", "
				<< (unsigned short)nCUC << ", "
				<< nSTR << ", "
				<< nDEX << ", "
				<< nINT << ", "
				<< nLUK << ", "
				<< nMaxHP << ", "
				<< nMaxMP << ", "
				<< nPAD << ", "
				<< nMAD << ", "
				<< nPDD << ", "
				<< nMDD << ", "
				<< nACC << ", "
				<< nEVA << ", "
				<< nSpeed << ", "
				<< nCraft << ", "
				<< nJump << ")";
		}
		else
		{
			queryStatement << "UPDATE ItemSlot_EQP Set "
				<< "ItemID = '" << nItemID << "', "
				<< "CharacterID = '" << nCharacterID << "', "
				<< "ExpireDate = '" << liExpireDate << "', "
				<< "Attribute = '" << nAttribute << "', "
				<< "POS ='" << nPOS << "', "
				<< "RUC ='" << (unsigned short)nRUC << "', "
				<< "CUC ='" << (unsigned short)nCUC << "', "
				<< "I_STR = '" << nSTR << "', "
				<< "I_DEX = '" << nDEX << "', "
				<< "I_INT = '" << nINT << "', "
				<< "I_LUK = '" << nLUK << "', "
				<< "I_MaxHP = '" << nMaxHP << "', "
				<< "I_MaxMP = '" << nMaxMP << "', "
				<< "I_PAD = '" << nPAD << "', "
				<< "I_MAD = '" << nMAD << "', "
				<< "I_PDD = '" << nPDD << "', "
				<< "I_MDD = '" << nMDD << "', "
				<< "I_ACC = '" << nACC << "', "
				<< "I_EVA = '" << nEVA << "', "
				<< "I_Speed = '" << nSpeed << "', "
				<< "I_Craft = '" << nCraft << "', "
				<< "I_Jump = '" << nJump << "' WHERE ItemSN = " << liItemSN;
		}
		queryStatement.execute();
	}
	catch (Poco::Data::MySQL::StatementException & e) {
		printf("SQL Exception : %s\n", queryStatement.toString().c_str());
	}
}

/*
Equip Encoder Entry (Then Encoding Position Info And Equip Attributes Info.)
*/
void GW_ItemSlotEquip::Encode(OutPacket *oPacket) const
{
	EncodeInventoryPosition(oPacket);
	RawEncode(oPacket);
}

/*
Encode Equip Information
*/
void GW_ItemSlotEquip::RawEncode(OutPacket *oPacket) const
{
	GW_ItemSlotBase::RawEncode(oPacket);
	oPacket->Encode1(0); //Unknown Type
	EncodeEquipBase(oPacket);
	EncodeEquipAdvanced(oPacket);
}

/*
Encode Equip Basic Information
*/
//ADD_EQUIP_FLAG cause compiler think it will convert short to char, but it actually wont.
#pragma warning(disable:4244)  
void GW_ItemSlotEquip::EncodeEquipBase(OutPacket *oPacket) const
{
	struct VALUE_HOLDER
	{
		int type = 1;
		union
		{
			unsigned char cValue;
			unsigned short sValue;
			unsigned int iValue;
			unsigned long long int liValue;
		};
	};
	std::vector<VALUE_HOLDER> aEquipBasicValue;
	int nFlag = 0;

	ADD_EQUIP_FLAG(RUC, aEquipBasicValue);
	ADD_EQUIP_FLAG(CUC, aEquipBasicValue);
	ADD_EQUIP_FLAG(STR, aEquipBasicValue);
	ADD_EQUIP_FLAG(DEX, aEquipBasicValue);
	ADD_EQUIP_FLAG(INT, aEquipBasicValue);
	ADD_EQUIP_FLAG(LUK, aEquipBasicValue);
	ADD_EQUIP_FLAG(MaxHP, aEquipBasicValue);
	ADD_EQUIP_FLAG(MaxMP, aEquipBasicValue);
	ADD_EQUIP_FLAG(PAD, aEquipBasicValue);
	ADD_EQUIP_FLAG(MAD, aEquipBasicValue);
	ADD_EQUIP_FLAG(PDD, aEquipBasicValue);
	ADD_EQUIP_FLAG(Craft, aEquipBasicValue);
	ADD_EQUIP_FLAG(Speed, aEquipBasicValue);
	ADD_EQUIP_FLAG(Jump, aEquipBasicValue);
	ADD_EQUIP_FLAG(Attribute, aEquipBasicValue);
	oPacket->Encode4(nFlag);
	for (const auto& x : aEquipBasicValue)
	{
		if (x.type == 1)
			oPacket->Encode1(x.cValue);
		else if (x.type == 2)
			oPacket->Encode2(x.sValue);
		else if (x.type == 4)
			oPacket->Encode4(x.iValue);
		else if (x.type == 8)
			oPacket->Encode8(x.liValue);
	}

	oPacket->Encode4(0);
	/*oPacket->Encode1(0);
	oPacket->Encode1(0);
	oPacket->Encode1(0);
	oPacket->EncodeTime(-1);
	oPacket->Encode4(0);*/
}
#pragma warning(default:4244)  

/*
Encode Equip Advanced Information, Including Potential, Sockets, And Etc.
*/
void GW_ItemSlotEquip::EncodeEquipAdvanced(OutPacket *oPacket) const
{
	oPacket->EncodeStr("Owner");
	oPacket->Encode1(0); //Potential State
	oPacket->Encode1(0); //Enhance

						 //Potential
	oPacket->Encode2(0);
	oPacket->Encode2(0);
	oPacket->Encode2(0);
	//Additional Potential
	oPacket->Encode2(0);
	oPacket->Encode2(0);
	oPacket->Encode2(0);

	//Fusion Anvil
	oPacket->Encode2(0);

	//Socket State
	oPacket->Encode2(0);
	oPacket->Encode2(0);
	oPacket->Encode2(0);
	oPacket->Encode2(0);

	if (liCashItemSN == -1)
		oPacket->Encode8(liItemSN);
	oPacket->EncodeTime(-2);
	oPacket->Encode4(-1);

	//sub_684A26
	oPacket->Encode8(0);
	oPacket->EncodeTime(-2);
	oPacket->Encode4(0);
	for (int i = 0; i < 3; ++i)
		oPacket->Encode4(0);

	oPacket->Encode2(0);
	oPacket->Encode2(0);
	oPacket->Encode2(0);

	if (nItemID / 10000 == 171)
	{
		oPacket->Encode2(0);
		oPacket->Encode4(0);
		oPacket->Encode2(0);
	}

	oPacket->Encode4(0);
	oPacket->EncodeTime(-2);
}

void GW_ItemSlotEquip::Decode(InPacket *iPacket)
{
	RawDecode(iPacket);
}

void GW_ItemSlotEquip::RawDecode(InPacket *iPacket)
{
	GW_ItemSlotBase::RawDecode(iPacket);
	iPacket->Decode1();
	DecodeEquipBase(iPacket);
	DecodeEquipAdvanced(iPacket);
}

#pragma warning(disable:4244)  
void GW_ItemSlotEquip::DecodeEquipBase(InPacket *iPacket)
{
	int nFlag = iPacket->Decode4();
	DECODE_EQUIP_FLAG(RUC);
	DECODE_EQUIP_FLAG(CUC);
	DECODE_EQUIP_FLAG(STR);
	DECODE_EQUIP_FLAG(DEX);
	DECODE_EQUIP_FLAG(INT);
	DECODE_EQUIP_FLAG(LUK);
	DECODE_EQUIP_FLAG(MaxHP);
	DECODE_EQUIP_FLAG(MaxMP);
	DECODE_EQUIP_FLAG(PAD);
	DECODE_EQUIP_FLAG(MAD);
	DECODE_EQUIP_FLAG(PDD);
	DECODE_EQUIP_FLAG(Craft);
	DECODE_EQUIP_FLAG(Speed);
	DECODE_EQUIP_FLAG(Jump);
	DECODE_EQUIP_FLAG(Attribute);

	nFlag = iPacket->Decode4();
}
#pragma warning(default:4244) 

void GW_ItemSlotEquip::DecodeEquipAdvanced(InPacket *iPacket)
{
	std::string strTitle = iPacket->DecodeStr();
	iPacket->Decode1(); //Potential State
	iPacket->Decode1(); //Enhance

						//Potential
	iPacket->Decode2();
	iPacket->Decode2();
	iPacket->Decode2();
	//Additional Potential
	iPacket->Decode2();
	iPacket->Decode2();
	iPacket->Decode2();

	//Fusion Anvil
	iPacket->Decode2();

	//Socket State
	iPacket->Decode2();
	iPacket->Decode2();
	iPacket->Decode2();
	iPacket->Decode2();

	if (liCashItemSN == -1)
		liItemSN = iPacket->Decode8();
	iPacket->Decode8();
	iPacket->Decode4();

	//sub_684A26
	iPacket->Decode8();
	iPacket->Decode8();
	iPacket->Decode4();
	for (int i = 0; i < 3; ++i)
		iPacket->Decode4();

	iPacket->Decode2();
	iPacket->Decode2();
	iPacket->Decode2();

	if (nItemID / 10000 == 171)
	{
		iPacket->Decode2();
		iPacket->Decode4();
		iPacket->Decode2();
	}

	iPacket->Decode4();
	iPacket->Decode8();
}

GW_ItemSlotBase * GW_ItemSlotEquip::MakeClone()
{
	GW_ItemSlotEquip* ret = new GW_ItemSlotEquip();
	*ret = *this;
	ret->liItemSN = -1;
	/*OutPacket cloneOut;
	Encode(&cloneOut);
	InPacket cloneIn(cloneOut.GetPacket(), cloneOut.GetPacketSize());
	ret->Decode(&cloneIn);*/
	return ret;
}
