/*
  ==============================================================================

	OnlineContentExplorer.cpp
	Created: 24 Nov 2023 2:32:38pm
	Author:  bkupe

  ==============================================================================
*/

#include "Media/MediaIncludes.h"
#include "OnlineContentExplorer.h"

OnlineContentExplorer::OnlineContentExplorer(const String& name) :
	ShapeShifterContentComponent(name),
	Thread("Online Explorer"),
	source("Source", ""),
	category("Category", ""),
	search("Search", "", ""),
	pageCount("Page Count", "", 1, 1, 100),
	maxResults("Max Results", "", 15, 1, 100)
{

	source.addOption("ShaderToy", ShaderToy)->addOption("ISF", ISF)->addOption("Pexels Photo", Pexels_Photo)->addOption("Pexels Video", Pexels_Video);
	category.addOption("All", "");

	source.addParameterListener(this);
	category.addParameterListener(this);
	search.addParameterListener(this);
	pageCount.addParameterListener(this);
	maxResults.addParameterListener(this);

	sourceUI.reset(source.createUI());
	categoryUI.reset(category.createUI());
	searchUI.reset(search.createStringParameterUI());
	pageCountUI.reset(pageCount.createStepper());
	maxResultsUI.reset(maxResults.createStepper());

	searchUI->useCustomBGColor = true;
	searchUI->customBGColor = Colours::black.brighter(.2f);
	searchUI->updateUIParams();

	addAndMakeVisible(sourceUI.get());
	addAndMakeVisible(categoryUI.get());
	addAndMakeVisible(searchUI.get());
	addAndMakeVisible(pageCountUI.get());
	addAndMakeVisible(maxResultsUI.get());
	addAndMakeVisible(viewport);
	viewport.setViewedComponent(&itemsComp, false);
	addAndMakeVisible(&viewport);

	linksLabel.setFont(16);
	linksLabel.setColour(linksLabel.textColourId, Colours::lightblue);
	linksLabel.setInterceptsMouseClicks(true, false);
	linksLabel.setJustificationType(Justification::centred);
	linksLabel.setMouseCursor(MouseCursor::PointingHandCursor);
	addAndMakeVisible(linksLabel);
	updateLinksLabel();

	linksLabel.addMouseListener(this, false);
	
	setInterceptsMouseClicks(true, true);

	repaint();
}

OnlineContentExplorer::~OnlineContentExplorer()
{
	stopThread(1000);
	items.clear();
}

void OnlineContentExplorer::paint(Graphics& g)
{
	ShapeShifterContentComponent::paint(g);
	g.setColour(BG_COLOR.brighter(.3f));
	g.fillRect(getLocalBounds().removeFromTop(24));

}

