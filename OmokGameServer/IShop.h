#pragma once

struct ItemData {
	int _itemId;
	int _itemPrice;
	std::string _itemName;
	std::string _itemExplanation;
};

class IShop
{
public:
	virtual std::unordered_map<int, ItemData> GetItemList() = 0;
	virtual void Purchase() = 0;
	{

	}
private:
	UINT32		_shopId;
	std::string _shopName;
};

class GoldShop : public IShop
{
public:
	virtual std::unordered_map<int, ItemData> GetItemList()
	{

	}
	virtual void Purchase()
	{

	}
	// 구매
	

	// 판매는 없음.



private:
	std::unordered_map<int, ItemData> _itemManager;
};