// Widget Documentation Plugin
// Created by Greg Shynkar
// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#include "WidgetDocumentationPluginBPLibrary.h"
#include "WidgetDocumentationPlugin.h"

UWidgetDocumentationPluginBPLibrary::UWidgetDocumentationPluginBPLibrary(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

FString UWidgetDocumentationPluginBPLibrary::SaveWidgetToPDF(UUserWidget *const UserWidget, const FVector2D &PageSize, const FString Filename, const bool bOverwritePDF)
{
	UWidgetSwitcher* WidgetSwitcherRef = NULL;
	int32 IndexOfWidgetSwitcher = 0;
	int32 CompressionQty = 0;

	FString FilePath;
	if (GIsEditor)
	{
		FilePath = FPaths::ProjectDir() + "Documents/";
	}
	else
	{
		FilePath = FPaths::RootDir() + "Documents/";
	}

	return AdvancedSaveWidgetToPDF(UserWidget, PageSize, WidgetSwitcherRef, IndexOfWidgetSwitcher, FilePath, Filename, CompressionQty, bOverwritePDF);
}

FString UWidgetDocumentationPluginBPLibrary::AdvancedSaveWidgetToPDF(UUserWidget *const UserWidget, const FVector2D &PageSize, UWidgetSwitcher* WidgetSwitcherIfUsed, const int32 LastIndexOfWidgetSwitcherIfUsed, FString Filepath, FString Filename, const int32 CompressionQty, const bool bOverwritePDF, FVector2D AndroidPlatform_RenderScale)
{
	/*
	 @note General structure of PDF:
		Header - version of PDF file;
		Body - actual content that will be displayed. Main objects are :
			a.Catalog - root object, contains a refence to a Pages Tree object;
			b.Page Tree - defines all the pages in the PDF document;
			c.Pages - instructions for drawing the graphical and textual content;
		XRefer - cross-reference table, that specifies the position of the objects;
		Trailer - information about where the document starts;
	*/

	FString PDFScript;
	int32 PagesQty;

	TArray<int32> XrefSizeArray;

	int32 PDFPageWidth = PageSize.X;
	int32 PDFPageHeight = PageSize.Y;
	TArray<int32> PagesWidthArray;
	TArray<int32> PagesHeightArray;

	TArray<FString> ImagesTextRef;
	TArray<FString> XrefpartsArray;

	FString ReturnFString;

	// Check Input Data 
	if (UserWidget == NULL)
	{
		ReturnFString = "Failed: Widget does not exist";
		return ReturnFString;
	}

	int8 MinRequireSize = 1;
	if (PageSize.X <= MinRequireSize || PageSize.Y <= MinRequireSize)
	{
		ReturnFString = "Failed: Texture Size is equal or below one";
		return ReturnFString;
	}

	if (!FPaths::ValidatePath(Filepath))
	{
		ReturnFString = "Failed: Wrong Directory ";
		return ReturnFString;
	}

	Filename = FPaths::MakeValidFileName(Filename);
	FString FilePathName = Filepath + Filename + ".pdf";

	// @Header (see structure at the top of this function);
	PDFScript = "%PDF-1.7\n\n";
	PagesQty = 0;
	bool bFileExist = false;

	// Get data of existing PDF file for adding new ones (if Overwrite is unchecked);
	if (!bOverwritePDF)
	{
		if (FPaths::FileExists(FilePathName))
		{
			bFileExist = true;
			FString LoadedStringFromFile;
			TArray <uint8> LoadedFileUint8Array;

			FFileHelper::LoadFileToArray(LoadedFileUint8Array, *FilePathName);

			// For some reason needs to remove 1 byte for each element of array;
			for (int64 x = 0; x < LoadedFileUint8Array.Num(); x++)
			{
				LoadedFileUint8Array[x] = LoadedFileUint8Array[x] - 1;
			}
			LoadedStringFromFile = BytesToString(LoadedFileUint8Array.GetData(), LoadedFileUint8Array.GetAllocatedSize());

			FString TempString;
			FString StringLeft;
			FString StringRight;

			// Get pages Quantity from existing PDF file;
			LoadedStringFromFile.Split("/Count", &StringLeft, &StringRight, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			TempString = StringRight;
			TempString.Split(">>", &StringLeft, &StringRight, ESearchCase::IgnoreCase, ESearchDir::FromStart);
			TempString = StringLeft;
			TempString.TrimStartAndEndInline();
			PagesQty = FCString::Atoi(*TempString);

			// Get pages Height and Width from existing PDF file;
			TempString = LoadedStringFromFile;
			for (int32 i = 0; i < PagesQty; i++)
			{
				TempString.Split("/Width", &StringLeft, &StringRight, ESearchCase::IgnoreCase, ESearchDir::FromStart);
				TempString = StringRight;
				TempString.Split("/Height", &StringLeft, &StringRight, ESearchCase::IgnoreCase, ESearchDir::FromStart);
				StringLeft.TrimStartAndEndInline();
				PagesWidthArray.Push(FCString::Atoi(*StringLeft));
				TempString = StringRight;
				TempString.Split("/BitsPerComponent", &StringLeft, &StringRight, ESearchCase::IgnoreCase, ESearchDir::FromStart);
				StringLeft.TrimStartAndEndInline();
				PagesHeightArray.Push(FCString::Atoi(*StringLeft));
				TempString = StringRight;
			}

			// Get XrefParts from existing PDF file;
			TempString.Split("65535 f ", &StringLeft, &StringRight, ESearchCase::IgnoreCase, ESearchDir::FromStart);
			TempString = StringRight;
			for (int32 i = 0; i < PagesQty + 1; i++)
			{
				TempString.Split("n ", &StringLeft, &StringRight, ESearchCase::IgnoreCase, ESearchDir::FromStart);
				XrefpartsArray.Push(StringLeft.Append("n"));
				TempString = StringRight;
			}

			// Get Image Data from existing PDF file;
			LoadedStringFromFile.Split("/XObject <<", &StringLeft, &StringRight, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			TempString = StringLeft;
			TempString.Split("endobj", &StringLeft, &StringRight, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
			PDFScript = StringLeft;
			PDFScript.Append("endobj\n\n");

			// Clear used resources;
			LoadedFileUint8Array.Empty();
			LoadedStringFromFile.Empty();
		}
	}

	// @Body: description and data of Image;
	TArray<uint8> FirstPartUint8Array;
	int32 PagesForSave;

	int32 PagesInDoc = PagesQty;;
	int8 AddOneForNextIndex = 1;

	if (WidgetSwitcherIfUsed != NULL)
	{
		PagesQty = PagesQty + LastIndexOfWidgetSwitcherIfUsed + AddOneForNextIndex;
		PagesForSave = LastIndexOfWidgetSwitcherIfUsed + AddOneForNextIndex;
	}
	else
	{
		PagesQty++;
		PagesForSave = 1;
	}

	for (int32 x = 0; x < PagesForSave; x++)
	{
		if (WidgetSwitcherIfUsed != NULL)
		{
			WidgetSwitcherIfUsed->SetActiveWidgetIndex(x);
		}

		PagesWidthArray.Push(PDFPageWidth);
		PagesHeightArray.Push(PDFPageHeight);

		XrefSizeArray.Add(PDFScript.Len() + FirstPartUint8Array.GetTypeSize()*FirstPartUint8Array.Num());

		PDFScript.Append(FString::FromInt(PagesInDoc + x + AddOneForNextIndex) + " 0 obj\n");
		PDFScript.Append("<<\n");
		PDFScript.Append("   /Type              /XObject\n");
		PDFScript.Append("   /Subtype           /Image\n");
		PDFScript.Append("   /Width             " + FString::FromInt(PDFPageWidth) + "\n");
		PDFScript.Append("   /Height            " + FString::FromInt(PDFPageHeight) + "\n");
		PDFScript.Append("   /BitsPerComponent  8\n");
		PDFScript.Append("   /ColorSpace        /DeviceRGB\n");
		PDFScript.Append("   /Filter            /DCTDecode\n");

		// Fill TextureUint8Array with RGB data to get size of Image;
		TArray<uint8> TextureUint8Array;
		GetHexTextureDataFromWidget(UserWidget, TextureUint8Array, PageSize, CompressionQty, AndroidPlatform_RenderScale);

		PDFScript.Append("   /Length            " + FString::FromInt(TextureUint8Array.Num()) + "\n");
		PDFScript.Append(">>\n");
		PDFScript.Append("stream\n");

		// Transforming PDFScript from FString to Uint8 array. Main goal - is to make process faster;
		TArray<uint8>XObjectData;
		XObjectData.AddUninitialized(PDFScript.Len());

		StringToBytes(PDFScript, XObjectData.GetData(), PDFScript.Len());

		// For some reason needs to add 1 byte for each element of array;
		for (int64 i = 0; i < XObjectData.Num(); i++)
		{
			XObjectData[i] = XObjectData[i] + 1;
		}

		// Append TextureUint8Array created and filled earlier; 
		XObjectData.Append(TextureUint8Array);

		FirstPartUint8Array.Append(XObjectData);

		// Clean up PDFscript and continue to make it in FString;
		XObjectData.Empty();
		TextureUint8Array.Empty();
		PDFScript.Empty();

		PDFScript.Append("\n");
		PDFScript.Append("endstream\n");
		PDFScript.Append("endobj\n\n");
	}

	// @Body: At this part we are telling the PDF that the data that describes the image can be found in the object number #;
	XrefSizeArray.Add(PDFScript.Len() + FirstPartUint8Array.Num());

	PDFScript.Append(FString::FromInt(AddOneForNextIndex + PagesQty) + " 0 obj\n");
	PDFScript.Append("<<\n");
	PDFScript.Append("   /XObject <<\n");

	for (int32 i = 0; i < PagesQty; i++)
	{
		PDFScript.Append("      /Img" + FString::FromInt(i + AddOneForNextIndex) + " " + FString::FromInt(i + AddOneForNextIndex) + " 0 R\n");

		// The next two lines describe where to place the image relative to the page. Will be specified further in the @Body: Page Recourses;
		ImagesTextRef.Push(("q " + FString::FromInt(PagesWidthArray[i]) + " " + FString::FromInt(0) + " " + FString::FromInt(0) + " " + FString::FromInt(PagesHeightArray[i]) + " " + FString::FromInt(0)
			+ " " + FString::FromInt(0) + " " + "cm" + " " + "/Img" + FString::FromInt(i + AddOneForNextIndex) + " " + "Do" + " " + "Q\n"));
		/*
		ImagesTextRef.Push(("q " + FString::FromInt(PagesWidthArray[i]) + " " + FString::FromInt(0) + " " + FString::FromInt(0) + " " + FString::FromInt(PagesHeightArray[i]) + " " + FString::FromInt(0)
			+ " " + FString::FromInt(0) + " " + "cm" + " " + "/Img" + FString::FromInt(i + AddOneForNextIndex) + " " + "Do" + " " + "Q\n"));
			*/

	}

	PDFScript.Append("   >>\n");
	PDFScript.Append(">> \n");
	PDFScript.Append("endobj\n\n");

	// @Body: Catalog (see PDF structure at the top of the function);
	XrefSizeArray.Add(PDFScript.Len() + FirstPartUint8Array.Num());

	int32 CatalogIndex = PagesQty + 2;
	int32 MainPageIndex = PagesQty + 3;
	PDFScript.Append(FString::FromInt(CatalogIndex) + " 0 obj\n");
	PDFScript.Append("<<\n");
	PDFScript.Append("   /Type /Catalog\n");
	PDFScript.Append("   /Pages " + FString::FromInt(MainPageIndex) + " 0 R\n");
	PDFScript.Append(">>\n");
	PDFScript.Append("endobj\n\n");

	// @Body: Page Tree (see PDF structure at the top of the function);
	XrefSizeArray.Add(PDFScript.Len() + FirstPartUint8Array.Num());

	PDFScript.Append(FString::FromInt(MainPageIndex) + " 0 obj\n");
	PDFScript.Append("<<\n");
	PDFScript.Append("   /Type /Pages\n");
	PDFScript.Append("   /Kids [");
	for (int32 i = 0; i < PagesQty; i++)
	{
		PDFScript.Append(" " + FString::FromInt(MainPageIndex + i + AddOneForNextIndex) + " 0 R");
	}
	PDFScript.Append(" ]\n");
	PDFScript.Append("   /Count " + FString::FromInt(PagesQty) + "\n");
	PDFScript.Append(">>\n");
	PDFScript.Append("endobj\n\n");

	// @Body: Pages (see PDF structure at the top of the function);
	for (int32 i = 0; i < PagesQty; i++)
	{
		XrefSizeArray.Add(PDFScript.Len() + FirstPartUint8Array.Num());

		PDFScript.Append(FString::FromInt(MainPageIndex + i + AddOneForNextIndex) + " 0 obj\n");
		PDFScript.Append("<<\n");
		PDFScript.Append("   /Type /Page\n");
		PDFScript.Append("   /Parent " + FString::FromInt(MainPageIndex) + " 0 R\n");
		PDFScript.Append("   /MediaBox  [0 0 " + FString::FromInt(PagesWidthArray[i]) + " " + FString::FromInt(PagesHeightArray[i]) + "] \n");
		PDFScript.Append("   /Rotate " + FString::FromInt(0) + "\n");
		PDFScript.Append("   /Contents " + FString::FromInt(MainPageIndex + AddOneForNextIndex + i + PagesQty) + " 0 R\n");
		PDFScript.Append("   /Resources " + FString::FromInt(PagesQty + AddOneForNextIndex) + " 0 R\n");
		PDFScript.Append(">>\n");
		PDFScript.Append("endobj\n\n");
	}

	// @Body: Page Recourses - info about where on the page the image was placed;	
	for (int32 i = 0; i < PagesQty; i++)
	{
		XrefSizeArray.Add(PDFScript.Len() + FirstPartUint8Array.Num());

		FString CurrentImageTextRef = ImagesTextRef[i];
		CurrentImageTextRef = "BT\n" + CurrentImageTextRef + "ET";

		PDFScript.Append(FString::FromInt(MainPageIndex + PagesQty + AddOneForNextIndex + i) + " 0 obj\n");
		PDFScript.Append("<<\n");
		PDFScript.Append("   /Length " + FString::FromInt(CurrentImageTextRef.Len()) + "\n");
		PDFScript.Append(">>\n");
		PDFScript.Append("stream\n");
		PDFScript.Append(CurrentImageTextRef + "\n");
		PDFScript.Append("endstream\n");
		PDFScript.Append("endobj\n\n");
	}

	//@XRefer: (see PDF structure at the top of the function);
	int32 SizeBeforeXpart = PDFScript.Len() + FirstPartUint8Array.Num();
	int32 ObjectsCount = (MainPageIndex + 2 * PagesQty + AddOneForNextIndex);

	PDFScript.Append("xref\n");
	PDFScript.Append("0 " + FString::FromInt(ObjectsCount) + "\n");
	PDFScript.Append("0000000000 65535 f "); // First line always general;

	//PDF String adjustment for bOverwriteCheck
	if (!bFileExist)
	{
		PDFScript.Append("\n");
	}

	// Make XrefSize to 10-digit byte offset of the object starting from the beginning of the document;
	if (!bOverwritePDF && bFileExist)
	{
		for (int32 i = 0; i < XrefpartsArray.Num(); i++)
		{
			PDFScript.Append(XrefpartsArray[i] + " ");
		}
	}
	for (int32 i = 0; i < XrefSizeArray.Num(); i++)
	{
		if (!bOverwritePDF && XrefpartsArray.IsValidIndex(i) && bFileExist && i == 0)
		{
			//PDF String adjustment for bOverwriteCheck
			PDFScript.Append("\n");
		}
		else
		{
			FString XRefIntToString = FString::FromInt(XrefSizeArray[i]);
			for (int j = 0; j < (10 - XRefIntToString.Len()); j++)
			{
				PDFScript.Append("0");
			}

			PDFScript.Append(XRefIntToString + " 00000 n " + "\n");
		}
	}

	// @Trailer: (see PDF structure at the top of the function);
	PDFScript.Append("trailer\n");
	PDFScript.Append("<<\n");
	PDFScript.Append("   /Size " + FString::FromInt(ObjectsCount) + "\n");
	PDFScript.Append("   /Root " + FString::FromInt(CatalogIndex) + " 0 R\n");
	PDFScript.Append(">>\n");
	PDFScript.Append("startxref\n");
	PDFScript.Append(FString::FromInt(SizeBeforeXpart) + "\n");
	PDFScript.Append("%%EOF\n");

	// PDF script is ready. Need to save it in binary format; 
	TArray<uint8> SecondPartUint8Array;
	SecondPartUint8Array.AddUninitialized(PDFScript.Len());

	StringToBytes(PDFScript, SecondPartUint8Array.GetData(), PDFScript.Len());

	// For some reason needs to add 1 byte for each element of array;
	for (int64 x = 0; x < SecondPartUint8Array.Num(); x++)
	{
		SecondPartUint8Array[x] = SecondPartUint8Array[x] + 1;
	}

	FirstPartUint8Array.Append(SecondPartUint8Array);

	if (!FFileHelper::SaveArrayToFile(FirstPartUint8Array, *FilePathName))
	{
		ReturnFString = "Failed to save on Disk";
		return ReturnFString;
	}

	// Clear used resources;
	FirstPartUint8Array.Empty();
	SecondPartUint8Array.Empty();
	PDFScript.Empty();
	XrefSizeArray.Empty();
	PagesWidthArray.Empty();
	PagesHeightArray.Empty();
	XrefpartsArray.Empty();

	ReturnFString = "Success: " + FilePathName;
	return ReturnFString;
}

FString UWidgetDocumentationPluginBPLibrary::GetHexTextureDataFromWidget(UUserWidget *const UserWidget, TArray<uint8>&OutTextureUint8Array, const FVector2D &TextureSize, const int32 CompressionQty, FVector2D AndroidPlatform_RenderScale)
{
	FString ReturnFString;
	int8 MinRequireSize = 1;
	if (UserWidget != NULL && UserWidget->IsValidLowLevel() && TextureSize.X >= MinRequireSize && TextureSize.Y >= MinRequireSize)
	{
		//For some reason at version 4.25 need to call firstly constructor of FWidgetRenderer with Gamma correction and bForceLinearGamma from UTextureRenderTarget2D;
		MakeTextureFromWidget(UserWidget, TextureSize, true);

		// Transform for Android Platform; 
		if (PLATFORM_ANDROID)
		{
			UserWidget->SetRenderScale(AndroidPlatform_RenderScale);
		}

		TSharedPtr<FWidgetRenderer> WidgetRender = MakeShareable(new FWidgetRenderer(true));
		TSharedPtr<SWidget> SlateWidget(UserWidget->TakeWidget());

		if (!WidgetRender.IsValid() || !SlateWidget.IsValid())
		{
			ReturnFString = "Failed with Widget Render Operation";
			return ReturnFString;
		}

		// Get FColor data from Widget`s RenderTarget; 
		UTextureRenderTarget2D *TextureRenderTarget2d = WidgetRender->DrawWidget(SlateWidget.ToSharedRef(), TextureSize);

		TextureRenderTarget2d->bForceLinearGamma = true;
		FRenderTarget *RenderTarget = TextureRenderTarget2d->GameThread_GetRenderTargetResource();
		TArray<FColor> ColorSurfData;
		RenderTarget->ReadPixels(ColorSurfData);


		TSharedPtr<IImageWrapper> ImageWrapper;
		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
		ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);
		
		int32 BytesPerRow = (4 * TextureSize.X);
		int64 RawSize = BytesPerRow * TextureSize.Y;
		ImageWrapper->SetRaw(ColorSurfData.GetData(), RawSize, TextureSize.X, TextureSize.Y, ERGBFormat::BGRA, 8, BytesPerRow);
	//	ImageWrapper->SetRaw(ColorSurfData.GetData(), ColorSurfData.GetAllocatedSize(), TextureSize.X, TextureSize.Y, ERGBFormat::BGRA, 8, BytesPerRow);
		OutTextureUint8Array = ImageWrapper->GetCompressed(CompressionQty);

		// Clear used resources
		ImageWrapper.Reset();
		SlateWidget.Reset();
		TextureRenderTarget2d->ConditionalBeginDestroy();
		WidgetRender.Reset();
		ColorSurfData.Empty();

		// Reverse Transform for Android Platform; 
		if (PLATFORM_ANDROID)
		{
			FVector2D WidgetTransform;
			WidgetTransform.Set(1, 1);
			UserWidget->SetRenderScale(WidgetTransform);
		}

		ReturnFString = "Success";
		return ReturnFString;
	}

	if (UserWidget != NULL && UserWidget->IsValidLowLevel())
	{
		return "Failed: Error with reference";
	}
	else
	{
		return "Failed: TextureSize bellow MinRequireSize";
	}
}

FString UWidgetDocumentationPluginBPLibrary::SaveWidgetAsImage(UUserWidget *const UserWidget, const FVector2D &ImageSize, FString Filename, const bool bOverwriteImage)
{

	FImageWriteOptions DefaultImageWriteOptions;
	DefaultImageWriteOptions.Format = EDesiredImageFormat::PNG;
	DefaultImageWriteOptions.bOverwriteFile = bOverwriteImage;
	DefaultImageWriteOptions.bAsync = true;
	DefaultImageWriteOptions.CompressionQuality = 0;

	FString Filepath;
	if (GIsEditor)
	{
		Filepath = FPaths::ProjectDir() + "Documents/";
	}
	else
	{
		Filepath = FPaths::RootDir() + "Documents/";
	}

	return AdvancedSaveWidgetAsImage(UserWidget, ImageSize, Filepath, Filename, DefaultImageWriteOptions);

}

FString UWidgetDocumentationPluginBPLibrary::AdvancedSaveWidgetAsImage(UUserWidget *const UserWidget, const FVector2D &ImageSize, FString Filepath, FString Filename, FImageWriteOptions Options, FVector2D AndroidPlatform_RenderScale)
{
	FString ImageExtension;
	FString FilePathName;
	FString ReturnFString;

	// Check Input Data
	if (UserWidget == NULL)
	{
		ReturnFString = "Failed: Widget does not exist";
		return ReturnFString;
	}
	int8 MinRequireSize = 15;
	if (ImageSize.X < MinRequireSize || ImageSize.Y < MinRequireSize)
	{
		ReturnFString = "Failed: Minimum Image size is 15 pixels";
		return ReturnFString;
	}
	if (!FPaths::ValidatePath(Filepath))
	{
		ReturnFString = "Failed: Wrong Directory";
		return ReturnFString;
	}
	Filename = FPaths::MakeValidFileName(Filename);

	// Make Image Extension as FString;
	switch (Options.Format)
	{
	case EDesiredImageFormat::PNG:
		ImageExtension = ".png";
		break;
	case EDesiredImageFormat::JPG:
		ImageExtension = ".jpg";
		break;
	case EDesiredImageFormat::EXR:
		ImageExtension = ".exr";
		break;
	case EDesiredImageFormat::BMP:
		ImageExtension = ".bmp";
		break;
	default:
		ImageExtension = ".png";
		break;
	}
	FilePathName = Filepath + Filename + ImageExtension;

	// Overwrite Image or create a copy;
	if (!Options.bOverwriteFile)
	{
		bool bbFileExists = FPaths::FileExists(FilePathName);
		for (int32 i = 1; bbFileExists; i++)
		{
			FilePathName = Filepath + Filename + FString::FromInt(i) + ImageExtension;
			bbFileExists = FPaths::FileExists(FilePathName);
		}
	}

	//For some reason at version 4.25 need to call firstly constructor of FWidgetRenderer with Gamma correction and bForceLinearGamma from UTextureRenderTarget2D;
	MakeTextureFromWidget(UserWidget, ImageSize, false);

	// Save Widget as Image thought RenderTarget;
	UTextureRenderTarget2D *MyTextureRenderTarget2d = GetRenderTarget2DFromWidget(UserWidget, ImageSize, AndroidPlatform_RenderScale);

	if (MyTextureRenderTarget2d == NULL)
	{
		ImageExtension = "Failed: Could not Make texture from Widget";
		return ImageExtension;
	}
	UImageWriteBlueprintLibrary::ExportToDisk(MyTextureRenderTarget2d, FilePathName, Options);
	MyTextureRenderTarget2d->ConditionalBeginDestroy();

	ReturnFString = "Success: " + FilePathName;

	return ReturnFString;
}

FString UWidgetDocumentationPluginBPLibrary::SaveWidgetAsImageV2(UUserWidget *const UserWidget, const FVector2D &ImageSize, FString Filename)
{
	FString ImageExtension = ".png";

	FString Filepath;
	if (GIsEditor)
	{
		Filepath = FPaths::ProjectDir() + "Documents/";
	}
	else
	{
		Filepath = FPaths::RootDir() + "Documents/";
	}

	return AdvancedSaveWidgetAsImageV2(UserWidget, ImageSize, Filepath, Filename, ImageExtension);


}

FString UWidgetDocumentationPluginBPLibrary::AdvancedSaveWidgetAsImageV2(UUserWidget *const UserWidget, const FVector2D &ImageSize, FString Filepath, FString Filename, FString ImageExtension, FVector2D AndroidPlatform_RenderScale /*= FVector2D(1.0f, -1.0f)*/)
{
	FString FilePathName;
	FString ReturnFString;

	// Check Input Data
	if (UserWidget == NULL)
	{
		ReturnFString = "Failed: Widget does not exist";
		return ReturnFString;
	}
	int8 MinRequireSize = 15;
	if (ImageSize.X < MinRequireSize || ImageSize.Y < MinRequireSize)
	{
		ReturnFString = "Failed: Minimum Image size is 15 pixels";
		return ReturnFString;
	}
	if (!FPaths::ValidatePath(Filepath))
	{
		ReturnFString = "Failed: Wrong Directory";
		return ReturnFString;
	}
	Filename = FPaths::MakeValidFileName(Filename);

	FilePathName = Filepath + Filename + ImageExtension;

	//For some reason at version 4.25 need to call firstly constructor of FWidgetRenderer with Gamma correction and bForceLinearGamma from UTextureRenderTarget2D;
	MakeTextureFromWidget(UserWidget, ImageSize, false);

	int32 CompressionQty = 0;
	TArray<uint8> TextureUint8Array;
	GetHexTextureDataFromWidget(UserWidget, TextureUint8Array, ImageSize, CompressionQty, AndroidPlatform_RenderScale);

	if (FFileHelper::SaveArrayToFile(TextureUint8Array, *FilePathName))
	{
		ReturnFString = "Success: " + FilePathName;
		return ReturnFString;

	}

	ReturnFString = "Failed To Save Image";
	return ReturnFString;

}

UTextureRenderTarget2D* UWidgetDocumentationPluginBPLibrary::GetRenderTarget2DFromWidget(UUserWidget *const UserWidget, const FVector2D &TextureSize, FVector2D AndroidPlatform_RenderScale)
{
	// Check Input Data;
	TSharedPtr<FWidgetRenderer> WidgetRender = MakeShareable(new FWidgetRenderer(false));
	if (!WidgetRender.IsValid())
	{
		return NULL;

	}
	TSharedPtr<SWidget> SlateWidget(UserWidget->TakeWidget());
	if (!SlateWidget.IsValid())
	{
		return NULL;
	}

	// Transform for Android Platform; 
	if (PLATFORM_ANDROID)
	{
		UserWidget->SetRenderScale(AndroidPlatform_RenderScale);
	}

	// Render Widget with WidgetRender;
	UTextureRenderTarget2D *RenderTarget2d = WidgetRender->DrawWidget(SlateWidget.ToSharedRef(), TextureSize);
	RenderTarget2d->bForceLinearGamma = false;
	FRenderTarget *RenderTarget = RenderTarget2d->GameThread_GetRenderTargetResource();
	TArray<FColor> SurfData;
	RenderTarget->ReadPixels(SurfData);

	// Clear used resources;
	SlateWidget.Reset();
	WidgetRender.Reset();
	SurfData.Empty();

	// Reverse Transform for Android Platform; 
	if (PLATFORM_ANDROID)
	{
		FVector2D WidgetTransform (1.0, 1.0);
		UserWidget->SetRenderScale(WidgetTransform);
	}

	return RenderTarget2d;
}

UTexture2D* UWidgetDocumentationPluginBPLibrary::MakeTextureFromWidget(UUserWidget *const UserWidget, const FVector2D &TextureSize, bool bNeedGammaCor)
{
	int8 MinRequireSize = 1;
	if (UserWidget != NULL && UserWidget->IsValidLowLevel() && TextureSize.X >= MinRequireSize && TextureSize.Y >= MinRequireSize)
	{

		TSharedPtr<FWidgetRenderer> MyWidgetRender = MakeShareable(new FWidgetRenderer(bNeedGammaCor));
		if (!MyWidgetRender.IsValid())
		{
			return NULL;
		}
		TSharedPtr<SWidget> MySlateWidget(UserWidget->TakeWidget());
		if (!MySlateWidget.IsValid())
		{
			return NULL;
		}

		// Render Widget with WidgetRender;
		UTextureRenderTarget2D *MyTextureRenderTarget2d = MyWidgetRender->DrawWidget(MySlateWidget.ToSharedRef(), TextureSize);
		MyTextureRenderTarget2d->bForceLinearGamma = bNeedGammaCor;
		FRenderTarget *MyRenderTarget = MyTextureRenderTarget2d->GameThread_GetRenderTargetResource();
		TArray<FColor> LocalSurfData;
		MyRenderTarget->ReadPixels(LocalSurfData);

		// Copy locked data;
		UTexture2D *MyTexture2d = UTexture2D::CreateTransient(TextureSize.X, TextureSize.Y, PF_B8G8R8A8);
#if WITH_EDITORONLY_DATA
		MyTexture2d->MipGenSettings = TMGS_NoMipmaps;
#endif	
		void* TextureData = MyTexture2d->GetPlatformData()->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
		const int32 TextureDataSize = LocalSurfData.Num() * 4;
		FMemory::Memcpy(TextureData, LocalSurfData.GetData(), TextureDataSize);
		MyTexture2d->GetPlatformData()->Mips[0].BulkData.Unlock();
		MyTexture2d->UpdateResource();

		// Clear used resources
		MySlateWidget.Reset();
		MyTextureRenderTarget2d->ConditionalBeginDestroy();
		MyWidgetRender.Reset();
		LocalSurfData.Empty();

		return MyTexture2d;
	}
	return NULL;
}

FString UWidgetDocumentationPluginBPLibrary::SaveTextToDisk(FString TextToSave, FString Filename, bool bOverwriteText, FString DefaultExtension)
{
	FString Filepath;

	if (GIsEditor)
	{
		Filepath = FPaths::ProjectDir() + "Documents/";
	}
	else
	{
		Filepath = FPaths::RootDir() + "Documents/";
	}

	// Set values for AdvancedSaveTextToDisk;
	return	AdvancedSaveTextToDisk(TextToSave, Filepath, Filename, bOverwriteText, DefaultExtension);

}

FString UWidgetDocumentationPluginBPLibrary::AdvancedSaveTextToDisk(FString TextToSave, FString Filepath, FString Filename, bool bOverwriteText, FString DefaultExtension)
{
	FString ReturnFString;

	// Check Input Data 
	if (!FPaths::ValidatePath(Filepath))
	{
		ReturnFString = "Failed: Wrong Directory ";
		return ReturnFString;
	}

	FString FilePathName;
	Filename = FPaths::MakeValidFileName(Filename);
	FilePathName = Filepath + Filename + DefaultExtension;

	// Get Text from file;
	FString TextFromFile;
	if (!bOverwriteText)
	{
		TextFromFile = LoadTextFromFile(Filepath, Filename);

		if (TextFromFile == "Failed: File does not exist")
		{
			bOverwriteText = true;
		}
	}

	if (bOverwriteText != true)
	{
		TextToSave = TextFromFile + "\n" + TextToSave;
	}

	// Save text to File;
	if (!FFileHelper::SaveStringToFile(TextToSave, *FilePathName))
	{
		ReturnFString = "Failed: Could not save text to file";
		return ReturnFString;
	}

	ReturnFString = "Success: " + FilePathName;

	return ReturnFString;
}

FString UWidgetDocumentationPluginBPLibrary::LoadTextFromFile(FString Filepath, FString Filename, FString DefaultExtension)
{
	FString ReturnFString;

	// Check Input Data
	if (!FPaths::ValidatePath(Filepath) || !FPaths::DirectoryExists(Filepath))
	{
		ReturnFString = "Failed: Wrong Directory ";
		return ReturnFString;
	}

	FString FilePathName;
	FilePathName = Filepath + Filename + DefaultExtension;
	if (!FPaths::FileExists(FilePathName))
	{
		ReturnFString = "Failed: File does not exist";
		return ReturnFString;
	}

	if (!FFileHelper::LoadFileToString(ReturnFString, *FilePathName))
	{
		ReturnFString = "Failed: Could not Load Text From File";
		return ReturnFString;
	}

	return ReturnFString;
}

FString UWidgetDocumentationPluginBPLibrary::DeleteFile(FString FilePathAndName)
{
	if (FSHA1::GetFileSHAHash(*FilePathAndName, NULL))
	{
		return "Can't delete signed game file";

	}
	
	bool bExists = IFileManager::Get().FileExists(*FilePathAndName);
	if (!bExists)
	{
		return "Could not delete because File doesn't exist";
	}
	
	if (!IFileManager::Get().Delete(*FilePathAndName))
	{
		return FString::Printf(TEXT("Error deleting file: (Error Code %i)"), FPlatformMisc::GetLastError());
	}

	return "Success";
}

FString UWidgetDocumentationPluginBPLibrary::ClearDirectory(FString DirectoryPath)
{
	FPaths::MakeValidFileName(*DirectoryPath);
	
	if (DirectoryPath == "" || !IFileManager::Get().DirectoryExists(*DirectoryPath))
	{
		return "Could not delete: Wrong DirectoryPath ";
	}

	if (!IFileManager::Get().DeleteDirectory(*DirectoryPath,true,true))
	{
		return FString::Printf(TEXT("Error deleting directory: (Error Code %i)"), FPlatformMisc::GetLastError());
	}

	IFileManager::Get().MakeDirectory(*DirectoryPath, true);

	return	FString::Printf(TEXT("successfully deleted : %s"), *DirectoryPath);
}

