#include "components/SliderComponent.h"
#include "WindowThemeData.h"
#include <assert.h>
#include "Renderer.h"
#include "resources/Font.h"
#include "Log.h"
#include "Util.h"

#define MOVE_REPEAT_DELAY 500
#define MOVE_REPEAT_RATE 40

SliderComponent::SliderComponent(Window* window, float min, float max, float increment, const std::string& suffix) : GuiComponent(window),
	mMin(min), mMax(max), mSingleIncrement(increment), mMoveRate(0), mKnob(window), mSuffix(suffix)
{
	assert((min - max) != 0);

	// Get theme data
	auto wTheme = WindowThemeData::getInstance()->getCurrentTheme();

	// some sane default value
	mValue = (max + min) / 2;

	mKnob.setOrigin(0.5f, 0.5f);
	mKnob.setImage(wTheme->slider.path);
	mKnob.setColorShift(wTheme->slider.color);
	
	setSize(Renderer::getScreenWidth() * 0.15f, Font::get(FONT_SIZE_MEDIUM)->getLetterHeight());
}

bool SliderComponent::input(InputConfig* config, Input input)
{
	if(config->isMappedTo("left", input))
	{
		if(input.value)
			setValue(mValue - mSingleIncrement);

		mMoveRate = input.value ? -mSingleIncrement : 0;
		mMoveAccumulator = -MOVE_REPEAT_DELAY;
		return true;
	}
	if(config->isMappedTo("right", input))
	{
		if(input.value)
			setValue(mValue + mSingleIncrement);

		mMoveRate = input.value ? mSingleIncrement : 0;
		mMoveAccumulator = -MOVE_REPEAT_DELAY;
		return true;
	}

	return GuiComponent::input(config, input);
}

void SliderComponent::update(int deltaTime)
{
	if(mMoveRate != 0)
	{
		mMoveAccumulator += deltaTime;
		while(mMoveAccumulator >= MOVE_REPEAT_RATE)
		{
			setValue(mValue + mMoveRate);
			mMoveAccumulator -= MOVE_REPEAT_RATE;
		}
	}
	
	GuiComponent::update(deltaTime);
}

void SliderComponent::render(const Eigen::Affine3f& parentTrans)
{
	Eigen::Affine3f trans = roundMatrix(parentTrans * getTransform());
	Renderer::setMatrix(trans);

	// render suffix
	if(mValueCache)
		mFont->renderTextCache(mValueCache.get());

	float width = mSize.x() - mKnob.getSize().x() - (mValueCache ? mValueCache->metrics.size.x() + 4 : 0);

	//render line
	const float lineWidth = 2;
	Renderer::drawRect(mKnob.getSize().x() / 2, mSize.y() / 2 - lineWidth / 2, width, lineWidth, 
		WindowThemeData::getInstance()->getCurrentTheme()->default_text.color);

	//render knob
	mKnob.render(trans);
	
	GuiComponent::renderChildren(trans);
}

void SliderComponent::setValue(float value)
{
	mValue = value;
	if(mValue < mMin)
		mValue = mMin;
	else if(mValue > mMax)
		mValue = mMax;

	onValueChanged();
}

float SliderComponent::getValue()
{
	return mValue;
}

void SliderComponent::onSizeChanged()
{
	// Get theme data
	auto wTheme = WindowThemeData::get();
	float size = 1;
	std::string path;

	if (wTheme->default_text.path != "") {
		path = wTheme->default_text.path;
		size = wTheme->default_text.size;
	}

	if(!mSuffix.empty())
		mFont = Font::get((int)(mSize.y()) * size, path.empty() ? FONT_PATH_LIGHT : path);
	
	onValueChanged();
}

void SliderComponent::onValueChanged()
{
	// update suffix textcache
	if(mFont)
	{
		std::stringstream ss;
		ss << std::fixed;
		ss.precision(0);
		ss << mValue;
		ss << mSuffix;
		const std::string val = ss.str();

		ss.str("");
		ss.clear();
		ss << std::fixed;
		ss.precision(0);
		ss << mMax;
		ss << mSuffix;
		const std::string max = ss.str();

		Eigen::Vector2f textSize = mFont->sizeText(max);
		mValueCache = std::shared_ptr<TextCache>(mFont->buildTextCache(val, mSize.x() - textSize.x(), (mSize.y() - textSize.y()) / 2, 
			WindowThemeData::getInstance()->getCurrentTheme()->default_text.color));
		mValueCache->metrics.size[0] = textSize.x(); // fudge the width
	}

	// update knob position/size
	mKnob.setResize(0, mSize.y() * 0.7f);
	float lineLength = mSize.x() - mKnob.getSize().x() - (mValueCache ? mValueCache->metrics.size.x() + 4 : 0);
	mKnob.setPosition(((mValue + mMin) / mMax) * lineLength + mKnob.getSize().x()/2, mSize.y() / 2);
}

std::vector<HelpPrompt> SliderComponent::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("left/right", "change"));
	return prompts;
}
