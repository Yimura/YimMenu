#include "api_service.hpp"

#include "http_client/http_client.hpp"
#include "pointers.hpp"
#include "services/creator_storage/creator_storage_service.hpp"


namespace big
{
	std::string ms_token_str = "";
	api_service::api_service()
	{
		g_api_service = this;
	}

	api_service::~api_service()
	{
		g_api_service = nullptr;
	}

	std::string api_service::get_translation_from_Deeplx(std::string message, std::string tar_lang)
	{
		std::string url = g.session.DeepLx_url;
		const auto response = g_http_client.post(url,
		    {{"Authorization", ""}, {"X-Requested-With", "XMLHttpRequest"}, {"Content-Type", "application/json"}}, std::format(R"({{"text":"{}", "source_lang":"", "target_lang": "{}"}})", message, tar_lang));
		if (response.status_code == 200)
		{
			try
			{
				nlohmann::json obj = nlohmann::json::parse(response.text);

				std::string result = obj["data"];
				std::string sourcelang = obj["source_lang"];
				if (sourcelang == g.session.DeepL_target_lang && g.session.hideduplicate)
					return "None";
				return result;
			}

			catch (std::exception& e)
			{
				LOG(WARNING)<< "[ChatTranslation]Error while reading json: " << e.what();
				return "Error";
			}
		}
		else
		{
			LOG(WARNING)<< "[ChatTranslation]http code eror: " << response.status_code;
			return "Error";
		}

		return "Error";
	}

	std::string api_service::get_translation_from_Bing(std::string message, std::string tar_lang)
	{
		const std::string bing_auth_url = "https://edge.microsoft.com/translate/auth";
		cpr::Response auth_response;
		if (ms_token_str == "")
		{
			auth_response = g_http_client.get(bing_auth_url, {{"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/123.0.0.0 Safari/537.36 Edg/123.0.0.0"}});
			ms_token_str = auth_response.text;
		}

		const std::string url = std::format("https://api-edge.cognitive.microsofttranslator.com/translate?to={}&api-version=3.0&includeSentenceLength=true", tar_lang);

		auto response = g_http_client.post(url,
		    {
		        {"accept", "*/*"},
		        {"accept-language", "zh-TW,zh;q=0.9,ja;q=0.8,zh-CN;q=0.7,en-US;q=0.6,en;q=0.5"},
		        {"authorization", "Bearer " + ms_token_str},
		        {"content-type", "application/json"},
		        {"Referer", "https://www.7-zip.org/"},
		        {"Referrer-Policy", "strict-origin-when-cross-origin"},
		        {"User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/123.0.0.0 Safari/537.36 Edg/123.0.0.0"},
		    },
		    {std::format(R"([{{"Text":"{}"}}])", message)});
		if (response.status_code == 200)
		{
			nlohmann::json result = nlohmann::json::parse(response.text);
			if (result[0]["translations"].is_array())
			{
				std::string source_lang = result[0]["detectedLanguage"]["language"].get<std::string>();
				if (source_lang == g.session.Bing_target_lang && g.session.hideduplicate)
					return "None";
				return result[0]["translations"][0]["text"].get<std::string>();
			}
			else
			{
				LOG(WARNING) << "[ChatTranslation]Error while reading json: " << response.text;
				return "Error";
			}
		}
		else
		{
			LOG(WARNING) << "json data"<< response.text;
			LOG(WARNING) << "[ChatTranslation]http code eror: " << response.status_code;
			return "Error";
		}
	}

	std::string api_service::get_translation_from_Google(std::string message, std::string tar_lang)
	{
		const std::string url = std::format("https://translate.google.com/translate_a/single?dt=t&client=gtx&sl=auto&q={}&tl={}",
		    message,tar_lang);
		std::string encoded_url;
		for (char c : url)
		{
			if (c == ' ')
			{
				encoded_url += '+';
			}
			else
			{
				encoded_url += c;
			}
		}
		const auto response = g_http_client.get(encoded_url, {{"content-type", "application/json"}}, {});
		if (response.status_code == 200)
		{
			try
			{
				nlohmann::json obj = nlohmann::json::parse(response.text);

				std::string result = obj[0][0][0];
				auto& array = obj.back().back();
				if (array[0] == g.session.Google_target_lang && g.session.hideduplicate)
					return "None";
				return result;

			}

			catch (std::exception& e)
			{
				LOG(WARNING) << "[ChatTranslation]Error while reading json: " << e.what();
				return "Error";
			}
		}
		else
		{
			LOG(WARNING)<< "json data"<< response.text;
			LOG(WARNING) << "[ChatTranslation]http code eror: " << response.status_code;
			return "Error";
		}
	}

