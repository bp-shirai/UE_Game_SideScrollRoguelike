// Fill out your copyright notice in the Description page of Project Settings.


#include "Settings/RogueDeveloperSettings.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RogueDeveloperSettings)

const URogueDeveloperSettings* URogueDeveloperSettings::Get()
{
	return GetDefault<URogueDeveloperSettings>();
}

FName URogueDeveloperSettings::GetContainerName() const
{
	static const FName ContainerName("Project");
	return ContainerName;
}

FName URogueDeveloperSettings::GetCategoryName() const
{
	static const FName EditorCategoryName("Project");
	return EditorCategoryName;
}

FName URogueDeveloperSettings::GetSectionName() const
{
	static const FName TargetSectionName("Rogue Developer Settings");
	return TargetSectionName;
}

#if WITH_EDITOR
FText URogueDeveloperSettings::GetSectionText() const
{
	static const FText TargetSectionText = FText::FromString("Rogue Settings"); 
	return TargetSectionText;
}

FText URogueDeveloperSettings::GetSectionDescription() const
{
	static const FText TargetSectionDescription = FText::FromString("Project settings specific to Rogue");
	return TargetSectionDescription;
}

#endif

bool URogueDeveloperSettings::ShouldSkipLogoTrain()
{
	bool result = false; 

#if WITH_EDITOR
	result = bSkipLogoTrain;
#endif

	return result;
}