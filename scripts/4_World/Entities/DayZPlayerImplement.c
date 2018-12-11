//#define Trader_Debug

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
	float m_Trader_HealthEnteredSafeZone;
	float m_Trader_HealthBloodEnteredSafeZone;
	bool m_Trader_PlayerDiedInSafezone = false;
	
	bool m_Trader_RecievedAllData = false;
	
	string m_Trader_CurrencyItemType;
	
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
	
#ifdef Trader_Debug
	Object TEST_PreviewObj;
	vector TEST_PreviewObjectPosition;
	bool TEST_PreviewObjectIsCreated = false;
	float TEST_PreviewObjYOffset = 0;
	vector TEST_PreviewObjPosOffset = "10 0 0";
	float TEST_PreviewObjDirOffset = 0;
	bool TEST_updateDir = false;
	bool TEST_PreviewObjectFreeze = false;
	
	bool TEST_ClassnamesInitialized = false;
	ref array<string> TEST_ClassnameCategoryName;
	int TEST_ClassnameCategoryID = 0;
	
	ref array<string> TEST_ClassnamesResidential;
	int TEST_ClassnamesResidentialID = 0;
	ref array<string> TEST_ClassnamesIndustrial;
	int TEST_ClassnamesIndustrialID = 0;
	ref array<string> TEST_ClassnamesMilitary;
	int TEST_ClassnamesMilitaryID = 0;
	ref array<string> TEST_ClassnamesWrecks;
	int TEST_ClassnamesWrecksID = 0;
	ref array<string> TEST_ClassnamesPlants;
	int TEST_ClassnamesPlantsID = 0;
	ref array<string> TEST_ClassnamesWalls;
	int TEST_ClassnamesWallsID = 0;
	ref array<string> TEST_ClassnamesRail;
	int TEST_ClassnamesRailID = 0;
	ref array<string> TEST_ClassnamesSpecific;
	int TEST_ClassnamesSpecificID = 0;
	
	ref array<Object> TEST_ObjectsToSave = new array<Object>;
