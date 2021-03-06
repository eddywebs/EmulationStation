#include "components/ButtonComponent.h"
#include "WindowThemeData.h"
#include "Renderer.h"
#include "Window.h"
#include "Util.h"
#include "Log.h"

ButtonComponent::ButtonComponent(Window* window, const std::string& text, const std::string& helpText, const std::function<void()>& func) : GuiComponent(window),
	mBox(window, ":/button.png"),
	mFont(Font::get(FONT_SIZE_MEDIUM)), 
	mFocused(false), 
	mEnabled(true), 
	mTextColorFocused(0xFFFFFFFF), mTextColorUnfocused(0x777777FF)
{
	setPressedFunc(func);
	setText(text, helpText);
	updateImage();

	auto wTheme = WindowThemeData::getInstance()->getCurrentTheme();
	mTextColorFocused = wTheme->button.color_focused;
	mTextColorUnfocused = wTheme->button.color;
}

void ButtonComponent::onSizeChanged()
{
	mBox.fitTo(mSize, Eigen::Vector3f::Zero(), Eigen::Vector2f(-32, -32));
}

void ButtonComponent::setPressedFunc(std::function<void()> f)
{
	mPressedFunc = f;
}

bool ButtonComponent::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("a", input) && input.value != 0)
	{
		if(mPressedFunc && mEnabled)
			mPressedFunc();
		return true;
	}

	return GuiComponent::input(config, input);
}

void ButtonComponent::setText(const std::string& text, const std::string& helpText)
{
	mText = strToUpper(text);
	mHelpText = helpText;
	
	mTextCache = std::unique_ptr<TextCache>(mFont->buildTextCache(mText, 0, 0, getCurTextColor()));

	float minWidth = mFont->sizeText("DELETE").x() + 12;
	setSize(std::max(mTextCache->metrics.size.x() + 12, minWidth), mTextCache->metrics.size.y());

	updateHelpPrompts();
}

void ButtonComponent::onFocusGained()
{
	mFocused = true;
	updateImage();
}

void ButtonComponent::onFocusLost()
{
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
	auto wTheme = WindowThemeData::getInstance()->getCurrentTheme();
	std::string btnFilled = wTheme->button.path_focused;
	std::string btn = wTheme->button.path;

	if (btnFilled == "") btnFilled = ":/button_filled.png";
	if (btn == "") btn = ":/button.png";

	if(!mEnabled || !mPressedFunc)
	{
		mBox.setImagePath(btnFilled);
		mBox.setCenterColor(0x770000FF);
		mBox.setEdgeColor(0x770000FF);
		return;
	}

	mBox.setCenterColor(0xFFFFFFFF);
	mBox.setEdgeColor(0xFFFFFFFF);
	mBox.setImagePath(mFocused ? btnFilled : btn);
}

void ButtonComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = roundMatrix(parentTrans * getTransform());
	
	mBox.render(trans);

	if(mTextCache)
	{
		Eigen::Vector3f centerOffset((mSize.x() - mTextCache->metrics.size.x()) / 2, (mSize.y() - mTextCache->metrics.size.y()) / 2, 0);
		centerOffset = roundVector(centerOffset);
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
	if(!mFocused)
		return mTextColorUnfocused;
	else
		return mTextColorFocused;
}

std::vector<HelpPrompt> ButtonComponent::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("a", mHelpText.empty() ? mText.c_str() : mHelpText.c_str()));
	return prompts;
}
