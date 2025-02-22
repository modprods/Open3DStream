#include "SOpen3DStreamFactory.h"
#include "Widgets/Input/SEditableTextBox.h"

#include "o3ds/o3ds.h"  // for version

#define LOCTEXT_NAMESPACE "Open3DStream"

FText SOpen3DStreamFactory::LastUrl;

void SOpen3DStreamFactory::Construct(const FArguments& Args)
{
	//LastTickTime = 0.0;
	OnSelectedEvent = Args._OnSelectedEvent;

	Options.Add(MakeShareable(new FString("Subscribe")));
	Options.Add(MakeShareable(new FString("Client")));
	Options.Add(MakeShareable(new FString("Server")));
	Options.Add(MakeShareable(new FString("TCP")));
	Options.Add(MakeShareable(new FString("UDP")));
	CurrentProtocol = Options[0];

	const char* version = O3DS::getVersion();

	if (SOpen3DStreamFactory::LastUrl.IsEmpty())
	{
		LastUrl = LOCTEXT("Open3DStreamUrlValue", "127.0.0.1:3001");
	}

	mUrl = LastUrl;

	ChildSlot
	[
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.3f)
			[
				SNew(STextBlock).Text(LOCTEXT("Open3DStreamUrl", "Url")).MinDesiredWidth(150)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.7f)
			[	
				SNew(SEditableTextBox)
				.Text(mUrl)
				.OnTextChanged(this, &SOpen3DStreamFactory::SetUrl)
			]
		]
		+ SVerticalBox::Slot()
		.Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.3f)
			[
				SNew(STextBlock).Text(LOCTEXT("Open3DStreamProtocol", "Protocol")).MinDesiredWidth(150)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.7f)
			[	
				SNew(SComboBox<FComboItemType>)
				.OptionsSource(&Options)
				.OnSelectionChanged(this, &SOpen3DStreamFactory::OnProtocolChanged)
				.OnGenerateWidget(this, &SOpen3DStreamFactory::MakeWidgetForOption)
				.InitiallySelectedItem(CurrentProtocol)
				[
					SNew(STextBlock)
					.Text(this, &SOpen3DStreamFactory::GetCurrentProtocol)
				]
			]
		]
		+ SVerticalBox::Slot()
		.Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.3f)
			[
				SNew(STextBlock).Text(LOCTEXT("Open3DStreamKey", "Key")).MinDesiredWidth(150)
			]
			+ SHorizontalBox::Slot()
			.FillWidth(0.7f)
			[	
				SNew(SEditableTextBox)
				.Text(this, &SOpen3DStreamFactory::GetKey)
				.OnTextChanged(this, &SOpen3DStreamFactory::SetKey)
			]
		]
		+ SVerticalBox::Slot()
		.Padding(5)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.FillWidth(0.3f)
			+ SHorizontalBox::Slot()
			.FillWidth(0.4f)
			[
				SNew(SButton)
				.OnClicked(this, &SOpen3DStreamFactory::OnSource)
				.Content()
				[
					SNew(STextBlock)
					.MinDesiredWidth(100)
					.Justification(ETextJustify::Center)
					.Text(LOCTEXT("OkayButton", "Okay"))
				]
			]
   		    + SHorizontalBox::Slot()
			.FillWidth(0.3f)
		]
		+ SVerticalBox::Slot()
			.Padding(5)
			[
				SNew(STextBlock).Text(LOCTEXT("Open3DStreamVersion", O3DS_VERSION))
			]

	];
}

FReply SOpen3DStreamFactory::OnSource()
{
	
	FString s = mUrl.ToString();
	const TCHAR* url = s.operator*();
	TSharedPtr<FOpen3DStreamSettings, ESPMode::ThreadSafe> Settings = MakeShared<FOpen3DStreamSettings, ESPMode::ThreadSafe>();
	Settings->TimeOffset = 0;
	Settings->Url = mUrl;
	Settings->Protocol = GetCurrentProtocol();
	OnSelectedEvent.ExecuteIfBound(Settings);
	return FReply::Handled();
}

TSharedRef<SWidget> SOpen3DStreamFactory::MakeWidgetForOption(FComboItemType InOption)
{
	return SNew(STextBlock).Text(FText::FromString(*InOption));
}

void SOpen3DStreamFactory::OnProtocolChanged(FComboItemType NewValue, ESelectInfo::Type)
{
	CurrentProtocol = NewValue;
}

FText SOpen3DStreamFactory::GetCurrentProtocol() const
{
	if (CurrentProtocol.IsValid())
	{
		return FText::FromString(*CurrentProtocol);
	}

	return LOCTEXT("InvalidComboEntryText", "<<Invalid option>>");
}