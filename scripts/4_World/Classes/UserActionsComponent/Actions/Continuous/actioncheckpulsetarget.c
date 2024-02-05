modded class ActionCheckPulseTarget
{
    override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
    {
        if (!super.ActionCondition(player, target, item))
            return false;

        PlayerBase targetPlayer = PlayerBase.Cast(target.GetObject());
        if (targetPlayer && targetPlayer.IsTrader())
            return false;

        return true;
    }
};