modded class ActionOpenCarDoors
{
    /*protected bool playerHasVehicleKeyInHands(PlayerBase player, int vehicleKeyHash)
    {
        VehicleKey vehicleKey = VehicleKey.Cast(player.GetHumanInventory().GetEntityInHands());

        if(vehicleKey)
        {
            if(vehicleKey.hash == vehicleKeyHash)
                return true;
        }

        return false;
    }*/

    override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
        CarScript carScript;
        if (Class.CastTo(carScript, target.GetParent()))
        { 
            //if (carScript.m_Trader_Locked && !playerHasVehicleKeyInHands(player, carScript.m_Trader_VehicleKeyHash) && !player.IsInVehicle())
            if (carScript.m_Trader_Locked && !player.IsInVehicle())
            {
                return false;
            }
        }
		
		return super.ActionCondition(player, target, item);
	}
}