#include "SwitchComponent.h"
#include "WindowThemeData.h"
#include "Renderer.h"
#include "resources/Font.h"
#include "Window.h"

SwitchComponent::SwitchComponent(Window* window, bool state) : GuiComponent(window), mImage(window), mState(state)
{
	auto wTheme = WindowThemeData::getInstance()->getCurrentTheme();
	mImage.setImage(wTheme->Switch.path);
	mImage.setColorShift(wTheme->Switch.color);
	mImage.setResize(0, Font::get(FONT_SIZE_MEDIUM)->getLetterHeight());
	mSize = mImage.getSize();
}

void SwitchComponent::onSizeChanged()
{
	mImage.setSize(mSize);
}

bool SwitchComponent::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("a", input) && input.value)
	{
		mState = !mState;
		onStateChanged();
		return true;
	}

	return false;
}

void SwitchComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = parentTrans * getTransform();
	
	mImage.render(trans);

	renderChildren(trans);
}

bool SwitchComponent::getState() const
{
	return mState;
}

void SwitchComponent::setState(bool state)
{
	mState = state;
	onStateChanged();
}

std::string SwitchComponent::getValue() const
{
	return mState ?  "true" : "false";
}

void SwitchComponent::setValue(const std::string& statestring)
{
	if (statestring == "true")
	{
		mState = true;
	}else
	{
		mState = false;
	}
	onStateChanged();
}

void SwitchComponent::onStateChanged()
{
	auto wTheme = WindowThemeData::getInstance()->getCurrentTheme();
	mImage.setImage(mState ? wTheme->Switch.path_selected : wTheme->Switch.path);
}

std::vector<HelpPrompt> SwitchComponent::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("a", "change"));
	return prompts;
}
