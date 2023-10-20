static const int TRADERMENU_UI  = 665001;
class TR_Helper
{
    static bool ItemHasQuantity(string itemClassname)
    {
        return GetItemMaxQuantity(itemClassname) > 0;
    }

    static bool GetItemMaxQuantity(string itemClassname)
    {
        string path = CFG_VEHICLESPATH + " " + itemClassname + " varQuantityMax";
        if (GetGame().ConfigIsExisting(path))
            return GetGame().ConfigGetInt(path);
        return -1;
    }

    static bool ItemHasCount(string itemClassname)
    {
        return GetItemCount(itemClassname) > 0;
    }

    static bool GetItemCount(string itemClassname)
    {
        string path = CFG_MAGAZINESPATH  + " " + itemClassname + " count";
        if (GetGame().ConfigIsExisting(path))
            return GetGame().ConfigGetInt(path);
        return -1;
    }

    static bool HasQuantityBar(string itemClassname)
    {
        string path = CFG_VEHICLESPATH  + " " + itemClassname + " quantityBar";
        if (GetGame().ConfigIsExisting(path))        
            return GetGame().ConfigGetInt(path) == 1;

        return false;
    }

    static int GetItemSlotCount(string classname)
	{
		string config_path = string.Format("CfgVehicles %1 Cargo itemsCargoSize", classname);
		if ( GetGame().ConfigIsExisting( config_path ) )
		{
			TIntArray CargoAlt = new TIntArray;
			g_Game.ConfigGetIntArray(config_path, CargoAlt);
			return CargoAlt[0]*CargoAlt[1];
		}
		string Vpath = CFG_VEHICLESPATH + " " + classname + " itemsCargoSize";
		if ( GetGame().ConfigIsExisting( Vpath ) )
		{
			TIntArray Cargo = new TIntArray;
			g_Game.ConfigGetIntArray(Vpath, Cargo);
			return Cargo[0]*Cargo[1];
		}
		return 0;
	}

    static ref TStringArray KitIgnoreArray = 
    {
        "BloodTestKit",
        "StartKitIV",
        "FirstAidKit",
        "MSFC_FirstAidKit",
        "WG_ITS_Medkit"
        "WG_AI2_Medkit",
        "SewingKit",
        "LeatherSewingKit",
        "WeaponCleaningKit",
        "KitchenKnife",
        "ElectronicRepairKit",
        "gunwall_kit_mung",
        "gunwall_metal_kit_mung"
    };
};