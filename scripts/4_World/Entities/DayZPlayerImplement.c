modded class DayZPlayerImplement
{
	static const string m_Trader_ConfigFilePath = "$profile:Trader/TraderConfig.txt";
	static const string m_Trader_ObjectsFilePath = "$profile:Trader/TraderObjects.txt";
	static const string m_Trader_VehiclePartsFilePath = "$profile:Trader/TraderVehicleParts.txt";
	
	float m_Trader_WelcomeMessageTimer = 25.0;
	float m_Trader_WelcomeMessageHandled = false;
	bool m_Trader_TraderModIsLoaded = false;
	bool m_Trader_TraderModIsLoadedHandled = false;
	
	bool m_Trader_IsInSafezone = false;
	float m_Trader_IsInSafezoneTimeout = 0;
	float m_Trader_HealthEnteredSafeZone;
	float m_Trader_HealthBloodEnteredSafeZone;
	int m_Trader_InfluenzaEnteredSafeZone;
	bool m_Trader_PlayerDiedInSafezone = false;
	
	bool m_Trader_RecievedAllData = false;
	
	string m_Trader_CurrencyItemType;
	int m_Player_CurrencyAmount;

	int m_Trader_LastSelledItemID = -1;
	
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
	
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);
			
		
		PlayerBase player;
		Object obj;
		string itemType;
		ItemBase item;
		EntityAI entity;
		int amount;
		int itemQuantity; // same as amount
		vector position;
		int traderUID;
		int itemID;
		string itemDisplayNameClient;
		vector playerPosition;
		
		if (GetGame().IsServer()) //////////////////////////////////////////////////////////////////////////////////////////////////////////////// SERVER RPC ///////////////////////////////////////////////////////////
		{			
			if (rpc_type == TRPCs.RPC_TRADER_MOD_IS_LOADED && !m_Trader_TraderModIsLoaded)
			{
				Param1<PlayerBase> rp0 = new Param1<PlayerBase>( NULL );
				ctx.Read(rp0);
				
				player = rp0.param1;
				
				m_Trader_TraderModIsLoaded = true;
				
				GetGame().RPCSingleParam(player, TRPCs.RPC_TRADER_MOD_IS_LOADED_CONFIRM, new Param1<PlayerBase>( player ), true, player.GetIdentity());
			}

			if (rpc_type == TRPCs.RPC_BUY)
			{
				Param3<int, int, string> rpb = new Param3<int, int, string>( -1, -1, "" );
				ctx.Read(rpb);

				traderUID = rpb.param1;
				itemID = rpb.param2;
				itemDisplayNameClient = rpb.param3;

				if (itemID >= m_Trader_ItemsClassnames.Count() || itemID < 0 || traderUID >= m_Trader_TraderPositions.Count() || traderUID < 0)
					return;

				itemType = m_Trader_ItemsClassnames.Get(itemID);
				itemQuantity = m_Trader_ItemsQuantity.Get(itemID);
				int itemCosts = m_Trader_ItemsBuyValue.Get(itemID);

				playerPosition = this.GetPosition();	

				if (vector.Distance(playerPosition, m_Trader_TraderPositions.Get(traderUID)) > 1.7)
				{
					traderServerLog("tried to access the Trader out of Range! This could be an Hacker!");
					return;
				}


				m_Player_CurrencyAmount = getPlayerCurrencyAmount();

				if (m_Player_CurrencyAmount < itemCosts)
				{
					GetGame().RPCSingleParam(this, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "Trader: Sorry, but you can't afford that!" ), true, this.GetIdentity());
					return;
				}

				traderServerLog("bought " + getItemDisplayName(itemType) + "(" + itemType + ")");

				if (itemQuantity == -2) // Is a Vehicle
				{
					if (!isVehicleSpawnFree(traderUID))
					{
						GetGame().RPCSingleParam(this, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "Trader: Something is blocking the Way!" ), true, this.GetIdentity());
						return;
					}

					GetGame().RPCSingleParam(this, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "Trader: " + itemDisplayNameClient + " was parked next to you!" ), true, this.GetIdentity());

					spawnVehicle(traderUID, itemType);

					GetGame().RPCSingleParam(this, TRPCs.RPC_SEND_MENU_BACK, new Param1<bool>( false ), true, this.GetIdentity());
				}
				else // Is not a Vehicle
				{			
					if (canCreateItemInPlayerInventory(itemType))
					{
						GetGame().RPCSingleParam(this, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "Trader: " + itemDisplayNameClient + " was added to your Inventory!" ), true, this.GetIdentity());
						
						createItemInPlayerInventory(itemType, itemQuantity);
					}
					else
					{
						GetGame().RPCSingleParam(this, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "Trader: Your Inventory is full! " + itemDisplayNameClient + " was placed on Ground!" ), true, this.GetIdentity());
											
						spawnItemOnGround(itemType, itemQuantity, playerPosition);
						
						GetGame().RPCSingleParam(this, TRPCs.RPC_SEND_MENU_BACK, new Param1<bool>( false ), true, this.GetIdentity());
					}
				}

				deductPlayerCurrency(itemCosts);
			}

			if (rpc_type == TRPCs.RPC_SELL)
			{
				Param3<int, int, string> rps = new Param3<int, int, string>( -1, -1, "" );
				ctx.Read(rps);

				traderUID = rps.param1;
				itemID = rps.param2;
				itemDisplayNameClient = rps.param3;

				if (itemID >= m_Trader_ItemsClassnames.Count() || itemID < 0 || traderUID >= m_Trader_TraderPositions.Count() || traderUID < 0)
					return;

				itemType = m_Trader_ItemsClassnames.Get(itemID);
				itemQuantity = m_Trader_ItemsQuantity.Get(itemID);
				int itemSellValue = m_Trader_ItemsSellValue.Get(itemID);

				playerPosition = this.GetPosition();	

				if (vector.Distance(playerPosition, m_Trader_TraderPositions.Get(traderUID)) > 1.7)
				{
					traderServerLog("tried to access the Trader out of Range! This could be an Hacker!");
					return;
				}


				Object vehicleToSell = GetVehicleToSell(traderUID, itemType);
				bool isValidVehicle = (itemQuantity == -2 && vehicleToSell);

				if (!isInPlayerInventory(itemType, itemQuantity) && !isValidVehicle)
				{
					GetGame().RPCSingleParam(this, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "Trader: Sorry, but you can't sell that!" ), true, this.GetIdentity());

					if (itemQuantity == -2)
						GetGame().RPCSingleParam(this, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "Trader: Place the Vehicle inside the Traffic Cones!" ), true, this.GetIdentity());
						//GetGame().RPCSingleParam(this, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "Trader: Turn the Engine on and place it inside the Traffic Cones!" ), true, this.GetIdentity());

					return;
				}

				traderServerLog("sold " + getItemDisplayName(itemType) + " (" + itemType + ")");

				GetGame().RPCSingleParam(this, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "Trader: " + itemDisplayNameClient + " was sold!" ), true, this.GetIdentity());

				if (isValidVehicle)
					deleteObject(vehicleToSell);
				else
					removeFromPlayerInventory(itemType, itemQuantity);
				
				increasePlayerCurrency(itemSellValue);
			}
		}
		else ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// CLIENT RPC ///////////////////////////////////////////////////////////
		{
			switch(rpc_type)
			{
				case TRPCs.RPC_SEND_TRADER_CURRENCYTYPE_ENTRY:
					ref Param1<string> currencyType_rp = new Param1<string>( "" );
					ctx.Read( currencyType_rp );
					
					m_Trader_CurrencyItemType = currencyType_rp.param1;
				break;
				
				case TRPCs.RPC_SEND_TRADER_MANCLASSNAME_ENTRY:
					
				break;
				
				case TRPCs.RPC_SEND_TRADER_NAME_ENTRY:
					ref Param1<string> tradername_rp = new Param1<string>( "" );
					ctx.Read( tradername_rp );
					
					m_Trader_TraderNames.Insert(tradername_rp.param1);
				break;
				
				case TRPCs.RPC_SEND_TRADER_CATEGORY_ENTRY:
					ref Param2<string, int> category_rp = new Param2<string, int>( "", 0 );
					ctx.Read( category_rp );					
					
					m_Trader_Categorys.Insert(category_rp.param1);
					m_Trader_CategorysTraderKey.Insert(category_rp.param2);
				break;
				
				case TRPCs.RPC_SEND_TRADER_ITEM_ENTRY:
					Param6<int, int, string, int, int, int> itemEntry_rp = new Param6<int, int, string, int, int, int>( 0, 0, "", 0, 0, 0);
					ctx.Read( itemEntry_rp );
					
					m_Trader_ItemsTraderId.Insert(itemEntry_rp.param1);
					m_Trader_ItemsCategoryId.Insert(itemEntry_rp.param2);
					m_Trader_ItemsClassnames.Insert(itemEntry_rp.param3);
					m_Trader_ItemsQuantity.Insert(itemEntry_rp.param4);
					m_Trader_ItemsBuyValue.Insert(itemEntry_rp.param5);
					m_Trader_ItemsSellValue.Insert(itemEntry_rp.param6);
				break;
				
				case TRPCs.RPC_SEND_TRADER_MARKER_ENTRY:
					ref Param5<int, vector, int, vector, vector> markerEntry = new Param5<int, vector, int, vector, vector>( 0, "0 0 0", 0, "0 0 0", "0 0 0" );
					ctx.Read( markerEntry );					
					
					m_Trader_TraderIDs.Insert(markerEntry.param1);
					m_Trader_TraderPositions.Insert(markerEntry.param2);
					m_Trader_TraderSafezones.Insert(markerEntry.param3);
					m_Trader_TraderVehicleSpawns.Insert(markerEntry.param4);
					m_Trader_TraderVehicleSpawnsOrientation.Insert(markerEntry.param5);
				break;
				
				case TRPCs.RPC_SEND_TRADER_DATA_CONFIRMATION:
					ref Param1<bool> conf_rp = new Param1<bool>( false );
					ctx.Read( conf_rp );
					
					m_Trader_RecievedAllData = conf_rp.param1;
				break;
				
				case TRPCs.RPC_SEND_TRADER_CLEAR:
					// clear all data here:
					m_Trader_RecievedAllData = false;	
					m_Trader_CurrencyItemType = "";
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
				break;
				
				case TRPCs.RPC_TRADER_MOD_IS_LOADED_CONFIRM:
					m_Trader_TraderModIsLoaded = true;
					m_Trader_TraderModIsLoadedHandled = true;
				break;

				case TRPCs.RPC_SEND_TRADER_IS_IN_SAFEZONE:
					ref Param1<bool> safezone_rp = new Param1<bool>( false );
					ctx.Read( safezone_rp );
					
					m_Trader_IsInSafezone = safezone_rp.param1;

					player.GetInputController().OverrideRaise(m_Trader_IsInSafezone, false);
				break;

				case TRPCs.RPC_SEND_TRADER_PLAYER_DIED_IN_SAFEZONE:
					ref Param1<bool> safezoneDied_rp = new Param1<bool>( false );
					ctx.Read( safezoneDied_rp );
					
					m_Trader_PlayerDiedInSafezone = safezoneDied_rp.param1;
				break;

				case TRPCs.RPC_SYNC_OBJECT_ORIENTATION:
					ref Param2<Object, vector> syncObject_rp = new Param2<Object, vector>( NULL, "0 0 0" );
					ctx.Read( syncObject_rp );
					
					Object objectToSync = syncObject_rp.param1;
					vector objectToSyncOrientation  = syncObject_rp.param2;

					objectToSync.SetOrientation(objectToSyncOrientation);
					//objectToSync.SetDirection(objectToSync.GetDirection()); // Thats a strange way to synchronize/update Objects.. But it works..
				break;

				case TRPCs.RPC_SYNC_CARSCRIPT_ISINSAFEZONE:
					ref Param2<CarScript, bool> synccarsscript_rp = new Param2<CarScript, bool>( NULL, false );
					ctx.Read( synccarsscript_rp );
					
					CarScript carToSync = synccarsscript_rp.param1;
					//bool objectToSyncOrientation  = synccarsscript_rp.param2;

					carToSync.m_Trader_IsInSafezone = synccarsscript_rp.param2;
				break;

				case TRPCs.RPC_SEND_MENU_BACK:
					GetGame().GetUIManager().Back();
				break;
			}
		}
	}

	private void traderServerLog(string message)
	{
		TraderServerLogs.PrintS("[TRADER] Player: (" + this.GetIdentity().GetName() + ") " + this.GetIdentity().GetId() + " " + message);
	}

	private string TrimUntPrefix(string str) // duplicate
	{
		str.Replace("$UNT$", "");
		return str;
	}

	private string getItemDisplayName(string itemClassname) // duplicate
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

	private int GetItemMaxQuantity(string itemClassname)
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

	private int getItemAmount(ItemBase item) // duplicate
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

	private bool SetItemAmount(ItemBase item, int amount)
	{
		if (!item)
			return false;

		if (amount == -1)
			amount = GetItemMaxQuantity(item.GetType());

		if (amount == -3)
			amount = 0;

		if (amount == -4)
			amount = 0;

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

	private int getPlayerCurrencyAmount() // duplicate
	{		
		int currencyAmount = 0;
		
		array<EntityAI> itemsArray = new array<EntityAI>;
		this.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);

		ItemBase item;
		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));
			if(item && item.GetType() == m_Trader_CurrencyItemType)
			{
				currencyAmount += getItemAmount(item);
			}
		}
		
		return currencyAmount;
	}

	private bool isInPlayerInventory(string itemClassname, int amount) // duplicate
	{
		itemClassname.ToLower();
		
		bool isMagazine = false;
		if (amount == -3)
			isMagazine = true;

		bool isWeapon = false;
		if (amount == -4)
			isWeapon = true;

		array<EntityAI> itemsArray = new array<EntityAI>;		
		this.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		
		//m_Player.MessageStatus("--------------");

		ItemBase item;		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));
			string itemPlayerClassname = "";

			if (!item)
				continue;

			if (item.IsRuined())
				continue;

			itemPlayerClassname = item.GetType();
			itemPlayerClassname.ToLower();

			//m_Player.MessageStatus("I: " + itemPlayerClassname + " == " + itemClassname);

			if(itemPlayerClassname == itemClassname && ((getItemAmount(item) >= amount && !isMagazine && !isWeapon) || isMagazine || isWeapon) && m_Trader_LastSelledItemID != item.GetID())
			{
				m_Trader_LastSelledItemID = item.GetID();

				return true;
			}
		}
		
		return false;
	}

	private bool canCreateItemInPlayerInventory(string itemType)
	{
		InventoryLocation il = new InventoryLocation;
		
		if (this.GetInventory().FindFirstFreeLocationForNewEntity(itemType, FindInventoryLocationType.ANY, il))
		{
			return true;
		}

		EntityAI entityInHands = this.GetHumanInventory().GetEntityInHands();
		if (!entityInHands)
			return true;

		return false;			
	}

	private void createItemInPlayerInventory(string itemType, int amount)
	{		
		EntityAI entity = this.GetHumanInventory().CreateInInventory(itemType);

		if (!entity)
			return;

		ItemBase item;
		Class.CastTo(item, entity);

		if (!item)
			return;
		
		SetItemAmount(item, amount);
	}

	private void spawnItemOnGround(string itemType, int amount, vector position)
	{		
		EntityAI entity = this.SpawnEntityOnGroundPos(itemType, position);

		if (!entity)
			return;

		ItemBase item;
		Class.CastTo(item, entity);

		if (!item)
			return;

		SetItemAmount(item, amount);
	}

	private void increasePlayerCurrency(int currencyAmount)
	{
		int itemMaxAmount = GetItemMaxQuantity(m_Trader_CurrencyItemType);
		EntityAI entity;
		ItemBase item;
		
		while (currencyAmount > 0)
		{
			bool freeSpaceForItem = false;
			InventoryLocation il = new InventoryLocation;		
			if (this.GetInventory().FindFirstFreeLocationForNewEntity(m_Trader_CurrencyItemType, FindInventoryLocationType.ANY, il))
				freeSpaceForItem = true;
			
			if (freeSpaceForItem)
			{						
				entity = this.GetHumanInventory().CreateInInventory(m_Trader_CurrencyItemType);
			}
			else
			{
				GetGame().RPCSingleParam(this, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "Trader: Your Inventory is full! Your Currencys were placed on Ground!" ), true, this.GetIdentity());
				
				entity = this.SpawnEntityOnGroundPos(m_Trader_CurrencyItemType, this.GetPosition());

				GetGame().RPCSingleParam(this, TRPCs.RPC_SEND_MENU_BACK, new Param1<bool>(true), true, this.GetIdentity());
			}
			
			// set currency amount of item:
			if (currencyAmount > itemMaxAmount)
			{
				Class.CastTo(item, entity);
				
				SetItemAmount(item, itemMaxAmount);

				currencyAmount -= itemMaxAmount;
			}
			else
			{					
				Class.CastTo(item, entity);
				
				SetItemAmount(item, currencyAmount);		
				
				currencyAmount = 0;
			}
		}
	}

	private bool removeFromPlayerInventory(string itemClassname, int amount)
	{
		itemClassname.ToLower();

		bool isMagazine = false;
		if (amount == -3)
			isMagazine = true;

		bool isWeapon = false;
		if (amount == -4)
			isWeapon = true;
		
		array<EntityAI> itemsArray = new array<EntityAI>;
		ItemBase item;
		this.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));
			string itemPlayerClassname = "";

			if (!item)
				continue;

			itemPlayerClassname = item.GetType();
			itemPlayerClassname.ToLower();

			if(itemPlayerClassname == itemClassname && ((getItemAmount(item) >= amount && !isMagazine && !isWeapon) || isMagazine || isWeapon))
			{
				int itemAmount = getItemAmount(item);
				
				if (itemAmount == amount || isMagazine || isWeapon)
				{
					deleteItem(item);
					
					this.UpdateInventoryMenu(); // RPC-Call needed?
					return true;
				}
				else
				{
					SetItemAmount(item, itemAmount - amount);
				
					this.UpdateInventoryMenu(); // RPC-Call needed?
					return true;
				}
			}
		}
		
		this.UpdateInventoryMenu(); // RPC-Call needed?
		return false;
	}

	private void deductPlayerCurrency(int currencyAmount)
	{		
		array<EntityAI> itemsArray = new array<EntityAI>;
		ItemBase item;
		this.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, itemsArray);
		
		for (int i = 0; i < itemsArray.Count(); i++)
		{
			Class.CastTo(item, itemsArray.Get(i));
			
			if (!item)
				continue;

			if(item.GetType() == m_Trader_CurrencyItemType)
			{
				int itemCurrencyAmount = getItemAmount(item);
				
				if(itemCurrencyAmount > currencyAmount)
				{
					SetItemAmount(item, itemCurrencyAmount - currencyAmount);
					return;
				}
				else
				{
					deleteItem(itemsArray.Get(i));
					
					this.UpdateInventoryMenu(); // RPC-Call needed?
					
					currencyAmount -= itemCurrencyAmount;
				}
			}
		}
	}

	private void deleteItem(ItemBase item)
	{
		if (item)
			item.Delete();
	}

	private void deleteObject(Object obj)
	{
		if (obj)
			GetGame().ObjectDelete(obj);
	}

	private bool isVehicleSpawnFree(int traderUID)
	{
		vector size = "3 5 9";
		array<Object> excluded_objects = new array<Object>;
		array<Object> nearby_objects = new array<Object>;

		return !(GetGame().IsBoxColliding( m_Trader_TraderVehicleSpawns.Get(traderUID), m_Trader_TraderVehicleSpawnsOrientation.Get(traderUID), size, excluded_objects, nearby_objects));
	}

	private Object GetVehicleToSell(int traderUID, string vehicleClassname) // duplicate
	{
		vector size = "3 5 9";
		array<Object> excluded_objects = new array<Object>;
		array<Object> nearby_objects = new array<Object>;

		if (GetGame().IsBoxColliding( m_Trader_TraderVehicleSpawns.Get(traderUID), m_Trader_TraderVehicleSpawnsOrientation.Get(traderUID), size, excluded_objects, nearby_objects))
		{
			for (int i = 0; i < nearby_objects.Count(); i++)
			{
				if (nearby_objects.Get(i).GetType() == vehicleClassname)
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

					// Check if Engine is running:
					/*Car car;
					Class.CastTo(car, nearby_objects.Get(i));
					if (car && vehicleIsEmpty)
					{
						if (car.EngineIsOn())
							return nearby_objects.Get(i);
					}*/

					//TODO: Check if Vehicle Owner is Player or Vehicle Owner is not set.

					if (m_Trader_LastSelledItemID == nearby_objects.Get(i).GetID())
						continue;

					m_Trader_LastSelledItemID = nearby_objects.Get(i).GetID();

					return nearby_objects.Get(i);		
				}					
			}
		}

		return NULL;
	}

	private void spawnVehicle(int traderUID, string vehicleType)
	{
		//Param3<vector, vector, string> rpv = new Param3<vector, vector, string>( "0 0 0", "0 0 0", "" );
		//ctx.Read(rpv);

		vector objectPosition = m_Trader_TraderVehicleSpawns.Get(traderUID);
		vector objectDirection = m_Trader_TraderVehicleSpawnsOrientation.Get(traderUID);

		// Get all Players to synchronize Things:
		ref array<Man> m_Players = new array<Man>;
		GetGame().GetWorld().GetPlayerList(m_Players);
		PlayerBase currentPlayer;

		// Spawn:
		Object obj = GetGame().CreateObject( vehicleType, objectPosition, false, false, true );

		obj.SetOrientation(objectDirection);
		obj.SetDirection(obj.GetDirection());

		for (int i = 0; i < m_Players.Count(); i++)
		{
			currentPlayer = PlayerBase.Cast(m_Players.Get(i));

			if ( !currentPlayer )
				continue;

			GetGame().RPCSingleParam(currentPlayer, TRPCs.RPC_SYNC_OBJECT_ORIENTATION, new Param2<Object, vector>( obj, objectDirection ), true, currentPlayer.GetIdentity());
		}
		
		// Attach Parts:
		EntityAI vehicle;
		Class.CastTo(vehicle, obj);

		int vehicleId = -1;
		for (i = 0; i < m_Trader_Vehicles.Count(); i++)
		{
			if (vehicleType == m_Trader_Vehicles.Get(i))
				vehicleId = i;
		}

		for (int j = 0; j < m_Trader_VehiclesParts.Count(); j++)
		{
			if (m_Trader_VehiclesPartsVehicleId.Get(j) == vehicleId)
				vehicle.GetInventory().CreateAttachment(m_Trader_VehiclesParts.Get(j));
		}

		// Try to fill Fuel, Oil, Brakeliquid, Coolantliquid and lock Vehicle:
		Car car;
		Class.CastTo(car, vehicle);
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

			CarScript carScript;
			if (Class.CastTo(carScript, vehicle))
			{
				carScript.m_Trader_OwnerPlayerUID = this.GetIdentity().GetId();
				carScript.m_Trader_IsInSafezone = true;

				for (i = 0; i < m_Players.Count(); i++)
				{
					currentPlayer = PlayerBase.Cast(m_Players.Get(i));
					
					if ( !currentPlayer )
						continue;

					GetGame().RPCSingleParam(currentPlayer, TRPCs.RPC_SYNC_CARSCRIPT_ISINSAFEZONE, new Param2<CarScript, bool>( car, true ), true, currentPlayer.GetIdentity());
				}
			}
		}
	}

	void ShowDeadScreen(bool show, float duration)
	{
	#ifndef NO_GUI
		if (show && IsPlayerSelected())
		{
		#ifdef PLATFORM_CONSOLE
			GetGame().GetUIManager().ScreenFadeIn(duration, "#dayz_implement_dead", FadeColors.DARK_RED, FadeColors.WHITE);
		#else
			if (!m_Trader_PlayerDiedInSafezone)
				GetGame().GetUIManager().ScreenFadeIn(duration, "#dayz_implement_dead", FadeColors.BLACK, FadeColors.WHITE);
			else
				GetGame().GetUIManager().ScreenFadeIn(0, "Someone killed you in the Safezone! Just EXIT and RECONNECT to the Server. DO NOT RESPAWN!", FadeColors.BLACK, 0xFFFF0000);
		#endif
		}
		else
		{
			GetGame().GetUIManager().ScreenFadeOut(0);
		}
		
		if (duration > 0)
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).CallLater(StopDeathDarkeningEffect, duration*1000, false);
		else
			GetGame().GetCallQueue(CALL_CATEGORY_GUI).Call(StopDeathDarkeningEffect);
	#endif
	}
}