#include "GamesDBScraper.h"
#include "../components/AsyncReqComponent.h"
#include "../Log.h"
#include "../pugiXML/pugixml.hpp"

std::vector<MetaDataList> GamesDBScraper::getResults(ScraperSearchParams params)
{
	std::shared_ptr<HttpReq> req = makeHttpReq(params);
	while(req->status() == HttpReq::REQ_IN_PROGRESS);

	return parseReq(params, req);
}

std::shared_ptr<HttpReq> GamesDBScraper::makeHttpReq(ScraperSearchParams params)
{
	std::string path = "/api/GetGame.php?";

	std::string cleanName = params.nameOverride;
	if(cleanName.empty())
		cleanName = params.game->getBaseName();
	
	path += "name=" + cleanName;
	//platform TODO, should use some params.system get method

	return std::make_shared<HttpReq>("thegamesdb.net", path);
}

std::vector<MetaDataList> GamesDBScraper::parseReq(ScraperSearchParams params, std::shared_ptr<HttpReq> req)
{
	std::vector<MetaDataList> mdl;

	if(req->status() != HttpReq::REQ_SUCCESS)
	{
		LOG(LogError) << "HttpReq error";
		return mdl;
	}

	pugi::xml_document doc;
	pugi::xml_parse_result parseResult = doc.load(req->getContent().c_str());
	if(!parseResult)
	{
		LOG(LogError) << "Error parsing XML";
		return mdl;
	}

	pugi::xml_node data = doc.child("Data");

	std::string baseImageUrl = data.child("baseImgUrl").text().get();
	
	unsigned int resultNum = 0;
	pugi::xml_node game = data.child("Game");
	while(game && resultNum < 5)
	{
		mdl.push_back(MetaDataList(params.system->getGameMDD()));
		mdl.back().set("name", game.child("GameTitle").text().get());
		mdl.back().set("desc", game.child("Overview").text().get());

		resultNum++;
		game = game.next_sibling("Game");
	}

	return mdl;
}

void GamesDBScraper::getResultsAsync(ScraperSearchParams params, Window* window, std::function<void(std::vector<MetaDataList>)> returnFunc)
{
	std::shared_ptr<HttpReq> httpreq = makeHttpReq(params);
	AsyncReqComponent* req = new AsyncReqComponent(window, httpreq,
		[this, params, returnFunc] (std::shared_ptr<HttpReq> r)
	{
		returnFunc(parseReq(params, r));
	}, [] ()
	{
	});

	window->pushGui(req);
}
