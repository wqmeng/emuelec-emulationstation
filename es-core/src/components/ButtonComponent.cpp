#include "components/ButtonComponent.h"
#include "resources/Font.h"
#include "utils/StringUtil.h"
#include "LocaleES.h"
#include "TextToSpeech.h"
#include "Settings.h"
#include "Window.h"
#include "utils/HtmlColor.h"

#define TEXT_PADDING Math::max(12, Renderer::getScreenWidth() * 0.014)

ButtonComponent::ButtonComponent(Window* window, const std::string& text, const std::string& helpText, const std::function<void()>& func, bool upperCase) : GuiComponent(window),
	mBox(window, ThemeData::getMenuTheme()->Button.path),	
	mFocused(false), 
	mEnabled(true),
	mMousePressed(false)	
{
	auto menuTheme = ThemeData::getMenuTheme();

	mFont = menuTheme->Text.font;
	mTextColorUnfocused = menuTheme->Text.color;
	mTextColorFocused = menuTheme->Text.selectedColor;
	mColor = menuTheme->Text.color;
	mColorFocused = menuTheme->Text.selectorColor;
	mBox.setCornerSize(menuTheme->Button.cornerSize);
	mRenderNonFocusedBackground = true;

	mPressedFunc = func;
	setText(text, helpText, upperCase);
	updateImage();
}

void ButtonComponent::onSizeChanged()
{
	GuiComponent::onSizeChanged();

	auto sz = mBox.getCornerSize();

	mBox.fitTo(
		Vector2f(mSize.x() - mPadding.x() - mPadding.z(), mSize.y() - mPadding.y() - mPadding.w()), 
		Vector3f(mPadding.x(), mPadding.y()), 
		Vector2f(-sz.x() * 2, -sz.y() * 2));
}

void ButtonComponent::setPressedFunc(std::function<void()> f)
{
	mPressedFunc = f;
}

bool ButtonComponent::input(InputConfig* config, Input input)
{
	if (config->isMappedTo(BUTTON_OK, input) && input.value != 0)
	{
		if (mPressedFunc && mEnabled)
			mPressedFunc();

		return true;
	}

	return GuiComponent::input(config, input);
}

void ButtonComponent::onOpacityChanged()
{
	mBox.setOpacity(getOpacity());

	if (mTextCache)
		mTextCache->setColor(getCurTextColor());
}

void ButtonComponent::setText(const std::string& text, const std::string& helpText, bool upperCase)
{
	mText = upperCase ? Utils::String::toUpper(text) : text;
	mHelpText = helpText;
	
	mTextCache = std::unique_ptr<TextCache>(mFont->buildTextCache(mText, 0, 0, getCurTextColor()));

	float padding = TEXT_PADDING;
	float minWidth = mFont->sizeText("DELETE").x() + padding;
	setSize(Math::max(mTextCache->metrics.size.x() + padding, minWidth), mTextCache->metrics.size.y());

	updateHelpPrompts();
}

void ButtonComponent::onFocusGained()
{
	if (mFocused)
		return;

	mFocused = true;
	updateImage();

	TextToSpeech::getInstance()->say(mText, true);
}

void ButtonComponent::onFocusLost()
{
	if (!mFocused)
		return;

	mFocused = false;
	updateImage();
}

void ButtonComponent::setEnabled(bool enabled)
{
	mEnabled = enabled;
	updateImage();
}

void ButtonComponent::updateImage()
{
	if (!mEnabled || !mPressedFunc)
	{
		mBox.setImagePath(":/button_filled.png");
		mBox.setCenterColor(0x770000FF);
		mBox.setEdgeColor(0x770000FF);
		return;
	}

	if (mNewColor)
	{
		mBox.setImagePath(ThemeData::getMenuTheme()->Button.filledPath);
		mBox.setCenterColor(mModdedColor);
		mBox.setEdgeColor(mModdedColor);
		return;
	}

	mBox.setCenterColor(getCurBackColor());
	mBox.setEdgeColor(getCurBackColor());
	mBox.setImagePath(mFocused ? ThemeData::getMenuTheme()->Button.filledPath : ThemeData::getMenuTheme()->Button.path);
}

void ButtonComponent::render(const Transform4x4f& parentTrans)
{
	Transform4x4f trans = parentTrans * getTransform();

	if (mRenderNonFocusedBackground || mFocused)
		mBox.render(trans);
	else
	{
		Renderer::setMatrix(trans);
		Renderer::drawRect(mPadding.x(), mPadding.y(), mSize.x() - mPadding.x() - mPadding.z(), mSize.y() - mPadding.y() - mPadding.w(), 0x60606025);
	}

	if (mMousePressed && mIsMouseOver)
	{
		Renderer::setMatrix(trans);
		Renderer::drawRect(0.0f, 0.0f, mSize.x(), mSize.y(), 0x00000010, 0x00000010);
	}
	else if (mIsMouseOver)
	{
		Renderer::setMatrix(trans);
		Renderer::drawRect(0.0f, 0.0f, mSize.x(), mSize.y(), 0xFFFFFF10, 0xFFFFFF10);
	}
	else if (Settings::DebugMouse() && mIsMouseOver)
	{
		Renderer::setMatrix(trans);
		Renderer::drawRect(0.0f, 0.0f, mSize.x(), mSize.y(), 0xFF000033, 0xFF000033);
	}

	if(mTextCache)
	{
		Vector3f centerOffset((mSize.x() - mTextCache->metrics.size.x()) / 2, (mSize.y() - mTextCache->metrics.size.y()) / 2, 0);
		trans = trans.translate(centerOffset);

		Renderer::setMatrix(trans);
		mTextCache->setColor(getCurTextColor());
		mFont->renderTextCache(mTextCache.get());
		trans = trans.translate(-centerOffset);
	}

	renderChildren(trans);
}

unsigned int ButtonComponent::getCurTextColor() const
{
	return Utils::HtmlColor::applyColorOpacity(mFocused ? mTextColorFocused : mTextColorUnfocused, getOpacity());
}

unsigned int ButtonComponent::getCurBackColor() const
{
	return Utils::HtmlColor::applyColorOpacity(mFocused ? mColorFocused : mColor, getOpacity());
}

std::vector<HelpPrompt> ButtonComponent::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt(BUTTON_OK, mHelpText.empty() ? mText.c_str() : mHelpText.c_str()));
	return prompts;
}

void ButtonComponent::onPaddingChanged()
{
	onSizeChanged();
}

bool ButtonComponent::onMouseClick(int button, bool pressed, int x, int y)
{
	if (button == 1)
	{
		if (pressed)
		{
			mMousePressed = true;
			mWindow->setMouseCapture(this);
		}
		else if (mMousePressed)
		{
			mWindow->releaseMouseCapture();

			mMousePressed = false;

			if (mIsMouseOver)
			{
				if (mPressedFunc && mEnabled)
					mPressedFunc();
			}
		}

		return true;
	}	

	return false;
}

