modded class ActionCheckPulse
{
    override bool ActionCondition( PlayerBase player, ActionTarget target, ItemBase item )
	{
        PlayerBase ntarget = PlayerBase.Cast(target.GetObject());
		if (ntarget && ntarget.IsTrader())
            return false;

		return super.ActionCondition(player, target, item);
	}
}