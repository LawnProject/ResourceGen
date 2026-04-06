#pragma once

#include <string>
#include <map>
#include <memory>

class BaseResource
{
	public:
		std::string mID;
		std::string mPath;
};

class ResourceSound : public BaseResource
{
	public:
		double mVolume = 0.0;
		int mPanning = 0;
};
class ResourceFont : public BaseResource
{
	public:
		bool mIsSystemFont = false;
		int mSize = 0;
		bool mBold = false;
		bool mItalic = false;
		bool mShadow = false;
		bool mUnderline = false;
};
class ResourceImage : public BaseResource
{
	public:
		bool mHasAlphaMask = false;
		std::string mAlphaGrid = "";
		int mRows = 1;
		int mCols = 1;
};

struct ResourceGroup
{
	std::map<std::string, std::shared_ptr<ResourceImage>> mImageMap;
	std::map<std::string, std::shared_ptr<ResourceSound>> mSoundMap;
	std::map<std::string, std::shared_ptr<ResourceFont>> mFontMap;
};

class ResourceManifest
{
public:
	ResourceManifest();
	~ResourceManifest();

	void AddGroup(std::string theName);
	std::shared_ptr<ResourceImage> AddImage(std::string theGroup, std::string theName);
	std::shared_ptr<ResourceSound> AddSound(std::string theGroup, std::string theName);
	std::shared_ptr<ResourceFont> AddFont(std::string theGroup, std::string theName);
	void DeleteItem(std::string theName);
	void Export(std::string theXMLPath);
	void Import(std::string theXMLPath);

	std::map<std::string, ResourceGroup> mGroupMap;

private:

};