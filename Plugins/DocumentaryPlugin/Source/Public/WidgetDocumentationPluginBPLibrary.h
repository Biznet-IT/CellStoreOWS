// Widget Documentation Plugin
// Created by Greg Shynkar
// Copyright 1998-2020 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Engine/Texture2D.h"
#include "UMG/Public/Blueprint/UserWidget.h"
#include "Slate/WidgetRenderer.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ImageWriteQueue/Public/ImageWriteBlueprintLibrary.h"
#include "Misc/FileHelper.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Components/WidgetSwitcher.h"
#include "HAL/PlatformFilemanager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "WidgetDocumentationPluginBPLibrary.generated.h"


UCLASS()
class UWidgetDocumentationPluginBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	/**
		* Save Widget as PDF page; 
		* Simple presentation of AdvancedSaveWidgetToPDF function. If OverwritePDF is false, it will add new page to existing document;
		*
		* @param UserWidget					Widget to save as PDF page;
		* @param PageSize					Size of PDF page;
		* @param Filename					Name of PDF file;
		* @param bOverwritePDF				Overwrite PDF or add page;
		* @return							Result status;
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Save Widget to PDF"), Category = "WidgetDocumentationPlugin|PDF")
		static FString SaveWidgetToPDF(UUserWidget *const UserWidget, const FVector2D &PageSize, const FString Filename, const bool bOverwritePDF);

	/**
		* Save Widget as PDF page;
		* If OverwritePDF is false, it will add new page to existing document;
		*
		* @param UserWidget							Widget to save as PDF page;
		* @param PageSize							Size of PDF page;
		* @param WidgetSwitcherIfUsed				Save array of pages via Widget Switcher;
		* @param LastIndexOfWidgetSwitcherIfUsed	Last index to save;
		* @param Filepath							Directory for saving file;
		* @param Filename							Name of PDF file;
		* @param CompressionQty						Compression of Image that used as PDF page;
		* @param bOverwritePDF						Overwrite PDF or add page;
		* @return									Result status;
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Advanced Save Widget to PDF"), Category = "WidgetDocumentationPlugin|PDF")
		static FString AdvancedSaveWidgetToPDF(UUserWidget *const UserWidget, const FVector2D &PageSize, UWidgetSwitcher* WidgetSwitcherIfUsed, const int32 LastIndexOfWidgetSwitcherIfUsed, FString Filepath, FString Filename, const int32 CompressionQty = 0, const bool bOverwritePDF = true, FVector2D AndroidPlatform_RenderScale = FVector2D(1.0f, -1.0f));

	/**
		* Fill Uint8 array with RGB data from widget;
		*
		* @param UserWidget					Widget needs to be rendered;
		* @param OutTextureUint8Array		Array to fill hex data;
		* @param TextureSize				Texture Size;
		* @param CompressionQty				Compression of Image;
		* @return							Result status;
	*/
	UFUNCTION()
		static FString GetHexTextureDataFromWidget(UUserWidget *const UserWidget, TArray<uint8>&OutTextureUint8Array, const FVector2D &TextureSize, const int32 CompressionQty, FVector2D AndroidPlatform_RenderScale);

	/**
		* Save Widget as Image;
		* Simple presentation of AdvancedSaveWidgetAsImage function. If OverwriteImage is false, it will create new one if file with same name is exist;
		*
		* @param UserWidget					Widget that need to be saved as Image;
		* @param ImageSize					Size of Image;
		* @param Filename					Name of Image;
		* @param bOverwriteImage			Overwrite Image or create a new one;
		* @return							Result status;
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Save Widget as Image"), Category = "WidgetDocumentationPlugin|Image")
		static FString SaveWidgetAsImage(UUserWidget *const UserWidget, const FVector2D &ImageSize, FString Filename, const bool bOverwriteImage);

	/**
		* Save widget as Image;
		* If OverwriteImage is false, it will create new one if file with same name is exist;
		*
		* @param UserWidget					Widget that need to be saved as Image;
		* @param ImageSize					Size of Image;
		* @param Filepath					Directory for saving file;
		* @param Filename					Name of Image;
		* @param FImageWriteOptions			Saving Image options;
		* @return							Result status;
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Advanced Save Widget as Image"), Category = "WidgetDocumentationPlugin|Image")
		static FString AdvancedSaveWidgetAsImage(UUserWidget *const UserWidget, const FVector2D &ImageSize, FString Filepath, FString Filename, FImageWriteOptions Options, FVector2D AndroidPlatform_RenderScale = FVector2D(1.0f,-1.0f));

	/**
		* Save Widget as Image;
		* Simple presentation of AdvancedSaveWidgetAsImage function. If OverwriteImage is false, it will create new one if file with same name is exist;
		*
		* @param UserWidget					Widget that need to be saved as Image;
		* @param ImageSize					Size of Image;
		* @param Filename					Name of Image;
		* @param bOverwriteImage			Overwrite Image or create a new one;
		* @return							Result status;
	*/


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Save Widget as Image 2"), Category = "WidgetDocumentationPlugin|Image")
		static FString SaveWidgetAsImageV2(UUserWidget *const UserWidget, const FVector2D &ImageSize, FString Filename);


	/**
		* Save widget as Image;
		* If OverwriteImage is false, it will create new one if file with same name is exist;
		*
		* @param UserWidget					Widget that need to be saved as Image;
		* @param ImageSize					Size of Image;
		* @param Filepath					Directory for saving file;
		* @param Filename					Name of Image;
		* @param FImageWriteOptions			Saving Image options;
		* @return							Result status;
	*/


	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Advanced Save Widget as Image 2"), Category = "WidgetDocumentationPlugin|Image")
		static FString AdvancedSaveWidgetAsImageV2(UUserWidget *const UserWidget, const FVector2D &ImageSize, FString Filepath, FString Filename, FString ImageExtension, FVector2D AndroidPlatform_RenderScale = FVector2D(1.0f, -1.0f));



	/**
		* Make RenderTargetTexture from widget;
		*
		* @param UserWidget					Widget that need to be rendered;
		* @param TextureSize				Size of Texture;
		* @return							Pointer to TextureRenderTarget2D;
	*/
	UFUNCTION()
		static UTextureRenderTarget2D* GetRenderTarget2DFromWidget(UUserWidget *const UserWidget, const FVector2D &TextureSize, FVector2D AndroidPlatform_RenderScale = FVector2D(1.0f, -1.0f));

	/**
		* Makes Texture2d from Widget;
		*
		* @param UserWidget					Widget that need to be rendered;
		* @param TextureSize				Size of Texture;
		* @param bNeedGammaCor				Applies Gamma correction in WidgetRenderer constructor;
		* @return							Pointer to Texture2D;
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Make Texture From Widget"), Category = "WidgetDocumentationPlugin|Texture")
		static UTexture2D* MakeTextureFromWidget(UUserWidget *const UserWidget, const FVector2D &TextureSize, bool bNeedGammaCor = true);

	/**
		* Saves text to Disk;
		* Simple presentation of AdvancedSaveTextToDisk function. If OverwriteText is false, it will add text to existing one;
		*
		* @param TextToSave					Text that needs to be saved;
		* @param Filename					Name of Image;
		* @param bOverwriteText				Overwrite Text or add it to existing;
		* @param DefaultExtension			Extension of file;
		* @return							Result status;
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Save Text to Disk"), Category = "WidgetDocumentationPlugin|Text")
		static FString SaveTextToDisk(FString TextToSave, FString Filename, bool bOverwriteText = true, FString DefaultExtension = ".txt");

	/**
		* Saves text to Disk;
		* If OverwriteText is false, it will add text to existing one;
		*
		* @param TextToSave					Text that needs to be saved;
		* @param Filepath					Directory for saving file;
		* @param Filename					Name of file;
		* @param bOverwriteText				Overwrite Text or add it to existing;
		* @param DefaultExtension			Extension of file;
		* @return							Result status;
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Advanced Save Text to Disk"), Category = "WidgetDocumentationPlugin|Text")
		static FString AdvancedSaveTextToDisk(FString TextToSave, FString Filepath, FString Filename, bool bOverwriteText = true, FString DefaultExtension = ".txt");

	/**
		* Load Text From File;
		*
		* @param Filepath					Directory of file;
		* @param Filename					Name of file;
		* @param DefaultExtension			Extension of file;
		* @return							Loaded Text or Error description;
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Load Text From File"), Category = "WidgetDocumentationPlugin|Text")
		static FString LoadTextFromFile(FString Filepath, FString Filename, FString DefaultExtension = ".txt");


	/**
		* Delete File;
		*
		* @param FilePathAndName			Directory of file with name;
		* @return							Success or Error description;
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Delete File"), Category = "WidgetDocumentationPlugin|Other")
		static FString DeleteFile(FString FilePathAndName);

	/**
		* Clear Directory;
		* Delete all files inside folder;
		*
		* @param DirectoryPath				Directory of folder;
		* @return							Success or Error description;
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Clear Directory"), Category = "WidgetDocumentationPlugin|Other")
		static FString ClearDirectory(FString DirectoryPath);

};
