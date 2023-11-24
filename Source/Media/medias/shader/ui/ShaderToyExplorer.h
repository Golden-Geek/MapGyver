/*
  ==============================================================================

	ShaderToyExplorer.h
	Created: 24 Nov 2023 2:32:38pm
	Author:  bkupe

  ==============================================================================
*/

#pragma once

class ShaderToyItem :
	public Component,
	public Thread,
	public DragAndDropContainer
{
public:
	ShaderToyItem(const String& id);
	~ShaderToyItem();

	String id;
	var data;

	String name;
	String description;
	bool isMultipass;
	Image previewImage;

	void paint(Graphics& g) override;
	void mouseDrag(const MouseEvent& e) override;

	void run() override;
};


class ShaderToyExplorer :
	public ShapeShifterContentComponent,
	public ParameterListener,
	public Thread
{
public:
	ShaderToyExplorer(const String& name);
	~ShaderToyExplorer();

	StringParameter search;
	IntParameter pageCount;
	IntParameter maxResults;

	std::unique_ptr<StringParameterUI> searchUI;
	std::unique_ptr<IntStepperUI> pageCountUI;
	std::unique_ptr<IntStepperUI> maxResultsUI;

	OwnedArray<ShaderToyItem> items;

	var itemsData;

	Viewport viewport;
	Component itemsComp;

	void resized();

	void updateItems();

	void run() override;

	void parameterValueChanged(Parameter* p) override;

	static ShaderToyExplorer* create(const String& name) { return new ShaderToyExplorer(name); }
};