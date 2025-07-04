#pragma once
#include <ui/RadioButton.hpp>

namespace rack::ui
{

struct OptionButton : RadioButton {
	void draw(const DrawArgs &args) override;
};

} // namespace rack::ui
