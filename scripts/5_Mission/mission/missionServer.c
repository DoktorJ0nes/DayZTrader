modded class MissionServer
{
	static const string m_Trader_ObjectsFilePath = "$profile:Trader/TraderObjects.txt";
		
	ref array<PlayerBase> m_Trader_SpawnedTraderCharacters;
	ref array<vector> m_Trader_SpawnedTraderCharactersDirections;
	float m_Trader_PlayerHiveUpdateTime = 0;
	
	//ref array<Object> objectstoSynchronize = new array<Object>;
	
	override void OnInit()
	{		
		SpawnTraderObjects();
	}
	
	override void OnUpdate(float timeslice)
	{
		UpdateDummyScheduler();
		TickScheduler(timeslice);
		UpdateLogoutPlayers();
		
		//---------------------------------------------- TRADER BEGIN ----------------------------

		m_Trader_PlayerHiveUpdateTime += timeslice;
		if (m_Trader_PlayerHiveUpdateTime >= 1)
			m_Trader_PlayerHiveUpdateTime = 0;
		
		for (int j = 0; j < m_Players.Count(); j++)
		{
			PlayerBase player = PlayerBase.Cast(m_Players.Get(j));
			
			if ( !player )
				continue;
			
			if ( !player.m_Trader_WelcomeMessageHandled )
			{
				if (player.m_Trader_WelcomeMessageTimer < 5.0 && !player.m_Trader_TraderCharacterSynchronizationHandled)
				{
					for ( int l = 0; l < m_Trader_SpawnedTraderCharacters.Count(); l++ )
					{
						//Print("DIRARRAYENTRYS: " + m_Trader_SpawnedTraderCharactersDirections.Count());
						m_Trader_SpawnedTraderCharacters.Get(l).SetDirection(m_Trader_SpawnedTraderCharactersDirections.Get(l));
						//Print("TRADERDIR: " + m_Trader_SpawnedTraderCharacters.Get(l).GetDirection());
						m_Trader_SpawnedTraderCharacters.Get(l).SetOrientation(m_Trader_SpawnedTraderCharacters.Get(l).GetOrientation());
						//Print("TRADERORI: " + m_Trader_SpawnedTraderCharacters.Get(l).GetOrientation());
					}
					
					player.m_Trader_TraderCharacterSynchronizationHandled = true;
				}
				
				if (player.m_Trader_WelcomeMessageTimer > 0)
					player.m_Trader_WelcomeMessageTimer -= timeslice;
				else
				{
					if ( !player.m_Trader_TraderModIsLoaded )
					{
						Param1<string> msgRp0 = new Param1<string>( " " );
						GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp0, true, player.GetIdentity());
						
						msgRp0.param1 = "This Server uses Dr_J0nes Trader Mod.";
						GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp0, true, player.GetIdentity());
						
						msgRp0.param1 = "Please download and install it!";
						GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp0, true, player.GetIdentity());
						
						msgRp0.param1 = " ";
						GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp0, true, player.GetIdentity());
					}
					
					player.m_Trader_WelcomeMessageHandled = true;
				}
			}

			// Kill player after some Time when not have the Mod loaded:
			if (player.m_Trader_WelcomeMessageHandled && !player.m_Trader_TraderModIsLoaded)
			{
				player.m_Trader_WelcomeMessageTimer -= timeslice;
				if (player.m_Trader_WelcomeMessageTimer < -5)
				{
					player.SetHealth( "", "", 0 );
					player.SetHealth( "", "Blood", 0 );
				}
			}
			
			//if (!player.m_Trader_TraderModIsLoaded || !player.m_Trader_RecievedAllData) // m_Trader_RecievedAllData unnoetig?
			if (!player.m_Trader_RecievedAllData)
				continue;	
			
			bool isInSafezone = false;
			
			for (int k = 0; k < player.m_Trader_TraderPositions.Count(); k++)
			{
				if (vector.Distance(player.GetPosition(), player.m_Trader_TraderPositions.Get(k)) <= player.m_Trader_TraderSafezones.Get(k))
					isInSafezone = true;
			}
			
			if (player.m_Trader_IsInSafezone == false && isInSafezone == true)
			{
				player.m_Trader_IsInSafezone = true;
				//player.SetAllowDamage(false);
				GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_IS_IN_SAFEZONE, new Param1<bool>( true ), true, player.GetIdentity());
				player.m_Trader_HealthEnteredSafeZone = player.GetHealth( "", "");
				player.m_Trader_HealthBloodEnteredSafeZone = player.GetHealth( "", "Blood" );
				
				//Param1<string> msgRp = new Param1<string>( " " );
				//GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp, true, player.GetIdentity());
				
				//msgRp.param1 = "[Trader] You entered the Safezone!";
				GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "[Trader] You entered the Safezone!" ), true, player.GetIdentity());
				
				//msgRp.param1 = "Press 'B'-Key to open the Trader Menu.";
				GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "Press 'B'-Key to open the Trader Menu." ), true, player.GetIdentity());
				
				//msgRp.param1 = " ";
				//GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp, true, player.GetIdentity());
			}
			
			if (player.m_Trader_IsInSafezone == true && isInSafezone == false)
			{
				player.m_Trader_IsInSafezone = false;
				//player.SetAllowDamage(true);
				GetGame().RPCSingleParam(player, TRPCs.RPC_SEND_TRADER_IS_IN_SAFEZONE, new Param1<bool>( false ), true, player.GetIdentity());

				//Param1<string> msgRp2 = new Param1<string>( " " );
				//GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp2, true, player.GetIdentity());
				
				//msgRp2.param1 = "[Trader] You left the Safezone!";
				GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, new Param1<string>( "[Trader] You left the Safezone!" ), true, player.GetIdentity());
				
				//msgRp2.param1 = " ";
				//GetGame().RPCSingleParam(player, ERPCs.RPC_USER_ACTION_MESSAGE, msgRp2, true, player.GetIdentity());
			}
			
			if (isInSafezone)
			{
				if (player.GetHealth( "", "") > player.m_Trader_HealthEnteredSafeZone) // allow regaining Health
					player.m_Trader_HealthEnteredSafeZone = player.GetHealth( "", "");
					
				if (player.GetHealth( "", "Blood" ) > player.m_Trader_HealthBloodEnteredSafeZone) // allow regaining Blood
					player.m_Trader_HealthBloodEnteredSafeZone = player.GetHealth( "", "Blood" );
				
				player.SetHealth( "", "", player.m_Trader_HealthEnteredSafeZone );
				player.SetHealth( "","Blood", player.m_Trader_HealthBloodEnteredSafeZone );

				player.SetHealth( "","Shock", player.GetMaxHealth( "", "Shock" ) );

				if (m_Trader_PlayerHiveUpdateTime == 0 && player.IsAlive())
					GetHive().CharacterSave(player);
			}
		}
		
		for ( int i = 0; i < m_Trader_SpawnedTraderCharacters.Count(); i++ )
		{
			m_Trader_SpawnedTraderCharacters.Get(i).SetHealth( m_Trader_SpawnedTraderCharacters.Get(i).GetMaxHealth( "", "" ) );
			m_Trader_SpawnedTraderCharacters.Get(i).SetHealth( "","Blood", m_Trader_SpawnedTraderCharacters.Get(i).GetMaxHealth( "", "Blood" ) );
			//m_Trader_SpawnedTraderCharacters.Get(i).GetStatStamina().Set(1000);
			m_Trader_SpawnedTraderCharacters.Get(i).GetStatEnergy().Set(1000);
			m_Trader_SpawnedTraderCharacters.Get(i).GetStatWater().Set(1000);
			m_Trader_SpawnedTraderCharacters.Get(i).GetStatStomachVolume().Set(300);		
			m_Trader_SpawnedTraderCharacters.Get(i).GetStatStomachWater().Set(300);
			m_Trader_SpawnedTraderCharacters.Get(i).GetStatStomachEnergy().Set(300);
			m_Trader_SpawnedTraderCharacters.Get(i).GetStatHeatComfort().Set(0);

			m_Trader_SpawnedTraderCharacters.Get(i).SetHealth( "","Shock", m_Trader_SpawnedTraderCharacters.Get(i).GetMaxHealth( "", "Shock" ) );

			//m_Trader_SpawnedTraderCharacters.Get(i).SetAllowDamage(false);
		}
		
		/*for (int l = 0; l < objectstoSynchronize.Count(); l++)
		{
			vector tempDir = objectstoSynchronize.Get(l).GetOrientation();
			objectstoSynchronize.Get(l).SetOrientation(tempDir);
		}*/
	}
	
	private void SpawnTraderObjects()
	{
		m_Trader_SpawnedTraderCharacters = new array<PlayerBase>;
		m_Trader_SpawnedTraderCharactersDirections = new array<vector>;
		
		Print("[TRADER] DEBUG START");
		
		FileHandle file_index = OpenFile(m_Trader_ObjectsFilePath, FileMode.READ);
				
		if ( file_index == 0 )
		{
			Print( "[TRADER] FOUND NO TRADEROBJECTS FILE!" );
			return;
		}
		
		int markerCounter = 0;
		bool skipDirEntry = false;
		
		string line_content = "";
		while ( markerCounter <= 5000 && line_content.Contains("<TraderEnd>") == false)
		{
			// Get Object Type ------------------------------------------------------------------------------------
			if (skipDirEntry)
				skipDirEntry = false;
			else
				line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<Object>", "<TraderEnd>");
			
			if (!line_content.Contains("<Object>"))
				continue;
			
			line_content.Replace("<Object>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			line_content = FileReadHelper.TrimSpaces(line_content);
			
			Print("[TRADER] READING OBJECT TYPE ENTRY..");
			
			string traderObjectType = line_content;
			
			// Get Object Position --------------------------------------------------------------------------------
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<ObjectPosition>", "<TraderEnd>");
			
			line_content.Replace("<ObjectPosition>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			
			Print("[TRADER] READING OBJECT POSITION ENTRY..");
			
			TStringArray strso = new TStringArray;
			line_content.Split( ",", strso );
			
			string traderObjectPosX = strso.Get(0);
			traderObjectPosX = FileReadHelper.TrimSpaces(traderObjectPosX);
			
			string traderObjectPosY = strso.Get(1);
			traderObjectPosY = FileReadHelper.TrimSpaces(traderObjectPosY);
			
			string traderObjectPosZ = strso.Get(2);
			traderObjectPosZ = FileReadHelper.TrimSpaces(traderObjectPosZ);
			
			vector objectPosition = "0 0 0";
			objectPosition[0] = traderObjectPosX.ToFloat();
			objectPosition[1] = traderObjectPosY.ToFloat();
			objectPosition[2] = traderObjectPosZ.ToFloat();
			
			Object obj = GetGame().CreateObject( traderObjectType, objectPosition, false, false, true );
			obj.SetPosition(objectPosition); // prevent automatic on ground placing
			obj.SetAllowDamage(false); // <- funzt nicht bei Charakteren..
			Print("[TRADER] SPAWNED OBJECT '" + traderObjectType + "' AT '" + objectPosition + "'");
			
			bool isTrader = false;
			PlayerBase man;
			if (Class.CastTo(man, obj))
			{
				Print("[TRADER] Object was a Man..");
				//m_Trader_SpawnedTraderCharacters.Insert(man);
				isTrader = true;
			}
			
			// Get Object Direction -------------------------------------------------------------------------------
			line_content = FileReadHelper.SearchForNextTermInFile(file_index, "<ObjectDirection>", "<Object>");
			
			if (line_content == string.Empty)
			{
				//Print("[TRADER] SKIPPED OBJECT DIRECTION..");
				
				line_content = "<TraderEnd>";
			}
			
			if (line_content.Contains("<Object>"))
			{
				//Print("[TRADER] SKIPPED OBJECT DIRECTION..");
				
				//obj.SetFlags(EntityFlags.SYNCHRONIZATION_DIRTY, true);
				
				if (isTrader)
				{
					man.m_Trader_IsTrader = true;
					m_Trader_SpawnedTraderCharactersDirections.Insert(obj.GetDirection());
					m_Trader_SpawnedTraderCharacters.Insert(man);
				}
				
				skipDirEntry = true;
				markerCounter++;
				continue;
			}
			
			if (!line_content.Contains("<ObjectDirection>"))
				continue;
			
			line_content.Replace("<ObjectDirection>", "");
			line_content = FileReadHelper.TrimComment(line_content);
			
			Print("[TRADER] READING OBJECT DIRECTION ENTRY..");
			
			TStringArray strsod = new TStringArray;
			line_content.Split( ",", strsod );
			
			string traderObjectDirX = strsod.Get(0);
			traderObjectDirX = FileReadHelper.TrimSpaces(traderObjectDirX);
			
			string traderObjectDirY = strsod.Get(1);
			traderObjectDirY = FileReadHelper.TrimSpaces(traderObjectDirY);
			
			string traderObjectDirZ = strsod.Get(2);
			traderObjectDirZ = FileReadHelper.TrimSpaces(traderObjectDirZ);
			
			vector objectDirection = vector.Zero;
			objectDirection[0] = traderObjectDirX.ToFloat();
			objectDirection[1] = traderObjectDirY.ToFloat();
			objectDirection[2] = traderObjectDirZ.ToFloat();
			
			obj.SetDirection(objectDirection);
			//obj.SetFlags(EntityFlags.SYNCHRONIZATION_DIRTY, true);
			//objectstoSynchronize.Insert(obj);
			obj.SetOrientation(obj.GetOrientation()); // Thats a strange way to synchronize/update Objects.. But it works..
			
			if (isTrader)
			{
				m_Trader_SpawnedTraderCharactersDirections.Insert(obj.GetDirection());
				m_Trader_SpawnedTraderCharacters.Insert(man);
			}
			
			Print("[TRADER] OBJECT DIRECTION = '" + obj.GetDirection() + "'");
			
			markerCounter++;
		}
		
		CloseFile(file_index);
		Print("[TRADER] DEBUG END");
	}
}