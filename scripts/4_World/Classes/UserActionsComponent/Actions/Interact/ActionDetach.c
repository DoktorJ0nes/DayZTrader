modded class ActionDetach
{
    override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
    {
		if (target)
		{
			if (target.GetObject().IsInherited(CarWheel))
			{
				CarScript car = CarScript.Cast(target.GetParent());
				if (car && car.m_Trader_Locked)
				{
					return false;
				}
			}
		}

        return super.ActionCondition(player, target, item);
    }
}