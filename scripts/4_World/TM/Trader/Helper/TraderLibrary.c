class TraderLibrary
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

	static int GetPlayerCurrencyAmount(int traderID, PlayerBase player)
	{		
		int currencyAmount = 0;
		
		array<EntityAI> itemsArray = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);

		ItemBase item;
        PluginTraderData traderDataPlugin = PluginTraderData.Cast(GetPlugin(PluginTraderData));
        if(traderDataPlugin)
        {
            TD_Trader trader = traderDataPlugin.GetTraderByID(traderUID);
            TR_Trader_Currency currency = traderDataPlugin.GetCurrencyByName(trader.Currency);
            if(currency)
            {
                for (int i = 0; i < itemsArray.Count(); i++)
                {
                    Class.CastTo(item, itemsArray.Get(i));

                    if (!item)
                        continue;

                    foreach (TR_Currency cr : currency.CurrencyNotes)
                    {
                        string itemType = item.GetType();
                        itemType.ToLower();
                        string crlower = cr.ClassName;
                        crlower.ToLower();
                        if(itemType == crlower)
                        {
                            currencyAmount += GetItemAmount(item) * cr.Value;
                            break;
                        }
                    }
                }
            }
            else
            {
                //TODO: error
            }
        }
		
		return currencyAmount;
	}
    
	static int GetItemAmount(ItemBase item)
	{
		Magazine mgzn = Magazine.Cast(item);
				
		int itemAmount = 0;
		if( item.IsMagazine() )
		{
			itemAmount = mgzn.GetAmmoCount();
		}
		else
		{
			itemAmount = QuantityConversions.GetItemQuantity(item);
		}
		
		return itemAmount;
	}
    
	static bool IsInPlayerInventory(PlayerBase player, string itemClassname, int amount)
	{
		itemClassname.ToLower();
		
		bool isMagazine = false;
		if (amount == -3)
			isMagazine = true;

		bool isWeapon = false;
		if (amount == -4)
			isWeapon = true;

		bool isSteak = false;
		if (amount == -5)
			isSteak = true;

		array<EntityAI> itemsArray = new array<EntityAI>;		
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);

		ItemBase item;		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));
			string itemPlayerClassname = "";

			if (!item)
				continue;

			if (item.IsRuined())
				continue;

			if (TraderLibrary.IsItemAttached(item))
				continue;

			itemPlayerClassname = item.GetType();
			itemPlayerClassname.ToLower();

			if(itemPlayerClassname == itemClassname && ((GetItemAmount(item) >= amount && !isMagazine && !isWeapon && !isSteak) || isMagazine || isWeapon || (isSteak && (GetItemAmount(item) >= TraderLibrary.GetMaxQuantityForClassName(itemPlayerClassname) * 0.5)))) // && m_Trader_LastSelledItemID != item.GetID())
			{
				return true;
			}
		}
		
		return false;
	}

	static int GetItemMaxQuantityForObject(Object itemObj)
	{
        if(itemObj.ConfigIsExisting("count"))
        {
		    return itemObj.ConfigGetInt("count");
        }
        if(itemObj.ConfigIsExisting("varQuantityMax"))
        {
		    return itemObj.ConfigGetInt("varQuantityMax");
        }
		return -1;
	}

	static int GetMaxQuantityForClassName(string itemClassname)
	{
		TStringArray searching_in = new TStringArray;
		searching_in.Insert( CFG_MAGAZINESPATH  + " " + itemClassname + " count");
		//searching_in.Insert( CFG_WEAPONSPATH );
		searching_in.Insert( CFG_VEHICLESPATH + " " + itemClassname + " varQuantityMax");

		for ( int s = 0; s < searching_in.Count(); ++s )
		{
			string path = searching_in.Get( s );

			if ( GetGame().ConfigIsExisting( path ) )
			{
				return g_Game.ConfigGetInt( path );
			}
		}

		return 0;
	}
	
	static string GetItemDisplayName(string itemClassname)
	{
		TStringArray itemInfos = new TStringArray;
		
		string cfg = "CfgVehicles " + itemClassname + " displayName";
		string displayName;
		GetGame().ConfigGetText(cfg, displayName);
	
		if (displayName == "")
		{
			cfg = "CfgAmmo " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
		
		if (displayName == "")
		{
			cfg = "CfgMagazines " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
		
		if (displayName == "")
		{
			cfg = "cfgWeapons " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
	
		if (displayName == "")
		{
			cfg = "CfgNonAIVehicles " + itemClassname + " displayName";
			GetGame().ConfigGetText(cfg, displayName);
		}
		
		
		if (displayName != "")
			return TrimUntPrefix(displayName);
		else
			return itemClassname;
	}

	static string TrimUntPrefix(string str)
	{
		str.Replace("$UNT$", "");
		return str;
	}

	static Object GetVehicleToSell(PlayerBase player, int traderUID, string vehicleClassname)
	{
        PluginTraderData traderDataPlugin = PluginTraderData.Cast(GetPlugin(PluginTraderData));
        if(traderDataPlugin)
        {
            vector size = "3 5 9";
            array<Object> excluded_objects = new array<Object>;
            array<Object> nearby_objects = new array<Object>;
            TD_Trader trader = traderDataPlugin.GetTraderByID(traderUID);
		    vector position = trader.VehiclePosition;
		    vector orientation = trader.VehicleOrientation;
            bool Colliding = GetGame().IsBoxColliding( position, orientation, size, excluded_objects, nearby_objects);
            if (Colliding)
            {
                for (int i = 0; i < nearby_objects.Count(); i++)
                {
                    string nearby = nearby_objects.Get(i).GetType();
                    nearby.ToLower();
                    vehicleClassname.ToLower();
                    if (nearby == vehicleClassname)
                    {
                        // Check if there is any Player in the Vehicle:
                        bool vehicleIsEmpty = true;

                        Transport transport;
                        Class.CastTo(transport, nearby_objects.Get(i));
                        if (transport)
                        {
                            int crewSize = transport.CrewSize();
                            for (int c = 0; c < crewSize; c++)
                            {
                                if (transport.CrewMember(c))
                                    vehicleIsEmpty = false;
                            }
                        }
                        else
                        {
                            continue;
                        }

                        if (!vehicleIsEmpty)
                            continue;

                        // Check if Player was last Driver:
                        CarScript carsScript = CarScript.Cast(nearby_objects.Get(i));
                        if(!carsScript)
                            continue;
                        
                        if(carsScript.m_Trader_LastDriverId != player.GetIdentity().GetId())
                            continue;					

                        return nearby_objects.Get(i);		
                    }					
                }
            }
        }

		return NULL;
	}

	static bool IsAttachedToEntity(EntityAI parentEntity, string attachmentClassname)
	{
		for ( int i = 0; i < parentEntity.GetInventory().AttachmentCount(); i++ )
		{
			EntityAI attachment = parentEntity.GetInventory().GetAttachmentFromIndex ( i );
			if ( attachment.IsKindOf ( attachmentClassname ) )
				return true;
		}

		return false;
	}

	static bool IsItemAttached(ItemBase item)
	{
		EntityAI parent = item.GetHierarchyParent();

		if (!parent)
			return false;

		if (item.GetInventory().IsAttachment() || item.GetNumberOfItems() > 0)
			return true;

		if (parent.IsWeapon() || parent.IsMagazine())
			return true;

		return false;
	}

	static bool IsAttachment(EntityAI parentEntity, string attachmentClassname)
	{
		// Check chamberable Ammunitions
		string type_name = parentEntity.GetType();
		TStringArray cfg_chamberables = new TStringArray;

		GetGame().ConfigGetTextArray(CFG_WEAPONSPATH + " " + type_name + " chamberableFrom", cfg_chamberables);

		for (int i = 0; i < cfg_chamberables.Count(); i++)
		{
			if (attachmentClassname == cfg_chamberables[i])
				return true;
		}


		// Check Magazines
		TStringArray cfg_magazines = new TStringArray;

		GetGame().ConfigGetTextArray(CFG_WEAPONSPATH + " " + type_name + " magazines", cfg_magazines);

		for (i = 0; i < cfg_magazines.Count(); i++)
		{
			if (attachmentClassname == cfg_magazines[i])
				return true;
		}


		return false;
	}
	
    static bool SetItemAmount(ItemBase item, int amount)
	{
		if (!item)
			return false;

		if (amount == -1)
			amount = TraderLibrary.GetMaxQuantityForClassName(item.GetType());

		if (amount == -3)
			amount = 0;

		if (amount == -4)
			amount = 0;

		if (amount == -5)
			amount = Math.RandomIntInclusive(TraderLibrary.GetMaxQuantityForClassName(item.GetType()) * 0.5, TraderLibrary.GetMaxQuantityForClassName(item.GetType()));

		Magazine mgzn = Magazine.Cast(item);
				
		if( item.IsMagazine() )
		{
			if (!mgzn)
				return false;

			mgzn.ServerSetAmmoCount(amount);
		}
		else
		{
			item.SetQuantity(amount);
		}

		return true;
	}
}