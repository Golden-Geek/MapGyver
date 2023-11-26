/*
  ==============================================================================

	OnlineContentExplorer.h
	Created: 24 Nov 2023 2:32:38pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class OnlineContentItem;

class OnlineContentExplorer :
	public ShapeShifterContentComponent,
	public ParameterListener,
	public Thread
{
public:
	OnlineContentExplorer(const String& name);
	~OnlineContentExplorer();


	enum OnlineSource { ShaderToy, ISF, Pexels_Photo, Pexels_Video };
	EnumParameter source;
	
	StringParameter search;

	EnumParameter category;

	IntParameter pageCount;
	IntParameter maxResults;

	std::unique_ptr<EnumParameterUI> sourceUI;
	std::unique_ptr<EnumParameterUI> categoryUI;
	std::unique_ptr<StringParameterUI> searchUI;
	std::unique_ptr<IntStepperUI> pageCountUI;
	std::unique_ptr<IntStepperUI> maxResultsUI;

	OwnedArray<OnlineContentItem> items;

	var itemsData;

	Viewport viewport;
	Component itemsComp;

	void paint(Graphics& g) override;
	void resized();

	void updateItems();

	var getResultsArray(var data);

	void run() override;

	URL getSearchURL();
	static String getExtraHeadersForSource(OnlineSource source);

	void parameterValueChanged(Parameter* p) override;

	static OnlineContentExplorer* create(const String& name) { return new OnlineContentExplorer(name); }
};

class OnlineContentItem :
	public Component,
	public Thread,
	public DragAndDropContainer
{
public:
	OnlineContentItem(OnlineContentExplorer::OnlineSource source, const var& data);
	~OnlineContentItem();

	OnlineContentExplorer::OnlineSource source;
	String id;
	var data;

	bool needsToLoadInfos;

	String name;
	String description;
	bool isSupported;
	Image previewImage;

	void paint(Graphics& g) override;
	void mouseDrag(const MouseEvent& e) override;

	void run() override;
};
