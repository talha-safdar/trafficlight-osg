#include "ButtonCommandSet.h"

CommandBase::CommandBase()
{

}

CommandBase::~CommandBase()
{

}

ButtonCommandSet::ButtonCommandSet()
{

}
ButtonCommandSet::~ButtonCommandSet()
{

}

// object instance
ButtonCommandSet* ButtonCommandSet::instance()
{
	static ButtonCommandSet bcs;
	return &bcs;
}

// Add the command associated with the menu id
void ButtonCommandSet::addCommand(int nId, CommandBase* pFunc)
{
	std::map<int, CommandBase*>::iterator iter = m_mapIdToCommands.find(nId);
	if (iter != m_mapIdToCommands.end())
		m_mapIdToCommands.erase(iter);

	m_mapIdToCommands.insert(std::pair<int, CommandBase*>(nId, pFunc));
}

// Get the associated command according to the menu id
CommandBase* ButtonCommandSet::getCommandById(int nId)
{
	std::map<int, CommandBase*>::iterator iter = m_mapIdToCommands.find(nId);
	if (iter == m_mapIdToCommands.end())
		return nullptr;

	return iter->second;
}