void OnlineContentExplorer::resized()
{
	Rectangle<int> r = getLocalBounds();

	Rectangle<int> hr = r.removeFromTop(24).reduced(0, 2);

	sourceUI->setBounds(hr.removeFromLeft(140).reduced(2));
	categoryUI->setBounds(hr.removeFromLeft(140).reduced(2));
	searchUI->setBounds(hr.removeFromLeft(200).reduced(2));
	pageCountUI->setBounds(hr.removeFromLeft(140).reduced(2));
	maxResultsUI->setBounds(hr.removeFromLeft(140).reduced(2));

	r.removeFromTop(10);
	r.reduce(10, 10);

	Rectangle<int> linksRect = r.removeFromBottom(20);
	linksLabel.setBounds(linksRect.reduced(2));
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

void OnlineContentExplorer::updateItems()
{
	for (auto& i : items) removeChildComponent(i);
	items.clear();

	OnlineSource os = source.getValueDataAsEnum<OnlineSource>();

	LOG(JSON::toString(itemsData, true));
	var results = getResultsArray(itemsData);
	for (int i = 0; i < results.size(); i++)
	{
		OnlineContentItem* item = new OnlineContentItem(os, results[i]);
		itemsComp.addAndMakeVisible(item);
		items.add(item);
	}

	resized();
}

var OnlineContentExplorer::getResultsArray(var data)
{
	OnlineSource os = source.getValueDataAsEnum<OnlineSource>();
	switch (os)
	{
	case ShaderToy: return data["Results"];
	case ISF: return data["shaders"];
	case Pexels_Photo: return data["photos"];
	case Pexels_Video: return data["videos"];
	}

	return var();
}

void OnlineContentExplorer::mouseDown(const MouseEvent& e)
{
	if (e.originalComponent == &linksLabel)
	{
		OnlineSource s = source.getValueDataAsEnum<OnlineSource>();
		switch (s)
		{
		case ShaderToy: URL("https://www.shadertoy.com/").launchInDefaultBrowser(); break;
		case ISF: URL("https://editor.isf.video/").launchInDefaultBrowser(); break;
		case Pexels_Photo:
		case Pexels_Video:
			URL("https://www.pexels.com/").launchInDefaultBrowser(); break;
		}
	}
}

void OnlineContentExplorer::updateLinksLabel()
{
	OnlineSource s = source.getValueDataAsEnum<OnlineSource>();
	switch (s)
	{
	case ShaderToy: linksLabel.setText("Sources from ShaderToy - https://www.shadertoy.com/", dontSendNotification); break;
	case ISF: linksLabel.setText("Sources from ISF Editor - https://editor.isf.video/", dontSendNotification); break;
	case Pexels_Photo:
	case Pexels_Video:
		linksLabel.setText("Sources from Pexels - https://www.pexels.com/", dontSendNotification); break;
	}
}

void OnlineContentExplorer::run()
{
	URL url = getSearchURL();

	OnlineSource os = source.getValueDataAsEnum<OnlineSource>();

	std::unique_ptr<InputStream> is = URL(url).createInputStream(URL::InputStreamOptions(URL::ParameterHandling::inAddress).withExtraHeaders(getExtraHeadersForSource(os)).withProgressCallback([this](int, int) { return !threadShouldExit();  }));

	if (threadShouldExit()) return;

	if (is == nullptr) return;

	String s = is->readEntireStreamAsString();

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

URL OnlineContentExplorer::getSearchURL()
{
	OnlineSource os = source.getValueDataAsEnum<OnlineSource>();
	switch (os)
	{
	case ShaderToy:
		return "https://www.shadertoy.com/api/v1/shaders/query/" + search.stringValue() + "?key=Bd8jRr&from=" + String((pageCount.intValue() - 1) * maxResults.intValue()) + "&num=" + String(maxResults.intValue());

	case ISF:
		return "https ://editor.isf.video/api/search?q=" + search.stringValue() + "&category=" + category.getValueData().toString() + "&showPrivate=false&from=" + String(pageCount.intValue() * 18);

	case Pexels_Photo:
		return "https ://api.pexels.com/v1/search?query=" + search.stringValue() + "&per_page=" + String(maxResults.intValue()) + "&page=" + String(pageCount.intValue());

	case Pexels_Video:
		return "https ://api.pexels.com/videos/search?query=" + search.stringValue() + "&per_page=" + String(maxResults.intValue()) + "&page=" + String(pageCount.intValue());
	}

	return "";
}

String OnlineContentExplorer::getExtraHeadersForSource(OnlineSource os)
{
	switch (os)
	{
	case Pexels_Photo:
	case Pexels_Video:
		return "Authorization: 55v71zcStnsFFKVHVroMIinNVSYEcYcBLokFE2uP3PeBGH5FfYi5NNcr";
		break;
	}

	return String();
}



void OnlineContentExplorer::parameterValueChanged(Parameter* p)
{
	if (p == &source)
	{
		updateLinksLabel();
		//rebuild category
		category.clearOptions();
		category.addOption("All", "");

		OnlineSource os = source.getValueDataAsEnum<OnlineSource>();
		switch (os)
		{
		case ShaderToy:
			break;

		case ISF:
			break;
		}
	}

	if (p == &source || p == &search || p == &pageCount || p == &maxResults)
	{
		maxResults.setEnabled(source.getValueDataAsEnum<OnlineSource>() != ISF);
		startThread();
	}
}

OnlineContentItem::OnlineContentItem(OnlineContentExplorer::OnlineSource source, const var& _data) :
	Thread("OnlineContent Item"),
	source(source),
	isSupported(true)
{
	needsToLoadInfos = source == OnlineContentExplorer::ShaderToy;

	switch (source)
	{
	case OnlineContentExplorer::ShaderToy:
	{
		id = _data.toString();
	}
	break;

	case OnlineContentExplorer::ISF:
	{
		data = _data;
		id = data["id"].toString();
	}
	break;

	case OnlineContentExplorer::Pexels_Photo:
	case OnlineContentExplorer::Pexels_Video:
	{
		data = _data;
		id = data["id"].toString();
	}
	break;
	}

	repaint();
	startThread();
}

OnlineContentItem::~OnlineContentItem()
{
	stopThread(2000);
}

void OnlineContentItem::paint(Graphics& g)
{
	g.setColour(PANEL_COLOR);
	g.fillRoundedRectangle(getLocalBounds().toFloat(), 5.f);

	g.setColour(Colours::white);
	if (previewImage.isValid()) g.drawImage(previewImage, getLocalBounds().toFloat().reduced(5.f));
	g.drawText(name, getLocalBounds(), Justification::centred);
}

void OnlineContentItem::mouseDown(const MouseEvent& e)
{
	if (e.mods.isRightButtonDown())
	{
		PopupMenu m;
		m.addItem("Open in browser", [this]() {
			URL url;
			switch (source)
			{
			case OnlineContentExplorer::ShaderToy: url = URL("https://www.shadertoy.com/view/" + id); break;
			case OnlineContentExplorer::ISF: url = URL("https://editor.isf.video/?id=" + id); break;
			case OnlineContentExplorer::Pexels_Photo:
			case OnlineContentExplorer::Pexels_Video: url = URL(data["url"].toString()); break;
			}

			url.launchInDefaultBrowser();
			});

		if (source == OnlineContentExplorer::Pexels_Photo || source == OnlineContentExplorer::Pexels_Video) m.addItem("Go to author's page", [this]() {
			URL(data["user"]["url"]).launchInDefaultBrowser();
			});

		m.showMenuAsync(PopupMenu::Options().withTargetComponent(this));
	}
}

void OnlineContentItem::mouseDrag(const MouseEvent& e)
{
	if (!isSupported) return;

	if (e.getDistanceFromDragStart() > 10 && !isDragAndDropActive())
	{
		var dragData(new DynamicObject());
		dragData.getDynamicObject()->setProperty("type", "OnlineContentItem");
		dragData.getDynamicObject()->setProperty("id", id);

		switch (source)
		{
		case OnlineContentExplorer::Pexels_Photo:
		{
			dragData.getDynamicObject()->setProperty("url", "https://images.pexels.com/photos/" + id + "/pexels-photo-" + id + ".jpeg?auto=compress&cs=tinysrgb&dpr=1&fit=crop&w=1920");
		}
		break;

		case OnlineContentExplorer::Pexels_Video:
		{
			var videoFiles = data["video_files"];
			for (int i = 0; i < videoFiles.size(); i++)
			{
				if (videoFiles[i]["quality"] == "sd") dragData.getDynamicObject()->setProperty("url", videoFiles[i]["link"]);
			}
		}
		default:
			break;
		}

		startDragging(dragData, this, ScaledImage(), true);
	}
}

void OnlineContentItem::run()
{
	//load preview image and infos from id 

	if (needsToLoadInfos)
	{
		URL url;
		switch (source)
		{
		case OnlineContentExplorer::ShaderToy:
		{
			url = URL("https://www.shadertoy.com/api/v1/shaders/" + id + "?key=Bd8jRr");
		}
		break;
		default:
			break;
		}

		LOG("Loading " + url.toString(true));

		std::unique_ptr<InputStream> is = url.createInputStream(URL::InputStreamOptions(URL::ParameterHandling::inAddress).withProgressCallback([this](int, int) { return !threadShouldExit();  }));


		String dataStr = is->readEntireStreamAsString();
		if (threadShouldExit()) return;

		data = JSON::parse(dataStr);
	}

	if (data.isObject())
	{
		String previewURL = "";
		switch (source)
		{
		case OnlineContentExplorer::ShaderToy:
		{
			name = data["Shader"]["info"]["name"].toString();
			description = data["Shader"]["info"]["description"].toString();
			if (data["Shader"]["info"]["usePreview"])
			{
				previewURL = "https://www.shadertoy.com/media/shaders/" + id + ".jpg";
			}
			bool isMultipass = data["Shader"]["renderpass"].size() > 1;
			if (isMultipass) isSupported = false;
		}
		break;

		case OnlineContentExplorer::ISF:
		{
			previewURL = "https://res.cloudinary.com/hrlz5rsqo/image/upload/c_fill,h_135,w_240/" + data["thumbnailCloudinaryId"].toString();
			name = data["title"].toString();
			description = data["description"].toString();
			bool isMultipass = false;
			if (isMultipass) isSupported = false;
		}
		break;

		case OnlineContentExplorer::Pexels_Photo:
		{
			previewURL = "https://images.pexels.com/photos/" + id + "/pexels-photo-" + id + ".jpeg?auto=compress&cs=tinysrgb&dpr=1&h=160&w=90";
			name = data["photographer"].toString() + " - " + data["alt"];
			description = data["alt"];
		}
		break;

		case OnlineContentExplorer::Pexels_Video:
		{
			previewURL = "https://images.pexels.com/videos/" + id + "/free-video-" + id + ".jpg?w=160&h=90&auto=compress&cs=tinysrg";
			name = data["user"]["name"].toString() + " - " + id;
			description = data["user"]["name"].toString() + " - " + id;
		}
		}

		if (previewURL.isNotEmpty())
		{

			URL url(previewURL);


			StringPairArray headers;
			std::unique_ptr<InputStream> is = url.createInputStream(URL::InputStreamOptions(URL::ParameterHandling::inAddress).withResponseHeaders(&headers).withProgressCallback([this](int, int) { return !threadShouldExit();  }));


			if (is != nullptr)
			{
				MemoryBlock block;
				is->readIntoMemoryBlock(block);
				MemoryInputStream mis(block, false);
				previewImage = ImageFileFormat::loadFrom(mis);
			}



			if (threadShouldExit()) return;
			MessageManager::callAsync([this]() { repaint(); });
		}
	}

	if (threadShouldExit()) return;

	MessageManager::callAsync([&]()
		{
			if (!isSupported)
			{
				setEnabled(false);
				setAlpha(.3f);
			}

			repaint();

		});
}