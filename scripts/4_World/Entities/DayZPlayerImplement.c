class TRITEM
{
	string classname;
	int quantity;
};

modded class DayZPlayerImplement
{
	static const string m_Trader_ConfigFilePath = "$profile:Trader/TraderConfig.txt";
	static const string m_Trader_ObjectsFilePath = "$profile:Trader/TraderObjects.txt";
	static const string m_Trader_VehiclePartsFilePath = "$profile:Trader/TraderVehicleParts.txt";

	ref TraderNotifications m_Trader_TraderNotifications;
		
	//This variable will soon be removed. If you're a modder start using HasReceivedAllTraderData() function instead
	bool m_Trader_RecievedAllData = false;
	//This variable is not being used anymore. Backwards compatibility with Garage mod
	bool m_Trader_IsInSafezone = false;
	
	string m_Trader_CurrencyName;
	ref array<string> m_Trader_CurrencyClassnames;
	ref array<int> m_Trader_CurrencyValues;
	int m_Player_CurrencyAmount;

	int m_Trader_LastSelledTime = 0;
	int m_Trader_LastBuyedTime = 0;
	
	ref array<string> m_Trader_TraderNames;
	ref array<vector> m_Trader_TraderPositions;
	ref array<int> m_Trader_TraderIDs;
	ref array<int> m_Trader_TraderSafezones;
	ref array<vector> m_Trader_TraderVehicleSpawns;
	ref array<vector> m_Trader_TraderVehicleSpawnsOrientation;
	
	ref array<string> m_Trader_Categorys;
	ref array<int> m_Trader_CategorysTraderKey;
	
	ref array<int> m_Trader_ItemsTraderId;
	ref array<int> m_Trader_ItemsCategoryId;
	ref array<string> m_Trader_ItemsClassnames;
	ref array<int> m_Trader_ItemsQuantity;
	ref array<int> m_Trader_ItemsBuyValue;
	ref array<int> m_Trader_ItemsSellValue;	

	ref array<string> m_Trader_Vehicles;
	ref array<string> m_Trader_VehiclesParts;
	ref array<int> m_Trader_VehiclesPartsVehicleId;

	ref array<string> m_Trader_NPCDummyClasses;

	float m_Trader_BuySellTimer = 0.3;

	string itemDisplayNameClient;
	bool m_Trader_IsSelling;

	string m_Trader_PlayerUID;
	ref array<string> m_Trader_AdminPlayerUIDs;
	ref array<ref TRITEM> TR_Items_To_Spawn;
	ref Timer ItemSpawnTimer;
	
	bool HasReceivedAllTraderData()
	{
		return m_Trader_RecievedAllData;
	}

	void SetReceivedAllTraderData(bool state)
	{
		m_Trader_RecievedAllData = state;
	}
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// OVERRIDES
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		
		if (GetGame().IsServer())
			handleServerRPCs(sender, rpc_type, ctx);
		else
			handleClientRPCs(sender, rpc_type, ctx);
	}

	bool CreateItemInInventory(PlayerBase player, string itemType, int amount)
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
		bool hasSomeQuant = (TR_Helper.ItemHasCount(itemType) || TR_Helper.ItemHasQuantity(itemType)) && !TR_Helper.HasQuantityBar(itemType) && amount >= 0;		
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
			InventoryLocationType foundLocType;
			EntityAI newItem = EntityAI.Cast(TM_InventoryTransactions.CreateItemInPlayerInventory(itemType, this, foundLocType));
			if (!newItem)
			{
				foundLocType = InventoryLocationType.UNKNOWN;
			}

			switch ( foundLocType )
			{
				case InventoryLocationType.CARGO:
				case InventoryLocationType.ATTACHMENT:
					TraderMessage.PlayerWhite(newItem.GetDisplayName() + "\n" + "#tm_added_to_inventory", PlayerBase.Cast(this));
					break;
				case InventoryLocationType.GROUND:
					TraderMessage.PlayerWhite(newItem.GetDisplayName() + "\n" + "#tm_was_placed_on_ground", PlayerBase.Cast(this));
					break;
				case InventoryLocationType.UNKNOWN:
				{
					Error("[Trader] Failed to spawn entity "+itemType+" ! Make sure the classname exists and item can be spawned");
					return false;
				}
			}

			Magazine newMagItem = Magazine.Cast(newItem);
			Ammunition_Base newammoItem = Ammunition_Base.Cast(newItem);
			if(newMagItem && !newammoItem)					
			{	
				newMagItem.ServerSetAmmoCount(amount);
				currentAmount = 0;
				UpdateInventoryMenu();
				return true;
			}
			if (hasSomeQuant)
			{
				if (newammoItem)
				{
					currentAmount = newammoItem.SetQuantityTR(currentAmount);
					UpdateInventoryMenu();
					return true;
				}	
				ItemBase newItemBase;
				if (Class.CastTo(newItemBase, newItem))
				{
					currentAmount = newItemBase.SetQuantityTR(currentAmount);
				}
			}
		}
		UpdateInventoryMenu(); // RPC-Call needed?
		return true;
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
			TraderMessage.PlayerWhite(getItemDisplayName("VehicleKey") + "\n " + "#tm_added_to_inventory", PlayerBase.Cast(this));
		if (!entity)
		{
			entity = EntityAI.Cast(GetGame().CreateObjectEx(classname, GetPosition(), ECE_PLACE_ON_SURFACE));
			TraderMessage.PlayerWhite("#tm_inventory_full" + "\n" + getItemDisplayName("VehicleKey") + "\n" + "#tm_was_placed_on_ground", PlayerBase.Cast(this));
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

	void increasePlayerCurrency(int currencyAmount)
	{
		if (currencyAmount == 0)
			return;

		EntityAI entity;
		ItemBase item;
		PlayerBase player = PlayerBase.Cast(this);
		for (int i = m_Trader_CurrencyClassnames.Count() - 1; i < m_Trader_CurrencyClassnames.Count(); i--)
		{
			int itemMaxAmount = GetItemMaxQuantity(m_Trader_CurrencyClassnames.Get(i));
			if(itemMaxAmount == 0)
			{
				Error("[Trader] Currency "+ m_Trader_CurrencyClassnames.Get(i) +" has max quantity 0 which might mean this class doesn't exist."); 
				continue;
			}

			while (currencyAmount / m_Trader_CurrencyValues.Get(i) > 0)
			{
				if (currencyAmount > itemMaxAmount * m_Trader_CurrencyValues.Get(i))
				{
					CreateItemInInventory(player, m_Trader_CurrencyClassnames.Get(i), itemMaxAmount);
					currencyAmount -= itemMaxAmount * m_Trader_CurrencyValues.Get(i);
				}
				else
				{
					CreateItemInInventory(player, m_Trader_CurrencyClassnames.Get(i), currencyAmount / m_Trader_CurrencyValues.Get(i));
					currencyAmount -= (currencyAmount / m_Trader_CurrencyValues.Get(i) * m_Trader_CurrencyValues.Get(i));
				}

				if (currencyAmount == 0)
					return;
			}
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// SERVER RPC HANDLING
	void handleServerRPCs(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		if (rpc_type == TRPCs.RPC_BUY)
			handleBuyRPC(sender, rpc_type, ctx);

		if (rpc_type == TRPCs.RPC_SELL)
			handleSellRPC(sender, rpc_type, ctx);
	}

	void handleBuyRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param3<int, int, string> rpb = new Param3<int, int, string>(-1, -1, "");
		ctx.Read(rpb);

		int traderIndex = rpb.param1;
		int itemID = rpb.param2;
		itemDisplayNameClient = rpb.param3;

		m_Trader_IsSelling = false;

		if (GetGame().GetTime() - m_Trader_LastBuyedTime < m_Trader_BuySellTimer * 1000)
			return;
		m_Trader_LastBuyedTime = GetGame().GetTime();

		if (itemID >= m_Trader_ItemsClassnames.Count() || itemID < 0 || traderIndex >= m_Trader_TraderPositions.Count() || traderIndex < 0)
			return;

		string itemType = m_Trader_ItemsClassnames.Get(itemID);
		int itemQuantity = m_Trader_ItemsQuantity.Get(itemID);
		int itemCosts = m_Trader_ItemsBuyValue.Get(itemID);

		vector playerPosition = GetPosition();
		PlayerBase player = PlayerBase.Cast(this);
		vector traderPosition = m_Trader_TraderPositions.Get(traderIndex);
		float distanceToPlayer = vector.Distance(playerPosition, traderPosition);
		if (distanceToPlayer > TR_Helper.GetTraderAllowedTradeDistance())
		{
			traderServerLog("tried to access the Trader out of Range! This could be an Hacker!");
			return;
		}

		m_Player_CurrencyAmount = getPlayerCurrencyAmount();

		if (itemCosts < 0)
		{
			TraderMessage.PlayerWhite("#tm_cant_be_bought", player);
			return;
		}

		if (m_Player_CurrencyAmount < itemCosts)
		{
			TraderMessage.PlayerWhite("#tm_cant_afford", player);
			return;
		}

		int vehicleKeyHash = 0;

		bool isDuplicatingKey = false;
		if (itemQuantity == -7) // is Key duplication
		{
			VehicleKeyBase vehicleKeyinHands = VehicleKeyBase.Cast(GetHumanInventory().GetEntityInHands());

			if (!vehicleKeyinHands)
			{
				TraderMessage.PlayerWhite("Put the Key you\nwant to duplicate\nin your Hands!", player);
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
			array<Transport> foundVehicles = GetVehicleToGetKeyFor(traderIndex);

			if (foundVehicles.Count() < 1)
			{
				TraderMessage.PlayerWhite("There is no Vehicle\nin the Spawn Area!\nMake sure you was the last Driver!", player);
				return;
			}

			if (foundVehicles.Count() > 1)
			{
				TraderMessage.PlayerWhite("Multiple Vehicles found\nin the Spawn Area!", player);
				return;
			}

			CarScript carScript;
			Class.CastTo(carScript, foundVehicles.Get(0));

			vehicleKeyHash = carScript.m_Trader_VehicleKeyHash;

			if (canCreateItemInPlayerInventory("VehicleKeyBase", 1))
			{
				TraderMessage.PlayerWhite(getItemDisplayName("VehicleKey") + "\n " + "#tm_added_to_inventory", player);
				vehicleKeyHash = createVehicleKeyInPlayerInventory(vehicleKeyHash);
			}
			else
			{
				TraderMessage.PlayerWhite("#tm_inventory_full" + "\n" + getItemDisplayName("VehicleKey") + "\n" + "#tm_was_placed_on_ground", player);
				vehicleKeyHash = spawnVehicleKeyOnGround(vehicleKeyHash);
				GetGame().RPCSingleParam(this, TRPCs.RPC_SEND_MENU_BACK, new Param1<bool>(false), true, this.GetIdentity());
			}

			deductPlayerCurrency(itemCosts);
			
			carScript.m_Trader_HasKey = true;
			carScript.m_Trader_VehicleKeyHash = vehicleKeyHash;
			carScript.SynchronizeValues();

			isLostKey = true;
			itemType = "VehicleKeyLost";
			itemQuantity = 1;
		}


		if (itemQuantity == -2 || itemQuantity == -6) // Is a Vehicle
		{
			string blockingObject = isVehicleSpawnFree(traderIndex);

			if (blockingObject != "FREE")
			{
				TraderMessage.PlayerWhite(getItemDisplayName(blockingObject) + " " + "#tm_way_blocked", player);
				return;
			}

			if (itemQuantity == -2)
				vehicleKeyHash = createVehicleKeyInPlayerInventory();

			deductPlayerCurrency(itemCosts);

			traderTradesLog("bought " + getItemDisplayName(itemType) + "(" + itemType + ")");
			TraderMessage.PlayerWhite("" + getItemDisplayName(itemType) + "\n" + "#tm_parked_next_to_you", player);

			spawnVehicle(traderIndex, itemType, vehicleKeyHash);

			GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_MENU_BACK, new Param1<bool>(false), true, GetIdentity());
		}
		else if (itemType != "VehicleKeyLost")// Is not a Vehicle
		{
			traderTradesLog("bought " + getItemDisplayName(itemType) + "(" + itemType + ")");
			deductPlayerCurrency(itemCosts);
			if (isDuplicatingKey)
				createVehicleKeyInPlayerInventory(vehicleKeyHash, itemType);
			else
				CreateItemInInventory(player, itemType, itemQuantity);
		}
	}

	void handleSellRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param3<int, int, string> rps = new Param3<int, int, string>( -1, -1, "" );
		ctx.Read(rps);

		int traderIndex = rps.param1;
		int itemID = rps.param2;
		itemDisplayNameClient = rps.param3;

		m_Trader_IsSelling = true;

		if (GetGame().GetTime() - m_Trader_LastSelledTime < m_Trader_BuySellTimer * 1000)
			return;
		m_Trader_LastSelledTime = GetGame().GetTime();

		if (itemID >= m_Trader_ItemsClassnames.Count() || itemID < 0 || traderIndex >= m_Trader_TraderPositions.Count() || traderIndex < 0)
			return;

		string itemType = m_Trader_ItemsClassnames.Get(itemID);
		int itemQuantity = m_Trader_ItemsQuantity.Get(itemID);
		int itemSellValue = m_Trader_ItemsSellValue.Get(itemID);

		vector playerPosition = GetPosition();	

		PlayerBase player = PlayerBase.Cast(this);
		if (vector.Distance(playerPosition, m_Trader_TraderPositions.Get(traderIndex)) > TR_Helper.GetTraderAllowedTradeDistance())
		{
			traderServerLog("tried to access the Trader out of Range! This could be an Hacker!");
			return;
		}
		
		vector position = m_Trader_TraderVehicleSpawns.Get(traderIndex);
		vector orientation = m_Trader_TraderVehicleSpawnsOrientation.Get(traderIndex);
		Object vehicleToSell;
		bool isValidVehicle = false;
		if(itemQuantity == -2 || itemQuantity == -6)
		{
			vehicleToSell = GetVehicleToSell(itemType, position, orientation);
			if(vehicleToSell)
			{
				isValidVehicle = true;
			}
		}

		if (itemSellValue < 0)
		{
			TraderMessage.PlayerWhite("#tm_cant_be_sold", player);
			return;
		}

		bool sold = false;
		string persistentID;
		int b1;
		int b2;
		int b3;
		int b4;
		if(!isValidVehicle)
		{
			ItemBase sellableItem;
			if (!isInPlayerInventory(itemType, itemQuantity, sellableItem))
			{
				TraderMessage.PlayerWhite("#tm_you_cant_sell", player);

				if (itemQuantity == -2 || itemQuantity == -6)
					TraderMessage.PlayerWhite("#tm_cant_sell_vehicle", player);
					//TraderMessage.PlayerWhite("Turn the Engine on and place it inside the Traffic Cones!", player);

				return;
			}
			if(sellableItem)
			{
				sellableItem.GetPersistentID(b1, b2, b3, b4);				
				persistentID = itemType+ "["+b1+" "+b2+" "+b3+" "+b4+"]";
				sold = RemoveItem(sellableItem , itemType, itemQuantity);
			}
		}
		
		if (isValidVehicle)
		{			
			
			EntityAI entityVehicle = EntityAI.Cast(vehicleToSell);
			if(entityVehicle)
			{
				entityVehicle.GetPersistentID(b1, b2, b3, b4);
				persistentID = itemType+ "_"+b1+"_"+b2+"_"+b3+"_"+b4;
			}
			else
			{
				persistentID = itemType;
			}
			deleteObject(vehicleToSell);
			sold = true;
		}
		
		if(sold)
		{				
			string itemAmount = "";
			if(itemQuantity > 0)
			{
				itemAmount = itemQuantity.ToString() + "x ";
			}
			traderTradesLog("sold" + " "+ itemAmount + getItemDisplayName(itemType) + " (" + persistentID + ")");

			TraderMessage.PlayerWhite("" + getItemDisplayName(itemType) + "\n" + "#tm_was_sold", player);
			increasePlayerCurrency(itemSellValue);
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// CLIENT RPC HANDLING
	void handleClientRPCs(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		switch(rpc_type)
		{
			case TRPCs.RPC_SEND_TRADER_CURRENCYNAME_ENTRY:
				handleSendTraderCurrencyNameEntryRPC(sender, rpc_type, ctx);
			break;

			case TRPCs.RPC_SEND_TRADER_CURRENCY_ENTRY:
				handleSendTraderCurrencyEntryRPC(sender, rpc_type, ctx);
			break;
			
			case TRPCs.RPC_SEND_TRADER_NAME_ENTRY:
				handleSendTraderNameEntryRPC(sender, rpc_type, ctx);
			break;
			
			case TRPCs.RPC_SEND_TRADER_CATEGORY_ENTRY:
				handleSendTraderCategoryEntryRPC(sender, rpc_type, ctx);
			break;

			case TRPCs.RPC_SEND_TRADER_NPCDUMMY_ENTRY:
				handleSendTraderNPCDummyEntryRPC(sender, rpc_type, ctx);
			break;
			
			case TRPCs.RPC_SEND_TRADER_ITEM_ENTRY:
				handleSendTraderItemEntryRPC(sender, rpc_type, ctx);
			break;
			
			case TRPCs.RPC_SEND_TRADER_MARKER_ENTRY:
				handleSendTraderMarkerEntryRPC(sender, rpc_type, ctx);
			break;
			
			case TRPCs.RPC_SEND_TRADER_CLEAR:
				handleSendTraderClearRPC(sender, rpc_type, ctx);
			break;

			/*case TRPCs.RPC_SYNC_CARSCRIPT_ISINSAFEZONE:
				handleSyncCarscriptIsInSafezoneRPC(sender, rpc_type, ctx);
			break;*/

			case TRPCs.RPC_SEND_MENU_BACK:
				handleSendMenuBackRPC(sender, rpc_type, ctx);
			break;

			case TRPCs.RPC_SEND_NOTIFICATION:
				HandleSendNotificationRPC(sender, rpc_type, ctx);
			break;

			case TRPCs.RPC_SYNC_OBJECT_ORIENTATION:
				handleSyncObjectOrientationRPC(sender, rpc_type, ctx);
			break;

			case TRPCs.RPC_DELETE_SAFEZONE_MESSAGES:	
				GetTraderNotifications().DeleteAllMessages();
			break;

			case TRPCs.RPC_SEND_TRADER_VARIABLES_ENTRY:
				handleSendTraderVariablesEntryRPC(sender, rpc_type, ctx);
			break;

			case TRPCs.RPC_SEND_TRADER_PLAYERUID:
				handleSendTraderPlayerUIDRPC(sender, rpc_type, ctx);
			break;

			case TRPCs.RPC_SEND_TRADER_ADMINS_ENTRY:
				handleSendTraderAdminsEntryRPC(sender, rpc_type, ctx);
			break;
		}
	}

	//Shitty garage mod is dependant on this
	void handleSyncObjectOrientationRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param2<Object, vector> syncObject_rp = new Param2<Object, vector>( NULL, "0 0 0" );
		ctx.Read( syncObject_rp );
		
		Object objectToSync = syncObject_rp.param1;
		vector objectToSyncOrientation  = syncObject_rp.param2;

		objectToSync.SetOrientation(objectToSyncOrientation);
	}
	
	void handleSendTraderCurrencyNameEntryRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param1<string> currencyName_rp = new Param1<string>( "" );
		ctx.Read( currencyName_rp );
		
		m_Trader_CurrencyName = currencyName_rp.param1;
	}

	void handleSendTraderCurrencyEntryRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param2<string, int> currency_rp = new Param2<string, int>( "", -1 );
		ctx.Read( currency_rp );
		
		m_Trader_CurrencyClassnames.Insert(currency_rp.param1);
		m_Trader_CurrencyValues.Insert(currency_rp.param2);
	}
	
	void handleSendTraderNameEntryRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param1<string> tradername_rp = new Param1<string>( "" );
		ctx.Read( tradername_rp );
		
		m_Trader_TraderNames.Insert(tradername_rp.param1);
	}
	
	void handleSendTraderCategoryEntryRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param2<string, int> category_rp = new Param2<string, int>( "", 0 );
		ctx.Read( category_rp );					
		
		m_Trader_Categorys.Insert(category_rp.param1);
		m_Trader_CategorysTraderKey.Insert(category_rp.param2);
	}

	void handleSendTraderNPCDummyEntryRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param1<string> npcDummy_rp = new Param1<string>( "" );
		ctx.Read( npcDummy_rp );					
		
		m_Trader_NPCDummyClasses.Insert(npcDummy_rp.param1);
	}
	
	void handleSendTraderItemEntryRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param6<int, int, string, int, int, int> itemEntry_rp = new Param6<int, int, string, int, int, int>( 0, 0, "", 0, 0, 0);
		ctx.Read( itemEntry_rp );
		
		m_Trader_ItemsTraderId.Insert(itemEntry_rp.param1);
		m_Trader_ItemsCategoryId.Insert(itemEntry_rp.param2);
		m_Trader_ItemsClassnames.Insert(itemEntry_rp.param3);
		m_Trader_ItemsQuantity.Insert(itemEntry_rp.param4);
		m_Trader_ItemsBuyValue.Insert(itemEntry_rp.param5);
		m_Trader_ItemsSellValue.Insert(itemEntry_rp.param6);
	}
	
	void handleSendTraderMarkerEntryRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param5<int, vector, int, vector, vector> markerEntry = new Param5<int, vector, int, vector, vector>( 0, "0 0 0", 0, "0 0 0", "0 0 0" );
		ctx.Read( markerEntry );					
		
		m_Trader_TraderIDs.Insert(markerEntry.param1);
		m_Trader_TraderPositions.Insert(markerEntry.param2);
		m_Trader_TraderSafezones.Insert(markerEntry.param3);
		m_Trader_TraderVehicleSpawns.Insert(markerEntry.param4);
		m_Trader_TraderVehicleSpawnsOrientation.Insert(markerEntry.param5);
	}
	
	void handleSendTraderClearRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		SetReceivedAllTraderData(false);
		m_Trader_CurrencyName = "";
		m_Trader_CurrencyClassnames = new array<string>;
		m_Trader_CurrencyValues = new array<int>;
		m_Trader_TraderNames = new array<string>;
		m_Trader_TraderPositions = new array<vector>;
		m_Trader_TraderIDs = new array<int>;
		m_Trader_TraderSafezones = new array<int>;
		m_Trader_TraderVehicleSpawns = new array<vector>;
		m_Trader_TraderVehicleSpawnsOrientation = new array<vector>;
		m_Trader_Categorys = new array<string>;
		m_Trader_CategorysTraderKey = new array<int>;
		m_Trader_ItemsTraderId = new array<int>;
		m_Trader_ItemsCategoryId = new array<int>;
		m_Trader_ItemsClassnames = new array<string>;
		m_Trader_ItemsQuantity = new array<int>;
		m_Trader_ItemsBuyValue = new array<int>;
		m_Trader_ItemsSellValue = new array<int>;
		m_Trader_Vehicles = new array<string>;
		m_Trader_VehiclesParts = new array<string>;
		m_Trader_VehiclesPartsVehicleId = new array<int>;
		m_Trader_PlayerUID = "";
		m_Trader_AdminPlayerUIDs = new array<string>;
		m_Trader_NPCDummyClasses = new array<string>;
	}

	/*void handleSyncCarscriptIsInSafezoneRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param2<CarScript, bool> synccarsscript_rp = new Param2<CarScript, bool>( NULL, false );
		ctx.Read( synccarsscript_rp );
		
		CarScript carToSync = synccarsscript_rp.param1;

		carToSync.m_Trader_IsInSafezone = synccarsscript_rp.param2;
	}*/

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
			GetTraderNotifications().ShowExitSafezoneMessage(msg.param2);
		}
		else
		{
			GetTraderNotifications().ShowMessage(msg.param1, msg.param2, msg.param3);
		}
	}
	
	//DEPRECATED. SHOULDNT BE USED ANYMORE
	void showTraderMessage(string message, float time, int color = 0)
	{
		GetTraderNotifications().ShowMessage(message, time, color);
	}

	void handleSendTraderVariablesEntryRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param1<float> traderVariables_rp = new Param1<float>( 0 );
		ctx.Read( traderVariables_rp );
		
		m_Trader_BuySellTimer = traderVariables_rp.param1;
	}

	void handleSendTraderPlayerUIDRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param1<string> traderPlayerUID_rp = new Param1<string>( "" );
		ctx.Read( traderPlayerUID_rp );
		
		m_Trader_PlayerUID = traderPlayerUID_rp.param1;
	}

	void handleSendTraderAdminsEntryRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		Param1<string> traderAdmins_rp = new Param1<string>( "" );
		ctx.Read( traderAdmins_rp );
		
		m_Trader_AdminPlayerUIDs.Insert(traderAdmins_rp.param1);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// LOGS & MESSAGES
	void traderServerLog(string message)
	{
		TraderMessage.ServerLog("[TRADER] Player: (" + GetIdentity().GetName() + ") " + GetIdentity().GetId() + " " + message);
	}

	void traderTradesLog(string message)
	{
		TraderMessage.TradesLog("[TRADER] Player: (" + GetIdentity().GetName() + ") " + GetIdentity().GetId() + " " + message);
	}

	TraderNotifications GetTraderNotifications()
	{		
		if (!m_Trader_TraderNotifications)
		{	
			m_Trader_TraderNotifications = new TraderNotifications();
			m_Trader_TraderNotifications.Init();
		}
		return m_Trader_TraderNotifications;
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// ITEM & INVENTORY
	string TrimUntPrefix(string str) // not duplicate anymore?
	{
		str.Replace("$UNT$", "");
		return str;
	}

	string getItemDisplayName(string itemClassname) // not duplicate anymore?
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

	int GetItemMaxQuantity(string itemClassname) // not duplicate anymore?
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

	int getItemAmount(ItemBase item) // not duplicate anymore?
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

	bool SetItemAmount(ItemBase item, int amount)
	{
		if (!item)
			return false;

		if (amount == -1)
			amount = GetItemMaxQuantity(item.GetType());

		if (amount == -3)
			amount = 0;

		if (amount == -4)
			amount = 0;

		if (amount == -5)
			amount = Math.RandomIntInclusive(GetItemMaxQuantity(item.GetType()) * 0.5, GetItemMaxQuantity(item.GetType()));

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

	bool isInPlayerInventory(string itemClassname, int amount, out ItemBase item) // not duplicate anymore?
	{
		itemClassname.ToLower();

		ItemBase item_in_hands = ItemBase.Cast(GetHumanInventory().GetEntityInHands());
		if (item_in_hands)
		{			
			if(DoItemSellChecks(item_in_hands, itemClassname, amount))
			{
				item = item_in_hands;
				return true;
			}
		}

		bool isWeapon = false;
		if (amount == -4)
			isWeapon = true;
		array<EntityAI> itemsArray = new array<EntityAI>;		
		GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		
		//TraderMessage.PlayerWhite("--------------");
	
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));

			if (!item)
				continue;

			if (item.IsRuined())
				continue;

			if (isAttached(item))
				continue;

			if(item.GetInventory())
			{
				if (!item.GetInventory().CanRemoveEntity())
				{
					continue;
				}
				if (item.GetNumberOfItems() > 0)
				{
					continue;
				}
				if(!isWeapon && item.GetInventory().AttachmentCount() > 0)
				{
					continue;
				}
			}

			if(DoItemSellChecks(item, itemClassname, amount))
			{
				return true;
			}
		}
		
		return false;
	}

	protected bool DoItemSellChecks(ItemBase item, string itemClassname, int amount)
	{
		string itemPlayerClassname = item.GetType();
		itemPlayerClassname.ToLower();	
		
		bool isMagazine = false;
		if (amount == -3)
			isMagazine = true;

		bool isWeapon = false;
		if (amount == -4)
			isWeapon = true;

		bool isSteak = false;
		if (amount == -5)
			isSteak = true;

		if(itemPlayerClassname == itemClassname)
		{
			float steakProcentage = 0.5;
			if(isSteak)
			{
				Edible_Base edible = Edible_Base.Cast(item);
				if(edible)
				{
					if (edible.GetFoodStage())
					{
						if(edible.GetFoodStageType() == FoodStageType.ROTTEN)
						{
							return false;
						}
					}
				}	
				int steakAmount = getItemAmount(item);
				int maxSteakAmount = GetItemMaxQuantity(itemPlayerClassname);
				int MaxPercentage = maxSteakAmount * steakProcentage;
				if(steakAmount >= MaxPercentage)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			if(isMagazine || isWeapon)
			{
				return true;
			}
			if(getItemAmount(item) >= amount)
			{
				return true;
			}
		}
		return false;
	}

	array<ItemBase> getMergeableItemFromPlayerInventory(string itemType, int amount, bool absolute = false)
	{
		array<ItemBase> mergableItems = new array<ItemBase>;

		array<EntityAI> itemsArray = new array<EntityAI>;		
		GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);

		ItemBase itemToCombine = ItemBase.Cast(GetGame().CreateObject(itemType, "0 0 0"));

		if (!itemToCombine)
			return new array<ItemBase>;

		SetItemAmount(itemToCombine, amount);

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

			if (item.CanBeCombined(itemToCombine) && getItemAmount(item) < GetItemMaxQuantity(item.GetType()))
			{
				amount -= GetItemMaxQuantity(item.GetType()) - getItemAmount(item);
				SetItemAmount(itemToCombine, amount);
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

		SetItemAmount(ItemBase.Cast(item), amount);
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

		//GetHive().CharacterSave(player);

		if (hash <= 0)
			hash = vehicleKey.GenerateNewHash();
		else
			hash = vehicleKey.SetNewHash(hash);

		return hash;
	}

	bool RemoveItem(ItemBase item, string itemClassname, int amount)
	{		
		if (item.IsRuined())
			return false;

		if (isAttached(item))
			return false;

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
		itemPlayerClassname = item.GetType();
		itemPlayerClassname.ToLower();
		if(itemPlayerClassname == itemClassname)
		{
			int itemAmount = getItemAmount(item);
			int maxAmount = GetItemMaxQuantity(itemPlayerClassname);
			float steakProcentage = 0.5;
			bool shouldDel = false;
			if(isSteak)
			{
				int MaxPercentage = maxAmount * steakProcentage;
				if(itemAmount >=  MaxPercentage )
				{
					shouldDel = true;
				}
			}
			if(isMagazine || isWeapon)
			{
				shouldDel = true;
			}
			if(itemAmount == amount)
			{
				shouldDel = true;
			}
			if(shouldDel)
			{
				deleteItem(item);
				return true;
			}
			else
			{
				SetItemAmount(item, itemAmount - amount);
				return true;
			}
		}
		return false;
	}

	//We need something like this but better
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
		{
			InventoryLocation src = new InventoryLocation();
			if (item.GetInventory() && item.GetInventory().GetCurrentInventoryLocation(src))
			{
				if(src.GetType() == InventoryLocationType.HANDS)
				{
					LocalDestroyEntityInHands();
					return;
				}
			}
			GetGame().ObjectDelete(item);
		}
		//TraderMessage.PlayerWhite("DELETED " + item.GetType() + "; QTY: " + getItemAmount(item), PlayerBase.Cast(this));
	}

	bool isAttached(ItemBase item) // not duplicate anymore?
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

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// CURRENCY
	int getPlayerCurrencyAmount() // not duplicate anymore?
	{
		PlayerBase m_Player = PlayerBase.Cast(this);
		
		int currencyAmount = 0;
		
		array<EntityAI> itemsArray = new array<EntityAI>;
		m_Player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);

		ItemBase item;
		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));

			if (!item)
				continue;

			for (int j = 0; j < m_Player.m_Trader_CurrencyClassnames.Count(); j++)
			{
				if(item.GetType() == m_Player.m_Trader_CurrencyClassnames.Get(j))
				{
					currencyAmount += getItemAmount(item) * m_Player.m_Trader_CurrencyValues.Get(j);
				}
			}
		}
		
		return currencyAmount;
	}

	void deductPlayerCurrency(int currencyAmount)
	{		
		if (currencyAmount == 0)
			return;

		array<EntityAI> itemsArray = new array<EntityAI>;
		ItemBase item;
		GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		
		for (int i = 0; i < m_Trader_CurrencyClassnames.Count(); i++)
		{
			for (int j = 0; j < itemsArray.Count(); j++)
			{
				Class.CastTo(item, itemsArray.Get(j));
				
				if (!item)
					continue;

				if(item.GetType() == m_Trader_CurrencyClassnames.Get(i))
				{
					int itemAmount = getItemAmount(item);

					if(itemAmount * m_Trader_CurrencyValues.Get(i) > currencyAmount)
					{
						if (currencyAmount >= m_Trader_CurrencyValues.Get(i))
						{
							SetItemAmount(item, itemAmount - (currencyAmount / m_Trader_CurrencyValues.Get(i)));

							UpdateInventoryMenu(); // RPC-Call needed?
							
							currencyAmount -= (currencyAmount / m_Trader_CurrencyValues.Get(i)) * m_Trader_CurrencyValues.Get(i);
						}


						if (currencyAmount < m_Trader_CurrencyValues.Get(i))
						{
							exchangeCurrency(item, currencyAmount, m_Trader_CurrencyValues.Get(i));

							return;
						}
					}
					else
					{
						GetGame().ObjectDelete(itemsArray.Get(j));
						
						UpdateInventoryMenu(); // RPC-Call needed?
						
						currencyAmount -= itemAmount * m_Trader_CurrencyValues.Get(i);
					}
				}
			}
		}
	}

	void exchangeCurrency(ItemBase item, int currencyAmount, int currencyValue)
	{
		if (!item)
			return;

		if (currencyAmount == 0)
			return;

		//TraderMessage.PlayerWhite("EXCHANGE " + item.GetType() + " [" + currencyValue + "] " + currencyAmount, PlayerBase.Cast(this));

		int itemAmount = getItemAmount(item);

		if (itemAmount <= 1)
			deleteItem(item);
		else
			SetItemAmount(item, itemAmount - 1);

		increasePlayerCurrency(currencyValue - currencyAmount);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// VEHICLE
	string isVehicleSpawnFree(int traderIndex)
	{
		vector size = "3 1 9";
		array<Object> excluded_objects = new array<Object>;
		array<Object> nearby_objects = new array<Object>;

		GetGame().IsBoxColliding(m_Trader_TraderVehicleSpawns.Get(traderIndex), m_Trader_TraderVehicleSpawnsOrientation.Get(traderIndex), size, excluded_objects, nearby_objects);
		if (nearby_objects.Count() > 0)
		{
			BuildingBase building = BuildingBase.Cast(nearby_objects.Get(0));
			if(!building)
			{
				return nearby_objects.Get(0).GetType();
			}
		}

		return "FREE";
	}

	array<Transport> GetVehicleToGetKeyFor(int traderIndex)
	{
		vector size = "3 5 9";
		array<Object> excluded_objects = new array<Object>;
		array<Object> nearby_objects = new array<Object>;

		array<Transport> found_vehicles = new array<Transport>;
		Transport transport;

		if (GetGame().IsBoxColliding(m_Trader_TraderVehicleSpawns.Get(traderIndex), m_Trader_TraderVehicleSpawnsOrientation.Get(traderIndex), size, excluded_objects, nearby_objects))
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

	Object GetVehicleToSell(string vehicleClassname, vector position, vector orientation) // not duplicate anymore?
	{
		vector size = "3 5 9";
		array<Object> excluded_objects = new array<Object>;
		array<Object> nearby_objects = new array<Object>;
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
					CarScript carsScript = CarScript.Cast(transport);					
					if(carsScript && carsScript.m_Trader_LastDriverId == GetIdentity().GetId())
					{
						return carsScript;
					}
					BoatScript boatScript = BoatScript.Cast(transport);				
					if(boatScript && boatScript.m_Trader_LastDriverId == GetIdentity().GetId())
					{
						return boatScript;
					}				
				}					
			}
		}

		return NULL;
	}

	void spawnVehicle(int traderIndex, string vehicleType, int vehicleKeyHash)
	{
		vector objectPosition = m_Trader_TraderVehicleSpawns.Get(traderIndex);
		vector objectOrientation = m_Trader_TraderVehicleSpawnsOrientation.Get(traderIndex);
		
		EntityAI vehicle = EntityAI.Cast(GetGame().CreateObjectEx(vehicleType, objectPosition, ECE_LOCAL | ECE_CREATEPHYSICS | ECE_TRACE));
		if(!vehicle)
		{
			//throw error
			return;
		}			
		GetGame().RemoteObjectCreate(vehicle);

		vehicle.SetPosition(objectPosition);
		vehicle.SetOrientation(objectOrientation);
		vehicle.SetDirection(vehicle.GetDirection());
		
		// Attach Parts:
		int vehicleId = -1;
		for (int i = 0; i < m_Trader_Vehicles.Count(); i++)
		{
			if (vehicleType == m_Trader_Vehicles.Get(i))
			{
				vehicleId = i;
				break;
			}
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

			if (vehicleKeyHash != 0)
			{
				car.m_Trader_Locked = true;
				car.m_Trader_HasKey = true;
				car.m_Trader_VehicleKeyHash = vehicleKeyHash;
			}

			car.SynchronizeValues();
		}
		
		BoatScript boat = BoatScript.Cast(vehicle);
		if (boat)
		{
			boat.Fill( BoatFluid.FUEL, boat.GetFluidCapacity( BoatFluid.FUEL ));
			boat.SetSynchDirty();
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// OBJECTS
	void deleteObject(Object obj)
	{
		if (obj)
			GetGame().ObjectDelete(obj);
	}
}