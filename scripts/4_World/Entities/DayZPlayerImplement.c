#define Trader_Debug

modded class DayZPlayerImplement
{
	static const string m_Trader_ConfigFilePath = "$profile:Trader/TraderConfig.txt";
	static const string m_Trader_ObjectsFilePath = "$profile:Trader/TraderObjects.txt";
	static const string m_Trader_VehiclePartsFilePath = "$profile:Trader/TraderVehicleParts.txt";
	
	float m_Trader_WelcomeMessageTimer = 5.0;
	float m_Trader_WelcomeMessageHandled = false;
	bool m_Trader_TraderModIsLoaded = false;
	bool m_Trader_TraderModIsLoadedHandled = false;
	
	bool m_Trader_IsInSafezone = false;
	float m_Trader_HealthEnteredSafeZone;
	float m_Trader_HealthBloodEnteredSafeZone;
	bool m_Trader_PlayerDiedInSafezone = false;
	
	bool m_Trader_IsReadingTraderFileEntrys = false;
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
			if (m_Trader_IsReadingTraderFileEntrys)
				return;
			
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
						carScript.m_Trader_OwnerPlayerUID = this.GetIdentity().GetId();
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
				
				entity = player.SpawnEntityOnGroundPos(itemType, position);
				Class.CastTo(item, entity);
			}
			
			if (rpc_type == TRPCs.RPC_CREATE_ITEM_IN_INVENTORY)
			{
				Param3<PlayerBase, string, int> rp2 = new Param3<PlayerBase, string, int>( NULL, "", 0);
				ctx.Read(rp2);
				
				player = rp2.param1;
				itemType = rp2.param2;
				amount = rp2.param3;
				
				entity = player.GetHumanInventory().CreateInInventory(itemType);
				Class.CastTo(item, entity);
				
				
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
				
				// get currency item max amount: TODO: GetFromConfig!
				vector ghostItemPosition = "0 0 0";
				EntityAI ghostEntity = player.SpawnEntityOnGroundPos(m_CurrencyItemType, ghostItemPosition);
				Class.CastTo(item, ghostEntity);			
				mgzn = Magazine.Cast(item);
				int itemMaxAmount = 0;
				if( item.IsMagazine() )
					itemMaxAmount = mgzn.GetAmmoMax();
				else
					itemMaxAmount = item.GetQuantityMax();
				GetGame().ObjectDelete(ghostEntity);
				
				
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
						//setItemAmount(item, itemMaxAmount); // TODO (unnoetig, da items immer mix maxammount gespawnt werden!
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
			
			if (rpc_type == TRPCs.RPC_REQUEST_TRADER_DATA && m_Trader_IsReadingTraderFileEntrys == false)
			{
				m_Trader_IsReadingTraderFileEntrys = true;
				
				Param1<PlayerBase> rp6 = new Param1<PlayerBase>( NULL );
				ctx.Read(rp6);
				
				player = rp6.param1;
				
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
				
				// request that client also clears all data:
				Param1<bool> crpClr = new Param1<bool>( true );
				GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_CLEAR, crpClr, true, player.GetIdentity());
				

				TraderServerLogs.PrintS("[TRADER] DEBUG START");
				
				FileHandle file_index = OpenFile(m_Trader_ConfigFilePath, FileMode.READ);
				
				if ( file_index == 0 )
				{
					Param1<string> msgRp2 = new Param1<string>( "[Trader] FOUND NO TRADERCONFIG FILE!" );
					GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp2, true, player.GetIdentity());
					return;
				}
				
				string line_content = "";
				
				TraderServerLogs.PrintS("[TRADER] READING CURRENCY ENTRY..");
				line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<Currency>", "");
				line_content.Replace("<Currency>", "");
				line_content = FileReadHelper.TrimComment(line_content);
				m_Trader_CurrencyItemType = line_content;
				
				bool traderInstanceDone = false;
				int traderCounter = 0;
				
				line_content = "";
				while (traderCounter <= 500 && line_content != "<FileEnd>")
				{
					TraderServerLogs.PrintS("[TRADER] READING TRADER ENTRY..");
					
					if (traderInstanceDone == false)
						line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<Trader>", "");
					else
						traderInstanceDone = false;
					
					line_content.Replace("<Trader>", "");
					line_content = FileReadHelper.TrimComment(line_content);
					
					m_Trader_TraderNames.Insert(line_content);
						
					int categoryCounter = 0;
					
					line_content = "";
					while (categoryCounter <= 500 && line_content != "<FileEnd>")
					{
						line_content = FileReadHelper.TrimComment(FileReadHelper.SearchForNextTermInFile(file_index, "<Category>", "<Trader>"));
						
						if (line_content.Contains("<Trader>"))
						{
							traderInstanceDone = true;
							break;
						}
						
						if (line_content == string.Empty)
						{
							line_content = "<FileEnd>";
							break;
						}
						
						TraderServerLogs.PrintS("[TRADER] READING CATEGORY ENTRY..");
						line_content.Replace("<Category>", "");
						m_Trader_Categorys.Insert(FileReadHelper.TrimComment(line_content));
						m_Trader_CategorysTraderKey.Insert(traderCounter);
						
						categoryCounter++;
					}
					
					traderCounter++;
				}
				
				CloseFile(file_index);
				
				//------------------------------------------------------------------------------------
				
				file_index = OpenFile(m_Trader_ConfigFilePath, FileMode.READ);
				
				int itemCounter = 0;
				int char_count = 0;
				int traderID = -1;
				int categoryId = -1;
				
				line_content = "";
				while ( itemCounter <= 5000 && char_count != -1 && line_content.Contains("<FileEnd>") == false)
				{
					TraderServerLogs.PrintS("[TRADER] READING ITEM ENTRY..");
					char_count = FGets( file_index,  line_content );
					
					line_content = FileReadHelper.TrimComment(line_content);

					if (line_content.Contains("<Trader>"))
					{
						traderID++;
						
						continue;
					}
					
					if (line_content.Contains("<Category>"))
					{
						categoryId++;
						
						continue;
					}
				
					
					if (!line_content.Contains(","))
						continue;
				
					TStringArray strs = new TStringArray;
					line_content.Split( ",", strs );
					
					string itemStr = strs.Get(0);
					itemStr = FileReadHelper.TrimSpaces(itemStr);
					
					string qntStr = strs.Get(1);
					qntStr = FileReadHelper.TrimSpaces(qntStr);
					
					if (qntStr.Contains("*") || qntStr.Contains("-1"))
					{
						entity = player.SpawnEntityOnGroundPos(itemStr, vector.Zero);
						Class.CastTo(item, entity);
						
						int itemQuantityMax = -1;
						mgzn = Magazine.Cast(item);
						
						if( item.IsMagazine() )
							itemQuantityMax = mgzn.GetAmmoMax();
						else
							itemQuantityMax = item.GetQuantityMax();				
						
						qntStr = itemQuantityMax.ToString();	
						
						item.Delete();
					}

					if (qntStr.Contains("V") || qntStr.Contains("v"))
					{
						qntStr = "-2";
					}
					
					string buyStr = strs.Get(2);
					buyStr = FileReadHelper.TrimSpaces(buyStr);
					
					string sellStr = strs.Get(3);
					sellStr = FileReadHelper.TrimSpaces(sellStr);
					
					m_Trader_ItemsTraderId.Insert(traderID);
					m_Trader_ItemsCategoryId.Insert(categoryId);
					m_Trader_ItemsClassnames.Insert(itemStr);
					m_Trader_ItemsQuantity.Insert(qntStr.ToInt());
					m_Trader_ItemsBuyValue.Insert(buyStr.ToInt());
					m_Trader_ItemsSellValue.Insert(sellStr.ToInt());
					
					itemCounter++;
				}
				
				CloseFile(file_index);
				
				//------------------------------------------------------------------------------------
				
				file_index = OpenFile(m_Trader_ObjectsFilePath, FileMode.READ);
				
				if ( file_index == 0 )
				{
					Param1<string> msgRp3 = new Param1<string>( "[Trader] FOUND NO TRADEROBJECTS FILE!" );
					GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp3, true, player.GetIdentity());
					return;
				}
				
				bool skipLine = false;
				int markerCounter = 0;				

				line_content = "";
				while ( markerCounter <= 5000 && line_content.Contains("<FileEnd>") == false)
				{
					// Get Trader Marker Trader ID:
					if (!skipLine)
						line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<TraderMarker>", "<FileEnd>");
					else
						skipLine = false;					
					
					if (!line_content.Contains("<TraderMarker>"))
						continue;
					
					line_content.Replace("<TraderMarker>", "");
					line_content = FileReadHelper.TrimComment(line_content);
					line_content = FileReadHelper.TrimSpaces(line_content);
					
					TraderServerLogs.PrintS("[TRADER] READING MARKER ID ENTRY..");
										
					m_Trader_TraderIDs.Insert(line_content.ToInt());
					
					// Get Trader Marker Position:		
					line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<TraderMarkerPosition>", "<FileEnd>");
					
					line_content.Replace("<TraderMarkerPosition>", "");
					line_content = FileReadHelper.TrimComment(line_content);
					
					TStringArray strsm = new TStringArray;
					line_content.Split( ",", strsm );
					
					string traderMarkerPosX = strsm.Get(0);
					traderMarkerPosX = FileReadHelper.TrimSpaces(traderMarkerPosX);
					
					string traderMarkerPosY = strsm.Get(1);
					traderMarkerPosY = FileReadHelper.TrimSpaces(traderMarkerPosY);
					
					string traderMarkerPosZ = strsm.Get(2);
					traderMarkerPosZ = FileReadHelper.TrimSpaces(traderMarkerPosZ);
					
					vector markerPosition = "0 0 0";
					markerPosition[0] = traderMarkerPosX.ToFloat();
					markerPosition[1] = traderMarkerPosY.ToFloat();
					markerPosition[2] = traderMarkerPosZ.ToFloat();
					
					TraderServerLogs.PrintS("[TRADER] READING MARKER POSITION ENTRY..");
					
					m_Trader_TraderPositions.Insert(markerPosition);
					
					// Get Trader Marker Safezone Radius:					
					line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<TraderMarkerSafezone>", "<FileEnd>");
					
					line_content.Replace("<TraderMarkerSafezone>", "");
					line_content = FileReadHelper.TrimComment(line_content);
					line_content = FileReadHelper.TrimSpaces(line_content);
					
					TraderServerLogs.PrintS("[TRADER] READING MARKER SAFEZONE ENTRY..");
					
					m_Trader_TraderSafezones.Insert(line_content.ToInt());

					// Get Trader Marker Vehicle Spawnpoint:					
					line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<VehicleSpawn>", "<TraderMarker>");

					if(line_content == string.Empty)
						break;

					if (line_content.Contains("<TraderMarker>"))
					{
						skipLine = true;
						m_Trader_TraderVehicleSpawns.Insert("0 0 0");
						m_Trader_TraderVehicleSpawnsOrientation.Insert("0 0 0");
						continue;
					}

					line_content.Replace("<VehicleSpawn>", "");
					line_content = FileReadHelper.TrimComment(line_content);

					TStringArray strtmv = new TStringArray;
					line_content.Split( ",", strtmv );
					
					string traderMarkerVehiclePosX = strtmv.Get(0);
					traderMarkerVehiclePosX = FileReadHelper.TrimSpaces(traderMarkerVehiclePosX);
					
					string traderMarkerVehiclePosY = strtmv.Get(1);
					traderMarkerVehiclePosY = FileReadHelper.TrimSpaces(traderMarkerVehiclePosY);
					
					string traderMarkerVehiclePosZ = strtmv.Get(2);
					traderMarkerVehiclePosZ = FileReadHelper.TrimSpaces(traderMarkerVehiclePosZ);
					
					vector markerVehiclePosition = "0 0 0";
					markerVehiclePosition[0] = traderMarkerVehiclePosX.ToFloat();
					markerVehiclePosition[1] = traderMarkerVehiclePosY.ToFloat();
					markerVehiclePosition[2] = traderMarkerVehiclePosZ.ToFloat();

					TraderServerLogs.PrintS("[TRADER] READING MARKER VEHICLE ENTRY..");

					m_Trader_TraderVehicleSpawns.Insert(markerVehiclePosition);

					// Get Trader Marker Vehicle Orientation:
					line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<VehicleSpawnOri>", "<TraderMarker>");

					if(line_content == string.Empty)
						break;

					if (line_content.Contains("<TraderMarker>"))
					{
						skipLine = true;
						m_Trader_TraderVehicleSpawnsOrientation.Insert("0 0 0");
						continue;
					}

					line_content.Replace("<VehicleSpawnOri>", "");
					line_content = FileReadHelper.TrimComment(line_content);

					TStringArray strtmvd = new TStringArray;
					line_content.Split( ",", strtmvd );
					
					string traderMarkerVehicleOriX = strtmvd.Get(0);
					traderMarkerVehicleOriX = FileReadHelper.TrimSpaces(traderMarkerVehicleOriX);
					
					string traderMarkerVehicleOriY = strtmvd.Get(1);
					traderMarkerVehicleOriY = FileReadHelper.TrimSpaces(traderMarkerVehicleOriY);
					
					string traderMarkerVehicleOriZ = strtmvd.Get(2);
					traderMarkerVehicleOriZ = FileReadHelper.TrimSpaces(traderMarkerVehicleOriZ);
					
					vector markerVehicleOrientation = "0 0 0";
					markerVehicleOrientation[0] = traderMarkerVehicleOriX.ToFloat();
					markerVehicleOrientation[1] = traderMarkerVehicleOriY.ToFloat();
					markerVehicleOrientation[2] = traderMarkerVehicleOriZ.ToFloat();

					m_Trader_TraderVehicleSpawnsOrientation.Insert(markerVehicleOrientation);


					markerCounter++;
				}
				
				CloseFile(file_index);
				
				//------------------------------------------------------------------------------------
				
				file_index = OpenFile(m_Trader_VehiclePartsFilePath, FileMode.READ);
				
				if ( file_index == 0 )
				{
					Param1<string> msgRp4 = new Param1<string>( "[Trader] FOUND NO VEHICLEPARTS FILE!" );
					GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp4, true, player.GetIdentity());
					return;
				}
				
				skipLine = false;
				int vehicleCounter = 0;
				
				line_content = "";
				while ( vehicleCounter <= 5000 && line_content.Contains("<FileEnd>") == false)
				{
					// Get Vehicle Name Entrys:
					if (!skipLine)
						line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<VehicleParts>", "<FileEnd>");
					else
						skipLine = false;
					
					if (!line_content.Contains("<VehicleParts>"))
						continue;
					
					line_content.Replace("<VehicleParts>", "");
					line_content = FileReadHelper.TrimComment(line_content);
					line_content = FileReadHelper.TrimSpaces(line_content);
					
					TraderServerLogs.PrintS("[TRADER] READING VEHICLE NAME ENTRY..");

					m_Trader_Vehicles.Insert(line_content);

					char_count = 0;
					int vehiclePartsCounter = 0;
					//while ( vehiclePartsCounter <= 5000  && char_count != -1 && line_content.Contains("<FileEnd>") == false)
					while (true)
					{
						// Get Vehicle Parts Entrys:
						char_count = FGets( file_index,  line_content );

						line_content = FileReadHelper.TrimComment(line_content);
						line_content = FileReadHelper.TrimSpaces(line_content);

						if (line_content == "")
							continue;

						if (line_content.Contains("<VehicleParts>"))
						{
							skipLine = true;						
							break;
						}

						if (line_content.Contains("<FileEnd>") || char_count == -1 || vehiclePartsCounter > 5000)
						{
							line_content = "<FileEnd>";
							break;
						}

						m_Trader_VehiclesParts.Insert(line_content);
						m_Trader_VehiclesPartsVehicleId.Insert(vehicleCounter);

						vehiclePartsCounter++;
					}
					
					vehicleCounter++;
				}
				
				CloseFile(file_index);
				
				//------------------------------------------------------------------------------------

				TraderServerLogs.PrintS("[TRADER] DONE READING!");
				
				Param1<string> crp1 = new Param1<string>( m_Trader_CurrencyItemType );
				GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_CURRENCYTYPE_ENTRY, crp1, true, player.GetIdentity());
				TraderServerLogs.PrintS("[TRADER] CURRENCYTYPE: " + m_Trader_CurrencyItemType);
				
				//int i = 0;
				for ( i = 0; i < m_Trader_TraderNames.Count(); i++ )
				{
					Param1<string> crp2 = new Param1<string>( m_Trader_TraderNames.Get(i) );
					GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_NAME_ENTRY, crp2, true, player.GetIdentity());
					TraderServerLogs.PrintS("[TRADER] TRADERNAME: " + m_Trader_TraderNames.Get(i));
				}
				
				for ( i = 0; i < m_Trader_Categorys.Count(); i++ )
				{
					Param2<string, int> crp3 = new Param2<string, int>( m_Trader_Categorys.Get(i), m_Trader_CategorysTraderKey.Get(i) );
					GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_CATEGORY_ENTRY, crp3, true, player.GetIdentity());
					TraderServerLogs.PrintS("[TRADER] TRADERCATEGORY: " + m_Trader_Categorys.Get(i) + ", " + m_Trader_CategorysTraderKey.Get(i));
				}
				
				for ( i = 0; i < m_Trader_ItemsClassnames.Count(); i++ )
				{
					Param6<int, int, string, int, int, int> crp4 = new Param6<int, int, string, int, int, int>( m_Trader_ItemsTraderId.Get(i), m_Trader_ItemsCategoryId.Get(i), m_Trader_ItemsClassnames.Get(i), m_Trader_ItemsQuantity.Get(i), m_Trader_ItemsBuyValue.Get(i), m_Trader_ItemsSellValue.Get(i) );
					GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_ITEM_ENTRY, crp4, true, player.GetIdentity());
					TraderServerLogs.PrintS("[TRADER] ITEMENTRY: " + m_Trader_ItemsTraderId.Get(i) + ", " + m_Trader_ItemsCategoryId.Get(i) + ", " + m_Trader_ItemsClassnames.Get(i) + ", " + m_Trader_ItemsQuantity.Get(i) + ", " + m_Trader_ItemsBuyValue.Get(i) + ", " + m_Trader_ItemsSellValue.Get(i));
				}
				
				for ( i = 0; i < m_Trader_TraderPositions.Count(); i++ )
				{
					Param5<int, vector, int, vector, vector> crp5 = new Param5<int, vector, int, vector, vector>( m_Trader_TraderIDs.Get(i), m_Trader_TraderPositions.Get(i), m_Trader_TraderSafezones.Get(i), m_Trader_TraderVehicleSpawns.Get(i), m_Trader_TraderVehicleSpawnsOrientation.Get(i) );
					GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_MARKER_ENTRY, crp5, true, player.GetIdentity());
					TraderServerLogs.PrintS("[TRADER] MARKERENTRY: " + m_Trader_TraderIDs.Get(i) + ", " + m_Trader_TraderPositions.Get(i) + ", " + m_Trader_TraderSafezones.Get(i) + ", " + m_Trader_TraderVehicleSpawns.Get(i) + ", " + m_Trader_TraderVehicleSpawnsOrientation.Get(i));
				}

				for ( i = 0; i < m_Trader_Vehicles.Count(); i++ )
				{
					Param1<string> crp6 = new Param1<string>( m_Trader_Vehicles.Get(i) );
					//GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_VEHICLE_ENTRY, crp6, true, player.GetIdentity());
					TraderServerLogs.PrintS("[TRADER] VEHICLEENTRY: " + m_Trader_Vehicles.Get(i));
				}

				for ( i = 0; i < m_Trader_VehiclesParts.Count(); i++ )
				{
					Param2<string, int> crp7 = new Param2<string, int>( m_Trader_VehiclesParts.Get(i), m_Trader_VehiclesPartsVehicleId.Get(i) );
					//GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_VEHICLEPART_ENTRY, crp7, true, player.GetIdentity());
					TraderServerLogs.PrintS("[TRADER] VEHICLEPARTENTRY: " + m_Trader_VehiclesPartsVehicleId.Get(i) + ", " + m_Trader_VehiclesParts.Get(i));
				}
				
				// confirm that all data was sended:
				m_Trader_RecievedAllData = true;
				
				Param1<bool> crpConf = new Param1<bool>( true );
				GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_DATA_CONFIRMATION, crpConf, true, player.GetIdentity());
				TraderServerLogs.PrintS("[TRADER] DEBUG END");
				
				m_Trader_IsReadingTraderFileEntrys = false;
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