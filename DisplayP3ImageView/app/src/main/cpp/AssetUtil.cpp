/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#include <algorithm>
#include "AssetUtil.h"
#include "android_debug.h"


#define IS_LOW_CHAR(c) ((c) >= 'a' && (c) <= 'z')
#define TO_UPPER_CHAR(c) (c + 'A' - 'a')
void StringToUpper(std::string& str) {
  for(auto& ch: str) {
    if (IS_LOW_CHAR(ch))
      ch = TO_UPPER_CHAR(ch);
  }
}
bool AssetEnumerateFileType(AAssetManager * assetManager,
                        const char* type, std::vector<std::string> & files) {

  if (!assetManager || !type || !*type)
    return false;

  std::string fileType(type);
  if (fileType[0] != '.') {
    fileType.insert(0, 1, '.');
  }
  StringToUpper(fileType);

  AAssetDir * dir = AAssetManager_openDir(assetManager, "");

  const char* name;
  while ((name = AAssetDir_getNextFileName(dir))) {
    std::string assetName(name);
    if (assetName.length() <= fileType.length())
      continue;
    std::string assetType =
        assetName.substr(assetName.length() - fileType.length(),
                         fileType.length());
    StringToUpper(assetType);

    if (assetType == fileType) {
      files.push_back(assetName);
    }
  }
  LOGI("Found %d PNG Files", static_cast<uint32_t>(files.size()));

  AAssetDir_close(dir);
  return true;
}

bool AssetReadFile(AAssetManager* assetManager,
              std::string& assetName, std::vector<uint8_t>& buf) {
  if (!assetName.length())
    return false;
  AAsset* assetDescriptor = AAssetManager_open(assetManager,
                                    assetName.c_str(),
                                    AASSET_MODE_BUFFER);
  ASSERT(assetDescriptor, "%s does not exist in %s",
         assetName.c_str(), __FUNCTION__);
  size_t fileLength = AAsset_getLength(assetDescriptor);

  buf.resize(fileLength);
  int64_t readSize = AAsset_read(assetDescriptor, buf.data(), buf.size());

  AAsset_close(assetDescriptor);
  return (readSize == buf.size());
}
