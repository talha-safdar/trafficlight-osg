#pragma once

#include <map>
#include <string>

class CommandBase
{
public:
	CommandBase();
	~CommandBase();

	virtual bool operator()() = 0;
};


class ButtonCommandSet
{
public:
	ButtonCommandSet();
	~ButtonCommandSet();

	// object instance
	static ButtonCommandSet* instance();

public:
	void addCommand(int nId, CommandBase* pFunc);

	CommandBase* getCommandById(int nId);
public:
	// command set
	std::map<int, CommandBase*> m_mapIdToCommands;
};

