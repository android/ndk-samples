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
 */

#ifndef CAMERA_IMAGE_READER_H
#define CAMERA_IMAGE_READER_H
#include <media/NdkImageReader.h>

/*
 * ImageResolution:
 *     A Data Structure to communicate resolution between camera and ImageReader
 */
struct ImageResolution {
    int32_t  width;
    int32_t  height;

    int32_t  format;  // Through out this demo, the format is fixed to
                      // YUV_420 format
};

class ImageReader {
  public:
    /*
     * Ctor and Dtor()
     */
    explicit ImageReader(ImageResolution* res);

    ~ImageReader();

    /*
     * Retrieve ANativeWindow from ImageReader. Used to create camera capture
     * session output. ANativeWindow is bufferQueue, automatically handled
     * by camera ( producer ) and Reader; display engine pull image out to display
     * via function DisplayImage()
     */
    ANativeWindow * GetNativeWindow(void);

    /*
     * Retrieve Image on the top of Reader's queue
     */
    AImage* GetNextImage(void);

    /*
     * ImageReader callback handler: get called each frame is captured into Reader
     * This app does not do anything ( just place holder ). This function is for
     * ImageReader() internal usage
     */
    void ImageCallback(AImageReader* reader);

    /*
     * DisplayImage()
     *   Present camera image to display.
     */
    bool DisplayImage(ANativeWindow_Buffer* buf);
    /*
     * Configure the rotation angle necessary to apply to
     * Camera image when presenting: all rotations should be accumulated:
     *    CameraSensorOrientation + Android Device Native Orientation +
     *    Human Rotation (rotated degree related to Phone native orientation
     */
    void SetPresentRotation(int32_t angle);

  private:
    int32_t presentRotation_;
    AImageReader* reader_;
    volatile int32_t imageWatermark_;

    void PresentImage(ANativeWindow_Buffer* buf, AImage* img);
    void PresentImage90(ANativeWindow_Buffer* buf, AImage* img);
    void PresentImage180(ANativeWindow_Buffer* buf, AImage* img);
    void PresentImage270(ANativeWindow_Buffer* buf, AImage* img);
    AImage* GetLastImage(void);

};

#endif //CAMERA_IMAGE_READER_H
