modded class DayZPlayerImplement
{
	ref TraderNotifications m_Trader_TraderNotifications;
	
	bool m_Trader_TraderModIsLoaded = false;
	bool m_Trader_TraderModIsLoadedHandled = false;
	
	bool m_Trader_IsInSafezone = false;
	float m_Trader_IsInSafezoneTimeout = 0;
	int m_Trader_InfluenzaEnteredSafeZone;
	
	bool m_Trader_RecievedAllData = false;
	
	int m_Trader_LastSelledTime = 0;
	int m_Trader_LastBuyedTime = 0;

	float m_Trader_BuySellTimer = 0.3;

	string itemDisplayNameClient;
	bool m_Trader_IsSelling;

	ref array<string> m_Trader_AdminPlayerUIDs;
	
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// OVERRIDES
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		
		if (GetGame().IsServer())
			handleServerRPCs(sender, rpc_type, ctx);
		else
			handleClientRPCs(sender, rpc_type, ctx);
	}

	override void SetSuicide(bool state)
	{
		super.SetSuicide(state);

		if (state && m_Trader_IsInSafezone && GetGame().IsServer())
			SetAllowDamage(true);
	}

	void CreateItemInInventory(string itemType, int amount)
	{
		array<EntityAI> itemsArray = new array<EntityAI>;
		GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		string itemLower = itemType;
		itemLower.ToLower();

		int currentAmount = amount;
		ItemBase item;
		Ammunition_Base ammoItem;
		//if item doesn't have count, quantity or it has quantitybar then we should spawn one item instead of trying to stack it
		//if item has quantitybar we should spawn one with full bar? maybe make it an option for stuff like gasoline canister
		bool hasSomeQuant = (TraderLibrary.ItemHasCount(itemType) || TraderLibrary.ItemHasQuantity(itemType)) && !TraderLibrary.HasQuantityBar(itemType) && amount >= 0;		
		int itemHasSpawnedOrStacked = 0;
		//autostacking
		//check if we have any stackable items of the type itemType
		//if we do, then add to each stack until no more stacks found or out of amount
		//we should keep count of how many items we spawned
		if (hasSomeQuant)
		{
			for (int i = 0; i < itemsArray.Count(); i++)
			{
				if (currentAmount <= 0)
					break;
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
						itemDisplayNameClient = item.GetDisplayName();
						itemHasSpawnedOrStacked++;
					}
				}

				Class.CastTo(ammoItem, itemsArray.Get(i));
				if (ammoItem)
				{
					if (ammoItem.IsRuined())
						continue;
					itemPlayerClassname = ammoItem.GetType();
					itemPlayerClassname.ToLower();
					if (itemLower == itemPlayerClassname && ammoItem.IsAmmoPile())
					{
						currentAmount = ammoItem.AddQuantityTR(currentAmount);
						itemDisplayNameClient = ammoItem.GetDisplayName();
						itemHasSpawnedOrStacked++;
					}
				}
			}
		}
		else
		{
			currentAmount = 1;
		}

		if (itemHasSpawnedOrStacked > 0 && !itemType.Contains("Ruble"))
			TraderMessage.PlayerWhite("#tm_some" + " " + itemDisplayNameClient + "\n" + "#tm_added_to_inventory", PlayerBase.Cast(this));

		//any leftover or new stacks
		if (currentAmount > 0 || !hasSomeQuant)
		{
			EntityAI newItem = EntityAI.Cast(GetInventory().CreateInInventory(itemType));
			if (!newItem)
			{
				for (int j = 0; j < itemsArray.Count(); j++)
				{
					Class.CastTo(item, itemsArray.Get(j));
					if (!item)
						continue;
					newItem = EntityAI.Cast(item.GetInventory().CreateInInventory(itemType)); //CreateEntityInCargo	
					if (newItem)
						break;
				}
			}
			if (newItem)
			{
				TraderMessage.PlayerWhite(newItem.GetDisplayName() + "\n" + "#tm_added_to_inventory", PlayerBase.Cast(this));
			}
			if (!newItem)
			{
				newItem = EntityAI.Cast(GetGame().CreateObjectEx(itemType, GetPosition(), ECE_PLACE_ON_SURFACE));
				TraderMessage.PlayerWhite(newItem.GetDisplayName() + "\n" + "#tm_was_placed_on_ground", PlayerBase.Cast(this));
				//GetGame().RPCSingleParam(PlayerBase.Cast(this), TRPCs.RPC_SEND_MENU_BACK, new Param1<bool>(true), true, GetIdentity());
				if (!newItem)
				{
					Error("[TraderFix] Failed to spawn entity "+itemType+" , make sure the classname exists and item can be spawned");
					return;
				}
			}
			
			Magazine newMagItem = Magazine.Cast(newItem);
			Ammunition_Base newammoItem = Ammunition_Base.Cast(newItem);
			if(newMagItem && !newammoItem)					
			{	
				newMagItem.ServerSetAmmoCount(amount);
				currentAmount = 0;
				UpdateInventoryMenu();
				return;
			}
			if (hasSomeQuant)
			{
				if (newammoItem)
				{
					currentAmount = newammoItem.SetQuantityTR(currentAmount);
					UpdateInventoryMenu();
					return;
				}	
				ItemBase newItemBase;
				if (Class.CastTo(newItemBase, newItem))
				{
					currentAmount = newItemBase.SetQuantityTR(currentAmount);
				}
			}
		}
		UpdateInventoryMenu(); // RPC-Call needed?
	}

	int createVehicleKeyInPlayerInventory(int hash = 0, string classname = "")
	{
		array<string> vehicleKeyClasses ={ "VehicleKeyRed", "VehicleKeyBlack", "VehicleKeyGrayCyan", "VehicleKeyYellow", "VehicleKeyPurple" };

		if (classname == "")
			classname = vehicleKeyClasses.Get(vehicleKeyClasses.GetRandomIndex());

		array<EntityAI> itemsArray = new array<EntityAI>;
		GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		EntityAI entity = EntityAI.Cast(GetInventory().CreateInInventory(classname));
		ItemBase item;
		if (!entity)
		{
			for (int j = 0; j < itemsArray.Count(); j++)
			{
				Class.CastTo(item, itemsArray.Get(j));
				if (!item)
					continue;
				entity = EntityAI.Cast(item.GetInventory().CreateInInventory(classname)); //CreateEntityInCargo	
				if (entity)
					break;
			}
		}
		if (entity)
			TraderMessage.PlayerWhite(TraderLibrary.GetItemDisplayName("VehicleKey") + "\n " + "#tm_added_to_inventory", PlayerBase.Cast(this));
		if (!entity)
		{
			entity = EntityAI.Cast(GetGame().CreateObjectEx(classname, GetPosition(), ECE_PLACE_ON_SURFACE));
			TraderMessage.PlayerWhite("#tm_inventory_full" + "\n" + TraderLibrary.GetItemDisplayName("VehicleKey") + "\n" + "#tm_was_placed_on_ground", PlayerBase.Cast(this));
			if (!entity)
			{
				Error("failed to spawn entity "+classname+" , make sure the classname exists and item can be spawned");
				return 0;
			}
		}

		VehicleKeyBase vehicleKey;
		Class.CastTo(vehicleKey, entity);

		if (!vehicleKey)
			return 0;

		if (hash <= 0)
			hash = vehicleKey.GenerateNewHash();
		else
			hash = vehicleKey.SetNewHash(hash);

		return hash;
	}

	

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// SERVER RPC HANDLING
	void handleServerRPCs(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		if (rpc_type == TRPCs.RPC_TRADER_MOD_IS_LOADED && !m_Trader_TraderModIsLoaded)
				handleTraderModIsLoadedRPC(sender, rpc_type, ctx);

		if (rpc_type == TRPCs.RPC_BUY)
			handleBuyRPC(sender, rpc_type, ctx);

		if (rpc_type == TRPCs.RPC_SELL)
			handleSellRPC(sender, rpc_type, ctx);
	}

	void handleTraderModIsLoadedRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param1<PlayerBase> rp0 = new Param1<PlayerBase>( NULL );
		ctx.Read(rp0);
		
		PlayerBase player = rp0.param1;
		
		m_Trader_TraderModIsLoaded = true;
		
		GetGame().RPCSingleParam(player, TRPCs.RPC_TRADER_MOD_IS_LOADED_CONFIRM, new Param1<PlayerBase>( player ), true, player.GetIdentity());
	}

	void handleBuyRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param3<int, int, string> rpb = new Param3<int, int, string>(-1, -1, "");
		ctx.Read(rpb);

		int traderUID = rpb.param1;
		int itemID = rpb.param2;
		itemDisplayNameClient = rpb.param3;

		m_Trader_IsSelling = false;

		if (GetGame().GetTime() - m_Trader_LastBuyedTime < m_Trader_BuySellTimer * 1000)
			return;
		m_Trader_LastBuyedTime = GetGame().GetTime();

		if (itemID >= m_Trader_ItemsClassnames.Count() || itemID < 0 || traderUID >= m_Trader_TraderPositions.Count() || traderUID < 0)
			return;

		string itemType = m_Trader_ItemsClassnames.Get(itemID);
		int itemQuantity = m_Trader_ItemsQuantity.Get(itemID);
		int itemCosts = m_Trader_ItemsBuyValue.Get(itemID);

		vector playerPosition = GetPosition();

		if (vector.Distance(playerPosition, m_Trader_TraderPositions.Get(traderUID)) > 1.7)
		{
			traderServerLog("tried to access the Trader out of Range! This could be an Hacker!");
			return;
		}

		int currencyAmount = TraderLibrary.GetPlayerCurrencyAmount(traderUID, PlayerBase.Cast(this));

		if (itemCosts < 0)
		{
			TraderMessage.PlayerWhite("#tm_cant_be_bought", PlayerBase.Cast(this));
			return;
		}

		if (currencyAmount < itemCosts)
		{
			TraderMessage.PlayerWhite("#tm_cant_afford", PlayerBase.Cast(this));
			return;
		}

		int vehicleKeyHash = 0;

		bool isDuplicatingKey = false;
		if (itemQuantity == -7) // is Key duplication
		{
			VehicleKeyBase vehicleKeyinHands = VehicleKeyBase.Cast(GetHumanInventory().GetEntityInHands());

			if (!vehicleKeyinHands)
			{
				TraderMessage.PlayerWhite("Put the Key you\nwant to duplicate\nin your Hands!", PlayerBase.Cast(this));
				return;
			}

			isDuplicatingKey = true;
			vehicleKeyHash = vehicleKeyinHands.GetHash();
			itemType = vehicleKeyinHands.GetType();
			itemQuantity = 1;
		}
		
		bool isLostKey = false;
		if (itemQuantity == -8) // is Lost Key
		{
			array<Transport> foundVehicles = GetVehicleToGetKeyFor(traderUID);

			if (foundVehicles.Count() < 1)
			{
				TraderMessage.PlayerWhite("There is no Vehicle\nin the Spawn Area!\nMake sure you was the last Driver!", this);
				return;
			}

			if (foundVehicles.Count() > 1)
			{
				TraderMessage.PlayerWhite("Multiple Vehicles found\nin the Spawn Area!", this);
				return;
			}

			CarScript carScript;
			Class.CastTo(carScript, foundVehicles.Get(0));

			vehicleKeyHash = carScript.m_Trader_VehicleKeyHash

			if (canCreateItemInPlayerInventory("VehicleKeyBase", 1))
			{
				TraderMessage.PlayerWhite(TraderLibrary.GetItemDisplayName("VehicleKey") + "\n " + "#tm_added_to_inventory", this);
				vehicleKeyHash = createVehicleKeyInPlayerInventory(vehicleKeyHash);
			}
			else
			{
				TraderMessage.PlayerWhite("#tm_inventory_full" + "\n" + TraderLibrary.GetItemDisplayName("VehicleKey") + "\n" + "#tm_was_placed_on_ground", this);
				vehicleKeyHash = spawnVehicleKeyOnGround(vehicleKeyHash);
				GetGame().RPCSingleParam(this, TRPCs.RPC_SEND_MENU_BACK, new Param1<bool>(false), true, this.GetIdentity());
			}

			deductPlayerCurrency(traderUID, itemCosts);
			
			carScript.m_Trader_HasKey = true;
			carScript.m_Trader_VehicleKeyHash = vehicleKeyHash;
			carScript.SynchronizeValues();

			isLostKey = true;
			itemType = "VehicleKeyLost";
			itemQuantity = 1;
		}

		traderServerLog("bought " + TraderLibrary.GetItemDisplayName(itemType) + "(" + itemType + ")");

		if (itemQuantity == -2 || itemQuantity == -6) // Is a Vehicle
		{
			string blockingObject = isVehicleSpawnFree(traderUID);

			if (blockingObject != "FREE")
			{
				TraderMessage.PlayerWhite(TraderLibrary.GetItemDisplayName(blockingObject) + " " + "#tm_way_blocked", PlayerBase.Cast(this));
				return;
			}

			if (itemQuantity == -2)
				vehicleKeyHash = createVehicleKeyInPlayerInventory();

			deductPlayerCurrency(traderUID, itemCosts);

			TraderMessage.PlayerWhite("" + itemDisplayNameClient + "\n" + "#tm_parked_next_to_you", PlayerBase.Cast(this));

			spawnVehicle(traderUID, itemType, vehicleKeyHash);

			GetGame().RPCSingleParam(PlayerBase.Cast(this), TRPCs.RPC_SEND_MENU_BACK, new Param1<bool>(false), true, GetIdentity());
		}
		else if (itemType != "VehicleKeyLost")// Is not a Vehicle
		{
			deductPlayerCurrency(traderUID, itemCosts);
			if (isDuplicatingKey)
				createVehicleKeyInPlayerInventory(vehicleKeyHash, itemType);
			else
				CreateItemInInventory(itemType, itemQuantity);
		}
	}

	void handleSellRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		//need cat id too
		Param3<int, int, string> rps = new Param3<int, int, string>( -1, -1, "" );
		ctx.Read(rps);

		int traderUID = rps.param1;
		int itemID = rps.param2;
		itemDisplayNameClient = rps.param3;

		m_Trader_IsSelling = true;

		if (GetGame().GetTime() - m_Trader_LastSelledTime < m_Trader_BuySellTimer * 1000)
			return;
		m_Trader_LastSelledTime = GetGame().GetTime();

		if (itemID >= m_Trader_ItemsClassnames.Count() || itemID < 0 || traderUID >= m_Trader_TraderPositions.Count() || traderUID < 0)
			return;

		string itemType = m_Trader_ItemsClassnames.Get(itemID);
		int itemQuantity = m_Trader_ItemsQuantity.Get(itemID);
		int itemSellValue = m_Trader_ItemsSellValue.Get(itemID);

		vector playerPosition = GetPosition();	

		if (vector.Distance(playerPosition, m_Trader_TraderPositions.Get(traderUID)) > 1.7)
		{
			traderServerLog("tried to access the Trader out of Range! This could be an Hacker!");
			return;
		}

		PlayerBase playerbase = PlayerBase.Cast(this);
		Object vehicleToSell = TraderLibrary.GetVehicleToSell(this, traderUID, itemType);

		if (itemSellValue < 0)
		{
			TraderMessage.PlayerWhite("#tm_cant_be_sold", playerbase);
			return;
		}

		if (!TraderLibrary.IsInPlayerInventory(playerbase, itemType, itemQuantity) && !vehicleToSell)
		{
			TraderMessage.PlayerWhite("#tm_you_cant_sell", playerbase);

			if (itemQuantity == -2 || itemQuantity == -6)
				TraderMessage.PlayerWhite("#tm_cant_sell_vehicle", playerbase);
				//TraderMessage.PlayerWhite("Turn the Engine on and place it inside the Traffic Cones!", PlayerBase.Cast(this));

			return;
		}

		traderServerLog("#tm_sold" + " " + TraderLibrary.GetItemDisplayName(itemType) + " (" + itemType + ")");

		TraderMessage.PlayerWhite("" + itemDisplayNameClient + "\n" + "#tm_was_sold", PlayerBase.Cast(this));

		if (vehicleToSell)
			deleteObject(vehicleToSell);
		else
			removeFromPlayerInventory(itemType, itemQuantity);
		
		increasePlayerCurrency(traderUID, itemSellValue);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// CLIENT RPC HANDLING
	void handleClientRPCs(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		switch(rpc_type)
		{			
			case TRPCs.RPC_SEND_MENU_BACK:
				handleSendMenuBackRPC(sender, rpc_type, ctx);
			break;

			case TRPCs.RPC_SEND_NOTIFICATION:
				HandleSendNotificationRPC(sender, rpc_type, ctx);
			break;

			case TRPCs.RPC_DELETE_SAFEZONE_MESSAGES:					
				DeleteAllMessages();
			break;
		}
	}

	
	void handleTraderModIsLoadedConfirmRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		m_Trader_TraderModIsLoaded = true;
		m_Trader_TraderModIsLoadedHandled = true;
	}

	void handleSyncObjectOrientationRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param2<Object, vector> syncObject_rp = new Param2<Object, vector>( NULL, "0 0 0" );
		ctx.Read( syncObject_rp );
		
		Object objectToSync = syncObject_rp.param1;
		vector objectToSyncOrientation  = syncObject_rp.param2;

		objectToSync.SetOrientation(objectToSyncOrientation);
	}

	void handleSendMenuBackRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		GetGame().GetUIManager().Back();
	}

	void HandleSendNotificationRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param4<string, float, int, bool> msg = new Param4<string, float, int, bool>( "", 0, 0, false);
		ctx.Read( msg );
		if(msg.param4)
		{
			ShowExitSafezoneMessage(msg.param2);
		}
		else
		{
			ShowTraderMessage(msg.param1, msg.param2, msg.param3);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// LOGS & MESSAGES
	void traderServerLog(string message)
	{
		TM_Print("[TRADER] Player: (" + GetIdentity().GetName() + ") " + GetIdentity().GetId() + " " + message);
	}

	void ShowTraderMessage(string message, float time, int color = 0)
	{
		if (!m_Trader_TraderNotifications)
		{
			m_Trader_TraderNotifications = new TraderNotifications();
			m_Trader_TraderNotifications.Init();
		}

		m_Trader_TraderNotifications.ShowMessage(message, time, color);
	}

	void ShowExitSafezoneMessage(float time)
	{
		if (!m_Trader_TraderNotifications)
		{
			m_Trader_TraderNotifications = new TraderNotifications();
			m_Trader_TraderNotifications.Init();
		}

		m_Trader_TraderNotifications.ShowExitSafezoneMessage(time);
	}

	void DeleteAllMessages()
	{
		if (!m_Trader_TraderNotifications)
			return;

		m_Trader_TraderNotifications.DeleteAllMessages();
	}

	array<ItemBase> getMergeableItemFromPlayerInventory(string itemType, int amount, bool absolute = false)
	{
		array<ItemBase> mergableItems = new array<ItemBase>;

		array<EntityAI> itemsArray = new array<EntityAI>;		
		GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);

		ItemBase itemToCombine = ItemBase.Cast(GetGame().CreateObject(itemType, "0 0 0"));

		if (!itemToCombine)
			return new array<ItemBase>;

		TraderLibrary.SetItemAmount(itemToCombine, amount);

		ItemBase item;		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));

			if (!item)
				continue;

			if (item.IsRuined())
				continue;

			if (itemToCombine.GetType() != item.GetType())
				continue;

			if (item.CanBeCombined(itemToCombine) && TraderLibrary.GetItemAmount(item) < TraderLibrary.GetMaxQuantityForClassName(item.GetType()))
			{
				amount -= TraderLibrary.GetMaxQuantityForClassName(item.GetType()) - TraderLibrary.GetItemAmount(item);
				TraderLibrary.SetItemAmount(itemToCombine, amount);
				mergableItems.Insert(item);
			}
		}

		GetGame().ObjectDelete(itemToCombine);

		if (absolute && amount > 0)
			return new array<ItemBase>;

		return mergableItems;
	}

	bool canCreateItemInPlayerInventory(string itemType, int amount)
	{
		array<ItemBase> mergeableItems = getMergeableItemFromPlayerInventory(itemType, amount);
		if (mergeableItems.Count() > 0)
			return true;

		EntityAI item = EntityAI.Cast(GetGame().CreateObject(itemType, "0 0 0"));
		if (!item)
			return false;

		TraderLibrary.SetItemAmount(ItemBase.Cast(item), amount);
		if(GetInventory().CanAddEntityToInventory(item))
		{
			GetGame().ObjectDelete(item);
			return true;
		}
		GetGame().ObjectDelete(item);

		EntityAI entityInHands = GetHumanInventory().GetEntityInHands();
		if (!entityInHands)
			return true;

		return false;			
	}

	int spawnVehicleKeyOnGround(int hash = 0, string classname = "")
	{
		array<string> vehicleKeyClasses = {"VehicleKeyRed", "VehicleKeyBlack", "VehicleKeyGrayCyan", "VehicleKeyYellow", "VehicleKeyPurple"};

		if (classname == "")
			classname = vehicleKeyClasses.Get(vehicleKeyClasses.GetRandomIndex());

		EntityAI entity;
		entity = SpawnEntityOnGroundPos(classname, GetPosition());

		if (!entity)
			return 0;

		VehicleKeyBase vehicleKey;
		Class.CastTo(vehicleKey, entity);

		if (!vehicleKey)
			return 0;

		//GetHive().CharacterSave(PlayerBase.Cast(this));

		if (hash <= 0)
			hash = vehicleKey.GenerateNewHash();
		else
			hash = vehicleKey.SetNewHash(hash);

		return hash;
	}

	void createItemInPlayerInventory(string itemType, int amount)
	{		
		EntityAI entity;

		array<ItemBase> mergeableItems = getMergeableItemFromPlayerInventory(itemType, amount);

		bool isMergingPossible = mergeableItems.Count() > 0;
		bool itemsAreLeftOverMerging = getMergeableItemFromPlayerInventory(itemType, amount, true).Count() == 0;

		if (isMergingPossible)
		{
			if (itemsAreLeftOverMerging)
			{
				entity = SpawnEntityOnGroundPos(itemType, GetPosition());
				
				if (m_Trader_IsSelling)
					TraderMessage.PlayerWhite("#tm_some_currency_on_ground", PlayerBase.Cast(this));
				else
					TraderMessage.PlayerWhite("#tm_some" + " " + itemDisplayNameClient + "\n" + "#tm_were_placed_on_ground", PlayerBase.Cast(this));

				GetGame().RPCSingleParam(PlayerBase.Cast(this), TRPCs.RPC_SEND_MENU_BACK, new Param1<bool>( false ), true, GetIdentity());
			}
			else
			{
				entity = SpawnEntityOnGroundPos(itemType, GetPosition());
			}
		}
		else
		{
			entity = GetHumanInventory().CreateInInventory(itemType);
		}

		if (!entity)
			return;

		ItemBase item;
		Class.CastTo(item, entity);

		if (!item)
			return;
		
		TraderLibrary.SetItemAmount(item, amount);

		for (int i = 0; i < mergeableItems.Count(); i++)
		{
			int mergeQuantity;

			if (TraderLibrary.GetItemAmount(mergeableItems.Get(i)) + TraderLibrary.GetItemAmount(item) <= TraderLibrary.GetMaxQuantityForClassName(mergeableItems.Get(i).GetType()))
				mergeQuantity = TraderLibrary.GetItemAmount(item);
			else
				mergeQuantity = TraderLibrary.GetMaxQuantityForClassName(mergeableItems.Get(i).GetType()) - TraderLibrary.GetItemAmount(mergeableItems.Get(i));

			//TraderMessage.PlayerWhite("MERGED " + mergeableItems.Get(i).GetType() + "; QTY: " + mergeQuantity, PlayerBase.Cast(this));

			TraderLibrary.SetItemAmount(mergeableItems.Get(i), TraderLibrary.GetItemAmount(mergeableItems.Get(i)) + mergeQuantity);
			TraderLibrary.SetItemAmount(item, TraderLibrary.GetItemAmount(item) - mergeQuantity);
			amount -= mergeQuantity;

			if (item)
			{
				if (TraderLibrary.GetItemAmount(item) <= 0 || amount <= 0)
				{
					deleteItem(item);
					UpdateInventoryMenu(); // RPC-Call needed?

					return;
				}
			}
		}

		UpdateInventoryMenu(); // RPC-Call needed?
	}

	void spawnItemOnGround(string itemType, int amount, vector position)
	{		
		EntityAI entity = SpawnEntityOnGroundPos(itemType, position);

		if (!entity)
			return;

		ItemBase item;
		Class.CastTo(item, entity);

		if (!item)
			return;

		TraderLibrary.SetItemAmount(item, amount);
	}

	bool removeFromPlayerInventory(string itemClassname, int amount)
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
		

		string itemPlayerClassname = "";
		int itemAmount = -1;

		ItemBase item = ItemBase.Cast(GetHumanInventory().GetEntityInHands());
		if (item)
		{
			itemPlayerClassname = item.GetType();
			itemPlayerClassname.ToLower();

			if(!TraderLibrary.IsItemAttached(item) && !item.IsRuined() && itemPlayerClassname == itemClassname && ((TraderLibrary.GetItemAmount(item) >= amount && !isMagazine && !isWeapon && !isSteak) || isMagazine || isWeapon || (isSteak && (TraderLibrary.GetItemAmount(item) >= TraderLibrary.GetMaxQuantityForClassName(itemPlayerClassname) * 0.5))))
			{
				itemAmount = TraderLibrary.GetItemAmount(item);
				
				if (itemAmount == amount || isMagazine || isWeapon || isSteak)
				{
					deleteItem(item);
					//DeleteItemAndOfferRewardsForAllAttachments(item);
					
					UpdateInventoryMenu(); // RPC-Call needed?
					return true;
				}
				else
				{
					TraderLibrary.SetItemAmount(item, itemAmount - amount);
				
					UpdateInventoryMenu(); // RPC-Call needed?
					return true;
				}
			}
		}


		array<EntityAI> itemsArray = new array<EntityAI>;
		GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		
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

			if(itemPlayerClassname == itemClassname && ((TraderLibrary.GetItemAmount(item) >= amount && !isMagazine && !isWeapon && !isSteak) || isMagazine || isWeapon || (isSteak && (TraderLibrary.GetItemAmount(item) >= TraderLibrary.GetMaxQuantityForClassName(itemPlayerClassname) * 0.5))))
			{
				itemAmount = TraderLibrary.GetItemAmount(item);
				
				if (itemAmount == amount || isMagazine || isWeapon || isSteak)
				{
					deleteItem(item);
					//DeleteItemAndOfferRewardsForAllAttachments(item);
					
					UpdateInventoryMenu(); // RPC-Call needed?
					return true;
				}
				else
				{
					TraderLibrary.SetItemAmount(item, itemAmount - amount);
				
					UpdateInventoryMenu(); // RPC-Call needed?
					return true;
				}
			}
		}
		
		UpdateInventoryMenu(); // RPC-Call needed?
		return false;
	}

	//TODO: We need something like this but better
	//removed for now
	void DeleteItemAndOfferRewardsForAllAttachments(ItemBase item)
	{
		int totalSellValue = 0;
		for ( int i = 0; i < item.GetInventory().AttachmentCount(); i++ )
		{
			EntityAI attachment = item.GetInventory().GetAttachmentFromIndex ( i );
			int itemID = m_Trader_ItemsClassnames.Find(attachment.GetType());
			if ( itemID != -1 )
			{
				int itemQuantity = m_Trader_ItemsQuantity.Get(itemID);
				int itemSellValue = m_Trader_ItemsSellValue.Get(itemID);
				if (itemSellValue < 0)
					continue;
				totalSellValue += itemSellValue;
			}
		}
		increasePlayerCurrency(totalSellValue);
		item.Delete();
	}

	void deleteItem(ItemBase item)
	{
		if (item)
			GetGame().ObjectDelete(item);
			//item.Delete();

		//TraderMessage.PlayerWhite("DELETED " + item.GetType() + "; QTY: " + TraderLibrary.GetItemAmount(item), PlayerBase.Cast(this));
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// CURRENCY
	void increasePlayerCurrency(int traderID, int currencyAmount)
	{
		if (currencyAmount == 0)
			return;

		EntityAI entity;
		ItemBase item;
 		PluginTraderData traderDataPlugin = PluginTraderData.Cast(GetPlugin(PluginTraderData));
        if(traderDataPlugin)
        {
            TD_Trader trader = traderDataPlugin.GetTraderByID(traderID);
            TR_Trader_Currency currency = traderDataPlugin.GetCurrencyByName(trader.Currency);
            if(currency)
            {
				for (int i = currency.CurrencyNotes.Count() - 1; i < currency.CurrencyNotes.Count(); i--)
				{
					int itemMaxAmount = TraderLibrary.GetMaxQuantityForClassName(currency.CurrencyNotes.Get(i).ClassName);

					while (currencyAmount / currency.CurrencyNotes.Get(i).Value > 0)
					{
						if (currencyAmount > itemMaxAmount * currency.CurrencyNotes.Get(i).Value)
						{
							CreateItemInInventory(currency.CurrencyNotes.Get(i), itemMaxAmount);
							currencyAmount -= itemMaxAmount * currency.CurrencyNotes.Get(i).Value;
						}
						else
						{
							CreateItemInInventory(currency.CurrencyNotes.Get(i).ClassName, currencyAmount / currency.CurrencyNotes.Get(i).Value);
							currencyAmount -= (currencyAmount / currency.CurrencyNotes.Get(i).Value * currency.CurrencyNotes.Get(i).Value);
						}

						if (currencyAmount == 0)
							return;
					}
				}
			}
		}
	}

	void deductPlayerCurrency(int traderID, int currencyAmount)
	{		
		if (currencyAmount == 0)
			return;

		array<EntityAI> itemsArray = new array<EntityAI>;
		ItemBase item;
		GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		PluginTraderData traderDataPlugin = PluginTraderData.Cast(GetPlugin(PluginTraderData));
        if(traderDataPlugin)
        {
            TD_Trader trader = traderDataPlugin.GetTraderByID(traderID);
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
									TraderLibrary.SetItemAmount(item, itemAmount - (currencyAmount / currency.CurrencyNotes.Get(i).Value));
									UpdateInventoryMenu(); // RPC-Call needed?									
									currencyAmount -= (currencyAmount / currency.CurrencyNotes.Get(i).Value) * currency.CurrencyNotes.Get(i).Value;
								}

								if (currencyAmount < currency.CurrencyNotes.Get(i).Value)
								{
									exchangeCurrency(traderID, item, currencyAmount, currency.CurrencyNotes.Get(i).Value);
									return;
								}
							}
							else
							{
								GetGame().ObjectDelete(itemsArray.Get(j));								
								UpdateInventoryMenu(); // RPC-Call needed?								
								currencyAmount -= itemAmount * currency.CurrencyNotes.Get(i).Value;
							}
						}
					}
				}
			}
		}
	}

	void exchangeCurrency(int traderID, ItemBase item, int currencyAmount, int currencyValue)
	{
		if (!item)
			return;

		if (currencyAmount == 0)
			return;

		//TraderMessage.PlayerWhite("EXCHANGE " + item.GetType() + " [" + currencyValue + "] " + currencyAmount, PlayerBase.Cast(this));

		int itemAmount = TraderLibrary.GetItemAmount(item);

		if (itemAmount <= 1)
			deleteItem(item);
		else
			TraderLibrary.SetItemAmount(item, itemAmount - 1);

		increasePlayerCurrency(traderID, currencyValue - currencyAmount);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// VEHICLE
	string isVehicleSpawnFree(int traderUID)
	{
		vector size = "3 5 9";
		array<Object> excluded_objects = new array<Object>;
		array<Object> nearby_objects = new array<Object>;

		GetGame().IsBoxColliding( m_Trader_TraderVehicleSpawns.Get(traderUID), m_Trader_TraderVehicleSpawnsOrientation.Get(traderUID), size, excluded_objects, nearby_objects);
		if (nearby_objects.Count() > 0)
			return nearby_objects.Get(0).GetType();

		return "FREE";
	}

	array<Transport> GetVehicleToGetKeyFor(int traderUID)
	{
		vector size = "3 5 9";
		array<Object> excluded_objects = new array<Object>;
		array<Object> nearby_objects = new array<Object>;

		array<Transport> found_vehicles = new array<Transport>;
		Transport transport;

		if (GetGame().IsBoxColliding(m_Trader_TraderVehicleSpawns.Get(traderUID), m_Trader_TraderVehicleSpawnsOrientation.Get(traderUID), size, excluded_objects, nearby_objects))
		{
			for (int i = 0; i < nearby_objects.Count(); i++)
			{
				transport = NULL;
				Class.CastTo(transport, nearby_objects.Get(i));
				if (transport)
				{
					// Check if Player was last Driver:
					CarScript carsScript = CarScript.Cast(transport);
					if(!carsScript)
						continue;
					
					if(carsScript.m_Trader_LastDriverId != GetIdentity().GetId())
						continue;	

					found_vehicles.Insert(transport);
				}
			}
		}

		return found_vehicles;
	}

	void spawnVehicle(int traderUID, string vehicleType, int vehicleKeyHash)
	{
		vector objectPosition = m_Trader_TraderVehicleSpawns.Get(traderUID);
		vector objectDirection = m_Trader_TraderVehicleSpawnsOrientation.Get(traderUID);

		EntityAI vehicle = EntityAI.Cast(GetGame().CreateObjectEx( vehicleType, objectPosition, ECE_LOCAL | ECE_CREATEPHYSICS | ECE_TRACE));
		if(!vehicle)
		{
			//throw error
			return;
		}			
		GetGame().RemoteObjectCreate(vehicle);
		vehicle.SetOrientation(objectDirection);

		int vehicleId = -1;
		for (int i = 0; i < m_Trader_Vehicles.Count(); i++)
		{
			if (vehicleType == m_Trader_Vehicles.Get(i))
				vehicleId = i;
		}

		for (int j = 0; j < m_Trader_VehiclesParts.Count(); j++)
		{
			if (m_Trader_VehiclesPartsVehicleId.Get(j) == vehicleId)
			{
				EntityAI vehiclePart = vehicle.GetInventory().CreateAttachment(m_Trader_VehiclesParts.Get(j));
				if(!vehiclePart)
				{
					vehicle.GetInventory().CreateInInventory(m_Trader_VehiclesParts.Get(j));
				}
			}
		}

		// Try to fill Fuel, Oil, Brakeliquid, Coolantliquid and lock Vehicle:
		CarScript car = CarScript.Cast(vehicle);
		if (car)
		{
			car.Fill( CarFluid.FUEL, car.GetFluidCapacity( CarFluid.FUEL ));
			car.Fill( CarFluid.OIL, car.GetFluidCapacity( CarFluid.OIL ));
			car.Fill( CarFluid.BRAKE, car.GetFluidCapacity( CarFluid.BRAKE ));
			car.Fill( CarFluid.COOLANT, car.GetFluidCapacity( CarFluid.COOLANT ));

			car.Fill( CarFluid.USER1, car.GetFluidCapacity( CarFluid.USER1 ));
			car.Fill( CarFluid.USER2, car.GetFluidCapacity( CarFluid.USER2 ));
			car.Fill( CarFluid.USER3, car.GetFluidCapacity( CarFluid.USER3 ));
			car.Fill( CarFluid.USER4, car.GetFluidCapacity( CarFluid.USER4 ));
			car.m_Trader_IsInSafezone = true;

			if (vehicleKeyHash != 0)
			{
				car.m_Trader_Locked = true;
				car.m_Trader_HasKey = true;
				car.m_Trader_VehicleKeyHash = vehicleKeyHash;
			}

			car.SynchronizeValues();

			car.SetAllowDamage(false);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// OBJECTS
	void deleteObject(Object obj)
	{
		if (obj)
			GetGame().ObjectDelete(obj);
	}
}