#endif
	
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);
			
		
		PlayerBase player;
		Object obj;
		string itemType;
		ItemBase item;
		EntityAI entity;
		Magazine mgzn;
		int amount;
		vector position;
		
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

			if (rpc_type == TRPCs.RPC_SPAWN_VEHICLE)
			{
				Param3<vector, vector, string> rpv = new Param3<vector, vector, string>( "0 0 0", "0 0 0", "" );
				ctx.Read(rpv);

				vector objectPosition = rpv.param1;
				vector objectDirection = rpv.param2;
				string vehicleType = rpv.param3;

				// Get all Players to synchronize Things:
				ref array<Man> m_Players = new array<Man>;
				GetGame().GetWorld().GetPlayerList(m_Players);
				PlayerBase currentPlayer;

				// Spawn:
				obj = GetGame().CreateObject( vehicleType, objectPosition, false, false, true );

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

			if (rpc_type == TRPCs.RPC_DELETE_VEHICLE)
			{
				Param1<Object> rpdv = new Param1<Object>( NULL );
				ctx.Read(rpdv);

				obj = rpdv.param1;

				GetGame().ObjectDelete(obj);
			}
			
			if (rpc_type == TRPCs.RPC_SPAWN_ITEM_ON_GROUND)
			{
				Param4<PlayerBase, string, vector, int> rp1 = new Param4<PlayerBase, string, vector, int>( NULL, "", "0 0 0", 0);
				ctx.Read(rp1);
				
				player = rp1.param1;
				itemType = rp1.param2;
				position = rp1.param3;
				amount = rp1.param4;

				if (amount == -3)
					amount = 0;

				if (amount == -4)
					amount = 0;
				
				entity = player.SpawnEntityOnGroundPos(itemType, position);
				Class.CastTo(item, entity);

				mgzn = Magazine.Cast(item);
						
				if( item.IsMagazine() )
				{
					mgzn.ServerSetAmmoCount(amount);
				}
				else
				{
					item.SetQuantity(amount);
				}
			}
			
			if (rpc_type == TRPCs.RPC_CREATE_ITEM_IN_INVENTORY)
			{
				Param3<PlayerBase, string, int> rp2 = new Param3<PlayerBase, string, int>( NULL, "", 0);
				ctx.Read(rp2);
				
				player = rp2.param1;
				itemType = rp2.param2;
				amount = rp2.param3;

				if (amount == -3)
					amount = 0;

				if (amount == -4)
					amount = 0;
				
				entity = player.GetHumanInventory().CreateInInventory(itemType);
				Class.CastTo(item, entity);
				
				//if (amount != -1)
				//{
				mgzn = Magazine.Cast(item);
					
				if( item.IsMagazine() )
				{
					mgzn.ServerSetAmmoCount(amount);
				}
				else
				{
					item.SetQuantity(amount);
				}
				//}
			}
			
			if (rpc_type == TRPCs.RPC_DELETE_ITEM)
			{
				Param1<ItemBase> rp3 = new Param1<ItemBase>( NULL );
				ctx.Read(rp3);
				
				item = rp3.param1;
				
				item.Delete();
			}
			
			if (rpc_type == TRPCs.RPC_SET_ITEM_AMOUNT)
			{
				Param2<ItemBase, int> rp4 = new Param2<ItemBase, int>( NULL, 0 );
				ctx.Read(rp4);
				
				item = rp4.param1;
				amount = rp4.param2;
				
				
				if (amount != -1)
				{
					mgzn = Magazine.Cast(item);
						
					if( item.IsMagazine() )
					{
						mgzn.ServerSetAmmoCount(amount);
					}
					else
					{
						item.SetQuantity(amount);
					}
				}
			}
			
			if (rpc_type == TRPCs.RPC_INCREASE_PLAYER_CURRENCY)
			{
				Param3<PlayerBase, string, int> rp5 = new Param3<PlayerBase, string, int>( NULL, "", 0);
				ctx.Read(rp5);
				
				player = rp5.param1;
				string m_CurrencyItemType = rp5.param2;
				int currencyAmount = rp5.param3;

				int itemMaxAmount = GetItemMaxQuantity(m_CurrencyItemType);

				
				while (currencyAmount > 0)
				{
					bool freeSpaceForItem = false;
					InventoryLocation il = new InventoryLocation;		
					if (player.GetInventory().FindFirstFreeLocationForNewEntity(m_CurrencyItemType, FindInventoryLocationType.ANY, il))
						freeSpaceForItem = true;
					
					if (freeSpaceForItem)
					{						
						entity = player.GetHumanInventory().CreateInInventory(m_CurrencyItemType);
					}
					else
					{
						Param1<string> msgRp = new Param1<string>( "Trader: Your Inventory is full! Your Currencys were placed on Ground!" );
						GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp, true, player.GetIdentity());
						
						entity = player.SpawnEntityOnGroundPos(m_CurrencyItemType, player.GetPosition());
					}
					
					// set currency amount of item:
					if (currencyAmount > itemMaxAmount)
					{
						//setItemAmount(item, itemMaxAmount); // TODO: Funktion daraus machen!
						// unnoetig, da items immer mix maxammount gespawnt werden! // Nachtrag: Nicht in jedem Fall!

						Class.CastTo(item, entity);
						mgzn = Magazine.Cast(item);
						
						if( item.IsMagazine() )
						{
							mgzn.ServerSetAmmoCount(itemMaxAmount);
						}
						else
						{
							item.SetQuantity(itemMaxAmount);
						}

						currencyAmount -= itemMaxAmount;
					}
					else
					{					
						Class.CastTo(item, entity);
						mgzn = Magazine.Cast(item);
						
						if( item.IsMagazine() )
						{
							mgzn.ServerSetAmmoCount(currencyAmount);
						}
						else
						{
							item.SetQuantity(currencyAmount);
						}				
						
						currencyAmount = 0;
					}
				}
			}
			
