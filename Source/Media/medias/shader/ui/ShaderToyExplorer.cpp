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
	maxResults("Max Results", "", 15, 1, 100)
{

	search.addParameterListener(this);
	pageCount.addParameterListener(this);
	maxResults.addParameterListener(this);

	searchUI.reset(search.createStringParameterUI());
	pageCountUI.reset(pageCount.createStepper());
	maxResultsUI.reset(maxResults.createStepper());

	searchUI->useCustomBGColor = true;
	searchUI->customBGColor = Colours::black.brighter(.2f);
	searchUI->updateUIParams();

	addAndMakeVisible(searchUI.get());
	addAndMakeVisible(pageCountUI.get());
	addAndMakeVisible(maxResultsUI.get());
	addAndMakeVisible(viewport);
	viewport.setViewedComponent(&itemsComp, false);
	addAndMakeVisible(&viewport);

	repaint();

	//startThread();
}

ShaderToyExplorer::~ShaderToyExplorer()
{
	stopThread(1000);
	items.clear();
}

void ShaderToyExplorer::resized()
{
	Rectangle<int> r = getLocalBounds();

	Rectangle<int> hr = r.removeFromTop(30);

	searchUI->setBounds(hr.removeFromLeft(200).reduced(2));
	pageCountUI->setBounds(hr.removeFromLeft(200).reduced(2));
	maxResultsUI->setBounds(hr.removeFromLeft(200).reduced(2));

	r.removeFromTop(10);
	r.reduce(10, 10);

	//draw grid with 5 elements per row, keeping ratio of 16/9
	viewport.setBounds(r);

	r.removeFromRight(20);

	int num = items.size();
	int numPerRow = 5;
	int itemWidth = r.getWidth() / numPerRow;
	int itemHeight = itemWidth * 9 / 16;

	Rectangle<int> itemBounds = Rectangle<int>(itemWidth, itemHeight);
	for (int i = 0; i < num; i++)
	{
		int row = i / numPerRow;
		int col = i % numPerRow;

		items[i]->setBounds(itemBounds.withPosition(col * itemWidth, row * itemHeight).reduced(2));
	}

	if (!items.isEmpty()) itemsComp.setBounds(viewport.getLocalBounds().withBottom(items[num - 1]->getBottom() + 2));
	else itemsComp.setBounds(viewport.getLocalBounds());
}

void ShaderToyExplorer::updateItems()
{
	for (auto& i : items) removeChildComponent(i);
	items.clear();

	var results = itemsData["Results"];
	for (int i = 0; i < results.size(); i++)
	{
		ShaderToyItem* item = new ShaderToyItem(results[i].toString());
		itemsComp.addAndMakeVisible(item);
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

	if (threadShouldExit()) return;

	if (s.isEmpty()) return;

	var result = JSON::parse(s);
	if (result.isObject())
	{
		itemsData = result;

		if (threadShouldExit()) return;

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
	repaint();
	startThread();
}

ShaderToyItem::~ShaderToyItem()
{
	stopThread(1000);
}

void ShaderToyItem::paint(Graphics& g)
{
	g.setColour(PANEL_COLOR);
	g.fillRoundedRectangle(getLocalBounds().toFloat(), 5.f);

	g.setColour(Colours::white);
	if (previewImage.isValid()) g.drawImage(previewImage, getLocalBounds().toFloat().reduced(5.f));
	g.drawText(name, getLocalBounds(), Justification::centred);
}

void ShaderToyItem::mouseDrag(const MouseEvent& e)
{
	if (e.getDistanceFromDragStart() > 10 && !isDragAndDropActive())
	{
		var data(new DynamicObject());
		data.getDynamicObject()->setProperty("type", "ShaderToyItem");
		data.getDynamicObject()->setProperty("id", id);
		startDragging(data, this, ScaledImage(), true);
	}
}

void ShaderToyItem::run()
{
	//load preview image and infos from id 

	URL url = "https://www.shadertoy.com/api/v1/shaders/" + id + "?key=Bd8jRr";

	LOG("Loading " + url.toString(true));

	String dataStr = url.readEntireTextStream();
	if (threadShouldExit()) return;

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

	if (threadShouldExit()) return;

	MessageManager::callAsync([this]() {
		repaint();
		});
}
