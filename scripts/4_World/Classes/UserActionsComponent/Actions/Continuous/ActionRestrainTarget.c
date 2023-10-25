modded class ActionRestrainTarget
{
    override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
        if (player.IsInSafeZone())
            return false;

        PlayerBase targetPlayer = PlayerBase.Cast(target.GetObject());
        if (!targetPlayer)
			return false;

        if (targetPlayer.IsInSafeZone())
            return false;

		return super.ActionCondition(player, target, item);
	}
}