	std::string api_service::get_translation_from_OpenAI(std::string message, std::string tar_lang)
	{
		std::string url         = g.session.OpenAI_endpoint + "v1/chat/completions";
		std::string body        = std::format(R"(
        {{
            "model": "{}",
            "messages": [
                {{"role": "system", "content": "You are a professional translation engine, please translate the text into a colloquial, professional, elegant and fluent content, without the style of machine translation. You must only translate the text content, never interpret it."}},
                {{"role": "user", "content": "Translate into {}: {}"}}
            ]
        }}
    )",g.session.OpenAI_model,g.session.OpenAI_target_lang,
            message);
		LOG(INFO) << "url " << url;
		const auto response = g_http_client.post(url, {{"Authorization", "Bearer " + g.session.OpenAI_key}, {"Content-Type", "application/json"}}, {body});
		if (response.status_code == 200)
		{
			try
			{
				nlohmann::json obj = nlohmann::json::parse(response.text);

				std::string result = obj["choices"][0]["message"]["content"];
				return result;
			}

			catch (std::exception& e)
			{
				LOG(WARNING) << "[ChatTranslation]Error while reading json: " << e.what();
				return "Error";
			}
		}
		else
		{
			LOG(WARNING) << "json data" << response.text;
			LOG(WARNING) << "[ChatTranslation]http code eror: " << response.status_code;
			return "Error";
		}
	}

	bool api_service::get_rid_from_username(std::string_view username, uint64_t& result)
	{
		const auto response = g_http_client.post("https://scui.rockstargames.com/api/friend/accountsearch", {{"Authorization", AUTHORIZATION_TICKET}, {"X-Requested-With", "XMLHttpRequest"}}, {std::format("searchNickname={}", username)});
		if (response.status_code == 200)
		{
			try
			{
				nlohmann::json obj = nlohmann::json::parse(response.text);

				if (obj["Total"] > 0 && username.compare(obj["Accounts"].at(0)["Nickname"]) == 0)
				{
					result = obj["Accounts"].at(0)["RockstarId"];
					return true;
				}
			}
			catch (std::exception& e)
			{
				return false;
			}
		}

		return false;
	}

	bool api_service::get_username_from_rid(uint64_t rid, std::string& result)
	{
		const auto response = g_http_client.post("https://scui.rockstargames.com/api/friend/getprofile", {{"Authorization", AUTHORIZATION_TICKET}, {"X-Requested-With", "XMLHttpRequest"}, {"Content-Type", "application/json"}}, std::format(R"({{"RockstarId":"{}"}})", rid));
		if (response.status_code == 200)
		{
			try
			{
				nlohmann::json obj = nlohmann::json::parse(response.text);
				result             = obj["Accounts"].at(0)["RockstarAccount"]["Name"];
				return true;
			}
			catch (std::exception& e)
			{
				return false;
			}
		}

		return false;
	}

	// Ratelimit: 10 per Minute, if exceeded than 5 min cooldown
	bool api_service::send_socialclub_message(uint64_t rid, std::string_view message)
	{
		const auto response = g_http_client.post("https://scui.rockstargames.com/api/messaging/sendmessage", {{"Authorization", AUTHORIZATION_TICKET}, {"X-Requested-With", "XMLHttpRequest"}, {"Content-Type", "application/json"}}, {std::format(R"({{"env":"prod","title":"gta5","version":11,"recipientRockstarId":"{}","messageText":"{}"}})", rid, message)});

		return response.status_code == 200;
	}

	bool api_service::get_job_details(std::string_view content_id, nlohmann::json& result)
	{
		const auto response = g_http_client.get("https://scapi.rockstargames.com/ugc/mission/details",
		    {{"X-AMC", "true"}, {"X-Requested-With", "XMLHttpRequest"}},
		    {{"title", "gtav"}, {"contentId", content_id.data()}});

		if (response.status_code != 200)
			return false;

		try
		{
			result = nlohmann::json::parse(response.text);
			return true;
		}
		catch (std::exception& e)
		{
			return false;
		}
	}

	bool api_service::download_job_metadata(std::string_view content_id, int f1, int f0, int lang)
	{
		const auto response = g_http_client.get(std::format("https://prod.cloud.rockstargames.com/ugc/gta5mission/{}/{}_{}_{}.json",
		    content_id,
		    f1,
		    f0,
		    languages.at(lang)));

		if (response.status_code == 200)
		{
			const auto of = creator_storage_service::create_file(std::string(content_id) + ".json");

			return g_http_client.download(response.url, of);
		}

		return false;
	}
}