/*
  ==============================================================================

	ShaderToyExplorer.cpp
	Created: 24 Nov 2023 2:32:38pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "ShaderToyExplorer.h"

ShaderToyExplorer::ShaderToyExplorer(const String& name) :
	ShapeShifterContentComponent(name),
	Thread("ShaderToy Explorer"),
	search("Search", "", ""),
	pageCount("Page Count", "", 1, 1, 100),
	maxResults("Max Results", "", 10, 1, 100)
{

	search.addParameterListener(this);
	pageCount.addParameterListener(this);
	maxResults.addParameterListener(this);

	searchUI.reset(search.createStringParameterUI());
	pageCountUI.reset(pageCount.createStepper());
	maxResultsUI.reset(maxResults.createStepper());

	addAndMakeVisible(searchUI.get());
	addAndMakeVisible(pageCountUI.get());
	addAndMakeVisible(maxResultsUI.get());

	startThread();
}

ShaderToyExplorer::~ShaderToyExplorer()
{
}

void ShaderToyExplorer::resized()
{
	Rectangle<int> r = getLocalBounds();

	Rectangle<int> hr = r.removeFromTop(30);

	searchUI->setBounds(hr.removeFromLeft(200).reduced(2));
	pageCountUI->setBounds(hr.removeFromLeft(100).reduced(2));
	maxResultsUI->setBounds(hr.removeFromLeft(100).reduced(2));

	r.removeFromTop(10);
	r.reduce(10, 10);

	//draw grid of items
	Point<int> itemSize = Point<int>(200, 140);
	int itemMargin = 10;
	int itemsPerRow = r.getWidth() / (itemSize.x + itemMargin);

	int x = 0;
	int y = 0;

	for (auto& item : items)
	{
		item->setBounds(r.getX() + x * (itemSize.x + itemMargin), r.getY() + y * (itemSize.y + itemMargin), itemSize.x, itemSize.y);

		x++;
		if (x >= itemsPerRow)
		{
			x = 0;
			y++;
		}
	}
}

void ShaderToyExplorer::updateItems()
{
	for (auto& i : items) removeChildComponent(i);
	items.clear();

	LOG(JSON::toString(itemsData, true));

	var results = itemsData["Results"];
	for (int i = 0; i < results.size(); i++)
	{
		ShaderToyItem* item = new ShaderToyItem(results[i].toString());
		addAndMakeVisible(item);
		items.add(item);
	}

	resized();
}

void ShaderToyExplorer::run()
{
	int from = (pageCount.intValue() - 1) * maxResults.intValue();
	int num = maxResults.intValue();
	String url = "https://www.shadertoy.com/api/v1/shaders/query/" + search.stringValue() + "?key=Bd8jRr&from=" + String(from) + "&num=" + String(num);

	LOG("Loading " + url);

	String s = URL(url).readEntireTextStream();

	if (s.isEmpty()) return;

	var result = JSON::parse(s);
	if (result.isObject())
	{
		itemsData = result;

		MessageManager::callAsync([this]() {
			updateItems();
			});

	}
}



void ShaderToyExplorer::parameterValueChanged(Parameter* p)
{
	if (p == &search || p == &pageCount || p == &maxResults)
	{
		startThread();
	}
}

ShaderToyItem::ShaderToyItem(const String& id) :
	Thread("ShaderToy Item"),
	id(id)
{
	startThread();
}

ShaderToyItem::~ShaderToyItem()
{
}

void ShaderToyItem::paint(Graphics& g)
{
	g.setColour(PANEL_COLOR);
	g.fillRoundedRectangle(getLocalBounds().toFloat(), 5.f);

	g.setColour(Colours::white);
	if (previewImage.isValid()) g.drawImage(previewImage, getLocalBounds().toFloat().reduced(5.f));
	g.drawText(name, getLocalBounds(), Justification::centred);
}

void ShaderToyItem::resized()
{
}

void ShaderToyItem::run()
{
	//load preview image and infos from id 

	URL url = "https://www.shadertoy.com/api/v1/shaders/" + id + "?key=Bd8jRr";

	LOG("Loading " + url.toString(true));

	String dataStr = url.readEntireTextStream();

	data = JSON::parse(dataStr);

	if (data.isObject())
	{
		name = data["Shader"]["info"]["name"].toString();
		description = data["Shader"]["info"]["description"].toString();
		bool usePreview = data["Shader"]["info"]["usePreview"];

		if (usePreview)
		{

			URL url = "https://www.shadertoy.com/media/shaders/" + id + ".jpg";
			LOG(url.toString(true));

			std::unique_ptr<InputStream> is = url.createInputStream(URL::InputStreamOptions(URL::ParameterHandling::inAddress));

			JPEGImageFormat jpg;
			previewImage = jpg.decodeImage(*is);
			MessageManager::callAsync([this]() { repaint(); });
		}

	}

	MessageManager::callAsync([this]() {
		resized();
		});
}