#ifdef Trader_Debug
			if (rpc_type == TRPCs.RPC_DEBUG_TELEPORT)
			{
				Param2<PlayerBase, vector> rp7 = new Param2<PlayerBase, vector>( NULL, "0 0 0" );
				ctx.Read(rp7);
				
				player = rp7.param1;
				
				player.SetPosition( rp7.param2 );
			}
			
			if (rpc_type == TRPCs.RPC_TEST_PLACE_PREVIEW_OBJECT)
			{
				Param4<PlayerBase, string, vector, vector> rpt1 = new Param4<PlayerBase, string, vector, vector>( NULL, "", vector.Zero, vector.Zero );
				ctx.Read(rpt1);
				
				player = rpt1.param1;
				itemType = rpt1.param2;
				position = rpt1.param3;
				
				TEST_PreviewObj = GetGame().CreateObject( itemType, position, false, false, true );
				TEST_PreviewObj.SetOrientation(rpt1.param4);
				
				Param1<Object> rptc1 = new Param1<Object>( TEST_PreviewObj );
				GetGame().RPCSingleParam(player, TRPCs.RPC_TEST_PLACE_PREVIEW_OBJECT, rptc1, true, player.GetIdentity());
			}
			
			if (rpc_type == TRPCs.RPC_TEST_PLACE_OBJECT)
			{
				Param3<string, vector, vector> rpt4 = new Param3<string, vector, vector>( "", vector.Zero, vector.Zero );
				ctx.Read(rpt4);
				
				itemType = rpt4.param1;
				position = rpt4.param2;
				
				TEST_PreviewObj = GetGame().CreateObject( itemType, position, false, false, true );
				TEST_PreviewObj.SetDirection(rpt4.param3);
				TEST_PreviewObj.SetPosition(position);
				
				TEST_ObjectsToSave.Insert(TEST_PreviewObj);
			}
			
			if (rpc_type == TRPCs.RPC_TEST_DELETE_OBJECT)
			{
				Param1<Object> rpt2 = new Param1<Object>( NULL );
				ctx.Read(rpt2);
				
				GetGame().ObjectDelete(rpt2.param1);
			}
			
			if (rpc_type == TRPCs.RPC_TEST_REPOS_OBJECT)
			{
				Param2<Object, vector> rpt3 = new Param2<Object, vector>( NULL, vector.Zero );
				ctx.Read(rpt3);
				
				rpt3.param1.SetPosition(rpt3.param2);
			}
			
			if (rpc_type == TRPCs.RPC_TEST_REDIR_OBJECT)
			{
				Param2<Object, vector> rpt5 = new Param2<Object, vector>( NULL, vector.Zero );
				ctx.Read(rpt5);
				
				rpt3.param1.SetOrientation(rpt5.param2);
			}
			
			if (rpc_type == TRPCs.RPC_TEST_SAVE_OBJECTS)
			{
				FileHandle file = OpenFile("$profile:Trader/TEST.txt", FileMode.WRITE);
				if (file != 0)
				{
					for (int f = 0; f < TEST_ObjectsToSave.Count(); f++)
					{
						FPrintln(file, "<Object>\t\t\t" + TEST_ObjectsToSave.Get(f).GetType());
						FPrintln(file, "<ObjectPosition>\t" + TEST_ObjectsToSave.Get(f).GetPosition()[0] + ",\t" + TEST_ObjectsToSave.Get(f).GetPosition()[1] + ",\t" + TEST_ObjectsToSave.Get(f).GetPosition()[2]);
						FPrintln(file, "<ObjectDirection>\t" + TEST_ObjectsToSave.Get(f).GetDirection()[0] + ",\t" + TEST_ObjectsToSave.Get(f).GetDirection()[1] + ",\t" + TEST_ObjectsToSave.Get(f).GetDirection()[2]);
						FPrintln(file, " ");
					}
					CloseFile(file);
				}
			}
#endif
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
				
#ifdef Trader_Debug
				case TRPCs.RPC_TEST_PLACE_PREVIEW_OBJECT:
				Param1<Object> rptcc1 = new Param1<Object>( NULL );
				ctx.Read(rptcc1);
				
				TEST_PreviewObj = rptcc1.param1;
				break;
#endif
			}
		}
	}

	private int GetItemMaxQuantity(string itemClassname) // TODO in seperate Class as static Method
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

	/*override void SetSuicide(bool state)
	{
		if (m_Trader_IsInSafezone)
		{
			m_Suicide = false;
			return;
		}

		super.SetSuicide(state);
	}*/

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