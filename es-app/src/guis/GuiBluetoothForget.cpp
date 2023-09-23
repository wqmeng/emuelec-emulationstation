#include "guis/GuiBackup.h"
#include "guis/GuiMsgBox.h"
#include "Window.h"
#include <string>
#include "Log.h"
#include "Settings.h"
#include "ApiSystem.h"
#include "LocaleES.h"
#include "GuiBluetoothForget.h"
#include "guis/GuiTextEditPopup.h"
#include "guis/GuiTextEditPopupKeyboard.h"
#include "GuiLoading.h"

GuiBluetoothForget::GuiBluetoothForget(Window* window)
	: GuiComponent(window), mMenu(window, _("FORGET A BLUETOOTH DEVICE").c_str())
{
	mWaitingLoad = false;

	auto theme = ThemeData::getMenuTheme();

	addChild(&mMenu);

	if (load())		
		mMenu.addButton(_("REMOVE ALL"), "manual input", [&] { onRemoveAll(); });

	mMenu.addButton(_("BACK"), "back", [&] { delete this; });

	if (Renderer::isSmallScreen())
		mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, (Renderer::getScreenHeight() - mMenu.getSize().y()) / 2);
	else
		mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, Renderer::getScreenHeight() * 0.15f);
}

bool GuiBluetoothForget::load()
{
	std::vector<std::string> ssids = ApiSystem::getInstance()->getPairedBluetoothDeviceList();

	mMenu.clear();

	if (ssids.size() == 0)
		mMenu.addEntry(_("NO BLUETOOTH DEVICES FOUND"), false);
	else
	{
		for (auto ssid : ssids)
		{
			if (ssid.empty())
				continue;

			if (Utils::String::startsWith(ssid, "<device "))
			{
				auto id = Utils::String::extractString(ssid, "id=\"", "\"", false);
				auto name = Utils::String::extractString(ssid, "name=\"", "\"", false);
				auto type = Utils::String::extractString(ssid, "type=\"", "\"", false);

				std::string icon = type.empty() ? "unknown" : type;
				mMenu.addWithDescription(name, id, nullptr, [this, id, name]() { GuiBluetoothForget::onRemoveDevice(id, name); }, icon);
			}
			else
				mMenu.addEntry(ssid, false, [this, ssid]() { GuiBluetoothForget::onRemoveDevice(ssid, ""); });
		}
	}

	mMenu.updateSize();

	if (Renderer::isSmallScreen())
		mMenu.setPosition((Renderer::getScreenWidth() - mMenu.getSize().x()) / 2, (Renderer::getScreenHeight() - mMenu.getSize().y()) / 2);	

	return ssids.size() > 0;
}

bool GuiBluetoothForget::input(InputConfig* config, Input input)
{
	if (GuiComponent::input(config, input))
		return true;

	if (input.value != 0 && config->isMappedTo(BUTTON_BACK, input))
	{
		if (!mWaitingLoad)
			delete this;

		return true;
	}

	return false;
}

std::vector<HelpPrompt> GuiBluetoothForget::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts = mMenu.getHelpPrompts();
	prompts.push_back(HelpPrompt(BUTTON_BACK, _("BACK")));
	prompts.push_back(HelpPrompt(BUTTON_OK, _("REMOVE")));
	return prompts;
}

void GuiBluetoothForget::onRemoveAll()
{
	if (mWaitingLoad)
		return;

	Window* window = mWindow;

	mWindow->pushGui(new GuiLoading<bool>(mWindow, _("PLEASE WAIT"),
		[this, window](auto gui)
		{
			mWaitingLoad = true;
#if WIN32 && _DEBUG
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));
#endif
			return ApiSystem::getInstance()->forgetBluetoothControllers();
		},
		[this, window](bool ret)
		{
			mWaitingLoad = false;
			mWindow->pushGui(new GuiMsgBox(mWindow, _("BLUETOOTH DEVICES HAVE BEEN DELETED."), _("OK")));
			delete this;
		}));	
}

void GuiBluetoothForget::onRemoveDevice(const std::string& id, const std::string& name)
{
	if (mWaitingLoad)
		return;

	std::string deviceName = name;
	std::string macAddress = id;

	auto idx = macAddress.find(" ");
	if (idx != std::string::npos)
	{
		deviceName = macAddress.substr(idx + 1);
		macAddress = macAddress.substr(0, idx);
	}

	if (deviceName.empty())
		deviceName = macAddress;

	Window* window = mWindow;
	window->pushGui(new GuiMsgBox(window, Utils::String::format(_("ARE YOU SURE YOU WANT TO REMOVE '%s' ?").c_str(), deviceName.c_str()),
		_("YES"), [this, window, macAddress]
		{ 			
			window->pushGui(new GuiLoading<bool>(window, _("PLEASE WAIT"),
				[this, window, macAddress](auto gui)
				{
					mWaitingLoad = true;
					return ApiSystem::getInstance()->removeBluetoothDevice(macAddress);
				},
				[this, window](bool ret)
				{
					mWaitingLoad = false;
					load();
				}));
		},
		_("NO"), nullptr));
}
