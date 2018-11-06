modded class MissionGameplay
{
	bool handledFirstInput = false;
	
	float traderModIsLoadedReplicationTimer = 0.1;
	
	/*override void OnMissionStart()
	{
		//does not display HUD until player is fully loaded
		//m_hud_root_widget.Show(true);
		GetUIManager().ShowUICursor(false);
		g_Game.SetMissionState( DayZGame.MISSION_STATE_GAME );
		
		//Param1<PlayerBase> rp4 = new Param1<PlayerBase>( g_Game.GetPlayer() );
		//GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_TRADER_MOD_IS_LOADED, new Param1<bool>( false ), true); // TO SERVER: traderModIsLoaded();
		//Print("[TRADER] Mod is loaded!");
	}*/
	
	void TickScheduler(float timeslice)
	{
		PlayerBase player = PlayerBase.Cast( GetGame().GetPlayer() );
				
		if( player )
			player.OnTick();
			
		//---------------------------------------------- TRADER BEGIN ----------------------------
		
		if ( player )
		{		
			player.OnTick();
			
			traderModIsLoadedReplicationTimer -= timeslice;
			
			if (traderModIsLoadedReplicationTimer > 0)
				return;
			
			traderModIsLoadedReplicationTimer = 0.5;
			
			if (!GetGame().IsServer() && !player.m_Trader_TraderModIsLoadedHandled)
			{
				GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_TRADER_MOD_IS_LOADED, new Param1<PlayerBase>( GetGame().GetPlayer() ), true); // TO SERVER: traderModIsLoaded();
			}

			//if ( player.GetActionManager() )	 player.GetActionManager().DisableActions();
			////player.GetInventory().LockInventory(LOCK_FROM_SCRIPT);
			//player.m_InventorySoftLocked = true;

			//player.GetInputController().OverrideAimChangeX(true,0);
			//player.GetInputController().OverrideAimChangeY(true,0);
			//player.OverrideShootFromCamera(true);
			//player.GetInputController().OverrideRaise(true, false);
		}
		
		// TEST ///
		if(!player.TEST_ClassnamesInitialized)
			TEST_InitialClassnames();
		
		if (!player.TEST_PreviewObjectFreeze)
		{			
			player.TEST_PreviewObjectPosition = player.GetPosition() + player.TEST_PreviewObjPosOffset;
			player.TEST_PreviewObjectPosition[1] = player.GetPosition()[1] + player.TEST_PreviewObjYOffset;
		}
		else
		{
			player.TEST_PreviewObjPosOffset = player.TEST_PreviewObjectPosition - player.GetPosition()  ;
		}
			
		if (player.TEST_PreviewObjectIsCreated)
		{
			Param2<Object, vector> rps1 = new Param2<Object, vector>( player.TEST_PreviewObj, player.TEST_PreviewObjectPosition );
			GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_TEST_REPOS_OBJECT, rps1, true); // TO SERVER
			
			if (player.TEST_updateDir)
			{
				player.TEST_updateDir = false;
				
				vector dir = vector.Zero;
				dir[0] = player.TEST_PreviewObjDirOffset;
				
				Param2<Object, vector> rps2 = new Param2<Object, vector>( player.TEST_PreviewObj, dir );
				GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_TEST_REDIR_OBJECT, rps2, true); // TO SERVER
			}
		}
	}
	
	override void OnKeyPress(int key)
	{
		super.OnKeyPress(key);
		m_Hud.KeyPress(key);
		
		
		//temporary
		//Gestures [.]
#ifdef DEVELOPER
		/*if ( key == KeyCode.KC_PERIOD )
		{
			//open gestures menu
			if ( !GetUIManager().IsMenuOpen( MENU_GESTURES ) )
			{
				//TODO reconnect when appropriate
				GesturesMenu.OpenMenu();
			}
		}*/
#endif
		/*
		//temporary
		//Radial Quickbar [,]
		if ( key == KeyCode.KC_COMMA )
		{
			//open radial quickbar menu
			if ( !GetGame().GetUIManager().IsMenuOpen( MENU_RADIAL_QUICKBAR ) )
			{
				//TODO reconnect when appropriate
				RadialQuickbarMenu.OpenMenu();
			}
		}
		*/
		
#ifdef DEVELOPER
		// drop item prototype
		if ( key == KeyCode.KC_N )
		{
			PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
			if (player && player.GetItemInHands() && !GetUIManager().GetMenu())
			{
				ActionManagerClient manager = ActionManagerClient.Cast(player.GetActionManager());
				manager.ActionDropItemStart(player.GetItemInHands(),null);
			}
		}
		
		/*if ( key == KeyCode.KC_P )
		{
			if (!player)
				player = PlayerBase.Cast(GetGame().GetPlayer());
			player.SetHealth("Brain","Health",0);
			player.SetHealth("","Health",0);
		}*/
		
		/*if ( key == KeyCode.KC_P )
		{
			if (!player)
				player = PlayerBase.Cast(GetGame().GetPlayer());
			int slot_id = InventorySlots.GetSlotIdFromString("Legs"); 
 			EntityAI players_legs = player.GetInventory().FindPlaceholderForSlot( slot_id );
			Print("--attachment type = " + players_legs.GetType());
		}*/
		
		/*if ( key == KeyCode.KC_P )
		{
			if (!player)
				player = PlayerBase.Cast(GetGame().GetPlayer());
			if (player && player.m_EmoteManager && player.GetItemInHands())
			{
				player.DropItem(player.GetItemInHands());
			}
		}*/
		
		if ( key == KeyCode.KC_Q )
		{
			//SEffectManager.PlaySound("HandCuffs_A_SoundSet", GetGame().GetPlayer().GetPosition(), 0, 0, false);
		}
#endif
		
		//---------------------------------------------- TRADER BEGIN ----------------------------
		
		
		PlayerBase player = g_Game.GetPlayer();
		
		if (handledFirstInput == false)
		{
			/*player.MessageImportant("Welcome to Kraxus Gaming!");
			player.MessageImportant("Press 'B'-Key to open the Trader.");
			player.MessageImportant(" ");*/
			
			if (player.m_Trader_RecievedAllData == false)
			{
				Param1<PlayerBase> rp1 = new Param1<PlayerBase>( g_Game.GetPlayer() );
				GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_REQUEST_TRADER_DATA, rp1, true); // TO SERVER: requestTraderData();
			}
			
			handledFirstInput = true;
		}
		
		if ( key == KeyCode.KC_T )
		{
			//vector teleportPosition = "10741.7 6.94919 2499.11";
			//Param2<PlayerBase, vector> rp3 = new Param2<PlayerBase, vector>( player, teleportPosition );
			//GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_DEBUG_TELEPORT, rp3, true); // TO SERVER: teleportPlayer();

			/*ItemBase m_item = player.GetItemInHands();
			if(m_item)
			{
				bool m_ItemIsOn = m_item.IsPilotLight();
				if (m_ItemIsOn)
					m_item.SetPilotLight(false);
				m_item.SetInvisible(true);
				//m_ItemToHands = true;
			}*/

			//player.SetSuicide(false);

			GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_CREATE_ITEM_IN_INVENTORY, new Param3<PlayerBase, string, int>(player, "LandMineTrap", 5), true); // TO SERVER: createInInventory(player, itemType);
		}

		/*vector playerPos = player.GetPosition();
		vector orientation = player.GetOrientation();
		vector size = "6 6 6";
		array<Object> excluded_objects = new array<Object>;
		array<Object> nearby_objects = new array<Object>;
		
		if ( key == KeyCode.KC_N )
		{
			if(GetGame().IsBoxColliding( playerPos, orientation, size, excluded_objects, nearby_objects))
			{
				for (int j = 0, c = nearby_objects.Count(); j < c; ++j)
				{
					player.MessageStatus("(D) DELETING: " + nearby_objects.Get(j).GetType());
					Param1<Object> rp2 = new Param1<Object>( nearby_objects.Get(j) );
					GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_DEBUG_DELETE_OBJECT, rp2, true); // TO SERVER: deleteObject();
				}
			}
		}*/

		if (key == KeyCode.KC_N)
		{
			GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_CREATE_ITEM_IN_INVENTORY, new Param3<PlayerBase, string, int>(player, "MountainBag_Green", -1), true); // TO SERVER: createInInventory(player, itemType);
		}
		
		if ( key == KeyCode.KC_M && player.m_Trader_RecievedAllData)
		{			
			/*if(GetGame().IsBoxColliding( playerPos, orientation, size, excluded_objects, nearby_objects))
			{
				for (int k = 0, d = nearby_objects.Count(); k < d; ++k)
				{
					player.MessageStatus("(C) CLASS: " + nearby_objects.Get(k).GetType());
				}
			}*/
		
			Param3<PlayerBase, string, int> rp4 = new Param3<PlayerBase, string, int>(player, player.m_Trader_CurrencyItemType, 35);
			GetGame().RPCSingleParam(GetGame().GetPlayer(), TRPCs.RPC_CREATE_ITEM_IN_INVENTORY, rp4, true); // TO SERVER: createInInventory(player, itemType);
		}

		if ( key == KeyCode.KC_B )
		{			
			bool traderNearby = false;
			int traderID = -1;
			
			if (player.m_Trader_RecievedAllData == false)
			{			
				player.MessageStatus("[Trader] MISSING TRADER DATA FROM SERVER!");
				player.MessageStatus("[Trader] TRYING TO GET TRADER DATA FROM SERVER..");
				
				Param1<PlayerBase> rp2 = new Param1<PlayerBase>( g_Game.GetPlayer() );
				GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_REQUEST_TRADER_DATA, rp2, true); // TO SERVER: requestTraderData();
				
				return;
			}
			
			for ( int i = 0; i < player.m_Trader_TraderPositions.Count(); i++ )
			{				
				if (vector.Distance(player.GetPosition(), player.m_Trader_TraderPositions.Get(i)) <= 1.7)
				{
					traderNearby = true;
					traderID = player.m_Trader_TraderIDs.Get(i);

					//player.MessageStatus("DIST: " + vector.Distance(player.GetPosition(), player.m_Trader_TraderPositions.Get(i)));
				}
			}
			
			if (!traderNearby)
			{
				player.MessageStatus("There is no Trader nearby..");
				return;
			}
				
			
			if ( g_Game.GetUIManager().GetMenu() == NULL )
			{					
				ref TraderMenu m_TraderMenu = new TraderMenu;
				m_TraderMenu.m_TraderID = traderID;
				m_TraderMenu.Init();
				GetGame().GetUIManager().ShowScriptedMenu( m_TraderMenu, NULL );
			}
		}
		
		// TEST!
		/*if ( key == KeyCode.KC_H )
		{
			if (!player.TEST_PreviewObjectIsCreated)
			{
				player.MessageStatus("START OBJECT PREVIEW");
				
				player.TEST_PreviewObjectIsCreated = true;
				
				vector dir = vector.Zero;
				dir[0] = player.TEST_PreviewObjDirOffset;
				
				//player.TEST_PreviewObj = GetGame().CreateObject( "GardenPlot", TEST_PreviewObjectPosition, false, false, true );				
				Param4<PlayerBase, string, vector, vector> rps1 = new Param4<PlayerBase, string, vector, vector>( player, "Land_BusStop_City", player.TEST_PreviewObjectPosition, dir );
				GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_TEST_PLACE_PREVIEW_OBJECT, rps1, true); // TO SERVER
			}
		}*/
		
		if (player.TEST_PreviewObjectIsCreated)
		{
			if ( key == 0x48) // Numpad 8
			{
				player.TEST_PreviewObjYOffset += 0.1;
			}
			if ( key == 0x50) // Numpad 2
			{
				player.TEST_PreviewObjYOffset -= 0.1;
			}
			
			if ( key == 0x4B ) // Numpad 4
			{
				player.TEST_PreviewObjDirOffset += 1;
				
				if (player.TEST_PreviewObjDirOffset >= 360)
					player.TEST_PreviewObjDirOffset -= 360;
				
				player.MessageStatus("DIROFFSET: " + player.TEST_PreviewObjDirOffset);
				
				player.TEST_updateDir = true;
			}
			if ( key == 0x4D ) // Numpad 6
			{
				player.TEST_PreviewObjDirOffset -= 1;
				
				if (player.TEST_PreviewObjDirOffset < 0)
					player.TEST_PreviewObjDirOffset += 360;
				
				player.MessageStatus("DIROFFSET: " + player.TEST_PreviewObjDirOffset);
				
				player.TEST_updateDir = true;
			}
			
			vector dir = vector.Zero;
			if ( key == 0x47 ) // Numpad 7
			{
				player.TEST_ClassnameCategoryID -= 1;
				
				if (player.TEST_ClassnameCategoryID < 0)
					player.TEST_ClassnameCategoryID = 0;
				
				player.MessageStatus("CATEGORY: " + player.TEST_ClassnameCategoryName.Get(player.TEST_ClassnameCategoryID));
				
				Param1<Object> rps1 = new Param1<Object>( player.TEST_PreviewObj );
				GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_TEST_DELETE_OBJECT, rps1, true); // TO SERVER
				
				dir[0] = player.TEST_PreviewObjDirOffset;
				
				Param4<PlayerBase, string, vector, vector> rps2 = new Param4<PlayerBase, string, vector, vector>( player, GetClassname(), player.TEST_PreviewObjectPosition, dir );
				GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_TEST_PLACE_PREVIEW_OBJECT, rps2, true); // TO SERVER
			}
			if ( key == 0x49 ) // Numpad 9
			{
				player.TEST_ClassnameCategoryID += 1;
				
				if (player.TEST_ClassnameCategoryID >= player.TEST_ClassnameCategoryName.Count())
					player.TEST_ClassnameCategoryID = player.TEST_ClassnameCategoryName.Count() - 1;
				
				player.MessageStatus("CATEGORY: " + player.TEST_ClassnameCategoryName.Get(player.TEST_ClassnameCategoryID));
				
				Param1<Object> rps3 = new Param1<Object>( player.TEST_PreviewObj );
				GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_TEST_DELETE_OBJECT, rps3, true); // TO SERVER
				
				dir[0] = player.TEST_PreviewObjDirOffset;
				
				Param4<PlayerBase, string, vector, vector> rps4 = new Param4<PlayerBase, string, vector, vector>( player, GetClassname(), player.TEST_PreviewObjectPosition, dir );
				GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_TEST_PLACE_PREVIEW_OBJECT, rps4, true); // TO SERVER
			}
			
			if ( key == 0x4F) // Numpad 1
			{
				DecreaseClassnameID();
				
				Param1<Object> rps5 = new Param1<Object>( player.TEST_PreviewObj );
				GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_TEST_DELETE_OBJECT, rps5, true); // TO SERVER
				
				dir[0] = player.TEST_PreviewObjDirOffset;
				
				Param4<PlayerBase, string, vector, vector> rps6 = new Param4<PlayerBase, string, vector, vector>( player, GetClassname(), player.TEST_PreviewObjectPosition, dir );
				GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_TEST_PLACE_PREVIEW_OBJECT, rps6, true); // TO SERVER
				
			}
			if ( key == 0x51) // Numpad 3
			{
				IncreaseClassnameID();
				
				Param1<Object> rps7 = new Param1<Object>( player.TEST_PreviewObj );
				GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_TEST_DELETE_OBJECT, rps7, true); // TO SERVER
				
				dir[0] = player.TEST_PreviewObjDirOffset;
				
				Param4<PlayerBase, string, vector, vector> rps8 = new Param4<PlayerBase, string, vector, vector>( player, GetClassname(), player.TEST_PreviewObjectPosition, dir );
				GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_TEST_PLACE_PREVIEW_OBJECT, rps8, true); // TO SERVER
			}
		}
	}
	
	override void OnKeyRelease(int key)
	{
		super.OnKeyRelease(key);
		
		//temporary
		//Gestures [.]
#ifdef DEVELOPER
		/*if ( key == KeyCode.KC_PERIOD )
		{
			//close gestures menu
			if ( GetUIManager().IsMenuOpen( MENU_GESTURES ) )
			{
				//TODO reconnect when appropriate
				GesturesMenu.CloseMenu();
			}
		}*/
#endif
		/*
		//temporary
		//Radial Quickbar [,]
		if ( key == KeyCode.KC_COMMA )
		{
			//close radial quickbar menu
			if ( GetGame().GetUIManager().IsMenuOpen( MENU_RADIAL_QUICKBAR ) )
			{
				//TODO reconnect when appropriate
				RadialQuickbarMenu.CloseMenu();
			}
		}
		*/
		
		//---------------------------------------------- TRADER BEGIN ----------------------------
		
		PlayerBase player = GetGame().GetPlayer();
		
		if ( key == 0x3B ) // F1
		{
			vector teleportPosition1 = "11876.8 140.012 12467";
			Param2<PlayerBase, vector> rpt1 = new Param2<PlayerBase, vector>( player, teleportPosition1 );
			GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_DEBUG_TELEPORT, rpt1, true); // TO SERVER: teleportPlayer();
		}
		
		if ( key == 0x3C ) // F2
		{
			vector teleportPosition2 = "8316.31 292.012 5975.25";
			Param2<PlayerBase, vector> rpt2 = new Param2<PlayerBase, vector>( player, teleportPosition2 );
			GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_DEBUG_TELEPORT, rpt2, true); // TO SERVER: teleportPlayer();
		}
		
		if ( key == 0x3D ) // F3
		{
			vector teleportPosition3 = "3702.01 402.012 5991.82";
			Param2<PlayerBase, vector> rpt3 = new Param2<PlayerBase, vector>( player, teleportPosition3 );
			GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_DEBUG_TELEPORT, rpt3, true); // TO SERVER: teleportPlayer();
		}

		if ( key == 0x3E ) // F4
		{
			player.MessageStatus("PDIR:" + player.GetDirection());
		}
		
		
		if ( key == 0x52 ) // Numpad 0
		{
			if (!player.TEST_PreviewObjectIsCreated)
			{
				player.MessageStatus("START OBJECT PREVIEW");
				
				player.TEST_PreviewObjectIsCreated = true;
				
				vector dir = vector.Zero;
				dir[0] = player.TEST_PreviewObjDirOffset;
				
				//player.TEST_PreviewObj = GetGame().CreateObject( "GardenPlot", TEST_PreviewObjectPosition, false, false, true );				
				Param4<PlayerBase, string, vector, vector> rps1 = new Param4<PlayerBase, string, vector, vector>( player, GetClassname(), player.TEST_PreviewObjectPosition, dir );
				GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_TEST_PLACE_PREVIEW_OBJECT, rps1, true); // TO SERVER
			}
			else
			{			
				player.MessageStatus("END OBJECT PREVIEW");
				
				//GetGame().ObjectDelete(player.TEST_PreviewObj);
				Param1<Object> rps2 = new Param1<Object>( player.TEST_PreviewObj );
				GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_TEST_DELETE_OBJECT, rps2, true); // TO SERVER
				
				player.TEST_PreviewObjectIsCreated = false;
			}
		}
		
		if ( key == KeyCode.KC_ADD && player.TEST_PreviewObjectIsCreated) // Numpad +
		{
			player.MessageStatus("PLACED OBJECT!");
			
			Param3<string, vector, vector> rps3 = new Param3<string, vector, vector>( player.TEST_PreviewObj.GetType(), player.TEST_PreviewObj.GetPosition(), player.TEST_PreviewObj.GetDirection() );
			GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_TEST_PLACE_OBJECT, rps3, true);
			
			player.TEST_PreviewObjectFreeze = false;
		}
		
		if ( key == 0x4C && player.TEST_PreviewObjectIsCreated) // Numpad 5
		{
			player.TEST_PreviewObjectFreeze = !player.TEST_PreviewObjectFreeze;
		}
		
		if ( key == KeyCode.KC_MULTIPLY ) // Numpad *
		{
			GetGame().RPCSingleParam(g_Game.GetPlayer(), TRPCs.RPC_TEST_SAVE_OBJECTS, new Param1<bool>( false ), true);
		}
	}
	
	private string GetClassname()
	{
		PlayerBase player = g_Game.GetPlayer();
		
		switch(player.TEST_ClassnameCategoryID)
		{
			case 0:
			return player.TEST_ClassnamesResidential.Get(player.TEST_ClassnamesResidentialID);
			
			case 1:
			return player.TEST_ClassnamesIndustrial.Get(player.TEST_ClassnamesIndustrialID);
			
			case 2:
			return player.TEST_ClassnamesMilitary.Get(player.TEST_ClassnamesMilitaryID);
			
			case 3:
			return player.TEST_ClassnamesWrecks.Get(player.TEST_ClassnamesWrecksID);
			
			case 4:
			return player.TEST_ClassnamesPlants.Get(player.TEST_ClassnamesPlantsID);
			
			case 5:
			return player.TEST_ClassnamesWalls.Get(player.TEST_ClassnamesWallsID);
			
			case 6:
			return player.TEST_ClassnamesRail.Get(player.TEST_ClassnamesRailID);
			
			case 7:
			return player.TEST_ClassnamesSpecific.Get(player.TEST_ClassnamesSpecificID);
		}
		
		return "ERROR";
	}
	
	private void IncreaseClassnameID()
	{
		PlayerBase player = g_Game.GetPlayer();
		
		switch(player.TEST_ClassnameCategoryID)
		{
			case 0:
				player.TEST_ClassnamesResidentialID += 1;
				if (player.TEST_ClassnamesResidentialID >= player.TEST_ClassnamesResidential.Count())
					player.TEST_ClassnamesResidentialID = player.TEST_ClassnamesResidential.Count() - 1;
			break;
			
			case 1:
				player.TEST_ClassnamesIndustrialID += 1;
				if (player.TEST_ClassnamesIndustrialID >= player.TEST_ClassnamesIndustrial.Count())
					player.TEST_ClassnamesIndustrialID = player.TEST_ClassnamesIndustrial.Count() - 1;
			break;
			
			case 2:
				player.TEST_ClassnamesMilitaryID += 1;
				if (player.TEST_ClassnamesMilitaryID >= player.TEST_ClassnamesMilitary.Count())
					player.TEST_ClassnamesMilitaryID = player.TEST_ClassnamesMilitary.Count() - 1;
			break;
			
			case 3:
				player.TEST_ClassnamesWrecksID += 1;
				if (player.TEST_ClassnamesWrecksID >= player.TEST_ClassnamesWrecks.Count())
					player.TEST_ClassnamesWrecksID = player.TEST_ClassnamesWrecks.Count() - 1;
			break;
			
			case 4:
				player.TEST_ClassnamesPlantsID += 1;
				if (player.TEST_ClassnamesPlantsID >= player.TEST_ClassnamesPlants.Count())
					player.TEST_ClassnamesPlantsID = player.TEST_ClassnamesPlants.Count() - 1;
			break;
			
			case 5:
				player.TEST_ClassnamesWallsID += 1;
				if (player.TEST_ClassnamesWallsID >= player.TEST_ClassnamesWalls.Count())
					player.TEST_ClassnamesWallsID = player.TEST_ClassnamesWalls.Count() - 1;
			break;
			
			case 6:
				player.TEST_ClassnamesRailID += 1;
				if (player.TEST_ClassnamesRailID >= player.TEST_ClassnamesRail.Count())
					player.TEST_ClassnamesRailID = player.TEST_ClassnamesRail.Count() - 1;
			break;
			
			case 7:
				player.TEST_ClassnamesSpecificID += 1;
				if (player.TEST_ClassnamesSpecificID >= player.TEST_ClassnamesSpecific.Count())
					player.TEST_ClassnamesSpecificID = player.TEST_ClassnamesSpecific.Count() - 1;
			break;
		}
	}
	
	private void DecreaseClassnameID()
	{
		PlayerBase player = g_Game.GetPlayer();
		
		switch(player.TEST_ClassnameCategoryID)
		{
			case 0:
				player.TEST_ClassnamesResidentialID -= 1;
				if (player.TEST_ClassnamesResidentialID < 0)
					player.TEST_ClassnamesResidentialID = 0;
			break;
			
			case 1:
				player.TEST_ClassnamesIndustrialID -= 1;
				if (player.TEST_ClassnamesIndustrialID < 0)
					player.TEST_ClassnamesIndustrialID = 0;
			break;
			
			case 2:
				player.TEST_ClassnamesMilitaryID -= 1;
				if (player.TEST_ClassnamesMilitaryID < 0)
					player.TEST_ClassnamesMilitaryID = 0;
			break;
			
			case 3:
				player.TEST_ClassnamesWrecksID -= 1;
				if (player.TEST_ClassnamesWrecksID < 0)
					player.TEST_ClassnamesWrecksID = 0;
			break;
			
			case 4:
				player.TEST_ClassnamesPlantsID -= 1;
				if (player.TEST_ClassnamesPlantsID < 0)
					player.TEST_ClassnamesPlantsID = 0;
			break;
			
			case 5:
				player.TEST_ClassnamesWallsID -= 1;
				if (player.TEST_ClassnamesWallsID < 0)
					player.TEST_ClassnamesWallsID = 0;
			break;
			
			case 6:
				player.TEST_ClassnamesRailID -= 1;
				if (player.TEST_ClassnamesRailID < 0)
					player.TEST_ClassnamesRailID = 0;
			break;
			
			case 7:
				player.TEST_ClassnamesSpecificID -= 1;
				if (player.TEST_ClassnamesSpecificID < 0)
					player.TEST_ClassnamesSpecificID = 0;
			break;
		}
	}
	
	private void TEST_InitialClassnames()
	{
		PlayerBase player = g_Game.GetPlayer();
		
		ref array<string> TEST_ClassnameCategoryName = { "Residential", "Industrial", "Military", "Wrecks", "Plants", "Walls", "Rail", "Specific" };
		player.TEST_ClassnameCategoryName = TEST_ClassnameCategoryName;
		
		ref array<string> TEST_ClassnamesResidential = { "GardenPlot", "Polytunnel", "Land_BusStation_building", "Land_BusStation_wall_bench", "Land_BusStop_City", "Land_City_FireStation", "Land_HouseBlock_1F_Corner", "Land_HouseBlock_1F1", "Land_HouseBlock_1F2", "Land_HouseBlock_1F3", "Land_HouseBlock_1F4", "Land_HouseBlock_2F_Corner", "Land_HouseBlock_2F1", "Land_HouseBlock_2F2", "Land_HouseBlock_2F3", "Land_HouseBlock_2F4", "Land_HouseBlock_2F5", "Land_HouseBlock_2F6", "Land_HouseBlock_2F7", "Land_HouseBlock_2F8", "Land_HouseBlock_2F9", "Land_HouseBlock_3F_Corner1", "Land_HouseBlock_3F_Corner2", "Land_HouseBlock_3F1", "Land_HouseBlock_3F2", "Land_HouseBlock_5F", "Land_City_Stand_FastFood", "Land_City_Stand_Grocery", "Land_City_Stand_News1", "Land_City_Stand_News2", "Land_House_1B01_Pub", "Land_House_1W01", "Land_House_1W02", "Land_House_1W03", "Land_House_1W04", "Land_House_1W05", "Land_House_1W06", "Land_House_1W07", "Land_House_1W08", "Land_House_1W09", "Land_House_1W10", "Land_House_1W11", "Land_House_1W12", "Land_House_2B01", "Land_House_2B02", "Land_House_2B03", "Land_House_2B04", "Land_House_2W01", "Land_House_2W02", "Land_House_2W03", "Land_House_2W04", "Land_Village_Pub", "Land_City_Hospital", "Land_Village_HealthCare", "Land_Ladder", "Land_Ladder_Half", "Land_Misc_FeedShack", "Land_Misc_DogHouse", "Land_Misc_DeerStand2", "Land_Misc_DeerStand1", "Land_Misc_Greenhouse", "Land_Misc_Toilet_Mobile", "Land_Misc_Toilet_Dry", "Land_Misc_Well_Pump_Yellow", "Land_Misc_Well_Pump_Blue", "Land_Lamp_City1_amp", "Land_Office_Municipal1", "Land_Office_Municipal2", "Land_Office1", "Land_Office2", "Land_Village_PoliceStation", "Land_City_PoliceStation", "Land_City_School", "Land_Shed_M1", "Land_Shed_M3", "Land_Shed_M4", "Land_Shed_W1", "Land_Shed_W2", "Land_Shed_W3", "Land_Shed_W4", "Land_Shed_W5", "Land_Shed_W6", "Land_City_Store", "Land_Village_store", "Land_Tenement_Big", "Land_Tenement_Small" };
		player.TEST_ClassnamesResidential = TEST_ClassnamesResidential;
		ref array<string> TEST_ClassnamesIndustrial = { "Land_Cementworks_Conveyorhall", "Land_CementWorks_SiloBig1A", "Land_CementWorks_ExpeditionA", "Land_CementWorks_Hall1", "Land_CementWorks_Hall2_Brick", "Land_CementWorks_Hall2_Grey", "Land_CementWorks_MillA", "Land_CementWorks_MillB", "Land_CementWorks_MillC", "Land_CementWorks_RotFurnace", "Land_CementWorks_ExpeditionB", "Land_CementWorks_ExpeditionC", "Land_Construction_Building", "Land_Construction_Crane", "Land_Construction_House1", "Land_Construction_House2", "Land_Container_1Aoh", "Land_Container_1Bo", "Land_Container_1Mo", "Land_Container_1Moh", "Land_Dam_Concrete_20_Floodgate", "Land_Barn_Brick1", "Land_Barn_Brick2", "Land_Barn_Wood1", "Land_Barn_Wood2", "Land_Barn_Metal_Big", "Land_Farm_CowshedA", "Land_Farm_CowshedB", "Land_Farm_CowshedC", "Land_Farm_Hopper", "Land_Farm_Watertower", "Land_Farm_WaterTower_Small", "Land_Garage_Big", "Land_Garage_Row_Big", "Land_Garage_Row_Small", "Land_Garage_Office", "Land_Garage_Small", "Land_Boathouse", "Land_Lighthouse", "Land_Pier_Crane_A", "Land_Pier_Crane_B", "Land_Factory_Lathes", "Land_Factory_Small", "Land_Guardhouse", "Land_Repair_Center", "Land_Water_Station", "Land_Workshop1", "Land_Workshop2", "Land_Workshop3", "Land_Workshop4", "Land_Workshop5", "Land_Mine_Building", "Land_Misc_Scaffolding", "Land_Pipe_Big_BuildR", "Land_Pipe_Big_BuildL", "Land_Pipe_Big_CornerL", "Land_Pipe_Big_CornerR", "Land_Pipe_Big_Ground1", "Land_Pipe_Big_Ground2", "Land_Power_Station", "Land_Power_Transformer_Build", "Land_Power_Pole_Conc1_Amp", "Land_Power_Pole_Conc4_Lamp_Amp", "Land_Power_Pole_Wood1_Amp", "Land_Power_Pole_Wood1_Lamp_Amp", "Land_Quarry_Main", "Land_Sawmill_Building", "Land_Sawmill_Illuminanttower", "Land_Shed_Closed", "Land_Smokestack_Big", "Land_Smokestack_Brick", "Land_Smokestack_Metal", "Land_Tank_Big", "Land_Tank_Medium_Stairs", "Land_CoalPlant_Main", "Land_Smokestack_Medium" };
		player.TEST_ClassnamesIndustrial = TEST_ClassnamesIndustrial;
		ref array<string> TEST_ClassnamesMilitary = { "Land_Mil_CamoNet_Roof_east", "Land_Mil_Airfield_HQ", "Land_Mil_ATC_Small", "Land_Mil_ATC_Big", "Land_Mil_Barracks1", "Land_Mil_Barracks2", "Land_Mil_Barracks3", "Land_Mil_Barracks4", "Land_Mil_Barracks5", "Land_Mil_Barracks6", "Land_Mil_Barracks_Round", "Land_Mil_Guardhouse1", "Land_Mil_Guardhouse2", "Land_Mil_Tower_Small", "Land_Mil_GuardTower", "Land_Mil_Fortified_Nest_Big", "Land_Mil_Fortified_Nest_Small", "Land_Mil_Fortified_Nest_Watchtower", "Land_Mil_GuardShed", "Land_Mil_Tent_Big1_1", "Land_Mil_Tent_Big1_2", "Land_Mil_Tent_Big1_3", "Land_Mil_Tent_Big1_4", "Land_Mil_Tent_Big1_5", "Land_Mil_Tent_Big2_1", "Land_Mil_Tent_Big2_2", "Land_Mil_Tent_Big2_3", "Land_Mil_Tent_Big2_4", "Land_Mil_Tent_Big2_5", "Land_Mil_Tent_Big3", "Land_Mil_Tent_Big4", "Land_Tisy_RadarPlatform_Top", "Land_Tisy_KitchenRoom", "Land_Tisy_HQ", "Land_Tisy_Garages", "Land_Tisy_RadarB_Base", "Land_Tisy_RadarB_Antenna", "Land_Tisy_Base_cooler", "Land_Tisy_Barracks", "Land_Airfield_Hangar_Green", "Land_Airfield_ServiceHangar_R", "Land_Airfield_ServiceHangar_L", "Land_Mil_AircraftShelter", "Land_Airfield_Radar_Tall", "Land_Mil_ReinforcedTank1", "Land_Mil_ReinforcedTank2" };
		player.TEST_ClassnamesMilitary = TEST_ClassnamesMilitary;
		ref array<string> TEST_ClassnamesWrecks = { "Land_Wreck_Uaz", "Land_Boat_Small1", "Land_Boat_Small2", "Land_Boat_Small3", "Land_Ship_Big_FrontA", "Land_Ship_Big_FrontB", "Land_Ship_Big_BackA", "Land_Ship_Big_BackB", "Land_Ship_Big_Castle", "Land_Train_742_Blue", "Land_Train_742_Red", "Land_Train_Wagon_Box", "Land_Train_Wagon_Tanker", "Land_Wreck_Caravan_MGreen", "Land_Wreck_Ikarus", "Land_Wreck_Lada_Green", "Land_Wreck_S1023_Blue", "Land_Wreck_V3S", "Land_Wreck_Volha_Blue", "Land_Wreck_C130J" };
		player.TEST_ClassnamesWrecks = TEST_ClassnamesWrecks;
		ref array<string> TEST_ClassnamesPlants = { "ChristmasTree", "BushHard_b_betulaHumilis_1s", "BushHard_b_corylusAvellana_1f", "BushHard_b_corylusAvellana_2s", "BushHard_b_crataegusLaevigata_1s", "BushHard_b_crataegusLaevigata_2s", "BushHard_b_naked_2s", "BushHard_b_prunusSpinosa_1s", "BushHard_b_prunusSpinosa_2s", "BushHard_b_rosaCanina_1s", "BushHard_b_rosaCanina_2s", "BushHard_b_sambucusNigra_1s", "BushHard_b_sambucusNigra_2s", "TreeHard_t_BetulaPendula_1f", "TreeHard_t_BetulaPendula_1fb", "TreeHard_t_BetulaPendula_1s", "TreeHard_t_BetulaPendula_2f", "TreeHard_t_BetulaPendula_2fb", "TreeHard_t_BetulaPendula_2fc", "TreeHard_t_BetulaPendula_2s", "TreeHard_t_BetulaPendula_3f", "TreeHard_t_BetulaPendula_3fb", "TreeHard_t_BetulaPendula_3fc", "TreeHard_t_BetulaPendula_3s", "TreeHard_t_BetulaPendula_2w" };
		player.TEST_ClassnamesPlants = TEST_ClassnamesPlants;
		ref array<string> TEST_ClassnamesWalls = { "Land_Wall_Gate_Camp", "Land_Wall_Gate_Fen2_L", "Land_Wall_Gate_Fen2_R", "Land_Wall_Gate_FenG", "Land_Wall_Gate_FenG_Big", "Land_Wall_Gate_FenG_Big_L", "Land_Wall_Gate_FenG_Big_Open", "Land_Wall_Gate_FenG_Big_R", "Land_Wall_Gate_FenG_Open", "Land_Wall_Gate_FenR", "Land_Wall_Gate_FenR_Big", "Land_Wall_Gate_FenR_Big_L", "Land_Wall_Gate_FenR_Big_Open", "Land_Wall_Gate_FenR_Big_R", "Land_Wall_Gate_FenR_Open", "Land_Wall_Gate_Ind1_L", "Land_Wall_Gate_Ind1_R", "Land_Wall_Gate_Ind2A_L", "Land_Wall_Gate_Ind2A_R", "Land_Wall_Gate_Ind2B_L", "Land_Wall_Gate_Ind2B_R", "Land_Wall_Gate_Ind2Rail_L", "Land_Wall_Gate_Ind2Rail_R", "Land_Wall_Gate_Village", "Land_Wall_Gate_Wood1", "Land_Wall_Gate_Wood2", "Land_Wall_Gate_Wood3", "Land_Wall_Gate_Wood4" };
		player.TEST_ClassnamesWalls = TEST_ClassnamesWalls;
		ref array<string> TEST_ClassnamesRail = { "Land_Rail_Station_Big", "Land_Rail_Station_Small", "Land_Rail_Warehouse_Small" };
		player.TEST_ClassnamesRail = TEST_ClassnamesRail;
		ref array<string> TEST_ClassnamesSpecific = { "Land_Radio_PanelBig", "Land_Radio_PanelPAS", "Land_Tower_TC3_Red", "Land_Tower_TC3_Grey", "Land_Church1_Yellow", "Land_Airfield_Small_Control", "Land_Airfield_Small_Hangar", "Land_Cableway_Base", "Land_Cableway_Tower", "Land_Cableway_Tower_Slope", "Land_Camp_House_white", "Land_Camp_House_brown", "Land_Camp_House_red", "Land_Castle_Bastion", "Land_Castle_Bastion_nolc", "Land_Castle_Gate", "Land_Castle_Wall1_20", "Land_Castle_Wall1_20_Turn", "Land_Castle_Wall1_Corner1", "Land_Castle_Wall1_End1", "Land_Castle_Wall1_End2", "Land_Castle_Wall2_30", "Land_Castle_Wall2_Corner1", "Land_Castle_Wall2_End1", "Land_Castle_Wall2_End2", "Land_Castle_WallS_10", "Land_Castle_WallS_5_D", "Land_Castle_WallS_End", "Land_Castle_Stairs", "Land_Castle_Stairs_nolc", "Land_Castle_Wall1_Corner2", "Land_Castle_Wall2_Corner2", "Land_Castle_Bergfrit", "Land_Castle_Donjon", "Land_Dead_MassGrave", "Land_FuelStation_Build", "Land_FuelStation_Feed", "Land_Hotel_Damaged", "Land_Hotel", "Land_Chapel", "Land_Church2_2", "Land_Church3", "Land_Lunapark_Autodrome", "Land_Lunapark_Carousel_Swan", "Land_Lunapark_Ferris_wheel", "Land_Lunapark_Shooting_Gallery", "Land_Prison_Main", "Land_Prison_Side", "Land_Prison_Wall_Large", "Land_Radio_building", "Land_Monument_T34", "Land_Tower_TC1", "Land_Tower_TC2_Base", "Land_Tower_TC2_Mid", "Land_Tower_TC2_Top", "Land_Tower_TC4_Base", "Land_Tower_TC4_Mid", "Land_Tower_TC4_Top" };
		player.TEST_ClassnamesSpecific = TEST_ClassnamesSpecific;
	}
}