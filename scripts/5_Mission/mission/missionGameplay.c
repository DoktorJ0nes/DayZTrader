modded class MissionGameplay
{		
	override void OnKeyRelease(int key)
	{
		super.OnKeyRelease(key);		

		if ( key == KeyCode.KC_ESCAPE )
		{	
			TraderMenu traderMenu = TraderMenu.Cast(GetGame().GetUIManager().GetMenu());
			if(traderMenu)
		  	{
                traderMenu.Close();
            }
		}
	}
}