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

	static int GetPlayerCurrencyAmount(TD_Trader trader, PlayerBase player)
	{		
		int currencyAmount = 0;
		
		array<EntityAI> itemsArray = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);

		ItemBase item;
        PluginTraderData traderDataPlugin = PluginTraderData.Cast(GetPlugin(PluginTraderData));
        if(traderDataPlugin)
        {
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
    
	static bool IsInPlayerInventory(PlayerBase player, TR_Trader_Item trItem, int amount)
	{
		string itemClassname = trItem.ClassName;
		itemClassname.ToLower();

		array<EntityAI> itemsArray = new array<EntityAI>;		
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);

		ItemBase item;		
		int totalAmount = 0;
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

			if(itemPlayerClassname == itemClassname)
			{
				switch (trItem.Type)
				{
					case TR_Item_Type.Food:
						if(GetItemAmount(item) >= TraderLibrary.GetMaxQuantityForClassName(itemPlayerClassname) * 0.75)
						{
							return true;
						}
						break;
					case TR_Item_Type.QuantItem: case TR_Item_Type.Ammo:
						int itemAmount = GetItemAmount(item);
						if(itemAmount >= amount)
						{
							return true;
						}
						totalAmount += itemAmount;
						if(totalAmount >= amount)
						{
							return true;
						}
						break;
					default:	
						return true;
						break;
				}			
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

	static void CreateItemInPlayerInventory(PlayerBase player, TR_Trader_Item trItem, int amount)
	{
		string itemLower = trItem.ClassName;
		string itemType = trItem.ClassName;
		itemLower.ToLower();
		
		if (amount > 0)
		{
			EntityAI newItem = EntityAI.Cast(GetGame().CreateObjectEx(itemType, player.GetPosition(), ECE_PLACE_ON_SURFACE));
			if (!newItem)
			{
				Error("[Trader] Failed to spawn entity "+itemType+" , make sure the classname exists and item can be spawned");
				return;
			}
			if (newItem)
			{
				if(trItem && trItem.IsPreset)
				{
					foreach(TraderObjectAttachment att : trItem.Attachments)
					{
						att.SpawnAttachment(newItem, player);
					}
				}
				if(player.ServerTakeEntityToInventory(FindInventoryLocationType.CARGO | FindInventoryLocationType.ATTACHMENT, newItem))
				{
					TraderMessage.PlayerWhite(newItem.GetDisplayName() + "\n" + "#tm_added_to_inventory", player);
				}
				else
				{
					TraderMessage.PlayerWhite(newItem.GetDisplayName() + "\n" + "#tm_was_placed_on_ground", player);
				}

				switch (trItem.Type)
				{
					case TR_Item_Type.Magazine: 
						Magazine newMagItem = Magazine.Cast(newItem);
						if(newMagItem)					
						{	
							newMagItem.ServerSetAmmoCount(amount);
						}
						break;
					case TR_Item_Type.Ammo: 
						Ammunition_Base newammoItem = Ammunition_Base.Cast(newItem);
						if(newammoItem)					
						{	
							newammoItem.SetQuantityTR(amount);
						}
						break;
					case TR_Item_Type.QuantItem: 
						ItemBase newItemBase;
						if (Class.CastTo(newItemBase, newItem))
						{
							newItemBase.SetQuantityTR(amount);
						}
						break;
					case TR_Item_Type.Bottle: 
						// Bottle_Base bottle = Bottle_Base.Cast(newItem);
						// if(bottle)
						// {
						// 	bottle.SetQuantity(0);
						// }
						break;
					default:
						break;
				}			
			}			
		}
	}

	static void CreateCurrencyItemInPlayerInventory(PlayerBase player, string classname, int amount)
	{
		array<EntityAI> itemsArray = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		string itemLower = classname;
		string itemType = classname;
		itemLower.ToLower();
		
		int currentAmount = amount;
		ItemBase item;
		if (currentAmount > 0)
		{
			//stack first
			for (int i = 0; i < itemsArray.Count(); i++)
			{
				if (currentAmount <= 0)
				{
					return;
				}
				Class.CastTo(item, itemsArray.Get(i));
				string itemPlayerClassname = "";
				if (item)
				{
					if (item.IsRuined())
						continue;

					itemPlayerClassname = item.GetType();
					itemPlayerClassname.ToLower();
					if (itemLower == itemPlayerClassname && !item.IsFullQuantity() && !item.IsMagazine())
					{
						currentAmount = item.AddQuantityTR(currentAmount);
					}
				}
			}

			EntityAI newItem = EntityAI.Cast(GetGame().CreateObjectEx(itemType, player.GetPosition(), ECE_PLACE_ON_SURFACE));
			if (!newItem)
			{
				Error("[Trader] Failed to spawn entity "+itemType+" , make sure the classname exists and item can be spawned");
				return;
			}
			if (newItem)
			{		
				Class.CastTo(item, newItem);
				item.SetQuantityTR(currentAmount);
				if(!player.ServerTakeEntityToInventory(FindInventoryLocationType.CARGO | FindInventoryLocationType.ATTACHMENT, newItem))
				{
					TraderMessage.PlayerWhite(newItem.GetDisplayName() + "\n" + "#tm_was_placed_on_ground", player);
				}			
			}			
		}
	}

	static bool RemoveFromPlayerInventory(PlayerBase player, TR_Trader_Item trItem, int amount)
	{
		string itemClassname = trItem.ClassName;
		itemClassname.ToLower();
		

		string itemPlayerClassname = "";
		int currentAmount = amount;
		ItemBase item = ItemBase.Cast(player.GetHumanInventory().GetEntityInHands());
		if (item)
		{
			itemPlayerClassname = item.GetType();
			itemPlayerClassname.ToLower();

			if(!TraderLibrary.IsItemAttached(item) && !item.IsRuined() && itemPlayerClassname == itemClassname)
			{
				//TODO: check if its okay to just delet item from hands				
				if(TraderLibrary.RemoveItem(item, trItem, currentAmount))
				{
					return true;
				}
			}
		}


		array<EntityAI> itemsArray = new array<EntityAI>;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);

		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));
			itemPlayerClassname = "";

			if (!item)
				continue;

			if (item.IsRuined())
				continue;

			if (TraderLibrary.IsItemAttached(item))
				continue;

			itemPlayerClassname = item.GetType();
			itemPlayerClassname.ToLower();

			if(itemPlayerClassname == itemClassname)
			{
				if(TraderLibrary.RemoveItem(item, trItem, currentAmount))
				{
					return true;
				}
			}
		}
		return false;
	}

	static bool RemoveItem(ItemBase item, TR_Trader_Item trItem, out int amount)
	{
		int itemAmount = TraderLibrary.GetItemAmount(item);
		switch (trItem.Type)
		{
			case TR_Item_Type.QuantItem:
				if(itemAmount == amount)
				{
					GetGame().ObjectDelete(item);
					return true;
				}
				else if(itemAmount > amount)
				{
					item.SetQuantityTR(itemAmount - amount);
					return true;
				}
				else if(itemAmount < amount)
				{					
					GetGame().ObjectDelete(item);
					amount = amount - itemAmount;
					return false;
				}
				return false;
				break;
			case TR_Item_Type.Ammo:
				Ammunition_Base newammoItem = Ammunition_Base.Cast(item);
				if(itemAmount == amount)
				{
					GetGame().ObjectDelete(item);
					return true;
				}
				else if(newammoItem)					
				{	
					if(itemAmount > amount)
					{
						newammoItem.SetQuantityTR(itemAmount - amount);
						return true;
					}
					else if(itemAmount < amount)
					{					
						GetGame().ObjectDelete(newammoItem);
						amount = amount - itemAmount;
						return false;
					}
				}
				return false;
				break;
			default:
				GetGame().ObjectDelete(item);
				return true;
				break;
		}	
		return false;
	}

	static void IncreasePlayerCurrency(PlayerBase player, TD_Trader trader, int currencyAmount)
	{
		if (currencyAmount == 0)
			return;

		EntityAI entity;
		ItemBase item;
 		PluginTraderData traderDataPlugin = PluginTraderData.Cast(GetPlugin(PluginTraderData));
        if(traderDataPlugin)
        {
            TR_Trader_Currency currency = traderDataPlugin.GetCurrencyByName(trader.Currency);
            if(currency)
            {
				for (int i = currency.CurrencyNotes.Count() - 1; i < currency.CurrencyNotes.Count(); i--)
				{
					string className = currency.CurrencyNotes.Get(i).ClassName;
					int itemMaxAmount = TraderLibrary.GetMaxQuantityForClassName(className);
					while (currencyAmount / currency.CurrencyNotes.Get(i).Value > 0)
					{
						if (currencyAmount > itemMaxAmount * currency.CurrencyNotes.Get(i).Value)
						{
							TraderLibrary.CreateCurrencyItemInPlayerInventory(player, className, itemMaxAmount);
							currencyAmount -= itemMaxAmount * currency.CurrencyNotes.Get(i).Value;
						}
						else
						{
							TraderLibrary.CreateCurrencyItemInPlayerInventory(player, className, currencyAmount / currency.CurrencyNotes.Get(i).Value);
							currencyAmount -= (currencyAmount / currency.CurrencyNotes.Get(i).Value * currency.CurrencyNotes.Get(i).Value);
						}

						if (currencyAmount == 0)
							return;
					}
				}
			}
		}
	}

	static void DeductPlayerCurrency(PlayerBase player, TD_Trader trader, int currencyAmount)
	{		
		if (currencyAmount == 0)
			return;

		array<EntityAI> itemsArray = new array<EntityAI>;
		ItemBase item;
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		PluginTraderData traderDataPlugin = PluginTraderData.Cast(GetPlugin(PluginTraderData));
        if(traderDataPlugin)
        {
            TR_Trader_Currency currency = traderDataPlugin.GetCurrencyByName(trader.Currency);
            if(currency)
            {
				for (int i = 0; i < currency.CurrencyNotes.Count(); i++)
				{
					for (int j = 0; j < itemsArray.Count(); j++)
					{
						Class.CastTo(item, itemsArray.Get(j));
						
						if (!item)
							continue;
						string itemType = item.GetType();
                        itemType.ToLower();
                        string crlower = currency.CurrencyNotes.Get(i).ClassName;
                        crlower.ToLower();
						if(itemType == crlower)
						{
							int itemAmount = TraderLibrary.GetItemAmount(item);
							if(itemAmount * currency.CurrencyNotes.Get(i).Value > currencyAmount)
							{
								if (currencyAmount >= currency.CurrencyNotes.Get(i).Value)
								{
									item.SetQuantityTR(itemAmount - (currencyAmount / currency.CurrencyNotes.Get(i).Value));								
									currencyAmount -= (currencyAmount / currency.CurrencyNotes.Get(i).Value) * currency.CurrencyNotes.Get(i).Value;
								}

								if (currencyAmount < currency.CurrencyNotes.Get(i).Value)
								{
									TraderLibrary.ExchangeCurrency(player, trader, item, currencyAmount, currency.CurrencyNotes.Get(i).Value);
									return;
								}
							}
							else
							{
								GetGame().ObjectDelete(itemsArray.Get(j));							
								currencyAmount -= itemAmount * currency.CurrencyNotes.Get(i).Value;
							}
						}
					}
				}
			}
		}
	}

	static void ExchangeCurrency(PlayerBase player, TD_Trader trader, ItemBase item, int currencyAmount, int currencyValue)
	{
		if (!item)
			return;

		if (currencyAmount == 0)
			return;

		//TraderMessage.PlayerWhite("EXCHANGE " + item.GetType() + " [" + currencyValue + "] " + currencyAmount, PlayerBase.Cast(this));

		int itemAmount = TraderLibrary.GetItemAmount(item);

		if (itemAmount <= 1)
		{
			item.Delete();
		}
		else
		{
			item.SetQuantityTR(itemAmount - 1);
		}

		TraderLibrary.IncreasePlayerCurrency(player, trader, currencyValue - currencyAmount);
	}
}