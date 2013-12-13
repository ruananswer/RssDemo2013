#ifndef _COMMAND_MANAGER_H_
#define _COMMAND_MANAGER_H_

#include <MyGUI.h>

namespace RssApp
{
	typedef MyGUI::delegates::CMultiDelegate2<const MyGUI::UString&, bool&> CommandDelegate;

	class CommandManager :	public MyGUI::Singleton<CommandManager>
	{
	public:
		CommandManager();
		virtual ~CommandManager();

		void initialise();
		void shutdown();

		void registerCommand(const MyGUI::UString& _command, CommandDelegate::IDelegate* _delegate);
		void unregisterCommand(const MyGUI::UString& _command, CommandDelegate::IDelegate* _delegate);

		bool executeCommand(const MyGUI::UString& _command);

		void setCommandData(const MyGUI::UString& _data);
		const MyGUI::UString& getCommandData() const;

	private:
		typedef std::map<MyGUI::UString, CommandDelegate> MapDelegate;
		MapDelegate mDelegates;
		MyGUI::UString mData;
	};
}

#endif