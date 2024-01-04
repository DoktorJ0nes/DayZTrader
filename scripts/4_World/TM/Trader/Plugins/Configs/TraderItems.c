//TP "CowSteakMeat,1,200,0.75,-1,60,0.85",

//PowderedMilk,    *,   -1,   10
//BearSteakMeat,   S,   -1,   100
/*
MoneyRuble1,   1,   1,   -1
MoneyRuble5,   1,   5,   -1
MoneyRuble10,   1,   10,   -1
MoneyRuble25,   1,   25,   -1
MoneyRuble50,   1,   50,   -1
MoneyRuble100,   1,   100,   -1
*/
//FAL, W,   -1,   1250
//Mag_Deagle_9rnd, M,   -1,   150
//dzn_Pot,  M,   3000,   500
//Spur_TrolleyKart, VNK, 80000,	8000

//from player perspective: player sells, player buys
enum TR_Item_Type
{
    Regular,
    QuantItem,
    Magazine, // Magazine_Base do we separate in diff types? eg Mag full buy, empty sell | Mag empty buy, empty sell (we should empty mag when selling)
    Ammo, // Ammunition_Base
    Weapon, // special Pistol_Base Rifle_Base Launcher_Base
    Vehicle, //CarScript special spawn with parts
    Food, //Edible_Base a percentage amount for selling but buy full quantity
    Bottle, //Bottle_Base Bottle can be barrel too. Can be sold with any quantity but bought empty?
    Special, // Barrel_ColorBase? BarrelHoles_ColorBase?
    Unknown //we don't know the type so it should be skipped?
};

class TR_Trader_Item
{
    int ID;
    bool IsPreset;
    string ClassName;
    string DisplayName;
    string Description;
    int Quantity;
    TR_Item_Type Type;
    int BuyPrice;
    int SellPrice;
    ref array<ref TraderObjectAttachment> Attachments;

    bool Load(int inID, string item)
    {
        ID = inID;
        //strip spaces
        item.Replace(" ", "");
        TStringArray strs = new TStringArray;
        item.Split( ",", strs );
        if(strs.Count() < 4)
        {
            TM_Print("Could not load item as it has less than 4 items in the definitions. Item: " + item);
            return false;
        }
        ClassName = strs.Get(0);
        PluginTraderData traderDataPlugin = PluginTraderData.Cast(GetPlugin(PluginTraderData));
        if(traderDataPlugin)
        {
            TR_Preset preset = traderDataPlugin.GetPresetByName(ClassName);
            if(preset)
            {
                TM_Print("Found preset: " + preset.PresetName);
                ClassName = preset.ClassName;
                Attachments = preset.Attachments;
                DisplayName = preset.DisplayName;
                Description = preset.Description;
                IsPreset = true;
            }
        }
        Object itemObj = GetGame().CreateObjectEx(ClassName, "0 0 0", ECE_PLACE_ON_SURFACE);
        if(!itemObj)
        {
            TM_Print("Could not identify item: " + ClassName);
            return false;
        }
        if(itemObj.IsMagazine())
        {
            Type = TR_Item_Type.Magazine;
        }
        if(itemObj.IsAmmoPile())
        {
            Type = TR_Item_Type.Ammo;
        }
        if(itemObj.IsWeapon())
        {
            Type = TR_Item_Type.Weapon;
        }
        if(itemObj.IsKindOf("CarScript"))
        {
            Type = TR_Item_Type.Vehicle;
        }
        if(itemObj.IsKindOf("Edible_Base"))
        {
            Type = TR_Item_Type.Food;
        }
        if(itemObj.IsKindOf("Bottle_Base"))
        {
            Type = TR_Item_Type.Bottle;
        }
        
        string qntStr = strs.Get(1);        
        if (qntStr.Contains("*") || qntStr.Contains("-1"))
        {
            Quantity = TraderLibrary.GetItemMaxQuantityForObject(itemObj);
            if(Quantity > 0 && Type == TR_Item_Type.Regular)
            {
                Type = TR_Item_Type.QuantItem;
            }
        }
        else
        {
            Quantity = qntStr.ToInt();
        }
        if(Quantity == -1 || Quantity == 0)
        {
            Quantity = 1;
        }
        
        BuyPrice = strs.Get(2).ToInt();
        SellPrice = strs.Get(3).ToInt();

        itemObj.Delete();
        //TM_Print("Loaded item: " + ClassName);
        //TM_Print(" Quant: " + Quantity.ToString() + " Type: " + Type.ToString());
        //TM_Print(" Buy: " + BuyPrice.ToString() + " Sell: " + SellPrice.ToString());
        return true;
    }
};