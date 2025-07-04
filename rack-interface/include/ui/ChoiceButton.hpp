#pragma once
#include <ui/Button.hpp>
#include <ui/common.hpp>

namespace rack
{
namespace ui
{

/** Button with a dropdown icon on its right.
*/
struct ChoiceButton : Button {
	void draw(const DrawArgs &args) override;
};

} // namespace ui
} // namespace rack
