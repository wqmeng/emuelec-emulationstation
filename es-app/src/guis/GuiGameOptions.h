#pragma once
#ifndef ES_APP_GUIS_GUI_GAME_OPTIONS_H
#define ES_APP_GUIS_GUI_GAME_OPTIONS_H

#include "components/MenuComponent.h"
#include "components/OptionListComponent.h"
#include "FileData.h"
#include "GuiComponent.h"

class IGameListView;
class SystemData;

class GuiGameOptions : public GuiComponent
{
public:
	GuiGameOptions(Window* window, FileData* game);
	virtual ~GuiGameOptions();

	virtual bool input(InputConfig* config, Input input) override;
	virtual std::vector<HelpPrompt> getHelpPrompts() override;
	virtual HelpStyle getHelpStyle() override;

	static std::vector<std::string> gridSizes;

	void close();

	virtual bool hitTest(int x, int y, Transform4x4f& parentTransform, std::vector<GuiComponent*>* pResult = nullptr) override;
	virtual bool onMouseClick(int button, bool pressed, int x, int y);

private:
	static void deleteGame(FileData* file);

#ifdef _ENABLEEMUELEC
	static void hideGame(FileData* file, bool hide);
	static void createMultidisc(FileData* file);
#endif

	inline void addSaveFunc(const std::function<void()>& func) { mSaveFuncs.push_back(func); };		
	void openMetaDataEd();

	std::string getCustomCollectionName();
	void deleteCollection();

	MenuComponent mMenu;

	FileData* mGame;
	SystemData* mSystem;
	IGameListView* getGamelist();

	bool mHasAdvancedGameOptions;


	std::vector<std::function<void()>> mSaveFuncs;
	bool mReloadAll;	
};

#endif // ES_APP_GUIS_GUI_GAME_OPTIONS_H
