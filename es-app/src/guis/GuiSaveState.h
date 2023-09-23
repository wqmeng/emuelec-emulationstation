#pragma once

#include <functional>

#include "GuiComponent.h"
#include "Window.h"
#include "components/ImageGridComponent.h"
#include "components/NinePatchComponent.h"
#include "components/ComponentGrid.h"
#include "components/TextComponent.h"
#include "SaveState.h"

#ifdef _ENABLEEMUELEC
	#include "CloudSaves.h"
#endif

class ThemeData;
class FileData;
class SaveStateRepository;

class GuiSaveState : public GuiComponent
{
public:
	GuiSaveState(Window* window, FileData* game, const std::function<void(const SaveState& state)>& callback);

	bool input(InputConfig* config, Input input) override;
	void onSizeChanged() override;
	std::vector<HelpPrompt> getHelpPrompts() override;

	bool hitTest(int x, int y, Transform4x4f& parentTransform, std::vector<GuiComponent*>* pResult = nullptr) override;
	bool onMouseClick(int button, bool pressed, int x, int y);

#ifdef _ENABLEEMUELEC
	void loadGridAndCenter() {
		loadGrid();
		centerWindow();
	};
#endif

protected:
	void centerWindow();
	void loadGrid();

	std::shared_ptr<ImageGridComponent<SaveState>> mGrid;
	std::shared_ptr<ThemeData> mTheme;
	std::shared_ptr<TextComponent>	mTitle;

	NinePatchComponent				mBackground;
	ComponentGrid					mLayout;
	
	std::function<void(const SaveState& state)>			mRunCallback;

	FileData* mGame;
	SaveStateRepository* mRepository;
};