#pragma once
#include "gta/enums.hpp"

namespace big
{
	const std::map<eGameLanguage, const std::string_view> languages = {
	    {eGameLanguage::ENGLISH, "English"},
	    {eGameLanguage::FRENCH, "French"},
	    {eGameLanguage::GERMAN, "German"},
	    {eGameLanguage::ITALIAN, "Italian"},
	    {eGameLanguage::SPANISH, "Spanish (Spain)"},
	    {eGameLanguage::BRAZILIAN_PORTUGUESE, "Portuguese (Brazil)"},
	    {eGameLanguage::POLISH, "Polish"},
	    {eGameLanguage::RUSSIAN, "Russian"},
	    {eGameLanguage::KOREAN, "Korean"},
	    {eGameLanguage::TRADITIONAL_CHINESE, "Chinese (Traditional)"},
	    {eGameLanguage::JAPANESE, "Japanese"},
	    {eGameLanguage::MEXICAN_SPANISH, "Spanish (Mexico)"},
	    {eGameLanguage::SIMPLIFIED_CHINESE, "Chinese (Simplified)"},
	};
}
