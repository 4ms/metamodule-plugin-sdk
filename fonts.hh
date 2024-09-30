#pragma once
#include <string_view>

namespace MetaModule::Font {

	// Built-in fonts (firmware v1.3.0)
	// Feel free to use these variables or the string literal itself.
	// To use custom fonts, see README
	inline constexpr std::string_view Default = "Default_12";
	inline constexpr std::string_view DefaultTiny = "Default_10";			// MuseoSansRounded 500weight 10pt
	inline constexpr std::string_view DefaultSmall = "Default_12";			// MuseoSansRounded 500weight 12pt
	inline constexpr std::string_view DefaultMedium = "Default_14";			// MuseoSansRounded 700weight 14pt
	inline constexpr std::string_view Montserrat_10 = "Montserrat_10";		// Montserrat 10pt
	inline constexpr std::string_view Segment7_24 = "Segment7Standard24";	// 7-segment font 24pt
	inline constexpr std::string_view Segment_32 = "Segment32";			// Alphanumeric segment font 32pt
	inline constexpr std::string_view Segment14_10 = "Segment14_10";		// 14-segment font 10pt
	inline constexpr std::string_view Segment14_12 = "Segment14_12";		// 14-segment font 12pt
	inline constexpr std::string_view Segment14_14 = "Segment14_14";		// 14-segment font 14pt
	inline constexpr std::string_view Segment14_16 = "Segment14_16";		// 14-segment font 16pt
	inline constexpr std::string_view Segment14_20 = "Segment14_20";		// 14-segment font 20pt
	inline constexpr std::string_view Segment14_24 = "Segment14_24";		// 14-segment font 24pt
	inline constexpr std::string_view Segment14_26 = "Segment14_26";		// 14-segment font 26pt

}
